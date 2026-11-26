// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Map/HOLFogMode.h"
#include "Map/HOLVisionSubsystem.h"
#include "Match/HOLMatchConfig.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassNavigationFragments.h"
#include "MassProcessingContext.h"
#include "Match/HOLGameState.h"
#include "Presentation/HOLArrowSubsystem.h"
#include "Presentation/HOLCombatSoundSubsystem.h"
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
	/** F8 T6: the same row with BuildingFiresEmpty false (a building that looses arrows only with an occupant). */
	const FName TestShotNoEmptyTowerType(TEXT("BuildingShotTestTowerNoEmpty"));

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
		/** F8 T6: the arrows released up to the step the occupants left (LeaveAtStep), and after it. */
		int64 ReleasedAtLeave = 0;
		int64 ReleasedAfterLeave = 0;
		/** F8 T7: the arrows the display subsystem was asked to draw during the window (-1: no display subsystem in the world). */
		int64 Launched = -1;
		/** AR6-d: the Attack sound events of the tower type raised during the window (UHOLCombatSoundSubsystem, -1: no subsystem in the world). */
		int32 TowerAttackSounds = -1;
		/** S3-a: the launch lines hol.Combat.LogBuildingShots wrote during the window (UHOLCombatProcessor::GetTotalBuildingShotsLogged), and whether at any step they differed from the arrows released by then. */
		int64 LaunchLogged = 0;
		bool bLaunchLogBehind = false;
		/** F8 T8: what the building shots cost the processor over the window (UHOLCombatProcessor::GetBuildingShotCost). */
		UHOLCombatProcessor::FBuildingShotCost Cost;
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
		/** F8 T5: the type of the towers (a row copied with another SightRadius), and the fog: 0 never started, 1 Normal, 2 Disabled (started, no fog). */
		FName TowerType = TestShotTowerType;
		int32 Fog = 0;
		/** F8 T5b: hol.Combat.Enabled 0 for the whole window (bCombatOff), or only from the end of the window on, before the towers are destroyed (bCombatOffAtEnd). */
		bool bCombatOff = false;
		bool bCombatOffAtEnd = false;
		/** F8 T6: occupants (allied Spearmen set in the tower's garrison before the first step); they all leave after the step LeaveAtStep (-1: they stay). */
		int32 Occupants = 0;
		int32 LeaveAtStep = -1;
		/** F8 T8: enemy Spearmen added beside the target (the bench: 199 + the target = 200 units). */
		int32 Crowd = 0;
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
			Tower->Setup(Options.TowerType, 0, 0, State);
			if (SecondTower)
			{
				SecondTower->Setup(Options.TowerType, 0, 0, State);
			}
			for (int32 Index = 2; Index < Options.Towers; ++Index)
			{
				// F8 T8: the bench's other towers (all within reach of the target, bodies may overlap: only the processor is under test).
				AHOLBuilding* Extra = World->SpawnActor<AHOLBuilding>(FVector(3000.0, 100.0 * Index, 0.0), FRotator::ZeroRotator);
				if (Test.TestNotNull(TEXT("extra tower"), Extra))
				{
					Extra->Setup(Options.TowerType, 0, 0, State);
				}
			}
			const TSharedRef<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
			UHOLCombatProcessor* Combat = NewObject<UHOLCombatProcessor>(Simulation);
			Combat->CallInitialize(Simulation, EntityManager);
			if (Options.Fog != 0)
			{
				// Owner 0 (the towers) and owner 3 (the target) in two teams; the fog of the last step end is what the towers' owner sees.
				FHOLMatchConfig Teams = FHOLMatchConfig::MakeDefault();
				for (FHOLMatchSlot& Slot : Teams.Slots)
				{
					Slot.State = EHOLSlotState::Closed;
					Slot.Team = 0;
				}
				Teams.Slots[0].State = EHOLSlotState::Human;
				Teams.Slots[0].Team = 1;
				if (Teams.Slots.IsValidIndex(3))
				{
					Teams.Slots[3].State = EHOLSlotState::AI;
					Teams.Slots[3].Team = 2;
				}
				UHOLVisionSubsystem* Vision = World->GetSubsystem<UHOLVisionSubsystem>();
				if (Test.TestNotNull(TEXT("the vision subsystem"), Vision))
				{
					Vision->Start(Teams, Options.Fog == 1 ? EHOLFogMode::Normal : EHOLFogMode::Disabled);
					Vision->Update();
				}
			}
			IConsoleVariable* CombatEnabledVar = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
			if (Options.bCombatOff && CombatEnabledVar)
			{
				CombatEnabledVar->Set(0, ECVF_SetByCode);
			}
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
				// F8 T6: allied occupants, set in the garrison of the first tower (the garrison rule is not applied: UHOLUnitSubsystem::SetGarrison checks nothing).
				TArray<uint32> OccupantIds;
				for (int32 Index = 0; Index < Options.Occupants; ++Index)
				{
					TArray<uint32> Before;
					Units->GetAllUnitIds(Before);
					FHOLUnitSpawnRequest OccupantRequest;
					OccupantRequest.TypeName = FName(TEXT("Spearman"));
					OccupantRequest.Count = 1;
					OccupantRequest.OwnerPlayer = 0;
					OccupantRequest.Center = Tower->GetActorLocation();
					Units->RequestUnits(OccupantRequest);
					Units->ServeBirthRequests();
					TArray<uint32> After;
					Units->GetAllUnitIds(After);
					for (const uint32 Id : After)
					{
						if (!Before.Contains(Id) && Units->SetGarrison(Id, Tower))
						{
							OccupantIds.Add(Id);
						}
					}
				}
				if (Options.Crowd > 0)
				{
					FHOLUnitSpawnRequest CrowdRequest;
					CrowdRequest.TypeName = FName(TEXT("Spearman"));
					CrowdRequest.Count = Options.Crowd;
					CrowdRequest.OwnerPlayer = static_cast<uint8>(VictimOwner);
					CrowdRequest.Center = VictimAt;
					Units->RequestUnits(CrowdRequest);
					Units->ServeBirthRequests();
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
				const UHOLArrowSubsystem* ArrowDisplay = World->GetSubsystem<UHOLArrowSubsystem>(); const int64 LaunchedBefore = ArrowDisplay ? ArrowDisplay->GetLaunchedCount() : 0; const float Start = HealthOf(Victim);
				const UHOLCombatSoundSubsystem* Sounds = World->GetSubsystem<UHOLCombatSoundSubsystem>();
				const int32 SoundsBefore = Sounds ? Sounds->GetEventCountFor(Options.TowerType, UHOLCombatSoundSubsystem::AttackSlot) : 0;
				Result.Steps = FMath::CeilToInt(Options.Seconds / StepSeconds) + 1;
				int64 Seen = ReleasedBefore; const int64 LoggedBefore = UHOLCombatProcessor::GetTotalBuildingShotsLogged();
				for (int32 Index = 0; Index < Result.Steps; ++Index)
				{
					RunStep();
					const int64 Now = UHOLCombatProcessor::GetTotalArrowsReleased();
					for (int64 Arrow = Seen; Arrow < Now; ++Arrow)
					{
						Result.ReleaseSteps.Add(Index);
					}
					Seen = Now;
					// S3-a: the launch line is written in the step the arrow leaves (before any impact, which comes steps later), one per arrow.
					Result.LaunchLogged = UHOLCombatProcessor::GetTotalBuildingShotsLogged() - LoggedBefore;
					if (Result.LaunchLogged != Now - ReleasedBefore)
					{
						Result.bLaunchLogBehind = true;
					}
					if (Index == Options.LeaveAtStep)
					{
						// F8 T6: the occupants leave (the unit goes back to the field); what the tower looses from the next step on is counted apart.
						for (const uint32 Id : OccupantIds)
						{
							Units->SetGarrison(Id, nullptr);
						}
						Result.ReleasedAtLeave = Now - ReleasedBefore;
					}
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
				Result.Released = UHOLCombatProcessor::GetTotalArrowsReleased() - ReleasedBefore; Result.Launched = ArrowDisplay ? ArrowDisplay->GetLaunchedCount() - LaunchedBefore : -1;
				Result.TowerAttackSounds = Sounds ? Sounds->GetEventCountFor(Options.TowerType, UHOLCombatSoundSubsystem::AttackSlot) - SoundsBefore : -1;
				Result.ReleasedAfterLeave = Result.Released - Result.ReleasedAtLeave;
				Result.EntriesAlive = Combat->GetBuildingCooldownCount();
				Result.Cost = Combat->GetBuildingShotCost();
				if (Options.bCombatOffAtEnd && CombatEnabledVar)
				{
					CombatEnabledVar->Set(0, ECVF_SetByCode);
				}
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
				if (CombatEnabledVar && (Options.bCombatOff || Options.bCombatOffAtEnd))
				{
					CombatEnabledVar->Set(1, ECVF_SetByCode);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingShotFiresEmptyTest, "HeroesOfLiberty.Combat.BuildingShotFiresEmpty", UE::HeroesOfLiberty::Tests::BuildingShot::Flags)

// F8 T6 (D884): BuildingFiresEmpty true: the tower looses arrows with no occupant; false: only with at least one occupant (the garrison list of the
// unit subsystem), and it stops when the last one leaves. The number of arrows per shot is 1 + one per occupant (D884, confirmed 2026-11-23, pinned by
// HeroesOfLiberty.BuildingShot.ArrowsPerOccupant): here the one occupant makes two arrows per shot. The rows are test rows (no row of the game is false today).
bool FHOLBuildingShotFiresEmptyTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::BuildingShot;
	UDataTable* Table = const_cast<UDataTable*>(GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous());
	const FHOLBuildingTypeRow* HouseRow = Table ? Table->FindRow<FHOLBuildingTypeRow>(HOLBuildingTypeNames::House, TEXT("building shot fires empty test")) : nullptr;
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
	TowerRow.BuildingFiresEmpty = false;
	Table->AddRow(TestShotNoEmptyTowerType, TowerRow);

	// FiresEmpty true, no occupant: it looses arrows and hurts the enemy in reach.
	FShotOptions Short;
	Short.Seconds = 3.f;
	const FShotResult EmptyTrue = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0, Short);
	if (TestTrue(TEXT("the FiresEmpty true scenario ran"), EmptyTrue.bRan))
	{
		TestTrue(TEXT("FiresEmpty true: an empty tower looses arrows"), EmptyTrue.Released >= 1);
		TestTrue(TEXT("FiresEmpty true: an empty tower hurts the enemy in reach"), EmptyTrue.HealthLost > 0.f);
	}
	// FiresEmpty false, no occupant: nothing is released, nothing is hurt, no countdown is held.
	FShotOptions NoEmptyEmpty = Short;
	NoEmptyEmpty.TowerType = TestShotNoEmptyTowerType;
	const FShotResult EmptyFalse = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0, NoEmptyEmpty);
	if (TestTrue(TEXT("the FiresEmpty false empty scenario ran"), EmptyFalse.bRan))
	{
		TestEqual(TEXT("FiresEmpty false: an empty tower looses no arrow"), EmptyFalse.Released, static_cast<int64>(0));
		TestEqual(TEXT("FiresEmpty false: an empty tower hurts nobody"), EmptyFalse.HealthLost, 0.f);
		TestEqual(TEXT("FiresEmpty false: an empty tower holds no countdown"), EmptyFalse.EntriesAlive, 0);
	}
	// FiresEmpty false, one occupant: it looses arrows, TWO per shot (1 + one occupant), both on the step of the shot.
	FShotOptions Garrisoned = NoEmptyEmpty;
	Garrisoned.Occupants = 1;
	const FShotResult Occupied = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0, Garrisoned);
	if (TestTrue(TEXT("the garrisoned scenario ran"), Occupied.bRan))
	{
		TestTrue(TEXT("FiresEmpty false: a garrisoned tower looses arrows"), Occupied.Released >= 1);
		TestTrue(TEXT("FiresEmpty false: a garrisoned tower hurts the enemy in reach"), Occupied.HealthLost > 0.f);
		TestEqual(TEXT("two arrows per shot (1 + one occupant): the arrows released are twice the distinct steps they left on"), static_cast<int64>(Occupied.ReleaseSteps.Num()), static_cast<int64>(2 * TSet<int32>(Occupied.ReleaseSteps).Num()));
	}
	// The occupant leaves after step 5 (the first arrow left at once): the tower stops.
	FShotOptions Leaving = Garrisoned;
	Leaving.Seconds = 6.f;
	Leaving.LeaveAtStep = 5;
	const FShotResult Left = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0, Leaving);
	if (TestTrue(TEXT("the leaving scenario ran"), Left.bRan))
	{
		TestTrue(TEXT("the garrisoned tower had loosed an arrow before the occupant left"), Left.ReleasedAtLeave >= 1);
		TestEqual(TEXT("an occupant who leaves stops a FiresEmpty false tower"), Left.ReleasedAfterLeave, static_cast<int64>(0));
		TestEqual(TEXT("a tower that stopped holds no countdown"), Left.EntriesAlive, 0);
	}

	Table->RemoveRow(TestShotNoEmptyTowerType);
	Table->RemoveRow(TestShotTowerType);
	CombatEnabled->Set(FormerCombat, ECVF_SetByCode);
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingShotArrowsPerOccupantTest, "HeroesOfLiberty.BuildingShot.ArrowsPerOccupant", UE::HeroesOfLiberty::Tests::BuildingShot::Flags)

// D884 (confirmed 2026-11-23, ai_garrison trial): a garrisoned tower looses 1 + one arrow per occupant at each shot, all on the same step (UHOLCombatProcessor::GetBuildingArrowCount).
// The window is one second, shorter than the 1.625 s interval: only the FIRST volley is counted. The tower row is a FiresEmpty true test row (the occupants only change the number).
bool FHOLBuildingShotArrowsPerOccupantTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::BuildingShot;
	UDataTable* Table = const_cast<UDataTable*>(GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous());
	const FHOLBuildingTypeRow* HouseRow = Table ? Table->FindRow<FHOLBuildingTypeRow>(HOLBuildingTypeNames::House, TEXT("building shot arrows per occupant test")) : nullptr;
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

	// 0, 1 and 3 occupants: the first volley is 1, 2 and 4 arrows, all on one step; a tough target outlives the window.
	const int32 OccupantCounts[] = {0, 1, 3};
	for (const int32 Occupants : OccupantCounts)
	{
		FShotOptions Options;
		Options.Seconds = 1.f;
		Options.VictimHealth = 100000.f;
		Options.Occupants = Occupants;
		const FShotResult Volley = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0, Options);
		if (TestTrue(*FString::Printf(TEXT("the %d occupants scenario ran"), Occupants), Volley.bRan))
		{
			TestEqual(*FString::Printf(TEXT("%d occupants: the first volley looses %d arrows"), Occupants, 1 + Occupants), Volley.Released, static_cast<int64>(1 + Occupants));
			TestEqual(*FString::Printf(TEXT("%d occupants: all the arrows of the volley leave on one step"), Occupants), TSet<int32>(Volley.ReleaseSteps).Num(), 1);
			if (Volley.Launched >= 0)
			{
				TestEqual(*FString::Printf(TEXT("%d occupants: the display is asked to draw every arrow of the volley"), Occupants), Volley.Launched, static_cast<int64>(1 + Occupants));
			}
		}
	}

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingShotFogTest, "HeroesOfLiberty.Combat.BuildingShotFog", UE::HeroesOfLiberty::Tests::BuildingShot::Flags)

// F8 T5: a tower only aims at what its OWNER sees (the same vision test as the units' shots: UHOLVisionSubsystem::IsVisible of the last step end).
// Three copies of the tower row differ by SightRadius only: 6000 (it sees its whole reach), 0 (blind: its owner sees nothing there), 1500 (sees the near
// target, not the one at the far end of its 3000 reach). No fog started, or the fog Disabled (hol.Fog.Mode 0): everything is seen, as for the units.
bool FHOLBuildingShotFogTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::BuildingShot;
	UDataTable* Table = const_cast<UDataTable*>(GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous());
	const FHOLBuildingTypeRow* HouseRow = Table ? Table->FindRow<FHOLBuildingTypeRow>(HOLBuildingTypeNames::House, TEXT("building shot fog test")) : nullptr;
	IConsoleVariable* CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
	if (!TestNotNull(TEXT("the building type table and its House row"), HouseRow) || !TestNotNull(TEXT("hol.Combat.Enabled"), CombatEnabled))
	{
		return false;
	}
	const int32 FormerCombat = CombatEnabled->GetInt();
	CombatEnabled->Set(1, ECVF_SetByCode);
	const FName SightedType(TEXT("BuildingShotTestSighted"));
	const FName BlindType(TEXT("BuildingShotTestBlind"));
	const FName ShortType(TEXT("BuildingShotTestShortSight"));
	const TPair<FName, float> Kinds[] = { { SightedType, 6000.f }, { BlindType, 0.f }, { ShortType, 1500.f } };
	for (const TPair<FName, float>& Kind : Kinds)
	{
		FHOLBuildingTypeRow Row = *HouseRow;
		Row.BuildingAttack = 5;
		Row.BuildingAttackInterval = 1.625f;
		Row.BuildingAttackRange = 3000.f;
		Row.BuildingFiresEmpty = true;
		Row.SightRadius = Kind.Value;
		Table->AddRow(Kind.Key, Row);
	}
	FShotOptions Fog;
	Fog.VictimHealth = 100000.f;
	Fog.Seconds = 6.f;
	Fog.Fog = 1;

	Fog.TowerType = SightedType;
	const FShotResult Sighted = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0, Fog);
	TestTrue(TEXT("a tower that sees its reach shoots (the fog pipeline is live)"), Sighted.bRan && Sighted.Released >= 1 && Sighted.HealthLost > 0.f);

	Fog.TowerType = BlindType;
	const FShotResult Blind = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0, Fog);
	TestTrue(TEXT("the blind scenario ran"), Blind.bRan);
	TestEqual(TEXT("a tower whose owner does not see the enemy looses no arrow"), Blind.Released, static_cast<int64>(0));
	TestEqual(TEXT("an enemy the owner does not see loses nothing"), Blind.HealthLost, 0.f);

	Fog.Fog = 2;
	const FShotResult NoFog = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0, Fog);
	TestTrue(TEXT("with the fog Disabled everything is seen: the blind tower shoots"), NoFog.bRan && NoFog.Released >= 1 && NoFog.HealthLost > 0.f);
	Fog.Fog = 0;
	const FShotResult NotStarted = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0, Fog);
	TestTrue(TEXT("with no fog started everything is seen: the blind tower shoots"), NotStarted.bRan && NotStarted.Released >= 1);

	// Out of sight is not chosen: the same tower (sight 1500, reach 3000) shoots the near enemy and ignores the one at the far end of its reach.
	Fog.Fog = 1;
	Fog.TowerType = ShortType;
	const FShotResult Near = RunScenario(*this, EHOLConstructionState::Finished, 3, 200.0, Fog);
	const FShotResult Far = RunScenario(*this, EHOLConstructionState::Finished, 3, 2500.0, Fog);
	TestTrue(TEXT("the near enemy, inside the tower's sight, is shot"), Near.bRan && Near.Released >= 1);
	TestEqual(TEXT("the enemy in reach but beyond the sight is not shot"), Far.Released, static_cast<int64>(0));

	for (const TPair<FName, float>& Kind : Kinds)
	{
		Table->RemoveRow(Kind.Key);
	}
	CombatEnabled->Set(FormerCombat, ECVF_SetByCode);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingShotGuardTest, "HeroesOfLiberty.Combat.BuildingShotGuard", UE::HeroesOfLiberty::Tests::BuildingShot::Flags)

// F8 T5b: with hol.Combat.Enabled 0 (bFighting false) no building looses an arrow and no countdown is held; combat turned off after the towers
// were met, then the towers destroyed: their countdowns are dropped all the same (no leak while the combat is off).
bool FHOLBuildingShotGuardTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::BuildingShot;
	UDataTable* Table = const_cast<UDataTable*>(GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous());
	const FHOLBuildingTypeRow* HouseRow = Table ? Table->FindRow<FHOLBuildingTypeRow>(HOLBuildingTypeNames::House, TEXT("building shot guard test")) : nullptr;
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

	FShotOptions Off;
	Off.VictimHealth = 100000.f;
	Off.Seconds = 6.f;
	Off.bCombatOff = true;
	const FShotResult Idle = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0, Off);
	TestTrue(TEXT("the combat-off scenario ran"), Idle.bRan);
	TestEqual(TEXT("with hol.Combat.Enabled 0 no building looses an arrow"), Idle.Released, static_cast<int64>(0));
	TestEqual(TEXT("with hol.Combat.Enabled 0 the enemy in reach loses nothing"), Idle.HealthLost, 0.f);
	TestEqual(TEXT("with hol.Combat.Enabled 0 no countdown is held"), Idle.EntriesAlive, 0);

	FShotOptions Leak;
	Leak.VictimHealth = 100000.f;
	Leak.Seconds = 4.f;
	Leak.Towers = 2;
	Leak.bDestroyAtEnd = true;
	Leak.bCombatOffAtEnd = true;
	const FShotResult Left = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0, Leak);
	TestEqual(TEXT("two countdowns while the combat is on and the towers stand"), Left.EntriesAlive, 2);
	TestEqual(TEXT("the combat off and the towers destroyed: no countdown left behind"), Left.EntriesAfterDeath, 0);

	Table->RemoveRow(TestShotTowerType);
	CombatEnabled->Set(FormerCombat, ECVF_SetByCode);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingShotNoUnitTest, "HeroesOfLiberty.Combat.BuildingShotNoUnit", UE::HeroesOfLiberty::Tests::BuildingShot::Flags)

// F8 T5c (CELEBORN-1122k): the one unit alive is killed by the first arrow, so every following step has NO unit at all and Execute leaves at its
// `Fighters.Num() == 0` return, before the buildings are met. The towers are then destroyed: their countdowns must be purged all the same (no leak without unit).
bool FHOLBuildingShotNoUnitTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::BuildingShot;
	UDataTable* Table = const_cast<UDataTable*>(GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous());
	const FHOLBuildingTypeRow* HouseRow = Table ? Table->FindRow<FHOLBuildingTypeRow>(HOLBuildingTypeNames::House, TEXT("building shot no unit test")) : nullptr;
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

	FShotOptions Alone;
	Alone.VictimHealth = 1.f; // the first arrow kills it: no unit left afterwards
	Alone.Seconds = 4.f;
	Alone.Towers = 2;
	Alone.bDestroyAtEnd = true;
	const FShotResult Result = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0, Alone);
	TestTrue(TEXT("the scenario ran"), Result.bRan);
	TestTrue(TEXT("the only unit was killed by an arrow (no unit left to gather)"), Result.bVictimDown);
	TestEqual(TEXT("the towers destroyed with no unit alive: no countdown left behind"), Result.EntriesAfterDeath, 0);

	Table->RemoveRow(TestShotTowerType);
	CombatEnabled->Set(FormerCombat, ECVF_SetByCode);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingShotIdleVictimTest, "HeroesOfLiberty.Combat.BuildingShotIdleVictim", UE::HeroesOfLiberty::Tests::BuildingShot::Flags)

// F8 T4 (CELEBORN-1122l): the only unit is an idle tough victim of the tower (no arrow in flight between two shots, nobody to search or to strike): the combat
// processor's idle step skip (hol.Combat.FastIdle) must not hide those steps from the tower's countdown. The gap of the first two arrows is computed here from the
// interval and the step; the measured fault was the countdown read one step in four (about four times the interval).
bool FHOLBuildingShotIdleVictimTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::BuildingShot;
	UDataTable* Table = const_cast<UDataTable*>(GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous());
	const FHOLBuildingTypeRow* HouseRow = Table ? Table->FindRow<FHOLBuildingTypeRow>(HOLBuildingTypeNames::House, TEXT("building shot idle victim test")) : nullptr;
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

	FShotOptions Tough;
	Tough.VictimHealth = 100000.f;
	Tough.Seconds = 6.f;
	const FShotResult Result = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0, Tough);
	if (TestTrue(TEXT("the scenario ran and released two arrows at least"), Result.bRan && Result.ReleaseSteps.Num() >= 2))
	{
		const int64 GapMs = static_cast<int64>(Result.ReleaseSteps[1] - Result.ReleaseSteps[0]) * Result.StepMs;
		TestTrue(TEXT("the second arrow is one interval after the first (not earlier)"), GapMs >= IntervalMs);
		TestTrue(TEXT("the second arrow is one interval after the first (not later than one step)"), GapMs < static_cast<int64>(IntervalMs) + Result.StepMs);
		TestEqual(TEXT("the arrows of six seconds follow the cadence"), Result.ReleaseSteps.Num(), ExpectedArrows(Result.ReleaseSteps[0], IntervalMs, Result.StepMs, Result.Steps));
	}

	Table->RemoveRow(TestShotTowerType);
	CombatEnabled->Set(FormerCombat, ECVF_SetByCode);
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingShotDisplayTest, "HeroesOfLiberty.Combat.BuildingShotDisplay", UE::HeroesOfLiberty::Tests::BuildingShot::Flags)

// F8 T7 (D884): a tower's arrow is drawn like an archer's (solo / host: the processor feeds UHOLArrowSubsystem::Launch at the release). The effect read is the
// display subsystem's own launched count over the window, against the arrows the processor released (both counted by the code under test, nothing copied).
bool FHOLBuildingShotDisplayTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::BuildingShot;
	UDataTable* Table = const_cast<UDataTable*>(GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous());
	const FHOLBuildingTypeRow* HouseRow = Table ? Table->FindRow<FHOLBuildingTypeRow>(HOLBuildingTypeNames::House, TEXT("building shot display test")) : nullptr;
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

	const FShotResult Shot = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0);
	if (TestTrue(TEXT("the scenario ran and the tower released arrows"), Shot.bRan && Shot.Released >= 1) && TestTrue(TEXT("the world has an arrow display subsystem"), Shot.Launched >= 0))
	{
		TestEqual(TEXT("every arrow a tower releases is launched on the display (one drawn arrow per released arrow)"), Shot.Launched, Shot.Released);
	}

	Table->RemoveRow(TestShotTowerType);
	CombatEnabled->Set(FormerCombat, ECVF_SetByCode);
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingShotSoundTest, "HeroesOfLiberty.Combat.BuildingShotSound", UE::HeroesOfLiberty::Tests::BuildingShot::Flags)

// AR6-d (ArtistesInProgress.md): a tower's volley rings its type's Attack sound ONCE (not once per arrow). Two occupants make volleys of 3 arrows; the
// sound events of the tower type (counted by UHOLCombatSoundSubsystem) against the arrows the processor released (both counted by the code under test).
bool FHOLBuildingShotSoundTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::BuildingShot;
	UDataTable* Table = const_cast<UDataTable*>(GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous());
	const FHOLBuildingTypeRow* HouseRow = Table ? Table->FindRow<FHOLBuildingTypeRow>(HOLBuildingTypeNames::House, TEXT("building shot sound test")) : nullptr;
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

	FShotOptions Volleys;
	Volleys.VictimHealth = 100000.f;
	Volleys.Occupants = 2;
	const FShotResult Shot = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0, Volleys);
	if (TestTrue(TEXT("the scenario ran and the tower released volleys"), Shot.bRan && Shot.Released >= 3)
		&& TestTrue(TEXT("the world has the sound subsystem"), Shot.TowerAttackSounds >= 0))
	{
		TestEqual(TEXT("volleys of 3 arrows (1 + 2 occupants)"), Shot.Released % 3, static_cast<int64>(0));
		TestEqual(TEXT("one Attack sound of the tower type per volley, not per arrow"), static_cast<int64>(Shot.TowerAttackSounds), Shot.Released / 3);
	}
	Table->RemoveRow(TestShotTowerType);
	CombatEnabled->Set(FormerCombat, ECVF_SetByCode);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingShotCostTest, "HeroesOfLiberty.Combat.BuildingShotCost", UE::HeroesOfLiberty::Tests::BuildingShot::Flags)

// F8 T8 (D884, measure): the bench of the plan (CELEBORN-1122d T8), 200 enemy units and 10 towers all in reach for 10 s of the REAL combat processor. The cost is
// read from the processor's own counters (GetBuildingShotCost: searches = a ready tower looking for a target, candidates = fighters it tests). Three bounds:
// (1) the towers do search and test fighters (the counter is alive), (2) the cadence gates the searches (a tower searches when its countdown is ready:
// at most one search per arrow of the schedule, ExpectedArrows + 1 per tower, never one per step), (3) the budget: no step tests more than Towers x Units
// candidates and the average step stays under BudgetAveragePerStep (value posed here, the session may move it).
bool FHOLBuildingShotCostTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::BuildingShot;
	constexpr int32 Towers = 10;
	constexpr int32 Units = 200;
	constexpr int64 BudgetAveragePerStep = 200;
	UDataTable* Table = const_cast<UDataTable*>(GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous());
	const FHOLBuildingTypeRow* HouseRow = Table ? Table->FindRow<FHOLBuildingTypeRow>(HOLBuildingTypeNames::House, TEXT("building shot cost test")) : nullptr;
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

	FShotOptions Bench;
	Bench.VictimHealth = 100000.f;
	Bench.Towers = Towers;
	Bench.Crowd = Units - 1;
	Bench.Seconds = 10.f;
	const FShotResult Run = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0, Bench);
	if (TestTrue(TEXT("the bench ran"), Run.bRan))
	{
		const UHOLCombatProcessor::FBuildingShotCost& Cost = Run.Cost;
		TestTrue(FString::Printf(TEXT("the towers searched (searches %lld)"), Cost.TotalSearches), Cost.TotalSearches >= Towers);
		TestTrue(FString::Printf(TEXT("each search tested at least one fighter (candidates %lld, searches %lld)"), Cost.TotalCandidates, Cost.TotalSearches), Cost.TotalCandidates >= Cost.TotalSearches && Cost.TotalCandidates > 0);
		const int64 SearchCeiling = static_cast<int64>(Towers) * (ExpectedArrows(0, IntervalMs, Run.StepMs, Run.Steps) + 1);
		TestTrue(FString::Printf(TEXT("the cadence gates the searches (%lld searches over %d steps, ceiling %lld)"), Cost.TotalSearches, Run.Steps, SearchCeiling), Cost.TotalSearches <= SearchCeiling);
		TestTrue(FString::Printf(TEXT("no step tests more than towers x units (%d <= %d)"), Cost.MaxStepCandidates, Towers * Units), Cost.MaxStepCandidates <= Towers * Units);
		const int64 Average = Cost.TotalCandidates / FMath::Max<int64>(1, Cost.StepsCounted);
		TestTrue(FString::Printf(TEXT("the average step stays under the budget (%lld <= %lld candidates, %lld steps)"), Average, BudgetAveragePerStep, Cost.StepsCounted), Average <= BudgetAveragePerStep && Cost.StepsCounted > 0);
		AddInfo(FString::Printf(TEXT("BuildingShotCost: %lld searches, %lld candidates over %lld steps (average %lld, dearest step %d)"), Cost.TotalSearches, Cost.TotalCandidates, Cost.StepsCounted, Average, Cost.MaxStepCandidates));
	}
	Table->RemoveRow(TestShotTowerType);
	CombatEnabled->Set(FormerCombat, ECVF_SetByCode);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingShotLaunchLogTest, "HeroesOfLiberty.Combat.BuildingShotLaunchLog", UE::HeroesOfLiberty::Tests::BuildingShot::Flags)

// S3-a: with hol.Combat.LogBuildingShots 1 a building writes ONE launch line per arrow, in the step the arrow leaves (the count is the processor's
// GetTotalBuildingShotsLogged, checked after every step against the arrows released: no file read); with the cvar off (its default) none.
bool FHOLBuildingShotLaunchLogTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::BuildingShot;
	UDataTable* Table = const_cast<UDataTable*>(GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous());
	const FHOLBuildingTypeRow* HouseRow = Table ? Table->FindRow<FHOLBuildingTypeRow>(HOLBuildingTypeNames::House, TEXT("building shot launch log test")) : nullptr;
	IConsoleVariable* CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
	IConsoleVariable* LogShots = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.LogBuildingShots"));
	if (!TestNotNull(TEXT("the building type table and its House row"), HouseRow) || !TestNotNull(TEXT("hol.Combat.Enabled"), CombatEnabled) || !TestNotNull(TEXT("hol.Combat.LogBuildingShots"), LogShots))
	{
		return false;
	}
	const int32 FormerCombat = CombatEnabled->GetInt();
	const int32 FormerLog = LogShots->GetInt();
	TestEqual(TEXT("the launch line is off by default"), FormerLog, 0);
	CombatEnabled->Set(1, ECVF_SetByCode);
	FHOLBuildingTypeRow TowerRow = *HouseRow;
	TowerRow.BuildingAttack = 5;
	TowerRow.BuildingAttackInterval = 1.625f;
	TowerRow.BuildingAttackRange = 3000.f;
	TowerRow.BuildingFiresEmpty = true;
	Table->AddRow(TestShotTowerType, TowerRow);

	LogShots->Set(1, ECVF_SetByCode);
	const FShotResult On = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0);
	if (TestTrue(TEXT("the lit scenario ran and released"), On.bRan && On.Released >= 1))
	{
		TestEqual(TEXT("one launch line per arrow released"), On.LaunchLogged, On.Released);
		TestFalse(TEXT("at every step the lines written equal the arrows released (the line is at the launch, before the impact)"), On.bLaunchLogBehind);
	}
	FShotOptions TwoTowers;
	TwoTowers.Towers = 2;
	const FShotResult Two = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0, TwoTowers);
	if (TestTrue(TEXT("the two-tower scenario ran and released"), Two.bRan && Two.Released >= 2))
	{
		TestEqual(TEXT("two towers: still one launch line per arrow"), Two.LaunchLogged, Two.Released);
		TestFalse(TEXT("two towers: the lines follow the releases at every step"), Two.bLaunchLogBehind);
	}
	LogShots->Set(0, ECVF_SetByCode);
	const FShotResult Off = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0);
	if (TestTrue(TEXT("the dark scenario ran and released"), Off.bRan && Off.Released >= 1))
	{
		TestEqual(TEXT("cvar off: no launch line"), Off.LaunchLogged, static_cast<int64>(0));
	}
	Table->RemoveRow(TestShotTowerType);
	LogShots->Set(FormerLog, ECVF_SetByCode);
	CombatEnabled->Set(FormerCombat, ECVF_SetByCode);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLArrowScaleCvarTest, "HeroesOfLiberty.Combat.ArrowScaleCvar", UE::HeroesOfLiberty::Tests::BuildingShot::Flags)

// S3-a: hol.Arrow.Scale multiplies the scale the arrow subsystem draws (default 1: the base 1.5 / 4 / 4, unchanged in the game), and
// hol.Arrow.SlowMotion the drawn flight time (default 1: unchanged). Both are read where the subsystem renders (GetArrowScale / GetFlightDurationSeconds).
bool FHOLArrowScaleCvarTest::RunTest(const FString& Parameters)
{
	IConsoleVariable* Scale = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Arrow.Scale"));
	IConsoleVariable* Slow = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Arrow.SlowMotion"));
	if (!TestNotNull(TEXT("hol.Arrow.Scale"), Scale) || !TestNotNull(TEXT("hol.Arrow.SlowMotion"), Slow))
	{
		return false;
	}
	const float FormerScale = Scale->GetFloat();
	const float FormerSlow = Slow->GetFloat();
	TestEqual(TEXT("hol.Arrow.Scale defaults to 1"), FormerScale, 1.f);
	TestEqual(TEXT("hol.Arrow.SlowMotion defaults to 1"), FormerSlow, 1.f);

	Scale->Set(1.f, ECVF_SetByCode);
	const FVector Base = UHOLArrowSubsystem::GetArrowScale();
	TestTrue(TEXT("scale 1: the base scale 1.5 / 4 / 4, unchanged"), Base.Equals(FVector(1.5, 4.0, 4.0), 0.001));
	Scale->Set(3.f, ECVF_SetByCode);
	const FVector Tripled = UHOLArrowSubsystem::GetArrowScale();
	TestTrue(TEXT("scale 3: three times the base"), Tripled.Equals(Base * 3.0, 0.001));
	TestTrue(TEXT("scale 3: 4.5 / 12 / 12"), Tripled.Equals(FVector(4.5, 12.0, 12.0), 0.001));
	Scale->Set(1.f, ECVF_SetByCode);
	TestTrue(TEXT("scale back to 1: the base again"), UHOLArrowSubsystem::GetArrowScale().Equals(Base, 0.001));

	Slow->Set(1.f, ECVF_SetByCode);
	TestEqual(TEXT("slow motion 1: a 300 ms flight is drawn 0.3 s"), UHOLArrowSubsystem::GetFlightDurationSeconds(300), 0.3, 0.0001);
	Slow->Set(4.f, ECVF_SetByCode);
	TestEqual(TEXT("slow motion 4: a 300 ms flight is drawn 1.2 s"), UHOLArrowSubsystem::GetFlightDurationSeconds(300), 1.2, 0.0001);
	Slow->Set(FormerSlow, ECVF_SetByCode);
	Scale->Set(FormerScale, ECVF_SetByCode);
	return true;
}
#endif // WITH_DEV_AUTOMATION_TESTS
