// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAIBrain.h"
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

	TArray<FHOLGatherOrder> Orders;
	Test.Commands->ConsumeGatherOrders(Orders);
	if (TestEqual(TEXT("two gather orders (food, wood)"), Orders.Num(), 2))
	{
		TestEqual(TEXT("first: the berries"), Orders[0].SourceIndex, Test.Berries);
		TestEqual(TEXT("first: three Peasants"), Orders[0].UnitIds.Num(), 3);
		TestEqual(TEXT("second: the tree"), Orders[1].SourceIndex, Test.Tree);
		TestEqual(TEXT("second: two Peasants"), Orders[1].UnitIds.Num(), 2);
		TestEqual(TEXT("given as slot 1"), Orders[0].PlayerIndex, 1);
	}

	Test.RunStep();
	TestEqual(TEXT("next step: a Peasant in the centre's queue"), Test.Centre->GetProductionQueue().Num(), 1);
	TestTrue(TEXT("paid: the food went down"), Test.AIState->GetResources().Food < Before.Food);
	TestEqual(TEXT("its rally on the berries"), Test.Centre->GetRallySource(), Test.Berries);
	TestEqual(TEXT("no house yet (5 places left)"), Test.CountBuildings(HOLBuildingTypeNames::House, 1), 0);
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
	Test.AIState->SetPopulation(8, 10);

	// The house: 2 places left.
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

#endif
