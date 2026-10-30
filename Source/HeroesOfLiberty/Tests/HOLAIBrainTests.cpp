// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAIBrain.h"
#include "AI/HOLAISummary.h"
#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Commands/HOLUnitOrderProcessor.h"
#include "Dev/HOLTestHooks.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyGameModeBase.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "MassSimulationSubsystem.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLMatchSettings.h"
#include "Match/HOLPlayerState.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
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
			// Room to spare (IA1.5: the opening places its house when the places run low, and takes a Peasant to build it).
			if (AIState)
			{
				AIState->SetPopulation(5, 100);
			}

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

	/**
	 * IA0.4 B5: a match's server (or one of its clients) with the real fixed step, for the brain's cadence. The game state
	 * gets Config (InitVictory), the AI player states are born by the real SpawnAIPlayerStates, then the two subsystems of
	 * the cadence begin play as UWorld::BeginPlay makes them (the bare world's other subsystems stay asleep): the
	 * simulation (its step pipeline emptied for the time of its begin play: the cadence needs no processor, and a bare
	 * world has no navigation for them) and the brain, whose real OnWorldBeginPlay subscribes it to the step's end. A
	 * client is a world whose travel URL names a host (UWorld::AttemptDeriveFromURL -> NM_Client), set once the player
	 * states exist (a client gets them by replication).
	 */
	struct FCadenceWorld
	{
		UWorld* World = nullptr;
		AHOLGameState* State = nullptr;
		UHOLSimulationSubsystem* Simulation = nullptr;
		UHOLAIBrainSubsystem* Brain = nullptr;
		int32 NumAIStates = 0;
		bool bMassSimulation = false;

		FCadenceWorld(const FHOLMatchConfig& Config, const bool bClient)
		{
			World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			State = World->SpawnActor<AHOLGameState>();
			Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
			Brain = UHOLAIBrainSubsystem::Find(World);
			// The simulation's begin play needs Mass' subsystems (it logs an error without them).
			bMassSimulation = World->GetSubsystem<UMassSimulationSubsystem>() != nullptr && World->GetSubsystem<UMassEntitySubsystem>() != nullptr;
			if (!State || !Simulation || !Brain || !bMassSimulation)
			{
				return;
			}
			State->InitVictory(Config);
			NumAIStates = AHeroesOfLibertyGameModeBase::SpawnAIPlayerStates(*World, Config, GetDefault<UHOLMatchSettings>()->StartingResources);
			if (bClient)
			{
				World->NextURL = TEXT("127.0.0.1");
				World->NextTravelType = TRAVEL_Absolute;
			}

			// The simulation's real begin play (protected in UHOLSimulationSubsystem, public in UWorldSubsystem), without processors.
			UHOLSimulationSettings* Settings = GetMutableDefault<UHOLSimulationSettings>();
			TArray<TSubclassOf<UMassProcessor>> SavedProcessors = MoveTemp(Settings->SimulationProcessors);
			Settings->SimulationProcessors.Reset();
			static_cast<UWorldSubsystem*>(Simulation)->OnWorldBeginPlay(*World);
			Settings->SimulationProcessors = MoveTemp(SavedProcessors);

			BeginPlayBrain();
		}

		~FCadenceWorld()
		{
			if (World)
			{
				World->NextURL.Reset();
				GEngine->DestroyWorldContext(World);
				World->DestroyWorld(false);
			}
		}

		bool IsReady() const { return State && Simulation && Brain && bMassSimulation; }

		/** The brain's real begin play (its subscription to the step's end). */
		void BeginPlayBrain() { Brain->OnWorldBeginPlay(*World); }

		/** The steps at which the step made Slot think: "4,9,14,19". */
		FString StepsOf(const int32 Slot) const
		{
			TArray<FString> Steps;
			for (const FHOLAIStepThought& Thought : Brain->GetStepThoughts())
			{
				if (Thought.Slot == Slot)
				{
					Steps.Add(FString::Printf(TEXT("%lld"), Thought.Step));
				}
			}
			return FString::Join(Steps, TEXT(","));
		}

		/** Whether two thoughts fell on the same step (the spread failed, or one slot thought twice). */
		bool AnyStepShared() const
		{
			TSet<int64> Seen;
			for (const FHOLAIStepThought& Thought : Brain->GetStepThoughts())
			{
				bool bAlready = false;
				Seen.Add(Thought.Step, &bAlready);
				if (bAlready)
				{
					return true;
				}
			}
			return false;
		}
	};

	/** The cadence's match by the real .json reader: slot 0 human, slots 1 and 3 AI, slot 2 AI switched off ("aiBrain": false). */
	bool MakeCadenceConfig(FHOLMatchConfig& OutConfig)
	{
		const FString Json = TEXT("{ \"map\": \"/Game/Maps/Plain.Plain\", \"slots\": [ { \"state\": \"Human\", \"colorIndex\": 0 }, ")
			TEXT("{ \"state\": \"AI\", \"colorIndex\": 1 }, { \"state\": \"AI\", \"colorIndex\": 2, \"aiBrain\": false }, ")
			TEXT("{ \"state\": \"AI\", \"colorIndex\": 3 } ] }");
		return HOLTestHooks::MakeScenarioConfig(&Json, OutConfig);
	}
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
 * IA0.4 B2 (IA0.4-Q1 A, the spreading by IA1.5): the real chain C++ -> script -> order -> queue -> processors. One thought
 * of AI slot 1 sends its idle Peasants to the nearest source of the resource that lacks Peasants by the opening's weights
 * (3 food for 2 wood), in one order applied at the next step. Case 1: no gatherer yet -> the berries. Case 2: 3 on the food
 * -> the tree. Case 3: no idle Peasant left -> no order in the queue. Slot 0's (human) Peasant never
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

	// Case 2 (IA1.5: the Peasants spread by the opening's weights, not by the stock): 3 already on the food and none on the
	// wood -> the idle Peasant goes to the tree.
	{
		FBrainWorld Test;
		if (!TestNotNull(TEXT("A brain in the second test world"), Test.Brain) || !TestTrue(TEXT("Second test world ready"), Test.IsReady()))
		{
			return false;
		}
		const UWorld& World = *Test.World;
		Test.AIState->SetGatherers(FHOLResources(3, 0, 0, 0));

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

/**
 * IA0.4 B4: the brain never sends its Peasants to a field it may not gather. The world of AI.Brain.GatherIdle plus slot 0's
 * (human) farm set up finished by the real path (AHOLBuilding::Setup -> its field, slot 0's) beside slot 1's centre,
 * nearer than the berries. The starting stock (food first by the tie): one thought of slot 1, one step -> its Peasant on
 * its way to the berries, not to slot 0's field. Red while the summary keeps the enemy field: the brain picks it as the
 * nearest food, the gather refuses the order (IsGatherableBy) and the Peasant stays idle, sent again at every thought.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIBrainGatherableOnlyTest, "HeroesOfLiberty.AI.Brain.GatherableOnly", UE::HeroesOfLiberty::Tests::AIBrain::Flags)
bool FHOLAIBrainGatherableOnlyTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIBrain;

	FBrainWorld Test;
	if (!TestNotNull(TEXT("A brain in the test world"), Test.Brain) || !TestTrue(TEXT("Test world ready"), Test.IsReady()))
	{
		return false;
	}
	const UWorld& World = *Test.World;
	// Slot 0's farm 1400 cm west of slot 1's centre (the berries are 1800 cm north): its field is the nearest food.
	AHOLBuilding* EnemyFarm = Test.World->SpawnActor<AHOLBuilding>(FVector(-1400.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (!TestNotNull(TEXT("Slot 0's farm spawned"), EnemyFarm))
	{
		return false;
	}
	EnemyFarm->Setup(HOLBuildingTypeNames::Farm, 0, 0);
	const int32 EnemyField = EnemyFarm->GetLinkedSource();
	const TArray<FHOLSource>& All = Test.Sources->GetSources();
	if (!TestTrue(TEXT("Slot 0's farm: its field stands"), All.IsValidIndex(EnemyField) && All[EnemyField].IsStanding()))
	{
		return false;
	}
	TestFalse(TEXT("Slot 1 may not gather slot 0's field"), All[EnemyField].IsGatherableBy(1));

	// What the brain reads: the berries are slot 1's nearest food, the enemy field left out.
	const FHOLAISummary Summary = HOLAISummary::Read(World, 1);
	TestTrue(TEXT("Slot 1's nearest food: the berries"), Summary.Food.Sources.Num() > 0 && Summary.Food.Sources[0].SourceIndex == Test.Berries);
	TestFalse(TEXT("Slot 1's food: slot 0's field left out"),
		Summary.Food.Sources.ContainsByPredicate([EnemyField](const FHOLAISourceNear& Near) { return Near.SourceIndex == EnemyField; }));

	AddExpectedMessage(FString::Printf(TEXT("Gather order of slot 1: 1 units to source %d \\(Food\\)"), Test.Berries),
		EAutomationExpectedMessageFlags::Contains, 1);
	TestTrue(TEXT("Slot 1 thinks"), Test.Brain->ThinkNow(1));
	Test.RunStep();
	TestEqual(TEXT("Next step: the AI Peasant on its way to food it may gather"), GetActivity(World, Test.AIPeasants[0]),
		EHOLUnitActivity::GatherTrip);
	TestEqual(TEXT("The human Peasant got no order, idle"), GetActivity(World, Test.HumanPeasants[0]), EHOLUnitActivity::Idle);
	return true;
}

/**
 * IA0.4 B5 (IA0.4-Q4 A): the brain thinks at the real step, every hol.AI.ThinkSteps steps, spread by slot. A match's
 * server (FCadenceWorld: the .json by the real reader, slots 1 and 3 AI, slot 2 AI "aiBrain": false, the player states
 * by the real SpawnAIPlayerStates, the real begin play of the simulation and of the brain), steps by the real RunStep.
 * OnStepEnded gives the index already advanced past the step run (1 after the first step), and slot S thinks at the
 * steps where (step + S) is a multiple of the interval:
 * - 20 steps at 5: slot 1 at 4, 9, 14, 19, slot 3 at 2, 7, 12, 17 (20 / 5 = 4 thoughts each, never at the same step),
 *   slot 2 never (it would at 3, 8, 13, 18);
 * - 20 more steps at hol.AI.ThinkSteps 10: half as many (slot 1 at 29, 39, slot 3 at 27, 37), the value restored after;
 * - a client: the simulation runs no step there, and even a step's end (broadcast by hand, as the other tests drive
 *   their steps) makes no slot think; ThinkNow refused.
 * A second begin play is not tested: UWorldSubsystem::OnWorldBeginPlay forbids it (ensureAlwaysMsgf). Red while the brain
 * does not record which slot thought at which step (GetStepThoughts empty, GetNumThoughts already right): the cadence
 * itself dates from B1.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIBrainCadenceTest, "HeroesOfLiberty.AI.Brain.Cadence", UE::HeroesOfLiberty::Tests::AIBrain::Flags)
bool FHOLAIBrainCadenceTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIBrain;

	FHOLMatchConfig Config;
	if (!TestTrue(TEXT(".json of the cadence read"), MakeCadenceConfig(Config)))
	{
		return false;
	}
	IConsoleVariable* ThinkSteps = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.AI.ThinkSteps"));
	if (!TestNotNull(TEXT("hol.AI.ThinkSteps"), ThinkSteps))
	{
		return false;
	}
	const int32 ThinkStepsBefore = ThinkSteps->GetInt();
	ON_SCOPE_EXIT
	{
		ThinkSteps->Set(ThinkStepsBefore, ECVF_SetByCode);
	};
	ThinkSteps->Set(5, ECVF_SetByCode);
	if (!TestEqual(TEXT("hol.AI.ThinkSteps at 5 (IA0.4-Q4 A)"), UHOLAIBrainSubsystem::GetThinkSteps(), 5))
	{
		return false;
	}

	// The server: 20 steps at 5, then 20 at 10.
	{
		FCadenceWorld Test(Config, /*bClient*/ false);
		if (!TestTrue(TEXT("Server world ready (game state, simulation, Mass, brain)"), Test.IsReady()))
		{
			return false;
		}
		TestEqual(TEXT("Three AI player states born by SpawnAIPlayerStates"), Test.NumAIStates, 3);
		TestEqual(TEXT("The thinking slots: 1 and 3 (slot 2 switched off)"),
			FString::JoinBy(Test.Brain->GetThinkingSlots(), TEXT(","), [](const int32 Slot) { return FString::FromInt(Slot); }), FString(TEXT("1,3")));
		TestEqual(TEXT("No step before the first"), Test.Simulation->GetStepIndex(), int64(0));

		TestEqual(TEXT("20 real steps run"), Test.Simulation->RunStepsForTests(20), 20);
		TestEqual(TEXT("Step index after 20 steps"), Test.Simulation->GetStepIndex(), int64(20));
		TestEqual(TEXT("At 5: slot 1 thinks at steps 4, 9, 14, 19"), Test.StepsOf(1), FString(TEXT("4,9,14,19")));
		TestEqual(TEXT("At 5: slot 3 thinks at steps 2, 7, 12, 17"), Test.StepsOf(3), FString(TEXT("2,7,12,17")));
		TestEqual(TEXT("At 5: slot 2 (\"aiBrain\": false) never thinks"), Test.StepsOf(2), FString());
		TestEqual(TEXT("At 5: slot 0 (human) never thinks"), Test.StepsOf(0), FString());
		TestFalse(TEXT("At 5: no two thoughts at the same step (spread)"), Test.AnyStepShared());
		TestEqual(TEXT("At 5: 8 thoughts counted (4 per thinking slot)"), Test.Brain->GetNumThoughts(), 8);

		ThinkSteps->Set(10, ECVF_SetByCode);
		TestEqual(TEXT("hol.AI.ThinkSteps at 10"), UHOLAIBrainSubsystem::GetThinkSteps(), 10);
		TestEqual(TEXT("20 more real steps run"), Test.Simulation->RunStepsForTests(20), 20);
		ThinkSteps->Set(5, ECVF_SetByCode);
		TestEqual(TEXT("At 10: slot 1 thinks at 29 and 39 (half as often)"), Test.StepsOf(1), FString(TEXT("4,9,14,19,29,39")));
		TestEqual(TEXT("At 10: slot 3 thinks at 27 and 37 (half as often)"), Test.StepsOf(3), FString(TEXT("2,7,12,17,27,37")));
		TestEqual(TEXT("At 10: slot 2 still never"), Test.StepsOf(2), FString());
		TestFalse(TEXT("At 10: no two thoughts at the same step"), Test.AnyStepShared());
		TestEqual(TEXT("At 10: 12 thoughts in all"), Test.Brain->GetNumThoughts(), 12);
	}

	// A client: no step, and a step's end makes no slot think.
	{
		FCadenceWorld Test(Config, /*bClient*/ true);
		if (!TestTrue(TEXT("Client world ready"), Test.IsReady()))
		{
			return false;
		}
		if (!TestEqual(TEXT("The client world is a client (NM_Client)"), static_cast<int32>(Test.World->GetNetMode()), static_cast<int32>(NM_Client)))
		{
			return false;
		}
		TestEqual(TEXT("Client: the simulation runs no step"), Test.Simulation->RunStepsForTests(20), 0);
		for (int64 Step = 1; Step <= 20; ++Step)
		{
			Test.Simulation->GetOnStepEnded().Broadcast(Step);
		}
		TestEqual(TEXT("Client: no thought at the steps' ends"), Test.Brain->GetStepThoughts().Num(), 0);
		TestFalse(TEXT("Client: ThinkNow refused"), Test.Brain->ThinkNow(1));
		TestEqual(TEXT("Client: no thought counted"), Test.Brain->GetNumThoughts(), 0);
	}
	return true;
}

#endif
