// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Match/HOLGameState.h"
#include "Simulation/HOLHealProcessor.h"
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

// D689, Sacred Light, the EFFECT (S1-a): the slot puts a Sacred Light at a point (UHOLHeroSubsystem::StartSacredLight); through the real heal
// processor (no healer on the field: it must wake for the zone) a WOUNDED ALLY in the zone gains health over the steps at the zone's rate, a
// wounded ally out of the zone and a wounded ENEMY in it gain nothing, and once the ~5 s are over the heal stops. The health read is the
// fragment's, after the steps the processor really ran (not a constant of the rules).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLZoneEffectSacredLightTest, "HeroesOfLiberty.Zone.SacredLight", UE::HeroesOfLiberty::Tests::ZoneEffect::Flags)

bool FHOLZoneEffectSacredLightTest::RunTest(const FString& Parameters)
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
		UHOLHealProcessor* Heal = NewObject<UHOLHealProcessor>(Simulation);
		Heal->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		int64 Step = 0;
		const auto RunStep = [Simulation, Units, Combat, Heal, &EntityManager, StepSeconds, &Step]
		{
			++Step;
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Combat, ProcessingContext);
			UE::Mass::Executor::Run(*Heal, ProcessingContext);
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
		const auto StateOf = [Units, &EntityManager](const uint32 Id)
		{
			const FMassEntityHandle Entity = Id != 0 ? Units->FindEntity(Id) : FMassEntityHandle();
			return Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLUnitStateFragment>(Entity) : nullptr;
		};

		// Sacred Light of slot 0 at the origin (6 m): its own unit 3 m from the centre, another 9 m from it, an enemy (slot 1) 3 m from it.
		const uint32 AllyIn = Place(Spearman, 0, FVector(300.0, 0.0, 0.0));
		const uint32 AllyOut = Place(Spearman, 0, FVector(900.0, 0.0, 0.0));
		const uint32 EnemyIn = Place(Spearman, 1, FVector(0.0, 300.0, 0.0));
		if (TestTrue(TEXT("the three units born"), AllyIn != 0 && AllyOut != 0 && EnemyIn != 0))
		{
			RunStep();
			FHOLUnitStateFragment* InState = StateOf(AllyIn);
			FHOLUnitStateFragment* OutState = StateOf(AllyOut);
			FHOLUnitStateFragment* EnemyState = StateOf(EnemyIn);
			if (TestTrue(TEXT("the three states"), InState && OutState && EnemyState))
			{
				// Wound the three units to 20 (their full health is much higher): the heal has room to show.
				InState->CurrentHealth = 20.f;
				OutState->CurrentHealth = 20.f;
				EnemyState->CurrentHealth = 20.f;
				TestTrue(TEXT("slot 0 puts the Sacred Light at the origin"), Heroes->StartSacredLight(0, FVector(0.0, 0.0, 0.0)));
				const int32 HealSteps = FMath::CeilToInt(1.f / StepSeconds);
				for (int32 Steps = 0; Steps < HealSteps; ++Steps)
				{
					RunStep();
				}
				const float SecondGain = InState->CurrentHealth - 20.f;
				const float Expected = static_cast<float>(HOLZoneEffect::SacredLightHealPerSecond);
				TestTrue(*FString::Printf(TEXT("the ally in the zone gained about one second of heal (%.2f, expected %.2f)"), SecondGain, Expected),
					SecondGain > Expected * 0.8f && SecondGain < Expected * 1.3f);
				TestEqual(TEXT("the ally 9 m from the centre, out of the 6 m zone: not healed"), OutState->CurrentHealth, 20.f);
				TestEqual(TEXT("the enemy in the zone: not healed"), EnemyState->CurrentHealth, 20.f);
				// Its ~5 s over: the heal stops.
				for (int32 Steps = 0; Steps < FMath::CeilToInt(HOLZoneEffect::SacredLightDurationMs / 1000.f / StepSeconds) + 2; ++Steps)
				{
					RunStep();
				}
				const float AfterZone = InState->CurrentHealth;
				TestTrue(TEXT("the ally in the zone gained more over the whole zone"), AfterZone > 20.f + SecondGain);
				for (int32 Steps = 0; Steps < HealSteps; ++Steps)
				{
					RunStep();
				}
				TestEqual(TEXT("the Sacred Light over: the heal has stopped"), InState->CurrentHealth, AfterZone);
			}
		}
	}
	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

// D689, Honor of the Dawn from the heal (S1-e): the Sacred Light of slot 0 heals a wounded ally; the processor credits slot 0's faction gauge
// (AHOLGameState::FactionResource) one Honor per HpPerPoint hit points healed. The expected Honor is CALCULATED from the health the fragment
// really gained (floor(gain / HpPerPoint)), never a constant; a slot that healed nothing (slot 1) gets nothing; the gauge stops at the cap.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLZoneEffectHealHonorTest, "HeroesOfLiberty.Zone.HealHonor", UE::HeroesOfLiberty::Tests::ZoneEffect::Flags)

bool FHOLZoneEffectHealHonorTest::RunTest(const FString& Parameters)
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
	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	World->SetGameState(State);

	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UHOLHeroSubsystem* Heroes = World->GetSubsystem<UHOLHeroSubsystem>();
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation)
		&& TestNotNull(TEXT("heroes"), Heroes) && TestNotNull(TEXT("entities"), EntitySubsystem))
	{
		Heroes->BindSteps();
		constexpr int32 HpPerPoint = 20;
		constexpr int32 Cap = 100;
		UHOLHealProcessor::ConfigureHonor(HpPerPoint, Cap);
		TSharedPtr<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
		UHOLCombatProcessor* Combat = NewObject<UHOLCombatProcessor>(Simulation);
		Combat->CallInitialize(Simulation, EntityManager.ToSharedRef());
		UHOLHealProcessor* Heal = NewObject<UHOLHealProcessor>(Simulation);
		Heal->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		int64 Step = 0;
		const auto RunStep = [Simulation, Units, Combat, Heal, &EntityManager, StepSeconds, &Step]
		{
			++Step;
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Combat, ProcessingContext);
			UE::Mass::Executor::Run(*Heal, ProcessingContext);
			Units->FlushDestructions();
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		FHOLUnitSpawnRequest Request;
		Request.TypeName = Spearman;
		Request.Count = 1;
		Request.OwnerPlayer = 0;
		Request.Center = FVector(300.0, 0.0, 0.0);
		Units->RequestUnits(Request);
		Units->ServeBirthRequests();
		TArray<uint32> Ids;
		Units->GetAllUnitIds(Ids);
		const FMassEntityHandle Entity = Ids.Num() == 1 ? Units->FindEntity(Ids[0]) : FMassEntityHandle();
		FHOLUnitStateFragment* Ally = Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLUnitStateFragment>(Entity) : nullptr;
		if (TestNotNull(TEXT("the ally born"), Ally))
		{
			RunStep();
			Ally->CurrentHealth = 20.f;
			TestEqual(TEXT("the gauge of slot 0 starts empty"), State->GetFactionResource(0), 0);
			TestTrue(TEXT("slot 0 puts the Sacred Light at the origin"), Heroes->StartSacredLight(0, FVector(0.0, 0.0, 0.0)));
			const int32 ZoneSteps = FMath::CeilToInt(HOLZoneEffect::SacredLightDurationMs / 1000.f / StepSeconds) + 2;
			for (int32 Steps = 0; Steps < ZoneSteps; ++Steps)
			{
				RunStep();
			}
			const int32 Gained = FMath::FloorToInt(Ally->CurrentHealth - 20.f);
			const int32 Expected = FMath::Min(Gained / HpPerPoint, Cap);
			TestTrue(*FString::Printf(TEXT("the zone healed something (%d hit points)"), Gained), Gained >= HpPerPoint);
			// The remainder under HpPerPoint waits for the next heal, so the gauge is within one point of the whole division.
			const int32 Honor = State->GetFactionResource(0);
			TestTrue(*FString::Printf(TEXT("slot 0 gauge %d = about %d (%d hit points healed, one Honor per %d)"), Honor, Expected, Gained, HpPerPoint),
				Honor >= Expected - 1 && Honor <= Expected && Honor > 0);
			TestEqual(TEXT("slot 1 healed nothing: no Honor"), State->GetFactionResource(1), 0);
		}
		UHOLHealProcessor::ConfigureHonor(0, 0);
	}
	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}