// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"
#include "UObject/Class.h"

#if WITH_DEV_AUTOMATION_TESTS

// L4 (LegionsInProgress.md, plan step 2): the Damned Warrior (profile A, Q-legions-l4 Q1 = A) on the REAL unit type table
// (/Game/Units/UnitTypes, imported from Tools/Data/UnitTypes.csv). Relations only, as in Tools/Tests/legions_warrior_test.py: the
// Legions' emblematic Barracks unit is a fragile berserker, so it is Profane and heavy, has less health than the Man-at-arms and hits
// harder per second. It stays RED until the DamnedWarrior row exists (plan step 3). The necrotic rage (plan steps 4 and 5) is tested
// on the real chain once its order and its state exist: Combat.DamnedWarrior.Rage is written with that code (its API is not designed yet).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDamnedWarriorTableTest, "HeroesOfLiberty.Combat.DamnedWarrior.Table", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLDamnedWarriorTableTest::RunTest(const FString& Parameters)
{
	const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
	if (!TestNotNull(TEXT("unit type table"), UnitTypes))
	{
		return false;
	}
	const FHOLUnitTypeRow* Warrior = UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("DamnedWarrior"), TEXT("Damned Warrior test"), false);
	const FHOLUnitTypeRow* ManAtArms = UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("ManAtArms"), TEXT("Damned Warrior test"), false);
	if (!TestNotNull(TEXT("DamnedWarrior in the unit type table"), Warrior) || !TestNotNull(TEXT("ManAtArms in the unit type table"), ManAtArms))
	{
		return false;
	}
	TestEqual(TEXT("the Damned Warrior is the Legions' (D395)"), Warrior->Faction, FName(TEXT("Legions")));
	TestEqual(TEXT("tier 1 (D395)"), Warrior->MinTier, 1);
	TestTrue(TEXT("Heavy infantry"), Warrior->ArmorCategories.Contains(EHOLArmorCategory::HeavyInfantry));
	TestTrue(TEXT("Heavy (D428, unit by unit)"), Warrior->ArmorCategories.Contains(EHOLArmorCategory::Heavy));
	const UEnum* Enum = StaticEnum<EHOLArmorCategory>();
	const int64 Profane = Enum ? Enum->GetValueByNameString(TEXT("Profane")) : INDEX_NONE;
	bool bProfane = false;
	for (const EHOLArmorCategory Category : Warrior->ArmorCategories)
	{
		bProfane |= Profane != INDEX_NONE && static_cast<int64>(Category) == Profane;
	}
	TestTrue(TEXT("Profane (D393)"), bProfane);
	TestFalse(TEXT("not a Peasant (it does not build or gather)"), Warrior->ArmorCategories.Contains(EHOLArmorCategory::Peasant));
	TestTrue(TEXT("profile A: fragile, less health than the Man-at-arms"), Warrior->MaxHealth < ManAtArms->MaxHealth);
	TestTrue(TEXT("... and more damage per second than the Man-at-arms"), Warrior->Attack / Warrior->AttackInterval > ManAtArms->Attack / ManAtArms->AttackInterval);
	TestTrue(TEXT("it has a cost"), Warrior->Cost.Food + Warrior->Cost.Wood + Warrior->Cost.Stone + Warrior->Cost.Gold > 0);
	return true;
}

// L4 step 5 (a): the necrotic rage on the REAL chain (Q-legions-l4 Q2 = A): a Damned Warrior of slot 0 hits a Peasant of slot 1 (it
// does not fight back, 100000 health) through the combat processor, as in a match; the rage is ordered by UHOLUnitSubsystem::CastRage,
// the call the player's button makes. Measured EFFECT, no value written by hand: over 9 s the rage gives more blows (cadence +25 %),
// each stronger (+40 %), and every blow costs the warrior 3 health; refused during the cool-down; once the 10 s are over, the blows are
// those of before and cost nothing.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDamnedWarriorRageTest, "HeroesOfLiberty.Combat.DamnedWarrior.Rage", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLDamnedWarriorRageTest::RunTest(const FString& Parameters)
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
		// As HeroesOfLiberty.HeroAbility.InMatch: born again until its identifier is a multiple of 4 (one search step of 4).
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

		const uint32 Warrior = Place(FName(TEXT("DamnedWarrior")), 0, FVector(0.0, 0.0, 0.0));
		const uint32 Dummy = Place(FName(TEXT("Peasant")), 1, FVector(150.0, 0.0, 0.0));
		FHOLUnitStateFragment* DummyState = StateOf(Dummy);
		if (TestTrue(TEXT("a Damned Warrior of slot 0 and a Peasant of slot 1"), StateOf(Warrior) != nullptr && DummyState != nullptr))
		{
			DummyState->CurrentHealth = 100000.f;
			RunStep();
			struct FWindow
			{
				int32 Hits = 0;
				float Dealt = 0.f;
				float WarriorLost = 0.f;
				float PerHit() const { return Hits > 0 ? Dealt / Hits : 0.f; }
			};
			// One blow = a step where the Peasant's health went down (the blows are 1.2 s apart at least: never two in a step).
			const auto Measure = [&](const float Seconds)
			{
				FWindow Window;
				const float WarriorBefore = StateOf(Warrior)->CurrentHealth;
				for (int32 Steps = 0; Steps < FMath::CeilToInt(Seconds / StepSeconds); ++Steps)
				{
					const float Before = StateOf(Dummy)->CurrentHealth;
					RunStep();
					const float After = StateOf(Dummy)->CurrentHealth;
					if (After < Before)
					{
						++Window.Hits;
						Window.Dealt += Before - After;
					}
				}
				Window.WarriorLost = WarriorBefore - StateOf(Warrior)->CurrentHealth;
				return Window;
			};

			const FWindow Calm = Measure(9.f);
			TestTrue(TEXT("without the rage the warrior strikes the Peasant (1.5 s apart)"), Calm.Hits >= 4);
			TestEqual(TEXT("... and it costs him nothing"), Calm.WarriorLost, 0.f, 0.01f);

			TestTrue(TEXT("the rage is cast"), Units->CastRage(Warrior));
			TestFalse(TEXT("... not again during its 40 s cool-down"), Units->CastRage(Warrior));
			const FWindow Rage = Measure(9.f);
			TestTrue(TEXT("under the rage: more blows in the same time (cadence +25 %)"), Rage.Hits > Calm.Hits);
			TestEqual(TEXT("under the rage: each blow 40 % stronger"), Rage.PerHit(), Calm.PerHit() * 1.4f, 0.05f);
			TestEqual(TEXT("under the rage: 3 health lost per blow"), Rage.WarriorLost, 3.f * Rage.Hits, 0.01f);

			// The 10 s are over (9 s measured + 1.5 s): the blows are those of before and cost nothing.
			Measure(1.5f);
			const FWindow After = Measure(6.f);
			TestTrue(TEXT("after the rage the warrior still strikes"), After.Hits >= 2);
			TestEqual(TEXT("after the rage: the blow of before"), After.PerHit(), Calm.PerHit(), 0.05f);
			TestEqual(TEXT("after the rage: no health lost"), After.WarriorLost, 0.f, 0.01f);
		}
	}

	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
