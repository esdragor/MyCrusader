// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Buildings/HOLProductionRules.h"
#include "Buildings/HOLProductionSubsystem.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Commands/HOLPlayerOrders.h"
#include "Commands/HOLProductionCommand.h"
#include "Commands/HOLUnitOrderProcessor.h"
#include "Dev/HOLTestHooks.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "HeroesOfLibertyGameModeBase.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Match/HOLForumMessages.h"
#include "Match/HOLForumRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLMatchSettings.h"
#include "Match/HOLPlayerState.h"
#include "Match/HOLResources.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLGatherProcessor.h"
#include "Simulation/HOLGatherRules.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitView.h"

#if WITH_DEV_AUTOMATION_TESTS

// IA0.1 checks: an AI slot gets a player state (a bot, the slot's colour, the starting stock), so that its economy
// (deposits, foundations, production) runs as a human's.
namespace UE::HeroesOfLiberty::Tests::AISlot
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** Slot 0 human; 1 and 3 AI, at colours 5 and 2 (not their index); 2 and the rest closed. */
	FHOLMatchConfig MakeConfig()
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
		{
			Config.Slots[Index].State = Index == 0 ? EHOLSlotState::Human
				: (Index == 1 || Index == 3 ? EHOLSlotState::AI : EHOLSlotState::Closed);
			Config.Slots[Index].ColorIndex = Index;
		}
		Config.Slots[1].ColorIndex = 5;
		Config.Slots[3].ColorIndex = 2;
		return Config;
	}

	TArray<AHOLPlayerState*> GetPlayerStates(const AGameStateBase& State)
	{
		TArray<AHOLPlayerState*> PlayerStates;
		for (APlayerState* Each : State.PlayerArray)
		{
			if (AHOLPlayerState* HOLEach = Cast<AHOLPlayerState>(Each))
			{
				PlayerStates.Add(HOLEach);
			}
		}
		return PlayerStates;
	}

	const FName Peasant(TEXT("Peasant"));

	/**
	 * IA0.1 E2: IA0.2's test world (a match's server: game state, the queue, the order and gathering processors at each step)
	 * with the construction and the production bound to the steps, slot 1's main centre at the origin, and the AI slots'
	 * player states born by the real SpawnAIPlayerStates (E1), or none (the counter-proof: the code before E1).
	 */
	struct FEconomyWorld
	{
		UWorld* World = nullptr;
		AHOLGameState* State = nullptr;
		UHOLUnitSubsystem* Units = nullptr;
		UHOLCommandSubsystem* Commands = nullptr;
		UHOLProductionSubsystem* Production = nullptr;
		UHOLSimulationSubsystem* Simulation = nullptr;
		TSharedPtr<FMassEntityManager> EntityManager;
		UHOLUnitOrderProcessor* Orders = nullptr;
		UHOLGatherProcessor* Gather = nullptr;
		AHOLBuilding* Centre = nullptr;
		int64 Step = 0;

		explicit FEconomyWorld(const bool bAIPlayerStates)
		{
			World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			State = World->SpawnActor<AHOLGameState>();
			Units = World->GetSubsystem<UHOLUnitSubsystem>();
			Commands = World->GetSubsystem<UHOLCommandSubsystem>();
			Production = World->GetSubsystem<UHOLProductionSubsystem>();
			Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
			UHOLConstructionSubsystem* Construction = World->GetSubsystem<UHOLConstructionSubsystem>();
			UHOLSourceSubsystem* Sources = World->GetSubsystem<UHOLSourceSubsystem>();
			UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
			if (!State || !Units || !Commands || !Production || !Simulation || !Construction || !Sources || !EntitySubsystem)
			{
				return;
			}
			const FHOLMatchConfig Config = MakeConfig();
			State->InitVictory(Config);
			if (bAIPlayerStates)
			{
				AHeroesOfLibertyGameModeBase::SpawnAIPlayerStates(*World, Config, GetDefault<UHOLMatchSettings>()->StartingResources);
			}
			Sources->OnWorldBeginPlay(*World);
			// As at BeginPlay: both apply their orders at each step's start.
			Construction->OnWorldBeginPlay(*World);
			Production->OnWorldBeginPlay(*World);
			Centre = World->SpawnActor<AHOLBuilding>(FVector::ZeroVector, FRotator::ZeroRotator);
			if (Centre)
			{
				Centre->Setup(HOLBuildingTypeNames::MainCentre, 1, static_cast<uint8>(Config.Slots[1].ColorIndex));
			}
			EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
			Gather = NewObject<UHOLGatherProcessor>(Simulation);
			Gather->CallInitialize(Simulation, EntityManager.ToSharedRef());
			Orders = NewObject<UHOLUnitOrderProcessor>(Simulation);
			Orders->CallInitialize(Simulation, EntityManager.ToSharedRef());
		}

		~FEconomyWorld()
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		bool IsReady() const { return Gather && Orders && Centre; }

		/** One fixed step: its start (construction, production), the order processor, the gathering, its end. */
		void RunStep()
		{
			++Step;
			Simulation->GetOnStepStarted().Broadcast(Step);
			const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
			{
				FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
				UE::Mass::Executor::Run(*Orders, ProcessingContext);
			}
			{
				FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
				UE::Mass::Executor::Run(*Gather, ProcessingContext);
			}
			Simulation->GetOnStepEnded().Broadcast(Step);
		}

		/** Units of Type born for Owner at Location (no walk). */
		void PlaceUnits(const FName Type, const int32 Owner, const FVector& Location, const int32 Count)
		{
			FHOLUnitSpawnRequest Request;
			Request.TypeName = Type;
			Request.Count = Count;
			Request.OwnerPlayer = static_cast<uint8>(Owner);
			Request.Center = Location;
			Units->RequestUnits(Request);
			Units->ServeBirthRequests();
		}

		/** Owner's living units of Type. */
		int32 CountUnits(const FName Type, const int32 Owner) const
		{
			TArray<uint32> Ids;
			Units->GetAllUnitIds(Ids);
			int32 Count = 0;
			for (const uint32 Id : Ids)
			{
				FHOLUnitView View;
				Count += Units->GetUnitView(Id, View) && View.OwnerPlayer == Owner && View.TypeName == Type ? 1 : 0;
			}
			return Count;
		}

		/** Owner's building of Type, if any. */
		AHOLBuilding* FindBuilding(const FName Type, const int32 Owner) const
		{
			for (TActorIterator<AHOLBuilding> It(World); It; ++It)
			{
				if (It->GetTypeName() == Type && It->GetOwnerPlayer() == Owner && !It->IsActorBeingDestroyed())
				{
					return *It;
				}
			}
			return nullptr;
		}
	};
}

// IA0.1 E1: the start gives each AI slot (1 and 3) its player state: a bot, its slot, its colour, its name, the starting stock
// of the game (UHOLMatchSettings, as a human's), found by GetSlotStock; none for the human slot (no login here) nor the
// closed one; called again, no second one.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAISlotSpawnTest, "HeroesOfLiberty.AI.Slot.Spawn", UE::HeroesOfLiberty::Tests::AISlot::Flags)
bool FHOLAISlotSpawnTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AISlot;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	if (TestNotNull(TEXT("game state"), State))
	{
		const FHOLMatchConfig Config = MakeConfig();
		State->InitVictory(Config);
		const FHOLResources& Starting = GetDefault<UHOLMatchSettings>()->StartingResources;
		TestTrue(TEXT("the game's starting stock is not empty"), Starting != FHOLResources());

		const int32 Spawned = AHeroesOfLibertyGameModeBase::SpawnAIPlayerStates(*World, Config, Starting);
		TestEqual(TEXT("two spawned"), Spawned, 2);
		const TArray<AHOLPlayerState*> PlayerStates = GetPlayerStates(*State);
		TestEqual(TEXT("two player states in PlayerArray, one per AI slot"), PlayerStates.Num(), 2);
		for (const int32 Slot : { 1, 3 })
		{
			AHOLPlayerState* const* Found = PlayerStates.FindByPredicate([Slot](const AHOLPlayerState* Each) { return Each->GetPlayerIndex() == Slot; });
			if (!TestNotNull(*FString::Printf(TEXT("slot %d: a player state"), Slot), Found ? *Found : nullptr))
			{
				continue;
			}
			const AHOLPlayerState& PlayerState = **Found;
			TestTrue(*FString::Printf(TEXT("slot %d: a bot"), Slot), PlayerState.IsABot());
			TestEqual(*FString::Printf(TEXT("slot %d: the colour of the configuration"), Slot), PlayerState.GetColorIndex(),
				Config.Slots[Slot].ColorIndex);
			TestEqual(*FString::Printf(TEXT("slot %d: its name"), Slot), PlayerState.GetPlayerName(),
				AHeroesOfLibertyGameModeBase::MakeAIPlayerName(Config, Slot));
			TestFalse(*FString::Printf(TEXT("slot %d: a name shown"), Slot), PlayerState.GetPlayerName().IsEmpty());
			TestFalse(*FString::Printf(TEXT("slot %d: not a spectator"), Slot), PlayerState.IsOnlyASpectator());
			AddInfo(FString::Printf(TEXT("slot %d: '%s', %s"), Slot, *PlayerState.GetPlayerName(), *PlayerState.DescribeEconomy()));
			TestTrue(*FString::Printf(TEXT("slot %d: the starting stock"), Slot), PlayerState.GetResources() == Starting);
			FHOLResources Stock;
			TestTrue(*FString::Printf(TEXT("slot %d: GetSlotStock finds it"), Slot), AHOLGameState::GetSlotStock(*World, Slot, Stock)
				&& Stock == Starting);
		}
		for (const int32 Slot : { 0, 2 })
		{
			FHOLResources Stock;
			TestFalse(*FString::Printf(TEXT("slot %d (human not logged in, or closed): no player state"), Slot),
				AHOLGameState::GetSlotStock(*World, Slot, Stock));
		}

		// A second call (a second start): the slots keep theirs.
		TestEqual(TEXT("second call: none spawned"), AHeroesOfLibertyGameModeBase::SpawnAIPlayerStates(*World, Config, Starting), 0);
		TestEqual(TEXT("second call: still two player states"), GetPlayerStates(*State).Num(), 2);
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// IA0.1 E2: with its player state (E1), AI slot 1's economy runs on the server's chain as a human's: a house's foundation
// placed by its order (HOLPlayerOrders::Place, as UHOLAIOrderLibrary's brain would) is paid the table's cost at the next
// step; a Peasant queued at its centre (the queue, as the controller's RPC) is paid, then born after its production time;
// 10 wood unloaded by its Peasant (the gathering's deposit) are credited. Counter-proof, the same world without the AI's
// player state (the code before E1): the foundation and the queue refused, each with its log, the 10 wood lost.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAISlotEconomyTest, "HeroesOfLiberty.AI.Slot.Economy", UE::HeroesOfLiberty::Tests::AISlot::Flags)
bool FHOLAISlotEconomyTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AISlot;

	const FHOLBuildingTypeRow* HouseType = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::House);
	if (!TestNotNull(TEXT("the house's row"), HouseType))
	{
		return false;
	}
	TestTrue(TEXT("a house costs something"), HouseType->Cost != FHOLResources());
	const FHOLResources& Starting = GetDefault<UHOLMatchSettings>()->StartingResources;
	const FVector HousePoint(0.0, 3000.0, 0.0);

	{
		FEconomyWorld Test(/*bAIPlayerStates*/ true);
		if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
		{
			return false;
		}
		UWorld& World = *Test.World;
		FHOLResources Stock;
		TestTrue(TEXT("slot 1: its starting stock (E1)"), AHOLGameState::GetSlotStock(World, 1, Stock) && Stock == Starting);
		Test.PlaceUnits(Peasant, 1, FVector(1500.0, 0.0, 0.0), 1);
		TestEqual(TEXT("a Peasant of slot 1"), Test.CountUnits(Peasant, 1), 1);
		FHOLResources Expected = Starting;

		// 1. A house's foundation, by the slot's order: applied at the next step, paid.
		TestTrue(TEXT("the place order enters the queue"), HOLPlayerOrders::Place(World, 1, HOLBuildingTypeNames::House, HousePoint, TArray<uint32>()));
		TestNull(TEXT("not placed in the frame of the order"), Test.FindBuilding(HOLBuildingTypeNames::House, 1));
		Test.RunStep();
		const AHOLBuilding* Foundation = Test.FindBuilding(HOLBuildingTypeNames::House, 1);
		if (TestNotNull(TEXT("next step: slot 1's house placed"), Foundation))
		{
			TestTrue(TEXT("a foundation"), Foundation->GetConstructionState() == EHOLConstructionState::Foundation);
			TestEqual(TEXT("in slot 1's colour (the configuration's)"), static_cast<int32>(Foundation->GetColorIndex()), 5);
		}
		Expected -= HouseType->Cost;
		TestTrue(TEXT("slot 1 paid the house's cost (the table's)"), AHOLGameState::GetSlotStock(World, 1, Stock) && Stock == Expected);
		AddInfo(FString::Printf(TEXT("after the foundation: %s (cost %s)"), *Stock.ToString(), *HouseType->Cost.ToString()));

		// 2. A Peasant queued at its centre: paid at the next step, born after its production time.
		const FHOLProductionCost PeasantCost = Test.Production->GetCost(Peasant);
		TestTrue(TEXT("a Peasant costs something"), PeasantCost.Resources != FHOLResources());
		FHOLProductionCommand Command;
		Command.PlayerIndex = 1;
		Command.Building = Test.Centre;
		Command.TypeName = Peasant;
		Test.Commands->IssueProductionOrder(Command);
		Test.RunStep();
		TestEqual(TEXT("a Peasant in the queue of slot 1's centre"), Test.Centre->GetProductionQueue().Num(), 1);
		Expected -= PeasantCost.Resources;
		TestTrue(TEXT("slot 1 paid the Peasant's cost"), AHOLGameState::GetSlotStock(World, 1, Stock) && Stock == Expected);
		for (int32 Index = 0; Index < PeasantCost.BuildSteps + 2 && !Test.Centre->GetProductionQueue().IsEmpty(); ++Index)
		{
			Test.RunStep();
		}
		TestEqual(TEXT("after its production time: the queue empty"), Test.Centre->GetProductionQueue().Num(), 0);
		Test.Units->ServeBirthRequests();
		TestEqual(TEXT("a second Peasant of slot 1 born"), Test.CountUnits(Peasant, 1), 2);

		// 3. Its Peasant unloads 10 wood (the gathering's deposit): credited.
		FHOLGatherLoad Load;
		Load.Resource = EHOLResource::Wood;
		Load.Amount = 10;
		FHOLResources After;
		TestEqual(TEXT("its Peasant unloads 10 wood"), UHOLGatherProcessor::DepositLoad(World, 1, Load, After), 10);
		TestEqual(TEXT("its load emptied"), Load.Amount, 0);
		Expected.Wood += 10;
		TestTrue(TEXT("the 10 wood credited to slot 1"), AHOLGameState::GetSlotStock(World, 1, Stock) && Stock == Expected);
		TestTrue(TEXT("the stock after the deposit given back"), After == Expected);
		AddInfo(FString::Printf(TEXT("at the end: %s"), *Stock.ToString()));
	}

	{
		FEconomyWorld Bare(/*bAIPlayerStates*/ false);
		if (!TestTrue(TEXT("counter-proof: test world ready"), Bare.IsReady()))
		{
			return false;
		}
		UWorld& World = *Bare.World;
		FHOLResources Stock;
		TestFalse(TEXT("counter-proof: slot 1 without a player state has no stock"), AHOLGameState::GetSlotStock(World, 1, Stock));

		// The rule knows no stock: the orders enter the queue; the step drops them, now with a log (IA0.1 E2, E4).
		AddExpectedMessage(TEXT("Construction order of slot 1 ignored: no player state"), EAutomationExpectedMessageFlags::Contains, 1);
		AddExpectedMessage(TEXT("Production order of slot 1 ignored: no player state"), EAutomationExpectedMessageFlags::Contains, 1);
		TestTrue(TEXT("counter-proof: the place order enters the queue"), HOLPlayerOrders::Place(World, 1, HOLBuildingTypeNames::House, HousePoint,
			TArray<uint32>()));
		FHOLProductionCommand Command;
		Command.PlayerIndex = 1;
		Command.Building = Bare.Centre;
		Command.TypeName = Peasant;
		Bare.Commands->IssueProductionOrder(Command);
		Bare.RunStep();
		TestNull(TEXT("counter-proof: no foundation"), Bare.FindBuilding(HOLBuildingTypeNames::House, 1));
		TestEqual(TEXT("counter-proof: nothing queued"), Bare.Centre->GetProductionQueue().Num(), 0);

		FHOLGatherLoad Load;
		Load.Resource = EHOLResource::Wood;
		Load.Amount = 10;
		FHOLResources After;
		TestEqual(TEXT("counter-proof: the Peasant unloads its 10 wood all the same"), UHOLGatherProcessor::DepositLoad(World, 1, Load, After), 10);
		TestEqual(TEXT("counter-proof: its load emptied"), Load.Amount, 0);
		TestFalse(TEXT("counter-proof: credited to nobody (lost)"), AHOLGameState::GetSlotStock(World, 1, Stock));
	}
	return true;
}

// IA0.1-Q1 (B, with the level) and Q1b (A): an AI slot is named « IA <level> N » everywhere its name shows: its player state
// (the real SpawnAIPlayerStates), the thread's messages and their coloured names, the players' table, the banner and the
// statistics (GetPlayerName); the level a field of each slot, Normal by default, read from a scenario's .json (the real
// MakeScenarioConfig): "Hard" read, missing Normal, unknown Normal with a warning. A human keeps « Joueur N » or his lobby name.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAISlotNameTest, "HeroesOfLiberty.AI.Slot.Name", UE::HeroesOfLiberty::Tests::AISlot::Flags)
bool FHOLAISlotNameTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AISlot;

	// Every slot Normal by default.
	const FHOLMatchConfig Default = FHOLMatchConfig::MakeDefault();
	for (int32 Slot = 0; Slot < Default.Slots.Num(); ++Slot)
	{
		TestTrue(*FString::Printf(TEXT("MakeDefault: slot %d Normal"), Slot), Default.Slots[Slot].Difficulty == EHOLAIDifficulty::Normal);
	}

	// Slot 1 Hard, slot 3 not given (Normal).
	FHOLMatchConfig Config = MakeConfig();
	Config.Slots[1].Difficulty = EHOLAIDifficulty::Hard;
	const FString Hard2 = TEXT("IA Difficile 2");
	const FString Normal4 = TEXT("IA Normal 4");
	TestEqual(TEXT("the level's name"), HOLForumMessages::GetDifficultyName(EHOLAIDifficulty::Hard).ToString(), FString(TEXT("Difficile")));
	TestEqual(TEXT("GetAIName"), HOLForumMessages::GetAIName(1, EHOLAIDifficulty::Hard).ToString(), Hard2);
	TestEqual(TEXT("GetAIName, Évolutive"), HOLForumMessages::GetAIName(2, EHOLAIDifficulty::Evolving).ToString(), FString(TEXT("IA Évolutive 3")));

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	if (TestNotNull(TEXT("game state"), State))
	{
		State->InitVictory(Config);
		AHeroesOfLibertyGameModeBase::SpawnAIPlayerStates(*World, Config, GetDefault<UHOLMatchSettings>()->StartingResources);
		for (const AHOLPlayerState* PlayerState : GetPlayerStates(*State))
		{
			const FString Expected = PlayerState->GetPlayerIndex() == 1 ? Hard2 : Normal4;
			TestEqual(*FString::Printf(TEXT("slot %d: player state's name"), PlayerState->GetPlayerIndex()), PlayerState->GetPlayerName(), Expected);
		}
		TestEqual(TEXT("two AI player states"), GetPlayerStates(*State).Num(), 2);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);

	// The thread: slot 1's forum fell, read by slot 0 and by a spectator; its name cut out for its colour.
	const FHOLForumMessage Fell = HOLForumMessages::GetMessage(FHOLForumEvent::Make(EHOLForumEventKind::Fell, 1, INDEX_NONE, 1), Config, 0);
	TestTrue(TEXT("thread: « IA Difficile 2 » named"), Fell.Text.ToString().Contains(Hard2));
	TestTrue(TEXT("thread, spectator: named"), HOLForumMessages::GetMessage(FHOLForumEvent::Make(EHOLForumEventKind::Fell, 3, INDEX_NONE, 1),
		Config, INDEX_NONE).Text.ToString().Contains(Normal4));
	const TArray<HOLForumMessages::FPiece> Pieces = HOLForumMessages::SplitNames(Fell, 0, Config);
	TestTrue(TEXT("thread: the AI's name a piece of slot 1"), Pieces.ContainsByPredicate([&Hard2](const HOLForumMessages::FPiece& Piece)
	{
		return Piece.Slot == 1 && Piece.Text == Hard2;
	}));
	TestEqual(TEXT("thread: the human still Joueur 1"), HOLForumMessages::GetSlotName(0, 1, Config).ToString(), FString(TEXT("Joueur 1")));
	TestEqual(TEXT("thread: the viewer Vous"), HOLForumMessages::GetSlotName(1, 1, Config).ToString(), FString(TEXT("Vous")));

	// The players' table (slot 0's view), the banner and the statistics (GetPlayerName).
	TArray<FHOLForumState> Forums;
	Forums.Init(FHOLForumState(), FHOLMatchConfig::MaxSlots);
	const TArray<FHOLPlayersTableRow> Rows = HOLForumRules::GetPlayersTable(Config, Forums, 0, 0, FHOLForumTuning());
	for (const FHOLPlayersTableRow& Row : Rows)
	{
		const FString Expected = Row.Slot == 0 ? FString(TEXT("Vous")) : Row.Slot == 1 ? Hard2 : Normal4;
		TestEqual(*FString::Printf(TEXT("table: slot %d"), Row.Slot), Row.Name.ToString(), Expected);
	}
	TestEqual(TEXT("table: three rows"), Rows.Num(), 3);
	TestEqual(TEXT("banner: slot 1"), HOLForumMessages::GetPlayerName(Config, 1, INDEX_NONE).ToString(), Hard2);
	TestEqual(TEXT("banner: slot 3"), HOLForumMessages::GetPlayerName(Config, 3, INDEX_NONE).ToString(), Normal4);
	FHOLMatchConfig Named = Config;
	Named.Slots[0].PlayerName = TEXT("Arthur");
	TestEqual(TEXT("banner: a lobby's name kept"), HOLForumMessages::GetPlayerName(Named, 0, INDEX_NONE).ToString(), FString(TEXT("Arthur")));
	TestEqual(TEXT("banner: a human without a name Joueur 1"), HOLForumMessages::GetPlayerName(Config, 0, INDEX_NONE).ToString(), FString(TEXT("Joueur 1")));

	// A scenario's .json (the real MakeScenarioConfig): "Hard" read; missing Normal; unknown Normal, with a warning.
	const auto Read = [this](const TCHAR* Difficulty, FHOLMatchConfig& Out)
	{
		const FString Field = Difficulty ? FString::Printf(TEXT(", \"difficulty\": \"%s\""), Difficulty) : FString();
		const FString Json = FString::Printf(TEXT("{ \"map\": \"/Game/Maps/Plain.Plain\", \"slots\": [ { \"state\": \"Human\", \"colorIndex\": 0 }, ")
			TEXT("{ \"state\": \"AI\", \"colorIndex\": 1%s } ] }"), *Field);
		return HOLTestHooks::MakeScenarioConfig(&Json, Out);
	};
	FHOLMatchConfig FromFile;
	TestTrue(TEXT(".json Hard read"), Read(TEXT("Hard"), FromFile));
	TestTrue(TEXT(".json Hard: slot 1 Hard"), FromFile.Slots[1].Difficulty == EHOLAIDifficulty::Hard);
	TestTrue(TEXT(".json Hard: slot 0 Normal"), FromFile.Slots[0].Difficulty == EHOLAIDifficulty::Normal);
	TestTrue(TEXT(".json without the field read"), Read(nullptr, FromFile));
	TestTrue(TEXT(".json without the field: Normal"), FromFile.Slots[1].Difficulty == EHOLAIDifficulty::Normal);
	AddExpectedMessage(TEXT("unknown AI difficulty 'Brutal'"), EAutomationExpectedMessageFlags::Contains, 1);
	TestTrue(TEXT(".json unknown value read"), Read(TEXT("Brutal"), FromFile));
	TestTrue(TEXT(".json unknown value: Normal"), FromFile.Slots[1].Difficulty == EHOLAIDifficulty::Normal);
	TestTrue(TEXT("round trip keeps the level"), [&Config]()
	{
		FHOLMatchConfig Back;
		return FHOLMatchConfig::FromJson(Config.ToJson(), Back) && Back.Slots[1].Difficulty == EHOLAIDifficulty::Hard;
	}());
	return true;
}

#endif
