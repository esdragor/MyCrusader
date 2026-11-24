// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Simulation/HOLMoraleRules.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitAbilityRules.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// PLAN-MORAL-1120 (Q4 = A, GALADRIEL-1124b step 1): the necrotic rage's damage bonus lives in the morale engine. The REAL chain: a Damned Warrior
// of slot 0 and a Peasant of slot 1 (100000 health, no answer) in the combat processor; UHOLUnitSubsystem::CastRage is the call the button makes.
// (a) the cast lays the rage as an effect of the warrior's morale fragment (the Dealt lever = the rage's damage percent, read by the rule
// HOLMorale::Total / Multiplier, never typed); (b) the rage ends at StopHp (the blow that takes the warrior to StopHp) AND takes its effect off.
// The measured 40 % per blow stays in HeroesOfLiberty.Combat.DamnedWarrior.Rage (the combat reads the lever alone: no pose, no bonus).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLRageMoraleDealtTest, "HeroesOfLiberty.Morale.NecroticRageIsAMoraleEffect", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLRageMoraleDealtTest::RunTest(const FString& Parameters)
{
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
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("entities"), EntitySubsystem))
	{
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
		// As HOLDamnedWarriorTests: born again until its identifier is a multiple of 4 (one search step of 4).
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

		const uint32 Warrior = Place(FName(TEXT("DamnedWarrior")), 0, FVector(0.0, 0.0, 0.0));
		const uint32 Dummy = Place(FName(TEXT("Peasant")), 1, FVector(150.0, 0.0, 0.0));
		const FMassEntityHandle WarriorEntity = Warrior != 0 ? Units->FindEntity(Warrior) : FMassEntityHandle();
		const FMassEntityHandle DummyEntity = Dummy != 0 ? Units->FindEntity(Dummy) : FMassEntityHandle();
		FHOLUnitStateFragment* WarriorState = WarriorEntity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLUnitStateFragment>(WarriorEntity) : nullptr;
		FHOLUnitStateFragment* DummyState = DummyEntity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLUnitStateFragment>(DummyEntity) : nullptr;
		FHOLCombatFragment* WarriorCombat = WarriorEntity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLCombatFragment>(WarriorEntity) : nullptr;
		FHOLMoraleFragment* WarriorMorale = WarriorEntity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLMoraleFragment>(WarriorEntity) : nullptr;
		if (TestTrue(TEXT("a Damned Warrior of slot 0 and a Peasant of slot 1, the warrior with a morale fragment"), WarriorState && DummyState && WarriorCombat && WarriorMorale))
		{
			DummyState->CurrentHealth = 100000.f;
			RunStep();
			TestEqual(TEXT("before the cast the warrior's damage lever is neutral"), HOLMorale::Total(WarriorMorale->State, HOLMorale::LeverDealt), 0);

			const int32 EffectsBefore = WarriorMorale->State.Num;
			TestTrue(TEXT("the cast is accepted"), Units->CastRage(Warrior));
			TestEqual(TEXT("the cast lays the rage as a morale effect on the Dealt lever (the rule reads the rage's damage percent)"),
				HOLMorale::Total(WarriorMorale->State, HOLMorale::LeverDealt), HOLUnitAbility::RageDamagePercent);
			TestTrue(TEXT("... and the combat's multiplier of that lever is above 100"), HOLMorale::Multiplier(WarriorMorale->State, HOLMorale::LeverDealt) > 100);
			// Step 2 (GALADRIEL-1124c): the +25 % cadence is the same effect's Rate lever (row NecroticRage), read by the combat through
			// HOLMorale::Multiplier(LeverRate); the combat no longer reads RageMs for the interval, so no Rate lever = no faster blows.
			TestEqual(TEXT("the cast lays the rage's cadence on the Rate lever (the rule reads the rage's rate percent)"),
				HOLMorale::Total(WarriorMorale->State, HOLMorale::LeverRate), HOLUnitAbility::RageRatePercent);
			TestEqual(TEXT("... and the combat's multiplier of the Rate lever is 100 + the rate percent"),
				HOLMorale::Multiplier(WarriorMorale->State, HOLMorale::LeverRate), 100 + HOLUnitAbility::RageRatePercent);
			TestEqual(TEXT("the rage is ONE effect (damage and rate together), not two"), WarriorMorale->State.Num, EffectsBefore + 1);

			// The rage ends at StopHp: one health above it, the next blow costs 1 and ends it. The effect leaves with it.
			WarriorState->CurrentHealth = static_cast<float>(HOLUnitAbility::RageStopHp) + 1.f;
			for (int32 Steps = 0; Steps < FMath::CeilToInt(6.f / StepSeconds) && WarriorCombat->RageMs > 0; ++Steps)
			{
				RunStep();
			}
			TestEqual(TEXT("the rage ends at the stop floor"), WarriorCombat->RageMs, 0);
			TestEqual(TEXT("... and the rage's effect is off the morale fragment (nothing left on the Dealt lever)"), HOLMorale::Total(WarriorMorale->State, HOLMorale::LeverDealt), 0);
			TestEqual(TEXT("... nor on the Rate lever"), HOLMorale::Total(WarriorMorale->State, HOLMorale::LeverRate), 0);
		}
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
