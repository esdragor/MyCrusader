// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Match/HOLForumMessages.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Misc/AutomationTest.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Presentation/HOLBannerSubsystem.h"
#include "Sound/SoundBase.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Simulation/HOLHeroAbilityRules.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLHeroXPRules.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// T4: the Captain's row-2 talent Boucliers levees (GDD 13.2, D111): +1 armour for the units in the Dawn Aura. A Spearman (armour 0) hit by a
// Man-at-arms (12): 10.2 in the aura (-15 %), and with the talent (12 - 1) x 0.85 = 9.35; out of the aura, no change.
// The talent is taken through the order queue (HeroesOfLiberty.HeroTalent.Orders), the hits come off the combat processor as in a match.
namespace UE::HeroesOfLiberty::Tests::HeroTalentEffect
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
	const FName Hero(TEXT("Hero"));
	const FName Spearman(TEXT("Spearman"));
	const FName ManAtArms(TEXT("ManAtArms"));
	constexpr float Tolerance = 1.e-3f;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroTalentShieldsUpTest, "HeroesOfLiberty.HeroTalent.ShieldsUp", UE::HeroesOfLiberty::Tests::HeroTalentEffect::Flags)

bool FHOLHeroTalentShieldsUpTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroTalentEffect;
	IConsoleVariable* CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
	if (!TestNotNull(TEXT("hol.Combat.Enabled"), CombatEnabled))
	{
		return false;
	}
	const int32 FormerEnabled = CombatEnabled->GetInt();
	CombatEnabled->Set(1, ECVF_SetByCode);

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
		// As HeroesOfLiberty.HeroXP.KillInMatch: born again until its identifier is a multiple of 4 (one search step of 4).
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
		// A Spearman of slot 0 at Location and a Man-at-arms of slot 1 1.5 m from it; the first hit it takes (5 s at most);
		// both go after it (-1: none).
		const auto FirstHit = [&](const FVector& Location)
		{
			const uint32 Victim = Place(Spearman, 0, Location);
			const uint32 Attacker = Place(ManAtArms, 1, Location + FVector(150.0, 0.0, 0.0));
			FHOLUnitStateFragment* State = StateOf(Victim);
			float Taken = -1.f;
			if (State && Attacker != 0)
			{
				const float Before = State->CurrentHealth;
				for (int32 Steps = 0; Steps < FMath::CeilToInt(5.f / StepSeconds); ++Steps)
				{
					RunStep();
					State = StateOf(Victim);
					if (State && State->CurrentHealth < Before)
					{
						Taken = Before - State->CurrentHealth;
						break;
					}
				}
			}
			const uint32 Gone[] = { Victim, Attacker };
			Units->RequestDestroyUnits(Gone);
			Units->FlushDestructions();
			return Taken;
		};

		const uint32 HeroId = Place(Hero, 0, FVector(0.0, 0.0, 0.0));
		if (TestTrue(TEXT("slot 0's hero born"), HeroId != 0))
		{
			RunStep();
			TestEqual(TEXT("no talent: 11 m from the hero, aura, 10.2"), FirstHit(FVector(0.0, 1100.0, 0.0)), 10.2f, Tolerance);
			Heroes->SetXP(0, HOLHeroXP::GetLevelXP(2));
			RunStep();
			// The Captain's row-2 talent (family 1) by the order queue's own path: in the aura (11 m) -1 armour, out of it (13 m) nothing.
			TestTrue(TEXT("the Captain's talent taken"), Heroes->PickTalent(0, 1));
			TestEqual(TEXT("Boucliers levés, 11 m from the hero: (12 - 1) x 0.85 = 9.35"), FirstHit(FVector(0.0, 1100.0, 0.0)), 9.35f, Tolerance);
			TestEqual(TEXT("... 13 m from the hero, out of the aura: the whole 12"), FirstHit(FVector(0.0, 1300.0, 0.0)), 12.f, Tolerance);
		}
	}
	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
