// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAIBrain.h"
#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Commands/HOLUnitOrderProcessor.h"
#include "Dev/HOLTestHooks.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HeroesOfLibertyGameModeBase.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLMatchSettings.h"
#include "Match/HOLPlayerState.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Resources/HOLSourceTypes.h"
#include "Simulation/HOLGatherProcessor.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitView.h"

#if WITH_DEV_AUTOMATION_TESTS

// IA0.4 checks: the AI brain is a script class (Script/AI/HOLAIBrain.as) deriving the C++ UHOLAIBrainSubsystem.
namespace UE::HeroesOfLiberty::Tests::AIBrain
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** A bare game world, as the other tests' worlds: no game state, no begin play. */
	struct FBareWorld
	{
		UWorld* World = nullptr;

		FBareWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
		}

		~FBareWorld()
		{
			if (World)
			{
				GEngine->DestroyWorldContext(World);
				World->DestroyWorld(false);
			}
		}
	};

	const FName Peasant(TEXT("Peasant"));

	/** Units of Type born for Owner around Location (no walk: the test world runs no movement); their identifiers. */
	TArray<uint32> PlaceUnits(UHOLUnitSubsystem& Units, const FName Type, const int32 Owner, const FVector& Location, const int32 Count)
	{
		TArray<uint32> Before;
		Units.GetAllUnitIds(Before);
		FHOLUnitSpawnRequest Request;
		Request.TypeName = Type;
		Request.Count = Count;
		Request.OwnerPlayer = static_cast<uint8>(Owner);
		Request.Center = Location;
		Units.RequestUnits(Request);
		Units.ServeBirthRequests();
		TArray<uint32> After;
		Units.GetAllUnitIds(After);
		After.RemoveAll([&Before](const uint32 Id) { return Before.Contains(Id); });
		After.Sort();
		return After;
	}

	EHOLUnitActivity GetActivity(const UWorld& World, const uint32 UnitId)
	{
		FHOLUnitView View;
		return HOLUnitView::Find(World, UnitId, View) ? View.Activity : EHOLUnitActivity::Dying;
	}

	/**
	 * A match's server as the brain sees it (IA0.4 B2), built like AI.Summary.World and AI.Orders.*: a game state (slot 0
	 * human, slot 1 AI), slot 1's PlayerState born by the real SpawnAIPlayerStates with the starting stock, slot 1's
	 * finished centre (its deposit), berries and a tree near it, an idle Peasant of slot 1 and one of slot 0, and the
	 * step's processors that apply the queue (orders, then gathering).
	 */
	struct FBrainWorld
	{
		UWorld* World = nullptr;
		AHOLGameState* State = nullptr;
		UHOLUnitSubsystem* Units = nullptr;
		UHOLSourceSubsystem* Sources = nullptr;
		UHOLCommandSubsystem* Commands = nullptr;
		UHOLAIBrainSubsystem* Brain = nullptr;
		AHOLPlayerState* AIState = nullptr;
		TSharedPtr<FMassEntityManager> EntityManager;
		UHOLGatherProcessor* Gather = nullptr;
		UHOLUnitOrderProcessor* Orders = nullptr;
		int32 NumAIStates = 0;
		int32 Berries = INDEX_NONE;
		int32 Tree = INDEX_NONE;
		TArray<uint32> AIPeasants;
		TArray<uint32> HumanPeasants;

		FBrainWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			State = World->SpawnActor<AHOLGameState>();
			Units = World->GetSubsystem<UHOLUnitSubsystem>();
			Sources = World->GetSubsystem<UHOLSourceSubsystem>();
			Commands = World->GetSubsystem<UHOLCommandSubsystem>();
			Brain = UHOLAIBrainSubsystem::Find(World);
			UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
			UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
			AHOLBuilding* Centre = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
			if (!State || !Units || !Sources || !Commands || !Brain || !Simulation || !EntitySubsystem || !Centre)
			{
				return;
			}
			FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
			for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
			{
				Config.Slots[Index].State = Index == 0 ? EHOLSlotState::Human : (Index == 1 ? EHOLSlotState::AI : EHOLSlotState::Closed);
			}
			State->InitVictory(Config);
			// Slot 1's PlayerState as at the match start (IA0.1 E1): its stock is the starting one.
			NumAIStates = AHeroesOfLibertyGameModeBase::SpawnAIPlayerStates(*World, Config, GetDefault<UHOLMatchSettings>()->StartingResources);
			for (APlayerState* Each : State->PlayerArray)
			{
				AHOLPlayerState* HOLEach = Cast<AHOLPlayerState>(Each);
				if (HOLEach && HOLEach->IsABot() && HOLEach->GetPlayerIndex() == 1)
				{
					AIState = HOLEach;
				}
			}
			Centre->Setup(HOLBuildingTypeNames::MainCentre, 1, 1);

			Sources->OnWorldBeginPlay(*World);
			Berries = Sources->AddSource(HOLSourceTypeNames::BerryBush, FVector(0.0, 1800.0, 0.0));
			Tree = Sources->AddSource(HOLSourceTypeNames::Tree, FVector(2000.0, 0.0, 0.0));
			AIPeasants = PlaceUnits(*Units, Peasant, 1, FVector(1000.0, 1000.0, 0.0), 1);
			HumanPeasants = PlaceUnits(*Units, Peasant, 0, FVector(-3000.0, 0.0, 0.0), 1);

			EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
			Gather = NewObject<UHOLGatherProcessor>(Simulation);
			Gather->CallInitialize(Simulation, EntityManager.ToSharedRef());
			Orders = NewObject<UHOLUnitOrderProcessor>(Simulation);
			Orders->CallInitialize(Simulation, EntityManager.ToSharedRef());
		}

		~FBrainWorld()
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		bool IsReady() const
		{
			return Gather && Orders && AIState && Berries != INDEX_NONE && Tree != INDEX_NONE && AIPeasants.Num() == 1 && HumanPeasants.Num() == 1;
		}

		/** One fixed step of the queue's processors (the test world runs no movement): the order processor, then the gathering. */
		void RunStep()
		{
			const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
			{
				FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
				UE::Mass::Executor::Run(*Orders, ProcessingContext);
			}
			{
				FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
				UE::Mass::Executor::Run(*Gather, ProcessingContext);
			}
		}
	};
}

/**
 * IA0.4 B1: a game world has a brain whose class comes from the script (not the abstract C++ class), and ThinkNow reaches
 * the script's Think. Red without the .as: no subclass, no brain.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIBrainExistsTest, "HeroesOfLiberty.AI.Brain.Exists", UE::HeroesOfLiberty::Tests::AIBrain::Flags)
bool FHOLAIBrainExistsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIBrain;

	FBareWorld Bare;
	if (!TestNotNull(TEXT("Test world"), Bare.World))
	{
		return false;
	}
	UHOLAIBrainSubsystem* Brain = UHOLAIBrainSubsystem::Find(Bare.World);
	if (!TestNotNull(TEXT("A brain subsystem in a game world (a script class deriving UHOLAIBrainSubsystem)"), Brain))
	{
		return false;
	}
	const UClass* BrainClass = Brain->GetClass();
	TestTrue(FString::Printf(TEXT("Brain class %s is a subclass, not the C++ base"), *BrainClass->GetName()),
		BrainClass != UHOLAIBrainSubsystem::StaticClass());
	TestFalse(FString::Printf(TEXT("Brain class %s comes from the script (not native)"), *BrainClass->GetName()),
		BrainClass->HasAnyClassFlags(CLASS_Native));

	// No game state: no AI slot, and nothing breaks.
	TestEqual(TEXT("No AI slot without a game state"), Brain->GetAISlots().Num(), 0);

	// The script's Think answers true (the C++ base answers false).
	TestEqual(TEXT("No thought before ThinkNow"), Brain->GetNumThoughts(), 0);
	TestTrue(TEXT("ThinkNow(1) reaches the script's Think"), Brain->ThinkNow(1));
	TestEqual(TEXT("One thought counted"), Brain->GetNumThoughts(), 1);
	return true;
}

/**
 * IA0.4 B2 (IA0.4-Q1 A): the real chain C++ -> script -> order -> queue -> processors. One thought of AI slot 1 sends its
 * idle Peasants to the nearest source of its lowest stock among the resources it has a source of (a tie: food, wood,
 * stone, gold), in one order applied at the next step. Case 1: the starting stock (all equal) -> the berries. Case 2:
 * more food than wood -> the tree. Case 3: no idle Peasant left -> no order in the queue. Slot 0's (human) Peasant never
 * gets an order. Red while the script's Think only counts.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIBrainGatherIdleTest, "HeroesOfLiberty.AI.Brain.GatherIdle", UE::HeroesOfLiberty::Tests::AIBrain::Flags)
bool FHOLAIBrainGatherIdleTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIBrain;

	// Case 1, then case 3 in the same world: the starting stock, food first by the tie.
	{
		FBrainWorld Test;
		if (!TestNotNull(TEXT("A brain in the test world"), Test.Brain) || !TestTrue(TEXT("Test world ready"), Test.IsReady()))
		{
			return false;
		}
		const UWorld& World = *Test.World;
		TestEqual(TEXT("One AI player state born by SpawnAIPlayerStates"), Test.NumAIStates, 1);
		const TArray<int32> Slots = Test.Brain->GetAISlots();
		TestTrue(TEXT("The brain's AI slots: slot 1 only"), Slots.Num() == 1 && Slots[0] == 1);
		const FHOLResources Stock = Test.AIState->GetResources();
		TestTrue(TEXT("Slot 1's stock is the starting one, all four equal"), Stock == GetDefault<UHOLMatchSettings>()->StartingResources
			&& Stock.Food == Stock.Wood && Stock.Wood == Stock.Stone && Stock.Stone == Stock.Gold);

		AddExpectedMessage(FString::Printf(TEXT("Gather order of slot 1: 1 units to source %d \\(Food\\)"), Test.Berries),
			EAutomationExpectedMessageFlags::Contains, 1);
		TestTrue(TEXT("Case 1: slot 1 thinks"), Test.Brain->ThinkNow(1));
		TestEqual(TEXT("Case 1: not applied in the thought's frame, the AI Peasant still idle"), GetActivity(World, Test.AIPeasants[0]),
			EHOLUnitActivity::Idle);
		Test.RunStep();
		TestEqual(TEXT("Case 1: next step, the AI Peasant on its way to the berries"), GetActivity(World, Test.AIPeasants[0]),
			EHOLUnitActivity::GatherTrip);
		TestEqual(TEXT("Case 1: the human Peasant got no order, idle"), GetActivity(World, Test.HumanPeasants[0]), EHOLUnitActivity::Idle);

		// Case 3: its only Peasant now gathers, none idle: the thought issues nothing (the queue stays empty).
		TestTrue(TEXT("Case 3: slot 1 thinks"), Test.Brain->ThinkNow(1));
		TArray<FHOLGatherOrder> Queued;
		Test.Commands->ConsumeGatherOrders(Queued);
		TestEqual(TEXT("Case 3: no idle Peasant, no gather order queued"), Queued.Num(), 0);
		Test.RunStep();
		TestEqual(TEXT("Case 3: the AI Peasant still on its round trip"), GetActivity(World, Test.AIPeasants[0]), EHOLUnitActivity::GatherTrip);
		TestEqual(TEXT("Case 3: the human Peasant still idle"), GetActivity(World, Test.HumanPeasants[0]), EHOLUnitActivity::Idle);
	}

	// Case 2: more food than wood (the rest equal to wood) -> wood is the lowest, first by the tie with stone and gold.
	{
		FBrainWorld Test;
		if (!TestNotNull(TEXT("A brain in the second test world"), Test.Brain) || !TestTrue(TEXT("Second test world ready"), Test.IsReady()))
		{
			return false;
		}
		const UWorld& World = *Test.World;
		FHOLResources Stock = Test.AIState->GetResources();
		Stock.Food += 100;
		Test.AIState->SetResources(Stock);

		AddExpectedMessage(FString::Printf(TEXT("Gather order of slot 1: 1 units to source %d \\(Wood\\)"), Test.Tree),
			EAutomationExpectedMessageFlags::Contains, 1);
		TestTrue(TEXT("Case 2: slot 1 thinks"), Test.Brain->ThinkNow(1));
		Test.RunStep();
		TestEqual(TEXT("Case 2: the AI Peasant on its way to the tree"), GetActivity(World, Test.AIPeasants[0]), EHOLUnitActivity::GatherTrip);
		TestEqual(TEXT("Case 2: the human Peasant got no order, idle"), GetActivity(World, Test.HumanPeasants[0]), EHOLUnitActivity::Idle);
	}
	return true;
}

/**
 * IA0.4 B2b (IA0.4-Q3 A): a slot's "aiBrain": false in a scenario's .json switches its brain off. The .json read by the
 * real MakeScenarioConfig (slot 1 off, slot 2 without the field, slot 3 "aiBrain": true), the configuration given to the
 * game state by InitVictory and the AI player states born by the real SpawnAIPlayerStates, as at a match start: the
 * brain's AI slots are 1, 2 and 3, the thinking ones (those the step makes think) 2 and 3. Missing = true; the field
 * survives ToJson / FromJson. Red while FromJson ignores "aiBrain" or GetThinkingSlots does not filter.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIBrainSwitchTest, "HeroesOfLiberty.AI.Brain.Switch", UE::HeroesOfLiberty::Tests::AIBrain::Flags)
bool FHOLAIBrainSwitchTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIBrain;

	const FHOLMatchConfig Default = FHOLMatchConfig::MakeDefault();
	for (int32 Slot = 0; Slot < Default.Slots.Num(); ++Slot)
	{
		TestTrue(*FString::Printf(TEXT("MakeDefault: slot %d brain on"), Slot), Default.Slots[Slot].bAIBrain);
	}

	// A scenario's .json, by the real MakeScenarioConfig.
	const FString Json = TEXT("{ \"map\": \"/Game/Maps/Plain.Plain\", \"slots\": [ { \"state\": \"Human\", \"colorIndex\": 0 }, ")
		TEXT("{ \"state\": \"AI\", \"colorIndex\": 1, \"aiBrain\": false }, { \"state\": \"AI\", \"colorIndex\": 2 }, ")
		TEXT("{ \"state\": \"AI\", \"colorIndex\": 3, \"aiBrain\": true } ] }");
	FHOLMatchConfig Config;
	if (!TestTrue(TEXT(".json read"), HOLTestHooks::MakeScenarioConfig(&Json, Config)))
	{
		return false;
	}
	TestFalse(TEXT(".json: slot 1 \"aiBrain\": false, brain off"), Config.Slots[1].bAIBrain);
	TestTrue(TEXT(".json: slot 2 without the field, brain on"), Config.Slots[2].bAIBrain);
	TestTrue(TEXT(".json: slot 3 \"aiBrain\": true, brain on"), Config.Slots[3].bAIBrain);
	TestTrue(TEXT(".json: slot 0 (human) without the field, on"), Config.Slots[0].bAIBrain);
	TestTrue(TEXT(".json: slot 4 (added by Normalize), on"), Config.Slots[4].bAIBrain);

	// ToJson / FromJson: the read configuration, then one switched off in code.
	FHOLMatchConfig Back;
	TestTrue(TEXT("round trip of the read configuration: read"), FHOLMatchConfig::FromJson(Config.ToJson(), Back));
	TestTrue(TEXT("round trip of the read configuration: slot 1 off, 2 and 3 on"), !Back.Slots[1].bAIBrain && Back.Slots[2].bAIBrain && Back.Slots[3].bAIBrain);
	FHOLMatchConfig Off = FHOLMatchConfig::MakeDefault();
	Off.Slots[5].bAIBrain = false;
	FHOLMatchConfig OffBack;
	TestTrue(TEXT("round trip of a switch set in code"), FHOLMatchConfig::FromJson(Off.ToJson(), OffBack) && !OffBack.Slots[5].bAIBrain && OffBack.Slots[4].bAIBrain);

	// The match: the launch configuration in the game state, the AI player states by the real SpawnAIPlayerStates.
	FBareWorld Bare;
	AHOLGameState* State = Bare.World ? Bare.World->SpawnActor<AHOLGameState>() : nullptr;
	UHOLAIBrainSubsystem* Brain = UHOLAIBrainSubsystem::Find(Bare.World);
	if (!TestNotNull(TEXT("Game state"), State) || !TestNotNull(TEXT("A brain in the test world"), Brain))
	{
		return false;
	}
	State->InitVictory(Config);
	TestEqual(TEXT("Three AI player states born by SpawnAIPlayerStates"),
		AHeroesOfLibertyGameModeBase::SpawnAIPlayerStates(*Bare.World, Config, GetDefault<UHOLMatchSettings>()->StartingResources), 3);
	const TArray<int32> AISlots = Brain->GetAISlots();
	TestEqual(TEXT("The brain's AI slots: 1, 2, 3"), FString::JoinBy(AISlots, TEXT(","), [](const int32 Slot) { return FString::FromInt(Slot); }),
		FString(TEXT("1,2,3")));
	const TArray<int32> Thinking = Brain->GetThinkingSlots();
	TestEqual(TEXT("The thinking slots: 2 and 3 (slot 1 switched off)"), FString::JoinBy(Thinking, TEXT(","), [](const int32 Slot) { return FString::FromInt(Slot); }),
		FString(TEXT("2,3")));
	return true;
}

#endif
