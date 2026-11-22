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

// F8 T2 (FortificationsInProgress.md, D884): a FINISHED building whose row has BuildingAttack > 0 and BuildingFiresEmpty looses ONE first arrow at the
// nearest enemy within its reach (HOLCombat::IsInBuildingShotRange), never at an ally, never from a foundation. The REAL chain: the real combat
// processor stepped like the garrison tests, the arrow landing through the projectiles' own damage path (flat BuildingAttack in T2: the armour and the
// credit are T3), the effect read as the target's lost health and the arrows released. No cadence (T4), no fog (T5), no display (T7).
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
	};

	/**
	 * One fresh world: a building of TestShotTowerType of owner 0 in State, one Spearman of VictimOwner standing VictimGapCm beyond the building's
	 * body edge (on the X axis), holding its ground; ten seconds of combat steps. The result: the Spearman's lost health, the arrows released.
	 */
	static FShotResult RunScenario(FAutomationTestBase& Test, const EHOLConstructionState State, const int32 VictimOwner, const double VictimGapCm, const float VictimHealth = 0.f, const bool bDestroyAfterShot = false)
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
		if (Test.TestNotNull(TEXT("units"), Units) && Test.TestNotNull(TEXT("simulation"), Simulation) && Test.TestNotNull(TEXT("entities"), EntitySubsystem)
			&& Test.TestNotNull(TEXT("tower"), Tower))
		{
			Tower->Setup(TestShotTowerType, 0, 0, State);
			const TSharedRef<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
			UHOLCombatProcessor* Combat = NewObject<UHOLCombatProcessor>(Simulation);
			Combat->CallInitialize(Simulation, EntityManager);
			const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
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
			if (Test.TestNotNull(TEXT("the target's combat fragment"), VictimCombat))
			{
				VictimCombat->Mode = EHOLCombatMode::Hold;
				if (VictimHealth > 0.f)
				{
					// A frail target: the first arrow kills it (the kill goes through the arrow's landing, shooter absent).
					EntityManager->GetFragmentDataPtr<FHOLUnitStateFragment>(Units->FindEntity(Victim))->CurrentHealth = VictimHealth;
				}
				RunStep();
				const float Start = HealthOf(Victim);
				const int64 ReleasedBefore = UHOLCombatProcessor::GetTotalArrowsReleased();
				const int32 TenSeconds = FMath::CeilToInt(10.f / StepSeconds);
				for (int32 Index = 0; Index < TenSeconds; ++Index)
				{
					RunStep();
					if (bDestroyAfterShot && IsValid(Tower) && UHOLCombatProcessor::GetTotalArrowsReleased() > ReleasedBefore)
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
			}
		}
		GEngine->DestroyWorldContext(World);
		return Result;
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

	// A finished tower, an enemy 1000 cm beyond its edge (reach 3000): the one first arrow lands, the enemy loses health.
	const FShotResult Hit = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0);
	if (TestTrue(TEXT("the in-range scenario ran"), Hit.bRan))
	{
		TestEqual(TEXT("one arrow is released at the enemy within reach"), Hit.Released, static_cast<int64>(1));
		TestTrue(TEXT("the enemy within reach of the finished tower loses health"), Hit.HealthLost > 0.f);
		TestTrue(TEXT("the first arrow takes at most the row's BuildingAttack off"), Hit.HealthLost <= static_cast<float>(TowerRow.BuildingAttack) + 0.01f);
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

	// F8 T3: the arrow of a building has no shooter entity; a frail target dies of it (no crash, the one arrow lands), and a building that dies in flight changes nothing.
	const FShotResult Kill = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0, 1.f);
	TestEqual(TEXT("the frail enemy is shot once"), Kill.Released, static_cast<int64>(1));
	TestTrue(TEXT("the frail enemy dies of the arrow of a building without a shooter"), Kill.bVictimDown);
	const FShotResult Gone = RunScenario(*this, EHOLConstructionState::Finished, 3, 1000.0, 0.f, true);
	TestTrue(TEXT("the tower was destroyed in flight"), Gone.bTowerGone);
	TestTrue(TEXT("the arrow of a building destroyed in flight still hurts the enemy"), Gone.HealthLost > 0.f);

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
