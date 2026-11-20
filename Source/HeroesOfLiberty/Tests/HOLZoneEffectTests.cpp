// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLZoneEffectRules.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace UE::HeroesOfLiberty::Tests::ZoneEffect
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
	const FName Spearman(TEXT("Spearman"));
}

// D859, Seal of Light: the pure rules of a zone on the ground. Geometry chosen by hand (a 3-4-5 triangle: the point (300, 400) is exactly
// 500 cm from the centre), so the expected answers do not come from the code under test.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLZoneEffectRulesTest, "HeroesOfLiberty.Zone.Rules", UE::HeroesOfLiberty::Tests::ZoneEffect::Flags)

bool FHOLZoneEffectRulesTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("the centre is in the zone"), HOLZoneEffect::IsInZone(1000, 2000, 600, 1000, 2000));
	TestTrue(TEXT("exactly on the edge (a 3-4-5 triangle, 500 cm): in"), HOLZoneEffect::IsInZone(0, 0, 500, 300, 400));
	TestFalse(TEXT("one centimetre beyond the edge: out"), HOLZoneEffect::IsInZone(0, 0, 499, 300, 400));
	TestFalse(TEXT("far away: out"), HOLZoneEffect::IsInZone(0, 0, 600, 5000, 0));

	TestFalse(TEXT("before its start: not active"), HOLZoneEffect::ZoneActive(5000, 10000, 4999));
	TestTrue(TEXT("at its start: active"), HOLZoneEffect::ZoneActive(5000, 10000, 5000));
	TestTrue(TEXT("one millisecond before its end: active"), HOLZoneEffect::ZoneActive(5000, 10000, 14999));
	TestFalse(TEXT("at its end (start + duration): over"), HOLZoneEffect::ZoneActive(5000, 10000, 15000));

	TestEqual(TEXT("an enemy in an active zone, -40 %: 60"), HOLZoneEffect::GetSpeedPercent(true, true, true, 40), 60);
	TestEqual(TEXT("not an enemy: 100"), HOLZoneEffect::GetSpeedPercent(false, true, true, 40), 100);
	TestEqual(TEXT("out of the zone: 100"), HOLZoneEffect::GetSpeedPercent(true, false, true, 40), 100);
	TestEqual(TEXT("the zone over: 100"), HOLZoneEffect::GetSpeedPercent(true, true, false, 40), 100);
	TestEqual(TEXT("a slow of more than 100 %: held in place (0), never negative"), HOLZoneEffect::GetSpeedPercent(true, true, true, 250), 0);
	return true;
}

// D859, Seal of Light, the EFFECT: the slot puts a Seal at a point (UHOLHeroSubsystem::StartLightSeal); through the real combat processor an
// ENEMY unit in the zone takes 60 % of its walk (FHOLCombatFragment::ZoneSpeedPercent, applied by the movement processor), the slot's own unit
// in the zone and an enemy out of it keep 100 %, and once the 10 s are over the enemy walks at 100 % again. Nobody fights (hol.Combat.Enabled 0).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLZoneEffectSlowTest, "HeroesOfLiberty.Zone.Slow", UE::HeroesOfLiberty::Tests::ZoneEffect::Flags)

bool FHOLZoneEffectSlowTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::ZoneEffect;
	IConsoleVariable* CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
	if (!TestNotNull(TEXT("hol.Combat.Enabled"), CombatEnabled))
	{
		return false;
	}
	const int32 FormerEnabled = CombatEnabled->GetInt();
	CombatEnabled->Set(0, ECVF_SetByCode);

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UHOLHeroSubsystem* Heroes = World->GetSubsystem<UHOLHeroSubsystem>();
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("heroes"), Heroes)
		&& TestNotNull(TEXT("entities"), EntitySubsystem))
	{
		Heroes->BindSteps();
		TSharedPtr<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
		UHOLCombatProcessor* Combat = NewObject<UHOLCombatProcessor>(Simulation);
		Combat->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		int64 Step = 0;
		const auto RunStep = [Simulation, Units, Combat, &EntityManager, StepSeconds, &Step]
		{
			++Step;
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Combat, ProcessingContext);
			Units->FlushDestructions();
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		const auto Place = [Units](const FName Type, const int32 Owner, const FVector& Location)
		{
			for (int32 Attempt = 0; Attempt < 8; ++Attempt)
			{
				TArray<uint32> Before;
				Units->GetAllUnitIds(Before);
				FHOLUnitSpawnRequest Request;
				Request.TypeName = Type;
				Request.Count = 1;
				Request.OwnerPlayer = static_cast<uint8>(Owner);
				Request.Center = Location;
				Units->RequestUnits(Request);
				Units->ServeBirthRequests();
				TArray<uint32> After;
				Units->GetAllUnitIds(After);
				uint32 Born = 0;
				for (const uint32 Id : After)
				{
					Born = Before.Contains(Id) ? Born : Id;
				}
				if (Born == 0 || Born % 4 == 0)
				{
					return Born;
				}
				const uint32 Again[] = { Born };
				Units->RequestDestroyUnits(Again);
				Units->FlushDestructions();
			}
			return 0u;
		};
		const auto SpeedOf = [Units, &EntityManager](const uint32 Id)
		{
			const FMassEntityHandle Entity = Id != 0 ? Units->FindEntity(Id) : FMassEntityHandle();
			const FHOLCombatFragment* Fragment = Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLCombatFragment>(Entity) : nullptr;
			return Fragment ? static_cast<int32>(Fragment->ZoneSpeedPercent) : -1;
		};

		// The zone of slot 0 at the origin (6 m): an enemy (slot 1) 3 m from the centre, another 9 m from it, a unit of slot 0 3 m from it.
		const uint32 EnemyIn = Place(Spearman, 1, FVector(300.0, 0.0, 0.0));
		const uint32 EnemyOut = Place(Spearman, 1, FVector(900.0, 0.0, 0.0));
		const uint32 Own = Place(Spearman, 0, FVector(0.0, 300.0, 0.0));
		if (TestTrue(TEXT("the three units born"), EnemyIn != 0 && EnemyOut != 0 && Own != 0))
		{
			RunStep();
			TestEqual(TEXT("before the Seal: the enemy in the future zone walks at 100"), SpeedOf(EnemyIn), 100);
			TestTrue(TEXT("slot 0 puts the Seal of Light at the origin"), Heroes->StartLightSeal(0, FVector(0.0, 0.0, 0.0)));
			RunStep();
			TestEqual(TEXT("the enemy 3 m from the centre: -40 %, 60"), SpeedOf(EnemyIn), 60);
			TestEqual(TEXT("the enemy 9 m from the centre, out of the 6 m zone: 100"), SpeedOf(EnemyOut), 100);
			TestEqual(TEXT("the slot's own unit in the zone: 100"), SpeedOf(Own), 100);
			// Its 10 s over: the whole speed again.
			for (int32 Steps = 0; Steps < FMath::CeilToInt(HOLZoneEffect::LightSealDurationMs / 1000.f / StepSeconds) + 2; ++Steps)
			{
				RunStep();
			}
			TestEqual(TEXT("the Seal over: the enemy walks at 100 again"), SpeedOf(EnemyIn), 100);
		}
	}
	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
