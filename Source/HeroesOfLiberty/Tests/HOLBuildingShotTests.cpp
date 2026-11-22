// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassNavigationFragments.h"
#include "MassProcessingContext.h"
#include "Match/HOLGameState.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLUnitTypes.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationProcessors.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS


// F8 T2 (FortificationsInProgress.md, D884): a FINISHED building whose row has BuildingAttack > 0 and BuildingFiresEmpty looses arrows at the
// nearest enemy within its reach (HOLCombat::IsInBuildingShotRange), never at an ally, never from a foundation. The REAL chain: the real combat
// processor stepped like the garrison tests, the arrow landing through the projectiles' own damage path, the effect read as the target's lost health
// and the arrows released. F8 T4: the cadence BuildingAttackInterval (simulation time, one countdown per building: first arrow at once, the interval
// kept exactly, no burst after a pause without target, the countdown dropped when the building dies). No fog (T5), no display (T7).
// The row is the House's with the attack columns of the TowerWood row (5 / 1.625 / 3000 / true), added to the loaded table for the run only.
namespace UE::HeroesOfLiberty::Tests::BuildingShot
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
	const FName TestShotTowerType(TEXT("BuildingShotTestTower"));

	struct FShotResult
	{
		bool bRan = false;
		float HealthLost = 0.f;
		int64 Released = 0;
		bool bVictimDown = false;
		bool bTowerGone = false;
		/** The step index (0 = the first step of the window) of each arrow released, in release order (one entry per arrow). */
		TArray<int32> ReleaseSteps;
		int32 StepMs = 0;
		int32 Steps = 0;
		int64 ReleasedDuringPause = 0;
		/** The countdown entries the processor holds at the end of the window, and two steps after the towers were destroyed (bDestroyAtEnd). */
		int32 EntriesAlive = -1;
		int32 EntriesAfterDeath = -1;
	};

	struct FShotOptions
	{
		/** A frail target (> 0), or a very tough one for the cadence runs. */
		float VictimHealth = 0.f;
		bool bDestroyAfterShot = false;
		/** The target stands out of every reach for this many steps first (no target: nothing released), then steps into the reach. */
		int32 PauseSteps = 0;
		/** Towers in all (the second stands 1500 cm along Y, both within reach of the target). */
		int32 Towers = 1;
		float Seconds = 10.f;
		bool bDestroyAtEnd = false;
	};

	/** The step at which the K-th arrow (K = 0 first) leaves a countdown that fires on the first step at which the elapsed time covers K intervals. */
	static int64 ExpectedStep(const int32 FirstStep, const int32 IntervalMs, const int32 StepMs, const int64 K)
	{
		return FirstStep + (K * IntervalMs + StepMs - 1) / StepMs;
	}

	static int32 ExpectedArrows(const int32 FirstStep, const int32 IntervalMs, const int32 StepMs, const int32 Steps)
	{
		int32 Count = 0;
		while (ExpectedStep(FirstStep, IntervalMs, StepMs, Count) <= Steps - 1)
		{
			++Count;
		}
		return Count;
	}

	/**
	 * One fresh world: Options.Towers buildings of TestShotTowerType of owner 0 in State, one Spearman of VictimOwner standing VictimGapCm beyond the first
	 * building's body edge (on the X axis), holding its ground; Options.Seconds of combat steps. The result: the Spearman's lost health, the arrows
	 * released and the step of each.
	 */
	static FShotResult RunScenario(FAutomationTestBase& Test, const EHOLConstructionState State, const int32 VictimOwner, const double VictimGapCm, const FShotOptions& Options = FShotOptions())
	{
		FShotResult Result;
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
		FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
		Context.SetCurrentWorld(World);
		World->InitializeActorsForPlay(FURL());
		AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
		World->SetGameState(GameState);
		UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
		UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
		UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
		AHOLBuilding* Tower = World->SpawnActor<AHOLBuilding>(FVector(3000.0, 0.0, 0.0), FRotator::ZeroRotator);
		AHOLBuilding* SecondTower = Options.Towers > 1 ? World->SpawnActor<AHOLBuilding>(FVector(3000.0, 1500.0, 0.0), FRotator::ZeroRotator) : nullptr;
		if (Test.TestNotNull(TEXT("units"), Units) && Test.TestNotNull(TEXT("simulation"), Simulation) && Test.TestNotNull(TEXT("entities"), EntitySubsystem)
			&& Test.TestNotNull(TEXT("tower"), Tower) && (Options.Towers <= 1 || Test.TestNotNull(TEXT("second tower"), SecondTower)))
		{
			Tower->Setup(TestShotTowerType, 0, 0, State);
			if (SecondTower)
			{
				SecondTower->Setup(TestShotTowerType, 0, 0, State);
			}
			const TSharedRef<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
			UHOLCombatProcessor* Combat = NewObject<UHOLCombatProcessor>(Simulation);
			Combat->CallInitialize(Simulation, EntityManager);
			const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
			Result.StepMs = HOLCombat::ToMilliseconds(StepSeconds);
			const auto RunStep = [Simulation, Units, Combat, EntityManager, StepSeconds]
			{
				const int64 Step = Simulation->AdvanceStepIndexForTests();
				Simulation->GetOnStepStarted().Broadcast(Step);
				Units->ServeBirthRequests();
				FMassProcessingContext CombatContext(EntityManager, StepSeconds);
				UE::Mass::Executor::Run(*Combat, CombatContext);
				Units->FlushDestructions();
				Simulation->GetOnStepEnded().Broadcast(Step);
			};
			// Born again until its identifier is a multiple of 4 (one search step of 4), as HeroesOfLiberty.Garrison.Shelter.
			uint32 Victim = 0;
			const FVector VictimAt(Tower->GetBodyBounds().Max.X + VictimGapCm, 0.0, 0.0);
			for (int32 Attempt = 0; Attempt < 8 && Victim == 0; ++Attempt)
			{
				TArray<uint32> Before;
				Units->GetAllUnitIds(Before);
				FHOLUnitSpawnRequest Request;
				Request.TypeName = FName(TEXT("Spearman"));
				Request.Count = 1;
				Request.OwnerPlayer = static_cast<uint8>(VictimOwner);
				Request.Center = VictimAt;
				Units->RequestUnits(Request);
				Units->ServeBirthRequests();
				TArray<uint32> After;
				Units->GetAllUnitIds(After);
				uint32 Born = 0;
				for (const uint32 Id : After)
				{
					Born = Before.Contains(Id) ? Born : Id;
				}
				if (Born != 0 && Born % 4 == 0)
				{
					Victim = Born;
				}
				else if (Born != 0)
				{
					const uint32 Again[] = { Born };
					Units->RequestDestroyUnits(Again);
					Units->FlushDestructions();
				}
			}
			const auto HealthOf = [Units, EntityManager](const uint32 Id)
			{
				const FMassEntityHandle Entity = Units->FindEntity(Id);
				const FHOLUnitStateFragment* UnitState = Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLUnitStateFragment>(Entity) : nullptr;
				return UnitState ? UnitState->CurrentHealth : -1.f;
			};
			FHOLCombatFragment* VictimCombat = Victim != 0 ? EntityManager->GetFragmentDataPtr<FHOLCombatFragment>(Units->FindEntity(Victim)) : nullptr;
			FTransformFragment* VictimTransform = Victim != 0 ? EntityManager->GetFragmentDataPtr<FTransformFragment>(Units->FindEntity(Victim)) : nullptr;
			if (Test.TestNotNull(TEXT("the target's combat fragment"), VictimCombat) && Test.TestNotNull(TEXT("the target's transform"), VictimTransform))
			{
				VictimCombat->Mode = EHOLCombatMode::Hold;
				if (Options.VictimHealth > 0.f)
				{
					// A frail target: the first arrow kills it (the kill goes through the arrow's landing, shooter absent). A tough one: it outlives the run.
					EntityManager->GetFragmentDataPtr<FHOLUnitStateFragment>(Units->FindEntity(Victim))->CurrentHealth = Options.VictimHealth;
				}
				const FVector Home = VictimTransform->GetTransform().GetLocation();
				int64 ReleasedBefore = UHOLCombatProcessor::GetTotalArrowsReleased();
				if (Options.PauseSteps > 0)
				{
					// No target in reach for a while: the countdown of the armed building must not pile up arrows meanwhile.
					VictimTransform->GetMutableTransform().SetLocation(Home + FVector(6000.0, 0.0, 0.0));
					for (int32 Index = 0; Index < Options.PauseSteps; ++Index)
					{
						RunStep();
					}
					Result.ReleasedDuringPause = UHOLCombatProcessor::GetTotalArrowsReleased() - ReleasedBefore;
					VictimTransform->GetMutableTransform().SetLocation(Home);
					ReleasedBefore = UHOLCombatProcessor::GetTotalArrowsReleased();
				}
				const float Start = HealthOf(Victim);
				Result.Steps = FMath::CeilToInt(Options.Seconds / StepSeconds) + 1;
				int64 Seen = ReleasedBefore;
				for (int32 Index = 0; Index < Result.Steps; ++Index)
				{
					RunStep();
					const int64 Now = UHOLCombatProcessor::GetTotalArrowsReleased();
					for (int64 Arrow = Seen; Arrow < Now; ++Arrow)
					{
						Result.ReleaseSteps.Add(Index);
					}
					Seen = Now;
					if (Options.bDestroyAfterShot && IsValid(Tower) && Now > ReleasedBefore)
					{
						// The building dies while its arrow flies: the arrow lands all the same (as an archer's whatever the shooter does meanwhile).
						Tower->Destroy();
						Result.bTowerGone = true;
					}
				}
				Result.bVictimDown = HealthOf(Victim) <= 0.f;
				Result.bRan = true;
				Result.HealthLost = Start - HealthOf(Victim);
				Result.Released = UHOLCombatProcessor::GetTotalArrowsReleased() - ReleasedBefore;
				Result.EntriesAlive = Combat->GetBuildingCooldownCount();
				if (Options.bDestroyAtEnd)
				{
					if (IsValid(Tower))
					{
						Tower->Destroy();
					}
					if (IsValid(SecondTower))
					{
						SecondTower->Destroy();
					}
					RunStep();
					RunStep();
					Result.EntriesAfterDeath = Combat->GetBuildingCooldownCount();
				}
			}
		}
		GEngine->DestroyWorldContext(World);
		return Result;
	}

	/** The arrows of Result follow the cadence exactly: the first at once, then the K-th on the step ExpectedStep says, Towers arrows per step. */
	static void CheckSchedule(FAutomationTestBase& Test, const TCHAR* What, const FShotResult& Result, const int32 IntervalMs, const int32 Towers)
	{
		if (!Test.TestTrue(FString::Printf(TEXT("%s: the scenario ran and released"), What), Result.bRan && Result.ReleaseSteps.Num() > 0))
		{
			return;
		}
		const int32 First = Result.ReleaseSteps[0];
		Test.TestTrue(FString::Printf(TEXT("%s: the first arrow leaves at once (step %d)"), What, First), First <= 1);
		const int32 Expected = ExpectedArrows(First, IntervalMs, Result.StepMs, Result.Steps) * Towers;
		Test.TestEqual(FString::Printf(TEXT("%s: the number of arrows follows the cadence"), What), Result.ReleaseSteps.Num(), Expected);
		for (int32 Index = 0; Index < Result.ReleaseSteps.Num() && Index < Expected; ++Index)
		{
			const int32 At = static_cast<int32>(ExpectedStep(First, IntervalMs, Result.StepMs, Index / Towers));
			if (Result.ReleaseSteps[Index] != At)
			{
				Test.AddError(FString::Printf(TEXT("%s: arrow %d left at step %d, the cadence says step %d"), What, Index, Result.ReleaseSteps[Index], At));
				break;
			}
		}
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingShotTest, "HeroesOfLiberty.Combat.BuildingShot", UE::HeroesOfLiberty::Tests::BuildingShot::Flags)

bool FHOLBuildingShotTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::BuildingShot;
	UDataTable* Table = const_cast<UDataTable*>(GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous());
	const FHOLBuildingTypeRow* HouseRow = Table ? Table->FindRow<FHOLBuildingTypeRow>(HOLBuildingTypeNames::House, TEXT("building shot test")) : nullptr;
	IConsoleVariable* CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
	if (!TestNotNull(TEXT("the building type table and its House row"), HouseRow) || !TestNotNull(TEXT("hol.Combat.Enabled"), CombatEnabled))
	{
		return false;
	}
	const int32 FormerCombat = CombatEnabled->GetInt();
	CombatEnabled->Set(1, ECVF_SetByCode);
	FHOLBuildingTypeRow TowerRow = *HouseRow;
	TowerRow.BuildingAttack = 5;
	TowerRow.BuildingAttackInterval = 1.625f;
	TowerRow.BuildingAttackRange = 3000.f;
	TowerRow.BuildingFiresEmpty = true;
	Table->AddRow(TestShotTowerType, TowerRow);

	// A finished tower, an enemy 1000 cm beyond its edge (reach 3000): the arrows land, the enemy loses health (at most the row's attack per arrow).
	const FShotResult Hit = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0);
	if (TestTrue(TEXT("the in-range scenario ran"), Hit.bRan))
	{
		TestTrue(TEXT("arrows are released at the enemy within reach"), Hit.Released >= 1);
		TestTrue(TEXT("the enemy within reach of the finished tower loses health"), Hit.HealthLost > 0.f);
		TestTrue(TEXT("each arrow takes at most the row's BuildingAttack off"), Hit.HealthLost <= static_cast<float>(TowerRow.BuildingAttack) * static_cast<float>(Hit.Released) + 0.01f);
	}
	// Counter-cases: nothing is released and nothing is hurt.
	const FShotResult Far = RunScenario(*this, EHOLConstructionState::Finished, 3, static_cast<double>(TowerRow.BuildingAttackRange) + 1500.0);
	TestEqual(TEXT("an enemy beyond the reach is not shot"), Far.Released, static_cast<int64>(0));
	TestEqual(TEXT("an enemy beyond the reach loses nothing"), Far.HealthLost, 0.f);
	const FShotResult Ally = RunScenario(*this, EHOLConstructionState::Finished, 0, 1000.0);
	TestEqual(TEXT("an ally of the tower's owner is not shot"), Ally.Released, static_cast<int64>(0));
	TestEqual(TEXT("an ally loses nothing"), Ally.HealthLost, 0.f);
	const FShotResult Site = RunScenario(*this, EHOLConstructionState::Foundation, 3, 1000.0);
	TestEqual(TEXT("a foundation (under construction) looses nothing"), Site.Released, static_cast<int64>(0));
	TestEqual(TEXT("an enemy near a foundation loses nothing"), Site.HealthLost, 0.f);
	// F8 T4: no armed standing building, no countdown held (nothing to pay for); an armed one without a target holds one.
	TestEqual(TEXT("a foundation holds no firing countdown"), Site.EntriesAlive, 0);
	TestEqual(TEXT("a finished armed tower holds one countdown even when no enemy is in reach"), Far.EntriesAlive, 1);

	// F8 T3: the arrow of a building has no shooter entity; a frail target dies of it (no crash), and a building that dies in flight changes nothing.
	FShotOptions Frail;
	Frail.VictimHealth = 1.f;
	const FShotResult Kill = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0, Frail);
	TestEqual(TEXT("the frail enemy is shot once (it is dead for the next arrow)"), Kill.Released, static_cast<int64>(1));
	TestTrue(TEXT("the frail enemy dies of the arrow of a building without a shooter"), Kill.bVictimDown);
	FShotOptions Destroyed;
	Destroyed.bDestroyAfterShot = true;
	const FShotResult Gone = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0, Destroyed);
	TestTrue(TEXT("the tower was destroyed in flight"), Gone.bTowerGone);
	TestTrue(TEXT("the arrow of a building destroyed in flight still hurts the enemy"), Gone.HealthLost > 0.f);

	Table->RemoveRow(TestShotTowerType);
	CombatEnabled->Set(FormerCombat, ECVF_SetByCode);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingShotCadenceTest, "HeroesOfLiberty.Combat.BuildingShotCadence", UE::HeroesOfLiberty::Tests::BuildingShot::Flags)

// F8 T4: the cadence BuildingAttackInterval (1.625 s on the TowerWood row), counted in SIMULATION time (the fixed step of the processor, never the wall clock):
// the arrows released over N seconds, the exact interval over ten arrows, no burst after a pause without target, two towers firing in the same step,
// and the countdown of a destroyed building dropped (no leak). The schedule is computed here from the step and the interval, not read from the code.
bool FHOLBuildingShotCadenceTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::BuildingShot;
	UDataTable* Table = const_cast<UDataTable*>(GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous());
	const FHOLBuildingTypeRow* HouseRow = Table ? Table->FindRow<FHOLBuildingTypeRow>(HOLBuildingTypeNames::House, TEXT("building shot cadence test")) : nullptr;
	IConsoleVariable* CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
	if (!TestNotNull(TEXT("the building type table and its House row"), HouseRow) || !TestNotNull(TEXT("hol.Combat.Enabled"), CombatEnabled))
	{
		return false;
	}
	const int32 FormerCombat = CombatEnabled->GetInt();
	CombatEnabled->Set(1, ECVF_SetByCode);
	FHOLBuildingTypeRow TowerRow = *HouseRow;
	TowerRow.BuildingAttack = 5;
	TowerRow.BuildingAttackInterval = 1.625f;
	TowerRow.BuildingAttackRange = 3000.f;
	TowerRow.BuildingFiresEmpty = true;
	Table->AddRow(TestShotTowerType, TowerRow);
	const int32 IntervalMs = HOLCombat::ToMilliseconds(TowerRow.BuildingAttackInterval);

	// A tough target (it outlives sixteen seconds of arrows), in reach all along: ten arrows over sixteen seconds, the tenth nine intervals after the first.
	FShotOptions Sixteen;
	Sixteen.VictimHealth = 100000.f;
	Sixteen.Seconds = 16.f;
	const FShotResult Steady = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0, Sixteen);
	CheckSchedule(*this, TEXT("steady fire"), Steady, IntervalMs, 1);
	if (Steady.ReleaseSteps.Num() >= 10)
	{
		TestEqual(TEXT("ten arrows over sixteen seconds at 1.625 s"), Steady.ReleaseSteps.Num(), 10);
		// The wall of the interval in real simulation time: nine intervals (14625 ms) between the first and the tenth arrow, to within one step.
		const int64 SpanMs = static_cast<int64>(Steady.ReleaseSteps[9] - Steady.ReleaseSteps[0]) * Steady.StepMs;
		TestTrue(TEXT("the tenth arrow is nine intervals after the first (not earlier)"), SpanMs >= 9 * static_cast<int64>(IntervalMs));
		TestTrue(TEXT("the tenth arrow is nine intervals after the first (not later than one step)"), SpanMs < 9 * static_cast<int64>(IntervalMs) + Steady.StepMs);
	}
	TestEqual(TEXT("one countdown for the one armed tower"), Steady.EntriesAlive, 1);

	// A pause without target (twenty seconds out of reach), then the target steps in: nothing was released during the pause, the first arrow leaves at once,
	// then the cadence goes on: no burst of the time that piled up.
	FShotOptions Paused = Sixteen;
	Paused.PauseSteps = FMath::CeilToInt(20.f / GetDefault<UHOLSimulationSettings>()->GetStepSeconds());
	Paused.Seconds = 5.f;
	const FShotResult Pause = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0, Paused);
	TestEqual(TEXT("nothing is released while no enemy is in reach"), Pause.ReleasedDuringPause, static_cast<int64>(0));
	CheckSchedule(*this, TEXT("after a pause"), Pause, IntervalMs, 1);

	// Two towers: each looses one arrow in the same step, then each follows the cadence.
	FShotOptions Pair = Sixteen;
	Pair.Towers = 2;
	Pair.Seconds = 8.f;
	const FShotResult Two = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0, Pair);
	CheckSchedule(*this, TEXT("two towers"), Two, IntervalMs, 2);
	if (Two.ReleaseSteps.Num() >= 2)
	{
		TestEqual(TEXT("the two towers loose their first arrow in the same step"), Two.ReleaseSteps[0], Two.ReleaseSteps[1]);
	}
	TestEqual(TEXT("one countdown per armed tower"), Two.EntriesAlive, 2);

	// A destroyed building leaves no countdown behind.
	FShotOptions Dies = Pair;
	Dies.Seconds = 4.f;
	Dies.bDestroyAtEnd = true;
	const FShotResult Cleanup = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0, Dies);
	TestEqual(TEXT("two countdowns while the towers stand"), Cleanup.EntriesAlive, 2);
	TestEqual(TEXT("no countdown left once the towers are destroyed"), Cleanup.EntriesAfterDeath, 0);

	Table->RemoveRow(TestShotTowerType);
	CombatEnabled->Set(FormerCombat, ECVF_SetByCode);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingShotDamageTest, "HeroesOfLiberty.Combat.BuildingShotDamage", UE::HeroesOfLiberty::Tests::BuildingShot::Flags)

// F8 T3: the building's arrow takes the ranged armour like an archer's arrow (the value is what HOLCombat::GetDamage computes for a plain ranged attacker
// of the same attack, not a copied number), 1 at least, the Forge percent and the talent's armour bonus count, a melee armour does not, no bonus is invented.
bool FHOLBuildingShotDamageTest::RunTest(const FString& Parameters)
{
	FHOLUnitTypeConstants Archer;
	Archer.Attack = 5;
	Archer.AttackType = EHOLAttackType::Ranged;
	FHOLUnitTypeConstants Victim;
	Victim.RangedArmor = 2;
	Victim.MeleeArmor = 40;
	const float Armoured = HOLCombat::GetBuildingShotDamage(5.f, Victim);
	TestEqual(TEXT("a building's arrow takes the ranged armour as an archer's arrow"), Armoured, HOLCombat::GetDamage(Archer, Victim));
	TestTrue(TEXT("the ranged armour reduces the base damage"), Armoured < 5.f);
	TestEqual(TEXT("the Forge percent counts as for an archer's arrow"), HOLCombat::GetBuildingShotDamage(5.f, Victim, 0, 50), HOLCombat::GetDamage(Archer, Victim, 1, 0, 0, 50));
	TestEqual(TEXT("the talent's armour bonus counts as for an archer's arrow"), HOLCombat::GetBuildingShotDamage(5.f, Victim, 3, 0), HOLCombat::GetDamage(Archer, Victim, 1, 3, 0, 0));
	Victim.RangedArmor = 99;
	TestEqual(TEXT("the minimum of 1 applies (a hit always hurts)"), HOLCombat::GetBuildingShotDamage(5.f, Victim), 1.f);
	Victim.RangedArmor = 0;
	TestEqual(TEXT("no armour: the base damage as is, no bonus invented (the melee armour does not count)"), HOLCombat::GetBuildingShotDamage(5.f, Victim), 5.f);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
