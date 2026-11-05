// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAIBrain.h"
#include "AI/HOLAIOrders.h"
#include "AI/HOLAISummary.h"
#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Buildings/HOLProductionSubsystem.h"
#include "Commands/HOLCommandSubsystem.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HeroesOfLibertyGameModeBase.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLMatchSettings.h"
#include "Match/HOLPlayerState.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Resources/HOLSourceTypes.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// IA1.5: the opening of the script brain (Script/AI/HOLAIOpening.as, HOLAIBrain.as) on the real chain: C++ summary -> script
// rules -> IA1.4's order library (the first Angelscript calls of those orders) -> queue -> construction and production.
namespace UE::HeroesOfLiberty::Tests::AIOpening
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/**
	 * A match's server: slot 0 human (its centre far away), slot 1 AI (real PlayerState, starting stock, population 5 of 10)
	 * with its finished centre at the origin, berries and a tree near it and 5 idle Peasants. Construction and production
	 * subscribe to the step as at the match start; the units' movement and gathering run no processor (the orders stay in
	 * the command queue, read by the test).
	 */
	struct FWorld
	{
		UWorld* World = nullptr;
		AHOLGameState* State = nullptr;
		AHOLPlayerState* AIState = nullptr;
		AHOLBuilding* Centre = nullptr;
		UHOLCommandSubsystem* Commands = nullptr;
		UHOLAIBrainSubsystem* Brain = nullptr;
		int32 Berries = INDEX_NONE;
		int32 Tree = INDEX_NONE;
		TArray<uint32> Peasants;

		FWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			State = World->SpawnActor<AHOLGameState>();
			Centre = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
			AHOLBuilding* EnemyCentre = World->SpawnActor<AHOLBuilding>(FVector(-20000.0, 0.0, 0.0), FRotator::ZeroRotator);
			UHOLConstructionSubsystem* Construction = World->GetSubsystem<UHOLConstructionSubsystem>();
			UHOLProductionSubsystem* Production = World->GetSubsystem<UHOLProductionSubsystem>();
			UHOLSourceSubsystem* Sources = World->GetSubsystem<UHOLSourceSubsystem>();
			UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
			Commands = World->GetSubsystem<UHOLCommandSubsystem>();
			Brain = UHOLAIBrainSubsystem::Find(World);
			if (!State || !Centre || !EnemyCentre || !Construction || !Production || !Sources || !Units || !Commands || !Brain
				|| !World->GetSubsystem<UHOLSimulationSubsystem>())
			{
				return;
			}
			FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
			for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
			{
				Config.Slots[Index].State = Index == 0 ? EHOLSlotState::Human : (Index == 1 ? EHOLSlotState::AI : EHOLSlotState::Closed);
			}
			State->InitVictory(Config);
			AHeroesOfLibertyGameModeBase::SpawnAIPlayerStates(*World, Config, GetDefault<UHOLMatchSettings>()->StartingResources);
			for (APlayerState* Each : State->PlayerArray)
			{
				AHOLPlayerState* HOLEach = Cast<AHOLPlayerState>(Each);
				if (HOLEach && HOLEach->GetPlayerIndex() == 1)
				{
					AIState = HOLEach;
				}
			}
			Centre->Setup(HOLBuildingTypeNames::MainCentre, 1, 1);
			EnemyCentre->Setup(HOLBuildingTypeNames::MainCentre, 0, 0);
			Sources->OnWorldBeginPlay(*World);
			Berries = Sources->AddSource(HOLSourceTypeNames::BerryBush, FVector(0.0, 1800.0, 0.0));
			Tree = Sources->AddSource(HOLSourceTypeNames::Tree, FVector(2000.0, 0.0, 0.0));
			Construction->OnWorldBeginPlay(*World);
			Production->OnWorldBeginPlay(*World);

			TArray<uint32> Before;
			Units->GetAllUnitIds(Before);
			FHOLUnitSpawnRequest Request;
			Request.TypeName = FName(TEXT("Peasant"));
			Request.Count = 5;
			Request.OwnerPlayer = 1;
			Request.Center = FVector(1000.0, 1000.0, 0.0);
			Units->RequestUnits(Request);
			Units->ServeBirthRequests();
			Units->GetAllUnitIds(Peasants);
			Peasants.RemoveAll([&Before](const uint32 Id) { return Before.Contains(Id); });
			Peasants.Sort();
			if (AIState)
			{
				AIState->SetPopulation(5, 10);
			}
		}

		~FWorld()
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		bool IsReady() const { return AIState && Brain && Peasants.Num() == 5 && Berries != INDEX_NONE && Tree != INDEX_NONE; }

		/** One step start: construction and production apply their orders. */
		void RunStep() { World->GetSubsystem<UHOLSimulationSubsystem>()->GetOnStepStarted().Broadcast(1); }

		int32 CountBuildings(const FName Type, const int32 Owner) const
		{
			int32 Count = 0;
			for (TActorIterator<AHOLBuilding> It(World); It; ++It)
			{
				Count += (It->GetTypeName() == Type && It->GetOwnerPlayer() == Owner) ? 1 : 0;
			}
			return Count;
		}

		/** Gives slot 1 more of the wood (the camps and the mill cost 100). */
		void AddWood(const int32 Wood)
		{
			FHOLResources Stock = AIState->GetResources();
			Stock.Wood += Wood;
			AIState->SetResources(Stock);
		}
	};
}

/**
 * IA1.5, the first thought: the 5 starting Peasants spread 3 on the berries and 2 on the tree (two gather orders), the centre
 * gets a Peasant in its queue (paid) and its rally on the berries, and no house yet (5 places left). Red while the brain only
 * sends all its idle Peasants to one source.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIOpeningFirstThoughtTest, "HeroesOfLiberty.AI.Opening.FirstThought", UE::HeroesOfLiberty::Tests::AIOpening::Flags)
bool FHOLAIOpeningFirstThoughtTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIOpening;
	FWorld Test;
	if (!TestTrue(TEXT("test world ready (a script brain, slot 1's state and Peasants)"), Test.IsReady()))
	{
		return false;
	}
	const FHOLResources Before = Test.AIState->GetResources();
	TestTrue(TEXT("slot 1 thinks"), Test.Brain->ThinkNow(1));

	// IA1.7: the lumber camp is the opening's first building, built at once by the first idle Peasant; the four others spread.
	TArray<FHOLGatherOrder> Orders;
	Test.Commands->ConsumeGatherOrders(Orders);
	if (TestEqual(TEXT("two gather orders (food, wood)"), Orders.Num(), 2))
	{
		TestEqual(TEXT("first: the berries"), Orders[0].SourceIndex, Test.Berries);
		TestEqual(TEXT("first: two Peasants"), Orders[0].UnitIds.Num(), 2);
		TestEqual(TEXT("second: the tree"), Orders[1].SourceIndex, Test.Tree);
		TestEqual(TEXT("second: two Peasants"), Orders[1].UnitIds.Num(), 2);
		TestEqual(TEXT("given as slot 1"), Orders[0].PlayerIndex, 1);
		TestFalse(TEXT("the builder is not among the gatherers"), Orders[0].UnitIds.Contains(Test.Peasants[0]) || Orders[1].UnitIds.Contains(Test.Peasants[0]));
	}

	Test.RunStep();
	TestEqual(TEXT("next step: a Peasant in the centre's queue"), Test.Centre->GetProductionQueue().Num(), 1);
	TestTrue(TEXT("paid: the food went down"), Test.AIState->GetResources().Food < Before.Food);
	TestEqual(TEXT("its rally on the berries"), Test.Centre->GetRallySource(), Test.Berries);
	TestEqual(TEXT("no house yet (5 places left)"), Test.CountBuildings(HOLBuildingTypeNames::House, 1), 0);
	TestEqual(TEXT("the lumber camp: a foundation of slot 1"), Test.CountBuildings(HOLBuildingTypeNames::LumberCamp, 1), 1);
	return true;
}

/**
 * IA1.5, the buildings: with few places left the next thought places a House (paid 50 wood, a Peasant sent to build it);
 * then a LumberCamp beside the tree (100 wood), then a Mill beside the berries (100 wood, once the stock allows); then
 * nothing more. Spots by the real placement rules (FindAIPlacement). Red while the brain places nothing.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIOpeningBuildingsTest, "HeroesOfLiberty.AI.Opening.Buildings", UE::HeroesOfLiberty::Tests::AIOpening::Flags)
bool FHOLAIOpeningBuildingsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIOpening;
	FWorld Test;
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	Test.AIState->SetPopulation(9, 10);

	// The house: 1 place left.
	const int32 WoodBefore = Test.AIState->GetResources().Wood;
	TestTrue(TEXT("thought 1"), Test.Brain->ThinkNow(1));
	Test.RunStep();
	TestEqual(TEXT("a House foundation of slot 1"), Test.CountBuildings(HOLBuildingTypeNames::House, 1), 1);
	TestEqual(TEXT("paid 50 wood"), Test.AIState->GetResources().Wood, WoodBefore - 50);
	TestEqual(TEXT("no camp yet"), Test.CountBuildings(HOLBuildingTypeNames::LumberCamp, 1), 0);

	// The camp, beside the tree.
	TestTrue(TEXT("thought 2"), Test.Brain->ThinkNow(1));
	Test.RunStep();
	TestEqual(TEXT("a LumberCamp foundation of slot 1"), Test.CountBuildings(HOLBuildingTypeNames::LumberCamp, 1), 1);
	for (TActorIterator<AHOLBuilding> It(Test.World); It; ++It)
	{
		if (It->GetTypeName() == HOLBuildingTypeNames::LumberCamp)
		{
			TestTrue(TEXT("the camp stands nearer the tree than the centre does"),
				FVector::Dist2D(It->GetActorLocation(), FVector(2000.0, 0.0, 0.0)) < 2000.0);
		}
	}

	// The mill waits for wood (200 - 50 - 100 = 50 left), then goes beside the berries.
	TestTrue(TEXT("thought 3 without the wood"), Test.Brain->ThinkNow(1));
	Test.RunStep();
	TestEqual(TEXT("no mill without 100 wood"), Test.CountBuildings(HOLBuildingTypeNames::Mill, 1), 0);
	Test.AddWood(100);
	TestTrue(TEXT("thought 4"), Test.Brain->ThinkNow(1));
	Test.RunStep();
	TestEqual(TEXT("a Mill foundation of slot 1"), Test.CountBuildings(HOLBuildingTypeNames::Mill, 1), 1);

	// Done: nothing more.
	Test.AddWood(300);
	TestTrue(TEXT("thought 5"), Test.Brain->ThinkNow(1));
	Test.RunStep();
	TestEqual(TEXT("one House still"), Test.CountBuildings(HOLBuildingTypeNames::House, 1), 1);
	TestEqual(TEXT("one LumberCamp still"), Test.CountBuildings(HOLBuildingTypeNames::LumberCamp, 1), 1);
	TestEqual(TEXT("one Mill still"), Test.CountBuildings(HOLBuildingTypeNames::Mill, 1), 1);
	return true;
}

/**
 * IA1.7, the houses come again: a first house finished and the places down to 2 of 20, the next thought places a second
 * House (a Peasant sent, 50 wood paid); with that house under construction (its 10 places coming) the thoughts after it
 * place no third. Red while the opening places one house only.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIBuildingsHousesTest, "HeroesOfLiberty.AI.Buildings.Houses", UE::HeroesOfLiberty::Tests::AIOpening::Flags)
bool FHOLAIBuildingsHousesTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIOpening;
	FWorld Test;
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	// The camp and the mill stand already (the opening is done), a first house is finished: 18 of 20 places taken.
	int32 Slot = 0;
	for (const FName Type : { HOLBuildingTypeNames::LumberCamp, HOLBuildingTypeNames::Mill, HOLBuildingTypeNames::House })
	{
		AHOLBuilding* Building = Test.World->SpawnActor<AHOLBuilding>(FVector(-6000.0 - 1500.0 * Slot++, 3000.0, 0.0), FRotator::ZeroRotator);
		if (!TestNotNull(TEXT("a finished building"), Building))
		{
			return false;
		}
		Building->Setup(Type, 1, 1);
	}
	Test.AIState->SetPopulation(19, 20);
	const FHOLResources Before = Test.AIState->GetResources();
	TestTrue(TEXT("thought 1"), Test.Brain->ThinkNow(1));
	Test.RunStep();
	TestEqual(TEXT("a second House of slot 1"), Test.CountBuildings(HOLBuildingTypeNames::House, 1), 2);
	TestEqual(TEXT("paid 50 wood"), Test.AIState->GetResources().Wood, Before.Wood - 50);

	// Its 10 places are coming: no third while it stands unfinished, however low the room.
	Test.AIState->SetPopulation(19, 20);
	TestTrue(TEXT("thought 2"), Test.Brain->ThinkNow(1));
	Test.RunStep();
	TestEqual(TEXT("still two Houses"), Test.CountBuildings(HOLBuildingTypeNames::House, 1), 2);
	return true;
}

/**
 * IA1.7, the farm block: the berries nearly used up (10 food left), the camp and the mill standing, wood in plenty: the
 * thoughts plan the block (FindAIFarmBlock around the centre), then place its mill and its farms one after the other, three
 * sites under way at most. Red while the opening stops at the mill.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIBuildingsFarmBlockTest, "HeroesOfLiberty.AI.Buildings.FarmBlock", UE::HeroesOfLiberty::Tests::AIOpening::Flags)
bool FHOLAIBuildingsFarmBlockTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIOpening;
	FWorld Test;
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	int32 Slot = 0;
	for (const FName Type : { HOLBuildingTypeNames::LumberCamp, HOLBuildingTypeNames::Mill })
	{
		AHOLBuilding* Building = Test.World->SpawnActor<AHOLBuilding>(FVector(-6000.0 - 1500.0 * Slot++, 3000.0, 0.0), FRotator::ZeroRotator);
		if (!TestNotNull(TEXT("a finished building"), Building))
		{
			return false;
		}
		Building->Setup(Type, 1, 1);
	}
	Test.AddWood(1000);
	Test.World->GetSubsystem<UHOLSourceSubsystem>()->Take(Test.Berries, 190);

	for (int32 Thought = 1; Thought <= 3; ++Thought)
	{
		TestTrue(*FString::Printf(TEXT("thought %d"), Thought), Test.Brain->ThinkNow(1));
		Test.RunStep();
	}
	TestEqual(TEXT("the block's mill (the second mill of slot 1)"), Test.CountBuildings(HOLBuildingTypeNames::Mill, 1), 2);
	TestEqual(TEXT("two farms after the mill"), Test.CountBuildings(HOLBuildingTypeNames::Farm, 1), 2);
	FVector BlockMill = FVector::ZeroVector;
	for (TActorIterator<AHOLBuilding> It(Test.World); It; ++It)
	{
		if (It->GetTypeName() == HOLBuildingTypeNames::Mill && It->GetActorLocation().X > -5000.0)
		{
			BlockMill = It->GetActorLocation();
		}
	}
	for (TActorIterator<AHOLBuilding> It(Test.World); It; ++It)
	{
		if (It->GetTypeName() == HOLBuildingTypeNames::Farm)
		{
			TestTrue(TEXT("a farm of the block: within 3 tiles of its mill"), FVector::Dist2D(It->GetActorLocation(), BlockMill) < 1300.0);
		}
	}

	// Three sites under way (the mill and two farms): the fourth thought places nothing.
	TestTrue(TEXT("thought 4"), Test.Brain->ThinkNow(1));
	Test.RunStep();
	TestEqual(TEXT("still two farms: three sites under way"), Test.CountBuildings(HOLBuildingTypeNames::Farm, 1), 2);
	return true;
}

/**
 * IA2.4, the army's production on the real chain: 20 Peasants (the economy stands), a Barracks and an Archery Range finished, the
 * stock in plenty. The thought queues one unit per building, of a type that building makes (the composition restricted to the
 * building: a Spearman at the Barracks, an Archer at the Range, the mix's first), sets both buildings' rally on a point 1500 cm
 * from the centre toward the enemy centre (a point, not a source), and the next thought queues a second unit per building but
 * not a third. Red while the brain queues no soldier and sets no military rally.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIArmyProductionTest, "HeroesOfLiberty.AI.Army.Production", UE::HeroesOfLiberty::Tests::AIOpening::Flags)
bool FHOLAIArmyProductionTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIOpening;
	FWorld Test;
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	UHOLUnitSubsystem* Units = Test.World->GetSubsystem<UHOLUnitSubsystem>();
	FHOLUnitSpawnRequest More;
	More.TypeName = FName(TEXT("Peasant"));
	More.Count = 15;
	More.OwnerPlayer = 1;
	More.Center = FVector(1000.0, -1000.0, 0.0);
	Units->RequestUnits(More);
	Units->ServeBirthRequests();
	AHOLBuilding* Barracks = Test.World->SpawnActor<AHOLBuilding>(FVector(-3000.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* Range = Test.World->SpawnActor<AHOLBuilding>(FVector(-3000.0, 3000.0, 0.0), FRotator::ZeroRotator);
	if (!TestNotNull(TEXT("the Barracks"), Barracks) || !TestNotNull(TEXT("the Archery Range"), Range))
	{
		return false;
	}
	Barracks->Setup(HOLBuildingTypeNames::Barracks, 1, 1);
	Range->Setup(HOLBuildingTypeNames::ArcheryRange, 1, 1);
	Test.AIState->SetPopulation(25, 100);
	Test.AIState->SetResources(FHOLResources(1000, 1000, 500, 500));

	TestTrue(TEXT("thought 1"), Test.Brain->ThinkNow(1));
	Test.RunStep();
	if (TestEqual(TEXT("one unit in the Barracks' queue"), Barracks->GetProductionQueue().Num(), 1))
	{
		TestEqual(TEXT("a Spearman (the mix's first, a unit of the Barracks)"), Barracks->GetProductionQueue()[0].TypeName, FName(TEXT("Spearman")));
	}
	if (TestEqual(TEXT("one unit in the Range's queue"), Range->GetProductionQueue().Num(), 1))
	{
		TestEqual(TEXT("an Archer (a unit of the Range)"), Range->GetProductionQueue()[0].TypeName, FName(TEXT("Archer")));
	}

	// The rally: a point 1500 cm from the centre toward the enemy centre (at -X), no source.
	for (const AHOLBuilding* Each : { Barracks, Range })
	{
		if (TestTrue(TEXT("a rally point"), Each->HasRallyPoint()))
		{
			TestTrue(TEXT("1500 cm toward the enemy centre"), FVector::Dist2D(Each->GetRallyPoint(), FVector(-1500.0, 0.0, 0.0)) < 100.0);
			TestEqual(TEXT("a point, not a source"), Each->GetRallySource(), static_cast<int32>(INDEX_NONE));
		}
	}

	// Two at most per building: the second thought adds one, the third none. (The production step recounts the population in this world: set again.)
	Test.AIState->SetPopulation(26, 100);
	TestTrue(TEXT("thought 2"), Test.Brain->ThinkNow(1));
	Test.RunStep();
	TestEqual(TEXT("two in the Barracks' queue"), Barracks->GetProductionQueue().Num(), 2);
	Test.AIState->SetPopulation(28, 100);
	TestTrue(TEXT("thought 3"), Test.Brain->ThinkNow(1));
	Test.RunStep();
	TestEqual(TEXT("still two in the Barracks' queue"), Barracks->GetProductionQueue().Num(), 2);
	TestEqual(TEXT("still two in the Range's queue"), Range->GetProductionQueue().Num(), 2);
	return true;
}

namespace UE::HeroesOfLiberty::Tests::AIOpening
{
	/** Units of Type born for Owner around Center (the world has no movement: they stand idle); their identifiers. */
	TArray<uint32> PlaceSoldiers(FWorld& Test, const FName Type, const int32 Owner, const FVector& Center, const int32 Count)
	{
		UHOLUnitSubsystem* Units = Test.World->GetSubsystem<UHOLUnitSubsystem>();
		TArray<uint32> Before;
		Units->GetAllUnitIds(Before);
		FHOLUnitSpawnRequest Request;
		Request.TypeName = Type;
		Request.Count = Count;
		Request.OwnerPlayer = static_cast<uint8>(Owner);
		Request.Center = Center;
		Units->RequestUnits(Request);
		Units->ServeBirthRequests();
		TArray<uint32> After;
		Units->GetAllUnitIds(After);
		After.RemoveAll([&Before](const uint32 Id) { return Before.Contains(Id); });
		After.Sort();
		return After;
	}

	/** The attack-move orders of slot 1 waiting in the queue (the thought's other orders are not moves). */
	TArray<FHOLMoveOrder> TakeAttackMoves(FWorld& Test)
	{
		TArray<FHOLMoveOrder> All;
		Test.Commands->ConsumeMoveOrders(All);
		TArray<FHOLMoveOrder> Found;
		for (const FHOLMoveOrder& Order : All)
		{
			if (Order.Kind == EHOLOrderKind::AttackMove && Order.PlayerIndex == 1)
			{
				Found.Add(Order);
			}
		}
		return Found;
	}

	int32 CountOrdered(const TArray<FHOLMoveOrder>& Orders, const TArray<uint32>& Ids)
	{
		int32 Count = 0;
		for (const uint32 Id : Ids)
		{
			for (const FHOLMoveOrder& Order : Orders)
			{
				if (Order.UnitIds.Contains(Id))
				{
					++Count;
					break;
				}
			}
		}
		return Count;
	}
}

/**
 * IA2.5, regrouping and base defence on the real chain: three Spearmen of slot 1 stand idle 4000 cm from the assembly point (1500 cm
 * from the centre toward the enemy centre, at -X) and no enemy is about: the thought sends them there on an attack-move (a
 * fight on the way). Then an enemy Spearman of slot 0 appears 2500 cm from the centre: the next thought sends all three on an
 * attack-move toward it (not to the point). Red while the brain sends no soldier anywhere.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIArmyDefenceTest, "HeroesOfLiberty.AI.Army.Defence", UE::HeroesOfLiberty::Tests::AIOpening::Flags)
bool FHOLAIArmyDefenceTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIOpening;
	FWorld Test;
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	const TArray<uint32> Soldiers = PlaceSoldiers(Test, FName(TEXT("Spearman")), 1, FVector(2000.0, 2500.0, 0.0), 3);
	if (!TestEqual(TEXT("three Spearmen of slot 1"), Soldiers.Num(), 3))
	{
		return false;
	}
	TArray<FHOLMoveOrder> Discard;
	Test.Commands->ConsumeMoveOrders(Discard);

	// 1. Regroup: no enemy, the soldiers far from the point.
	TestTrue(TEXT("thought 1"), Test.Brain->ThinkNow(1));
	TArray<FHOLMoveOrder> Regroup = TakeAttackMoves(Test);
	TestEqual(TEXT("regroup: the three Spearmen are ordered"), CountOrdered(Regroup, Soldiers), 3);
	for (const FHOLMoveOrder& Order : Regroup)
	{
		TestTrue(TEXT("regroup: toward the assembly point, 1500 cm from the centre toward the enemy centre"), FVector::Dist2D(Order.Target, FVector(-1500.0, 0.0, 0.0)) < 300.0);
	}

	// 2. Defence: an enemy Spearman 2500 cm east of the centre (the House of the test world is not needed: the centre is a building).
	const TArray<uint32> Enemy = PlaceSoldiers(Test, FName(TEXT("Spearman")), 0, FVector(2500.0, 0.0, 0.0), 1);
	TestEqual(TEXT("an enemy Spearman"), Enemy.Num(), 1);
	TestTrue(TEXT("thought 2"), Test.Brain->ThinkNow(1));
	TArray<FHOLMoveOrder> Defence = TakeAttackMoves(Test);
	TestEqual(TEXT("defence: the three Spearmen are ordered"), CountOrdered(Defence, Soldiers), 3);
	for (const FHOLMoveOrder& Order : Defence)
	{
		TestTrue(TEXT("defence: toward the enemy, not the assembly point"), FVector::Dist2D(Order.Target, FVector(2500.0, 0.0, 0.0)) < 400.0);
	}
	return true;
}

/**
 * IA2.5, no suicide: one Spearman of slot 1 at the assembly point against six Men-at-arms of slot 0 within 2500 cm of the centre (far
 * from the point, power ratio under 0.3): the thought sends it nowhere.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIArmyDefenceWeakTest, "HeroesOfLiberty.AI.Army.DefenceWeak", UE::HeroesOfLiberty::Tests::AIOpening::Flags)
bool FHOLAIArmyDefenceWeakTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIOpening;
	FWorld Test;
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	const TArray<uint32> Soldiers = PlaceSoldiers(Test, FName(TEXT("Spearman")), 1, FVector(-1500.0, 0.0, 0.0), 1);
	const TArray<uint32> Enemy = PlaceSoldiers(Test, FName(TEXT("ManAtArms")), 0, FVector(2500.0, 0.0, 0.0), 6);
	if (!TestTrue(TEXT("one Spearman of slot 1, six Men-at-arms of slot 0"), Soldiers.Num() == 1 && Enemy.Num() == 6))
	{
		return false;
	}
	TArray<FHOLMoveOrder> Discard;
	Test.Commands->ConsumeMoveOrders(Discard);
	TestTrue(TEXT("thought"), Test.Brain->ThinkNow(1));
	TestEqual(TEXT("the weak army is sent against nobody"), TakeAttackMoves(Test).Num(), 0);
	return true;
}

/**
 * IA2.6, the attack wave on the real chain: nine Spearmen of slot 1 stand idle at the assembly point (1500 cm from the centre toward
 * the enemy centre, at -X), the game time is past 450 s (the step index moved on), the enemy centre is known (at -20000) and one
 * enemy Spearman is seen far away: the thought sends all nine on an attack-move toward the enemy centre. A second world with
 * twelve Men-at-arms seen: the army is far behind the 1.5 margin and nobody is sent.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIArmyAttackTest, "HeroesOfLiberty.AI.Army.Attack", UE::HeroesOfLiberty::Tests::AIOpening::Flags)
bool FHOLAIArmyAttackTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIOpening;
	const FVector EnemyCentre(-20000.0, 0.0, 0.0);
	{
		FWorld Test;
		if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
		{
			return false;
		}
		const TArray<uint32> Soldiers = PlaceSoldiers(Test, FName(TEXT("Spearman")), 1, FVector(-1500.0, 0.0, 0.0), 9);
		const TArray<uint32> Enemy = PlaceSoldiers(Test, FName(TEXT("Spearman")), 0, FVector(-18000.0, 0.0, 0.0), 1);
		if (!TestTrue(TEXT("nine Spearmen of slot 1, one of slot 0"), Soldiers.Num() == 9 && Enemy.Num() == 1))
		{
			return false;
		}
		UHOLSimulationSubsystem* Simulation = Test.World->GetSubsystem<UHOLSimulationSubsystem>();
		for (int32 Count = 0; Count < 9100; ++Count)
		{
			Simulation->AdvanceStepIndexForTests();
		}
		TArray<FHOLMoveOrder> Discard;
		Test.Commands->ConsumeMoveOrders(Discard);
		TestTrue(TEXT("thought"), Test.Brain->ThinkNow(1));
		const TArray<FHOLMoveOrder> Orders = TakeAttackMoves(Test);
		TestEqual(TEXT("wave: the nine Spearmen are ordered"), CountOrdered(Orders, Soldiers), 9);
		for (const FHOLMoveOrder& Order : Orders)
		{
			TestTrue(TEXT("wave: toward the enemy centre"), FVector::Dist2D(Order.Target, EnemyCentre) < 500.0);
		}
	}
	{
		FWorld Test;
		if (!TestTrue(TEXT("second test world ready"), Test.IsReady()))
		{
			return false;
		}
		const TArray<uint32> Soldiers = PlaceSoldiers(Test, FName(TEXT("Spearman")), 1, FVector(-1500.0, 0.0, 0.0), 9);
		const TArray<uint32> Enemy = PlaceSoldiers(Test, FName(TEXT("ManAtArms")), 0, FVector(-18000.0, 0.0, 0.0), 12);
		if (!TestTrue(TEXT("nine Spearmen of slot 1, twelve Men-at-arms of slot 0"), Soldiers.Num() == 9 && Enemy.Num() == 12))
		{
			return false;
		}
		UHOLSimulationSubsystem* Simulation = Test.World->GetSubsystem<UHOLSimulationSubsystem>();
		for (int32 Count = 0; Count < 9100; ++Count)
		{
			Simulation->AdvanceStepIndexForTests();
		}
		TArray<FHOLMoveOrder> Discard;
		Test.Commands->ConsumeMoveOrders(Discard);
		TestTrue(TEXT("thought"), Test.Brain->ThinkNow(1));
		TestEqual(TEXT("a strong enemy army seen: no wave"), TakeAttackMoves(Test).Num(), 0);
	}
	return true;
}

/**
 * D717 (T2), a held seat launches no wave, on the real chain: the setting of HeroesOfLiberty.AI.Army.Attack (nine Spearmen at the
 * assembly point, past 450 s, one enemy Spearman seen far away). Slot 1 not held: the wave goes (the control, so that the test
 * cannot pass for nothing); slot 1 held by SetHeld (what the disconnect of its player does): the same thought sends nobody, the
 * soldiers stay home. Held then released: the wave goes again.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIHoldNoWaveTest, "HeroesOfLiberty.AI.Hold.NoWave", UE::HeroesOfLiberty::Tests::AIOpening::Flags)
bool FHOLAIHoldNoWaveTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIOpening;
	for (const int32 Case : { 0, 1, 2 })
	{
		FWorld Test;
		if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
		{
			return false;
		}
		const TArray<uint32> Soldiers = PlaceSoldiers(Test, FName(TEXT("Spearman")), 1, FVector(-1500.0, 0.0, 0.0), 9);
		const TArray<uint32> Enemy = PlaceSoldiers(Test, FName(TEXT("Spearman")), 0, FVector(-18000.0, 0.0, 0.0), 1);
		if (!TestTrue(TEXT("nine Spearmen of slot 1, one of slot 0"), Soldiers.Num() == 9 && Enemy.Num() == 1))
		{
			return false;
		}
		UHOLSimulationSubsystem* Simulation = Test.World->GetSubsystem<UHOLSimulationSubsystem>();
		for (int32 Count = 0; Count < 9100; ++Count)
		{
			Simulation->AdvanceStepIndexForTests();
		}
		// Case 0: not held (control). Case 1: held. Case 2: held then released.
		if (Case >= 1)
		{
			Test.Brain->SetHeld(1, true);
			TestTrue(TEXT("the seat is held"), Test.Brain->IsHeld(1));
		}
		if (Case == 2)
		{
			Test.Brain->SetHeld(1, false);
			TestFalse(TEXT("the seat is released"), Test.Brain->IsHeld(1));
		}
		TArray<FHOLMoveOrder> Discard;
		Test.Commands->ConsumeMoveOrders(Discard);
		TestTrue(TEXT("thought"), Test.Brain->ThinkNow(1));
		const TArray<FHOLMoveOrder> Orders = TakeAttackMoves(Test);
		if (Case == 1)
		{
			TestEqual(TEXT("held: no soldier is sent to the enemy"), CountOrdered(Orders, Soldiers), 0);
		}
		else
		{
			TestEqual(*FString::Printf(TEXT("case %d, not held: the nine Spearmen are sent"), Case), CountOrdered(Orders, Soldiers), 9);
		}
	}
	return true;
}

/**
 * IA1.6, the figures: the brain's costs come from the type tables (UHOLAIOrderLibrary::ReadAIUnitFigures / ReadAIBuildingFigures),
 * not from numbers copied into the script. The Peasant and the House as the tables have them; an unknown type is refused.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAINeedsFiguresTest, "HeroesOfLiberty.AI.Needs.Figures", UE::HeroesOfLiberty::Tests::AIOpening::Flags)
bool FHOLAINeedsFiguresTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIOpening;
	FWorld Test;
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	const FHOLUnitTypeRow* PeasantRow = Test.World->GetSubsystem<UHOLUnitSubsystem>()->FindTypeRow(FName(TEXT("Peasant")));
	const FHOLBuildingTypeRow* HouseRow = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::House);
	if (!TestNotNull(TEXT("the Peasant row"), PeasantRow) || !TestNotNull(TEXT("the House row"), HouseRow))
	{
		return false;
	}
	FHOLAITypeFigures Peasant;
	TestTrue(TEXT("the Peasant is known"), UHOLAIOrderLibrary::ReadAIUnitFigures(Test.World, FName(TEXT("Peasant")), Peasant));
	TestEqual(TEXT("Peasant food"), Peasant.CostFood, PeasantRow->Cost.Food);
	TestTrue(TEXT("a Peasant costs food"), Peasant.CostFood > 0);
	TestEqual(TEXT("Peasant build time"), Peasant.BuildTime, PeasantRow->BuildTime);
	TestEqual(TEXT("Peasant population"), Peasant.Population, PeasantRow->Population);
	TestEqual(TEXT("Peasant speed"), Peasant.Speed, PeasantRow->Speed);
	TestEqual(TEXT("Peasant load"), Peasant.CarryCapacity, PeasantRow->CarryCapacity);
	TestEqual(TEXT("Peasant wood rate"), Peasant.GatherWood, PeasantRow->GatherRates.Wood);
	TestTrue(TEXT("a Peasant gathers"), Peasant.GatherFood > 0.f);
	FHOLAITypeFigures House;
	TestTrue(TEXT("the House is known"), UHOLAIOrderLibrary::ReadAIBuildingFigures(HOLBuildingTypeNames::House, House));
	TestEqual(TEXT("House wood"), House.CostWood, HouseRow->Cost.Wood);
	TestEqual(TEXT("House places"), House.Population, HouseRow->PopulationProvided);
	TestTrue(TEXT("a House gives places"), House.Population > 0);
	FHOLAITypeFigures Nothing;
	// The table lookup logs an error for a name it does not know (the order library's other lookups do as well).
	AddExpectedError(TEXT("No building type NoSuchBuilding"), EAutomationExpectedErrorFlags::Contains, 1);
	TestFalse(TEXT("unknown unit"), UHOLAIOrderLibrary::ReadAIUnitFigures(Test.World, FName(TEXT("NoSuchUnit")), Nothing));
	TestFalse(TEXT("unknown building"), UHOLAIOrderLibrary::ReadAIBuildingFigures(FName(TEXT("NoSuchBuilding")), Nothing));
	return true;
}

/**
 * IA1.6, the needs on the real chain: the starting stocks give the opening's 3 food for 2 wood, but with the wood gone and the
 * food in plenty the same five Peasants go 1 to the berries, 4 to the tree. Red while the split follows the fixed weights.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAINeedsSpreadTest, "HeroesOfLiberty.AI.Needs.Spread", UE::HeroesOfLiberty::Tests::AIOpening::Flags)
bool FHOLAINeedsSpreadTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIOpening;
	FWorld Test;
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	Test.AIState->SetResources(FHOLResources(1000, 0, 0, 0));
	TestTrue(TEXT("slot 1 thinks"), Test.Brain->ThinkNow(1));
	TArray<FHOLGatherOrder> Orders;
	Test.Commands->ConsumeGatherOrders(Orders);
	if (TestEqual(TEXT("two gather orders"), Orders.Num(), 2))
	{
		TestEqual(TEXT("first: the berries"), Orders[0].SourceIndex, Test.Berries);
		TestEqual(TEXT("one on the berries"), Orders[0].UnitIds.Num(), 1);
		TestEqual(TEXT("second: the tree"), Orders[1].SourceIndex, Test.Tree);
		TestEqual(TEXT("four on the tree"), Orders[1].UnitIds.Num(), 4);
	}
	Test.RunStep();
	TestEqual(TEXT("the rally of the new Peasants: the tree, the wood being short"), Test.Centre->GetRallySource(), Test.Tree);
	return true;
}

/**
 * IA1.6, a source used up: its Peasants stand idle (the summary leaves it out) and the next thought sends them to what is
 * left, all of them, the berries' Peasants included.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAINeedsUsedUpTest, "HeroesOfLiberty.AI.Needs.UsedUpSource", UE::HeroesOfLiberty::Tests::AIOpening::Flags)
bool FHOLAINeedsUsedUpTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIOpening;
	FWorld Test;
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	TestTrue(TEXT("thought 1"), Test.Brain->ThinkNow(1));
	TArray<FHOLGatherOrder> Orders;
	Test.Commands->ConsumeGatherOrders(Orders);
	TestEqual(TEXT("the start: two gather orders"), Orders.Num(), 2);

	UHOLSourceSubsystem* Sources = Test.World->GetSubsystem<UHOLSourceSubsystem>();
	Sources->Take(Test.Berries, 100000);
	Orders.Reset();
	TestTrue(TEXT("thought 2"), Test.Brain->ThinkNow(1));
	Test.Commands->ConsumeGatherOrders(Orders);
	if (TestEqual(TEXT("one gather order: the tree"), Orders.Num(), 1))
	{
		TestEqual(TEXT("to the tree"), Orders[0].SourceIndex, Test.Tree);
		TestEqual(TEXT("all four (the fifth builds the camp)"), Orders[0].UnitIds.Num(), 4);
	}
	return true;
}

#endif
