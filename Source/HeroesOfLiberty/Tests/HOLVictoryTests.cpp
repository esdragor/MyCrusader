// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLProductionSubsystem.h"
#include "Buildings/HOLRubble.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerState.h"
#include "Match/HOLVictoryRules.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitView.h"

#if WITH_DEV_AUTOMATION_TESTS

// V1.1 checks (D356, D357): who has won, from the slots and teams of the launch and the defeated slots.
namespace UE::HeroesOfLiberty::Tests::Victory
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	struct FOccupied
	{
		EHOLSlotState State;
		int32 Team;
	};

	/** Slots 0, 1, 2... as given, every other slot closed. */
	FHOLMatchConfig MakeConfig(std::initializer_list<FOccupied> Occupied)
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (FHOLMatchSlot& Slot : Config.Slots)
		{
			Slot.State = EHOLSlotState::Closed;
			Slot.Team = 0;
		}
		int32 Index = 0;
		for (const FOccupied& Slot : Occupied)
		{
			Config.Slots[Index].State = Slot.State;
			Config.Slots[Index].Team = Slot.Team;
			++Index;
		}
		return Config;
	}

	uint8 Mask(std::initializer_list<int32> Slots)
	{
		uint8 Result = 0;
		for (const int32 Slot : Slots)
		{
			Result |= 1 << Slot;
		}
		return Result;
	}

	constexpr EHOLSlotState Human = EHOLSlotState::Human;
	constexpr EHOLSlotState AI = EHOLSlotState::AI;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryOneVsOneTest, "HeroesOfLiberty.Victory.Rules.OneVsOne", UE::HeroesOfLiberty::Tests::Victory::Flags)
bool FHOLVictoryOneVsOneTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Victory;
	const FHOLMatchConfig Config = MakeConfig({ { Human, 0 }, { AI, 0 } });

	TestFalse(TEXT("nobody defeated: not over"), HOLVictoryRules::Evaluate(Config, 0).bOver);
	const FHOLVictoryResult Result = HOLVictoryRules::Evaluate(Config, Mask({ 1 }));
	TestTrue(TEXT("slot 1 defeated: over"), Result.bOver);
	TestEqual(TEXT("slot 0 wins"), Result.GetWinners(), TArray<int32>{ 0 });
	TestEqual(TEXT("the other way round"), HOLVictoryRules::Evaluate(Config, Mask({ 0 })).GetWinners(), TArray<int32>{ 1 });
	TestTrue(TEXT("two slots of team 0: two camps"), HOLVictoryRules::GetCamp(Config, 0) != HOLVictoryRules::GetCamp(Config, 1));
	TestEqual(TEXT("a closed slot has no camp"), HOLVictoryRules::GetCamp(Config, 2), static_cast<int32>(INDEX_NONE));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryTeamsTest, "HeroesOfLiberty.Victory.Rules.Teams", UE::HeroesOfLiberty::Tests::Victory::Flags)
bool FHOLVictoryTeamsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Victory;
	// 2v2: slots 0 and 2 in team 1, slots 1 and 3 in team 2.
	const FHOLMatchConfig Config = MakeConfig({ { Human, 1 }, { AI, 2 }, { AI, 1 }, { Human, 2 } });

	TestFalse(TEXT("one of team 2 down: not over"), HOLVictoryRules::Evaluate(Config, Mask({ 1 })).bOver);
	TestFalse(TEXT("one down on each side: not over"), HOLVictoryRules::Evaluate(Config, Mask({ 1, 2 })).bOver);
	const FHOLVictoryResult Result = HOLVictoryRules::Evaluate(Config, Mask({ 1, 2, 3 }));
	TestTrue(TEXT("team 2 all down: over"), Result.bOver);
	TestEqual(TEXT("team 1 wins, its fallen slot 2 too (D357)"), Result.GetWinners(), (TArray<int32>{ 0, 2 }));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryFreeForAllTest, "HeroesOfLiberty.Victory.Rules.FreeForAll", UE::HeroesOfLiberty::Tests::Victory::Flags)
bool FHOLVictoryFreeForAllTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Victory;
	const FHOLMatchConfig Config = MakeConfig({ { Human, 0 }, { AI, 0 }, { AI, 0 } });

	TestFalse(TEXT("one of three down: not over"), HOLVictoryRules::Evaluate(Config, Mask({ 0 })).bOver);
	const FHOLVictoryResult Result = HOLVictoryRules::Evaluate(Config, Mask({ 0, 2 }));
	TestTrue(TEXT("two of three down: over"), Result.bOver);
	TestEqual(TEXT("the last one standing wins"), Result.GetWinners(), TArray<int32>{ 1 });
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictorySingleCampTest, "HeroesOfLiberty.Victory.Rules.SingleCamp", UE::HeroesOfLiberty::Tests::Victory::Flags)
bool FHOLVictorySingleCampTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Victory;
	const FHOLMatchConfig Alone = MakeConfig({ { Human, 0 } });
	TestFalse(TEXT("a human alone: no victory at the launch"), HOLVictoryRules::Evaluate(Alone, 0).bOver);
	const FHOLVictoryResult AloneDown = HOLVictoryRules::Evaluate(Alone, Mask({ 0 }));
	TestTrue(TEXT("a human alone who falls: over"), AloneDown.bOver);
	TestEqual(TEXT("a human alone who falls: nobody wins"), AloneDown.WinnerMask, static_cast<uint8>(0));

	const FHOLMatchConfig OneTeam = MakeConfig({ { Human, 3 }, { AI, 3 }, { Human, 3 } });
	TestFalse(TEXT("everyone in one team: no victory at the launch"), HOLVictoryRules::Evaluate(OneTeam, 0).bOver);
	TestFalse(TEXT("one team, two down: not over"), HOLVictoryRules::Evaluate(OneTeam, Mask({ 0, 1 })).bOver);
	const FHOLVictoryResult OneTeamDown = HOLVictoryRules::Evaluate(OneTeam, Mask({ 0, 1, 2 }));
	TestTrue(TEXT("one team, all down: over"), OneTeamDown.bOver);
	TestEqual(TEXT("one team, all down: nobody wins"), OneTeamDown.WinnerMask, static_cast<uint8>(0));

	TestFalse(TEXT("no occupied slot: never over"), HOLVictoryRules::Evaluate(MakeConfig({}), 0).bOver);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryAllDefeatedTest, "HeroesOfLiberty.Victory.Rules.AllDefeated", UE::HeroesOfLiberty::Tests::Victory::Flags)
bool FHOLVictoryAllDefeatedTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Victory;
	const FHOLMatchConfig Config = MakeConfig({ { Human, 1 }, { AI, 2 }, { AI, 0 } });
	const FHOLVictoryResult Result = HOLVictoryRules::Evaluate(Config, Mask({ 0, 1, 2 }));
	TestTrue(TEXT("everyone defeated: over"), Result.bOver);
	TestEqual(TEXT("nobody wins"), Result.WinnerMask, static_cast<uint8>(0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryGameStateTest, "HeroesOfLiberty.Victory.GameState", UE::HeroesOfLiberty::Tests::Victory::Flags)
bool FHOLVictoryGameStateTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Victory;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	if (TestNotNull(TEXT("game state spawned"), State))
	{
		State->InitVictory(MakeConfig({ { Human, 0 }, { AI, 0 } }));
		int32 Defeats = 0;
		int32 Ends = 0;
		State->OnSlotDefeated.AddLambda([&Defeats](int32, EHOLDefeatReason) { ++Defeats; });
		State->OnMatchOver.AddLambda([&Ends] { ++Ends; });

		TestFalse(TEXT("a closed slot cannot lose"), State->DefeatSlot(4, EHOLDefeatReason::Cheat));
		TestFalse(TEXT("not over at the launch"), State->IsMatchOver());
		TestTrue(TEXT("slot 1 surrenders"), State->DefeatSlot(1, EHOLDefeatReason::Surrender));
		TestTrue(TEXT("slot 1 defeated"), State->IsSlotDefeated(1));
		TestTrue(TEXT("by surrender"), State->GetDefeatReason(1) == EHOLDefeatReason::Surrender);
		TestFalse(TEXT("slot 0 not defeated"), State->IsSlotDefeated(0));
		TestTrue(TEXT("over"), State->IsMatchOver());
		TestEqual(TEXT("slot 0 wins"), State->GetWinners(), TArray<int32>{ 0 });
		TestTrue(TEXT("IsWinner"), State->IsWinner(0) && !State->IsWinner(1));
		TestFalse(TEXT("once over, nobody loses any more"), State->DefeatSlot(0, EHOLDefeatReason::Cheat));
		TestFalse(TEXT("never defeated twice"), State->DefeatSlot(1, EHOLDefeatReason::Cheat));
		TestEqual(TEXT("one defeat told"), Defeats, 1);
		TestEqual(TEXT("one end told"), Ends, 1);
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// V1.2 (D358): the collapse of the defeated. Slot 1 falls: its units at 0 health on the combat's dying path, its buildings
// destroyed as in combat (rubble, none for a bare foundation), its stock at 0, its orders and births refused; slot 0 kept.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryCollapseTest, "HeroesOfLiberty.Victory.Collapse", UE::HeroesOfLiberty::Tests::Victory::Flags)
bool FHOLVictoryCollapseTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Victory;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>();
	AHOLPlayerState* Loser = World->SpawnActor<AHOLPlayerState>();
	AHOLPlayerState* Keeper = World->SpawnActor<AHOLPlayerState>();
	AHOLBuilding* House = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* Foundation = World->SpawnActor<AHOLBuilding>(FVector(3000.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* OtherHouse = World->SpawnActor<AHOLBuilding>(FVector(-3000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("commands"), Commands)
		&& TestNotNull(TEXT("player states"), Loser) && TestNotNull(TEXT("player states"), Keeper)
		&& TestNotNull(TEXT("buildings"), House) && TestNotNull(TEXT("buildings"), Foundation) && TestNotNull(TEXT("buildings"), OtherHouse))
	{
		// Three slots: slot 1 falls, the match goes on.
		State->InitVictory(MakeConfig({ { Human, 0 }, { Human, 0 }, { AI, 0 } }));
		Loser->SetMatchSlot(1, 1);
		Keeper->SetMatchSlot(0, 0);
		const FHOLResources Stock(100, 200, 50, 25);
		Loser->SetResources(Stock);
		Keeper->SetResources(Stock);
		House->Setup(HOLBuildingTypeNames::House, 1, 1);
		Foundation->Setup(HOLBuildingTypeNames::House, 1, 1, EHOLConstructionState::Foundation);
		OtherHouse->Setup(HOLBuildingTypeNames::House, 0, 0);

		const FName Peasant(TEXT("Peasant"));
		FHOLUnitSpawnRequest Request;
		Request.TypeName = Peasant;
		Request.Count = 3;
		Request.Center = FVector(1000.0, 1000.0, 0.0);
		Request.OwnerPlayer = 1;
		Units->RequestUnits(Request);
		Request.Count = 2;
		Request.OwnerPlayer = 0;
		Units->RequestUnits(Request);
		Units->ServeBirthRequests();
		TArray<uint32> Ids;
		Units->GetAllUnitIds(Ids);
		TestEqual(TEXT("5 units born"), Ids.Num(), 5);
		// The population the HUD reads (AHOLPlayerState), written by the production at the start of each step.
		UHOLProductionSubsystem* ProductionSystem = World->GetSubsystem<UHOLProductionSubsystem>();
		UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
		const bool bSteps = TestNotNull(TEXT("production"), ProductionSystem) && TestNotNull(TEXT("simulation"), Simulation);
		if (bSteps)
		{
			ProductionSystem->OnWorldBeginPlay(*World);
			Simulation->GetOnStepStarted().Broadcast(1);
			TestEqual(TEXT("before the defeat, the HUD of slot 1: its 3 units"), Loser->GetPopulationUsed(), 3);
			TestTrue(TEXT("before the defeat, the HUD of slot 1: the places of its house"), Loser->GetPopulationCap() > 0);
		}
		// A birth of slot 1 still waiting at its defeat: dropped.
		Request.OwnerPlayer = 1;
		Request.Count = 4;
		Units->RequestUnits(Request);

		AddExpectedMessage(TEXT("Victory: slot 1 collapsed \\(3 units, 2 buildings\\)"), EAutomationExpectedMessageFlags::Contains, 1);
		TestTrue(TEXT("slot 1 defeated"), State->DefeatSlot(1, EHOLDefeatReason::Cheat));

		TArray<uint32> LoserIds;
		TArray<uint32> KeeperIds;
		for (const uint32 Id : Ids)
		{
			FHOLUnitView View;
			if (Units->GetUnitView(Id, View))
			{
				(View.OwnerPlayer == 1 ? LoserIds : KeeperIds).Add(Id);
				if (View.OwnerPlayer == 1)
				{
					TestEqual(TEXT("a unit of slot 1: 0 health"), View.Health, 0);
					TestTrue(TEXT("a unit of slot 1: dying (the combat's path)"), View.Activity == EHOLUnitActivity::Dying);
				}
				else
				{
					TestTrue(TEXT("a unit of slot 0: full health"), View.bFullHealth);
				}
			}
		}
		TestEqual(TEXT("3 units of slot 1 still there, dying (their fall and body to come)"), LoserIds.Num(), 3);
		TestEqual(TEXT("2 units of slot 0"), KeeperIds.Num(), 2);
		TArray<int32> Population;
		Units->GetPopulationByOwner(Population);
		TestEqual(TEXT("slot 1: no population"), Population.IsValidIndex(1) ? Population[1] : 0, 0);
		TestEqual(TEXT("slot 0: its 2 units"), Population.IsValidIndex(0) ? Population[0] : 0, 2);
		TestEqual(TEXT("the birth of slot 1 waiting: dropped"), Units->GetNumPendingBirths(), 0);
		if (bSteps)
		{
			// Seen at the trial of victory_surrender: the HUD of the defeated still read its population of before.
			Simulation->GetOnStepStarted().Broadcast(2);
			TestEqual(TEXT("after the collapse, the HUD of slot 1: no population"), Loser->GetPopulationUsed(), 0);
			TestEqual(TEXT("after the collapse, the HUD of slot 1: no places (no building left)"), Loser->GetPopulationCap(), 0);
			TestEqual(TEXT("after the collapse, the HUD of slot 0: its 2 units"), Keeper->GetPopulationUsed(), 2);
		}

		TestFalse(TEXT("the house of slot 1 destroyed"), IsValid(House));
		TestFalse(TEXT("the foundation of slot 1 destroyed"), IsValid(Foundation));
		TestTrue(TEXT("the house of slot 0 stands"), IsValid(OtherHouse));
		int32 Rubbles = 0;
		for (TActorIterator<AHOLRubble> It(World); It; ++It)
		{
			++Rubbles;
		}
		TestEqual(TEXT("rubble of the house only (none for a bare foundation)"), Rubbles, 1);

		TestTrue(TEXT("slot 1: stock lost"), Loser->GetResources() == FHOLResources());
		TestTrue(TEXT("slot 0: stock kept"), Keeper->GetResources() == Stock);

		// Nothing more for slot 1 (refused at the queue); slot 0 still obeyed.
		Commands->IssueMoveOrder(LoserIds, FVector(500.0, 0.0, 0.0), 1);
		TestEqual(TEXT("a move order of slot 1 refused"), Commands->GetNumPendingOrders(), 0);
		Commands->IssueCombatOrder(LoserIds, EHOLOrderKind::Stop, 0, 1);
		TestEqual(TEXT("a stop of slot 1 refused"), Commands->GetNumPendingOrders(), 0);
		Commands->IssueMoveOrder(KeeperIds, FVector(500.0, 0.0, 0.0), 0);
		TestEqual(TEXT("a move order of slot 0 queued"), Commands->GetNumPendingOrders(), 1);
		FHOLProductionCommand Production;
		Production.PlayerIndex = 1;
		Production.Building = OtherHouse;
		Production.TypeName = Peasant;
		Commands->IssueProductionOrder(Production);
		TArray<FHOLProductionCommand> Productions;
		Commands->ConsumeProductionOrders(Productions);
		TestEqual(TEXT("a production of slot 1 refused"), Productions.Num(), 0);
		FHOLConstructionCommand Construction;
		Construction.PlayerIndex = 1;
		Commands->IssueConstructionOrder(Construction);
		TArray<FHOLConstructionCommand> Constructions;
		Commands->ConsumeConstructionOrders(Constructions);
		TestEqual(TEXT("a construction of slot 1 refused"), Constructions.Num(), 0);
		Commands->IssueGatherOrder(LoserIds, 0, 1);
		TArray<FHOLGatherOrder> Gathers;
		Commands->ConsumeGatherOrders(Gathers);
		TestEqual(TEXT("a gather order of slot 1 refused"), Gathers.Num(), 0);
		Request.OwnerPlayer = 1;
		Units->RequestUnits(Request);
		TestEqual(TEXT("a birth of slot 1 refused"), Units->GetNumPendingBirths(), 0);
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

#endif
