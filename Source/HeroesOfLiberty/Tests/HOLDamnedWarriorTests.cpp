// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Simulation/HOLUnitAbilityRules.h"
#include "Simulation/HOLRageVisual.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Player/HOLCommandGrid.h"
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

// L4: the C++ copy of the rage's constants (Simulation/HOLUnitAbilityRules.h, which the combat processor and the grid's tooltip read)
// equals the tables the design is written in: the NecroticRage rows of Tools/Data/UnitAbilities.csv (Name, DisplayName, UnitType, Faction,
// Effect, CooldownMs, DurationMs, HpCostPerHit, MinHp, StopHp, Description) and Tools/Data/Effects.csv (Name, DisplayName, Sign, Dealt,
// Taken, Rate, Form, DurationMs, ...). Read from the files, as the Angelscript table is generated from them: a number changed in one
// place only fails here. Description (quoted, with commas) is the last field of each row and is not read.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDamnedWarriorRageConstantsTest, "HeroesOfLiberty.Combat.DamnedWarrior.RageConstants", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLDamnedWarriorRageConstantsTest::RunTest(const FString& Parameters)
{
	const auto FieldsOf = [](const FString& File, TArray<FString>& OutFields) -> bool
	{
		TArray<FString> Lines;
		if (!FFileHelper::LoadFileToStringArray(Lines, *FPaths::Combine(FPaths::ProjectDir(), TEXT("Tools/Data"), File)))
		{
			return false;
		}
		for (const FString& Line : Lines)
		{
			if (Line.StartsWith(TEXT("NecroticRage,")))
			{
				Line.ParseIntoArray(OutFields, TEXT(","), false);
				return true;
			}
		}
		return false;
	};
	TArray<FString> Ability;
	TArray<FString> Effect;
	if (!TestTrue(TEXT("UnitAbilities.csv has a NecroticRage row"), FieldsOf(TEXT("UnitAbilities.csv"), Ability)) || !TestTrue(TEXT("Effects.csv has a NecroticRage row"), FieldsOf(TEXT("Effects.csv"), Effect)))
	{
		return false;
	}
	if (!TestTrue(TEXT("enough fields"), Ability.Num() >= 10 && Effect.Num() >= 8))
	{
		return false;
	}
	TestEqual(TEXT("the unit type that casts it"), Ability[2], HOLUnitAbility::RageUnitType.ToString());
	TestEqual(TEXT("cool-down"), FCString::Atoi(*Ability[5]), HOLUnitAbility::RageCooldownMs);
	TestEqual(TEXT("duration"), FCString::Atoi(*Ability[6]), HOLUnitAbility::RageDurationMs);
	TestEqual(TEXT("hit points lost per blow"), FCString::Atoi(*Ability[7]), HOLUnitAbility::RageHpCostPerHit);
	TestEqual(TEXT("refused under (MinHp)"), FCString::Atoi(*Ability[8]), HOLUnitAbility::RageMinHp);
	TestEqual(TEXT("ends at (StopHp)"), FCString::Atoi(*Ability[9]), HOLUnitAbility::RageStopHp);
	TestEqual(TEXT("damage dealt, in percent (Effects.csv)"), FCString::Atoi(*Effect[3]), HOLUnitAbility::RageDamagePercent);
	TestEqual(TEXT("attack rate, in percent (Effects.csv)"), FCString::Atoi(*Effect[5]), HOLUnitAbility::RageRatePercent);
	TestEqual(TEXT("effect duration (Effects.csv) is the ability's"), FCString::Atoi(*Effect[7]), HOLUnitAbility::RageDurationMs);
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

			// L4 step 5 (f): the player's own route, as the button makes it: the grid offers the button to a selection with a Damned
			// Warrior (and not to another), the click queues the order (ServerCastRage -> IssueRageOrder), the simulation casts it.
			FHOLGridContext Selection;
			Selection.bOwnUnits = true;
			Selection.bRageUnit = true;
			TestTrue(TEXT("the grid shows the rage button for a selection with a Damned Warrior"), HOLCommandGrid::GetCommand(Selection, HOLCommandGrid::GetRageCell()).Command == EHOLGridCommand::NecroticRage);
			Selection.bRageUnit = false;
			TestTrue(TEXT("... and none for another selection"), HOLCommandGrid::GetCommand(Selection, HOLCommandGrid::GetRageCell()).Command != EHOLGridCommand::NecroticRage);
			UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>();
			if (TestNotNull(TEXT("commands"), Commands))
			{
				Commands->IssueRageOrder(1, { Warrior });
				RunStep();
				FHOLCombatFragment* WarriorCombat = EntityManager->GetFragmentDataPtr<FHOLCombatFragment>(Units->FindEntity(Warrior));
				TestTrue(TEXT("an order of another player does not cast the rage"), WarriorCombat && WarriorCombat->RageMs == 0);
				Commands->IssueRageOrder(0, { Warrior });
				RunStep();
				TestTrue(TEXT("the order of its owner casts the rage (the rage is running)"), WarriorCombat && WarriorCombat->RageMs > 0);
			}
			TestFalse(TEXT("... not again during its 40 s cool-down"), Units->CastRage(Warrior));

			// L4 visual (Q-legions-warrior-rage-visual): the rage the player's button just cast is SEEN, behind hol.Rage.Visual, from the
			// simulation's own state. Mode 0 (the default) draws nothing; 1 tints the body (colour measured against the owner's), 2 gives a ring,
			// 3 a state on the cell, 4 tint + cell state. A unit not in rage is never tinted.
			IConsoleVariable* RageVisual = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Rage.Visual"));
			if (TestNotNull(TEXT("hol.Rage.Visual"), RageVisual))
			{
				int32 RageMs = 0;
				int32 CooldownMs = 0;
				TestTrue(TEXT("the rage's state is read from the combat fragment, running and cooling down"), HOLRageVisual::GetRageState(World, Warrior, RageMs, CooldownMs) && RageMs > 0 && CooldownMs > RageMs);
				const FVector3f Base(0.2f, 0.4f, 0.9f);
				const auto DrawnAt = [&](const int32 Mode, const int32 Ms)
				{
					RageVisual->Set(Mode, ECVF_SetByCode);
					return HOLRageVisual::GetDrawColor(HOLRageVisual::GetMode(), Ms, Base);
				};
				TestTrue(TEXT("mode 0 (default): the warrior in rage keeps its owner's colour"), DrawnAt(0, RageMs).Equals(Base, 0.001f));
				const FVector3f Tinted = DrawnAt(HOLRageVisual::ModeTint, RageMs);
				TestTrue(TEXT("mode 1 (tint): the warrior in rage is darker and redder than its owner's colour"), !Tinted.Equals(Base, 0.05f) && Tinted.Y < Base.Y && Tinted.Z < Base.Z && Tinted.X >= Tinted.Y);
				TestTrue(TEXT("mode 4 (tint + state): the body is tinted too"), DrawnAt(HOLRageVisual::ModeTintIcon, RageMs).Equals(Tinted, 0.001f));
				TestTrue(TEXT("mode 1: a warrior NOT in rage is not tinted"), DrawnAt(HOLRageVisual::ModeTint, 0).Equals(Base, 0.001f));
				TestTrue(TEXT("mode 2 (ring) and 3 (cell state) leave the body's colour alone"), DrawnAt(HOLRageVisual::ModeRing, RageMs).Equals(Base, 0.001f) && DrawnAt(HOLRageVisual::ModeIcon, RageMs).Equals(Base, 0.001f));
				TestTrue(TEXT("the ring is drawn in mode 2 only"), HOLRageVisual::HasRing(HOLRageVisual::ModeRing) && !HOLRageVisual::HasRing(HOLRageVisual::ModeNone) && !HOLRageVisual::HasRing(HOLRageVisual::ModeTintIcon));
				TestTrue(TEXT("mode 2 (ring): a warrior in rage gets a ring on the ground"), HOLRageVisual::ShouldDrawRing(HOLRageVisual::ModeRing, RageMs));
				TestFalse(TEXT("mode 2: a warrior NOT in rage gets no ring"), HOLRageVisual::ShouldDrawRing(HOLRageVisual::ModeRing, 0));
				TestFalse(TEXT("modes 0, 1, 3, 4: no ring even in rage"), HOLRageVisual::ShouldDrawRing(HOLRageVisual::ModeNone, RageMs) || HOLRageVisual::ShouldDrawRing(HOLRageVisual::ModeTint, RageMs) || HOLRageVisual::ShouldDrawRing(HOLRageVisual::ModeIcon, RageMs) || HOLRageVisual::ShouldDrawRing(HOLRageVisual::ModeTintIcon, RageMs));
				TestTrue(TEXT("the cell state is shown in modes 3 and 4 only"), HOLRageVisual::HasIcon(HOLRageVisual::ModeIcon) && HOLRageVisual::HasIcon(HOLRageVisual::ModeTintIcon) && !HOLRageVisual::HasIcon(HOLRageVisual::ModeNone) && !HOLRageVisual::HasIcon(HOLRageVisual::ModeTint));
				RageVisual->Set(0, ECVF_SetByCode);
			}
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
