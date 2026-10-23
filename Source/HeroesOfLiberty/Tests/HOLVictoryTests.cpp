// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Buildings/HOLProductionSubsystem.h"
#include "Buildings/HOLRubble.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Engine/Engine.h"
#include "Commands/HOLConstructionCommand.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Match/HOLForumRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerState.h"
#include "Match/HOLVictoryRules.h"
#include "Misc/AutomationTest.h"
#include "Components/StaticMeshComponent.h"
#include "HOLCollision.h"
#include "Player/HOLCommandGrid.h"
#include "Presentation/HOLUnitAnimation.h"
#include "Buildings/HOLConstructionRules.h"
#include "Simulation/HOLBuildProcessor.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitTypes.h"
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

// V2.1 (D349, D352): the main forum's lives and phases, pure.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryForumRulesTest, "HeroesOfLiberty.Victory.Forum.Rules", UE::HeroesOfLiberty::Tests::Victory::Flags)
bool FHOLVictoryForumRulesTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("tier 0, no fall: 2 lives (D349)"), HOLForumRules::GetLives(0, 0), 2);
	TestEqual(TEXT("tier 0, one fall: 1 life"), HOLForumRules::GetLives(0, 1), 1);
	TestEqual(TEXT("tier 0, two falls: 0 life"), HOLForumRules::GetLives(0, 2), 0);
	TestEqual(TEXT("never below 0"), HOLForumRules::GetLives(0, 3), 0);
	TestEqual(TEXT("tier 1, no fall: 3 lives"), HOLForumRules::GetLives(1, 0), 3);

	// The game's values (DefaultGame.ini): 2 lives, ruin ~30 s, rebuilding alone ~120 s.
	const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
	const FHOLForumTuning Game = FHOLForumTuning::FromSettings();
	TestEqual(TEXT("the game's tier: 0"), Game.Tier, 0);
	TestEqual(TEXT("the game's ruin: 30 s"), Game.RuinSteps * StepSeconds, 30.f, 0.1f);
	TestEqual(TEXT("the game's rebuilding alone: 120 s"), Game.RebuildSteps * StepSeconds, 120.f, 0.1f);
	TestEqual(TEXT("the game's builder exponent (D312)"), Game.BuilderExponent, 0.63f, 0.001f);
	// D312 calibrated so that one Peasant does as much as the rebuilding alone (V2-Q2).
	const auto Seconds = [&Game, StepSeconds](const int32 Builders) { return HOLForumRules::GetStepsToRebuild(Game, Builders) * StepSeconds; };
	TestEqual(TEXT("0 Peasant: 120 s"), Seconds(0), 120.f, 0.1f);
	TestEqual(TEXT("1 Peasant: 60 s"), Seconds(1), 60.f, 0.1f);
	TestEqual(TEXT("5 Peasants: ~32 s"), Seconds(5), 32.f, 0.5f);
	TestEqual(TEXT("10 Peasants: ~23 s"), Seconds(10), 23.f, 0.5f);
	TestEqual(TEXT("40 Peasants: ~11 s"), Seconds(40), 11.f, 0.5f);

	FHOLForumTuning Tuning;
	Tuning.RuinSteps = 3;
	Tuning.RebuildSteps = 4;
	FHOLForumState State;
	TestFalse(TEXT("standing: not down"), State.IsDown());
	TestEqual(TEXT("standing: progress 1"), HOLForumRules::GetRebuildProgress(State, Tuning), 1.f);
	TestFalse(TEXT("standing: nothing to advance"), HOLForumRules::Advance(State, Tuning, 5));
	TestEqual(TEXT("first fall: 1 life left"), HOLForumRules::Fall(State, Tuning), 1);
	TestTrue(TEXT("first fall: ruin"), State.Phase == EHOLForumPhase::Ruin);
	TestEqual(TEXT("a fall in ruin refused (unbreakable)"), HOLForumRules::Fall(State, Tuning), static_cast<int32>(INDEX_NONE));
	TestEqual(TEXT("still one fall"), static_cast<int32>(State.Falls), 1);
	TestFalse(TEXT("ruin step 1"), HOLForumRules::Advance(State, Tuning, 40));
	TestFalse(TEXT("ruin step 2"), HOLForumRules::Advance(State, Tuning, 40));
	TestEqual(TEXT("the Peasants build nothing in the ruin"), State.RebuildWork, static_cast<int64>(0));
	TestTrue(TEXT("ruin step 3: rebuilding"), HOLForumRules::Advance(State, Tuning, 0) && State.Phase == EHOLForumPhase::Rebuilding);
	TestEqual(TEXT("a fall in rebuilding refused (unbreakable)"), HOLForumRules::Fall(State, Tuning), static_cast<int32>(INDEX_NONE));
	TestFalse(TEXT("rebuilding step 1"), HOLForumRules::Advance(State, Tuning, 0));
	TestEqual(TEXT("rebuilding: a quarter done"), HOLForumRules::GetRebuildProgress(State, Tuning), 0.25f, 0.001f);
	// One Peasant doubles the rate: the 3 steps of work left take 2 steps (1.5).
	TestFalse(TEXT("rebuilding step 2, one Peasant"), HOLForumRules::Advance(State, Tuning, 1));
	TestTrue(TEXT("rebuilding step 3, one Peasant: standing"), HOLForumRules::Advance(State, Tuning, 1) && State.Phase == EHOLForumPhase::Standing);
	TestEqual(TEXT("rebuilt: lives kept"), HOLForumRules::GetLives(State, Tuning), 1);
	TestEqual(TEXT("second fall: no life left, the defeat"), HOLForumRules::Fall(State, Tuning), 0);

	FHOLForumState Skipped;
	TestFalse(TEXT("skip: nothing when standing"), HOLForumRules::Skip(Skipped));
	HOLForumRules::Fall(Skipped, Tuning);
	TestTrue(TEXT("skip: ruin -> rebuilding"), HOLForumRules::Skip(Skipped) && Skipped.Phase == EHOLForumPhase::Rebuilding);
	TestTrue(TEXT("skip: rebuilding -> standing"), HOLForumRules::Skip(Skipped) && Skipped.Phase == EHOLForumPhase::Standing);
	return true;
}

// V2.1 (D349-D352) on the server's chain: slot 0's main forum hit to 0 health by DamageBuilding falls into ruin (a life
// less, kept in the world, queue refunded, rally kept, unbreakable, inert, population kept), rises again at the pace of the
// simulation's steps with all its health, and its second fall defeats slot 0 at the end of the step (V1's collapse).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryForumFallTest, "HeroesOfLiberty.Victory.Forum.Fall", UE::HeroesOfLiberty::Tests::Victory::Flags)
bool FHOLVictoryForumFallTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Victory;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>();
	UHOLProductionSubsystem* ProductionSystem = World->GetSubsystem<UHOLProductionSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	AHOLPlayerState* Owner = World->SpawnActor<AHOLPlayerState>();
	AHOLPlayerState* Enemy = World->SpawnActor<AHOLPlayerState>();
	AHOLBuilding* Forum = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* Secondary = World->SpawnActor<AHOLBuilding>(FVector(5000.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* EnemyForum = World->SpawnActor<AHOLBuilding>(FVector(-5000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("commands"), Commands) && TestNotNull(TEXT("production"), ProductionSystem)
		&& TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("player states"), Owner) && TestNotNull(TEXT("player states"), Enemy)
		&& TestNotNull(TEXT("buildings"), Forum) && TestNotNull(TEXT("buildings"), Secondary) && TestNotNull(TEXT("buildings"), EnemyForum))
	{
		State->InitVictory(MakeConfig({ { Human, 0 }, { AI, 0 } }));
		Owner->SetMatchSlot(0, 0);
		Enemy->SetMatchSlot(1, 1);
		const FHOLResources Stock(1000, 1000, 1000, 1000);
		Owner->SetResources(Stock);
		Forum->Setup(HOLBuildingTypeNames::MainCentre, 0, 0);
		Forum->SetPrimaryForum(true);
		Secondary->Setup(HOLBuildingTypeNames::MainCentre, 0, 0);
		EnemyForum->Setup(HOLBuildingTypeNames::MainCentre, 1, 1);
		EnemyForum->SetPrimaryForum(true);
		ProductionSystem->OnWorldBeginPlay(*World);
		const FHOLForumTuning Tuning = FHOLForumTuning::FromSettings();
		int64 Step = 0;
		const auto RunStep = [Simulation, &Step]
		{
			++Step;
			Simulation->GetOnStepStarted().Broadcast(Step);
			Simulation->GetOnStepEnded().Broadcast(Step);
		};

		TestEqual(TEXT("2 lives at the start (D349)"), State->GetLives(0), 2);
		TestTrue(TEXT("the forum stands"), State->GetForumState(0).Phase == EHOLForumPhase::Standing);
		TestTrue(TEXT("the forum takes deliveries"), Forum->TakesDeliveries());

		// A secondary centre has no life: destroyed as any building.
		int32 Hits = 0;
		while (IsValid(Secondary) && !Secondary->IsActorBeingDestroyed() && Secondary->GetHealth() > 0 && Hits++ < 1000)
		{
			UHOLConstructionSubsystem::DamageBuilding(*Secondary, 1000, EHOLAttackType::Melee);
		}
		TestTrue(TEXT("a secondary centre at 0 health: destroyed"), !IsValid(Secondary) || Secondary->IsActorBeingDestroyed());
		TestEqual(TEXT("a secondary centre's destruction costs no life"), State->GetLives(0), 2);
		int32 RubblesBefore = 0;
		for (TActorIterator<AHOLRubble> It(World); It; ++It)
		{
			++RubblesBefore;
		}

		// Two Peasants queued (paid), a rally point.
		const FName Peasant(TEXT("Peasant"));
		FHOLProductionCommand Production;
		Production.PlayerIndex = 0;
		Production.Building = Forum;
		Production.TypeName = Peasant;
		Commands->IssueProductionOrder(Production);
		Commands->IssueProductionOrder(Production);
		RunStep();
		TestEqual(TEXT("two Peasants queued"), Forum->GetProductionQueue().Num(), 2);
		TestFalse(TEXT("the queue paid"), Owner->GetResources() == Stock);
		const FVector Rally(800.0, 600.0, 0.0);
		Forum->SetRallyPoint(Rally);
		const int32 Cap = Owner->GetPopulationCap();
		TestTrue(TEXT("the forum gives places"), Cap > 0);

		AddExpectedMessage(TEXT("Victory: forum of slot 0 fell, lives 1"), EAutomationExpectedMessageFlags::Contains, 1);
		Hits = 0;
		while (Forum->GetHealth() > 0 && Hits++ < 1000)
		{
			UHOLConstructionSubsystem::DamageBuilding(*Forum, 500, EHOLAttackType::Melee);
		}
		TestTrue(TEXT("the forum at 0 health: still in the world"), IsValid(Forum) && !Forum->IsActorBeingDestroyed());
		TestEqual(TEXT("a life less"), State->GetLives(0), 1);
		TestTrue(TEXT("in ruin"), State->GetForumState(0).Phase == EHOLForumPhase::Ruin);
		TestTrue(TEXT("the forum down"), State->IsForumDown(0) && Forum->IsForumDown());
		TestEqual(TEXT("the queue emptied"), Forum->GetProductionQueue().Num(), 0);
		TestTrue(TEXT("the queue refunded in full"), Owner->GetResources() == Stock);
		TestTrue(TEXT("the rally point kept"), Forum->HasRallyPoint() && Forum->GetRallyPoint().Equals(Rally, 1.0));
		TestFalse(TEXT("no deposit in ruin"), Forum->TakesDeliveries());
		TestEqual(TEXT("unbreakable in ruin"), UHOLConstructionSubsystem::DamageBuilding(*Forum, 500, EHOLAttackType::Melee), 0);
		int32 RubblesAfter = 0;
		for (TActorIterator<AHOLRubble> It(World); It; ++It)
		{
			++RubblesAfter;
		}
		TestEqual(TEXT("no rubble of a destruction"), RubblesAfter, RubblesBefore);
		TestFalse(TEXT("not defeated"), State->IsSlotDefeated(0));
		TestEqual(TEXT("the enemy's lives untouched"), State->GetLives(1), 2);

		// No production in ruin; the places kept (D351).
		Commands->IssueProductionOrder(Production);
		RunStep();
		TestEqual(TEXT("no production in ruin"), Forum->GetProductionQueue().Num(), 0);
		TestTrue(TEXT("nothing paid in ruin"), Owner->GetResources() == Stock);
		TestEqual(TEXT("the places kept in ruin"), Owner->GetPopulationCap(), Cap);

		// The ruin lasts its steps (the one just run included), then the rebuilding alone, then it stands with all its health.
		AddExpectedMessage(TEXT("Victory: forum of slot 0 ruin -> rebuilding"), EAutomationExpectedMessageFlags::Contains, 1);
		int32 RuinSteps = 1;
		while (State->GetForumState(0).Phase == EHOLForumPhase::Ruin && RuinSteps < 100000)
		{
			RunStep();
			++RuinSteps;
		}
		TestEqual(TEXT("the ruin's steps (~30 s)"), RuinSteps, Tuning.RuinSteps);
		TestTrue(TEXT("rebuilding"), State->GetForumState(0).Phase == EHOLForumPhase::Rebuilding);
		TestEqual(TEXT("unbreakable while rebuilding"), UHOLConstructionSubsystem::DamageBuilding(*Forum, 500, EHOLAttackType::Melee), 0);
		TestFalse(TEXT("no deposit while rebuilding"), Forum->TakesDeliveries());
		Commands->IssueProductionOrder(Production);
		RunStep();
		TestEqual(TEXT("no production while rebuilding"), Forum->GetProductionQueue().Num(), 0);
		TestTrue(TEXT("rebuilding: some progress"), State->GetForumRebuildProgress(0) > 0.f && State->GetForumRebuildProgress(0) < 1.f);
		AddExpectedMessage(TEXT("Victory: forum of slot 0 rebuilt"), EAutomationExpectedMessageFlags::Contains, 1);
		int32 RebuildSteps = 1;
		while (State->GetForumState(0).Phase == EHOLForumPhase::Rebuilding && RebuildSteps < 100000)
		{
			RunStep();
			++RebuildSteps;
		}
		TestEqual(TEXT("the rebuilding alone's steps (~120 s)"), RebuildSteps, HOLForumRules::GetStepsToRebuild(Tuning, 0));
		TestTrue(TEXT("standing again"), State->GetForumState(0).Phase == EHOLForumPhase::Standing);
		TestEqual(TEXT("standing again: its block"), Forum->GetForumBlock(false).Shown, FString(TEXT(", lives 1")));
		TestEqual(TEXT("all its health back"), Forum->GetHealth(), Forum->GetMaxHealth());
		TestTrue(TEXT("deposit again"), Forum->TakesDeliveries());
		TestEqual(TEXT("still 1 life"), State->GetLives(0), 1);
		Commands->IssueProductionOrder(Production);
		RunStep();
		TestEqual(TEXT("production again"), Forum->GetProductionQueue().Num(), 1);

		// The second fall: no life left, the defeat at the end of the step (V1's collapse), the enemy wins.
		AddExpectedMessage(TEXT("Victory: forum of slot 0 fell, lives 0"), EAutomationExpectedMessageFlags::Contains, 1);
		AddExpectedMessage(TEXT("Victory: slot 0 defeated \\(Forum\\)"), EAutomationExpectedMessageFlags::Contains, 1);
		Hits = 0;
		while (IsValid(Forum) && Forum->GetHealth() > 0 && Hits++ < 1000)
		{
			UHOLConstructionSubsystem::DamageBuilding(*Forum, 500, EHOLAttackType::Melee);
		}
		TestEqual(TEXT("second fall: 0 life"), State->GetLives(0), 0);
		TestFalse(TEXT("the defeat waits for the end of the step"), State->IsSlotDefeated(0));
		Simulation->GetOnStepEnded().Broadcast(Step);
		TestTrue(TEXT("slot 0 defeated"), State->IsSlotDefeated(0));
		TestTrue(TEXT("by its forum"), State->GetDefeatReason(0) == EHOLDefeatReason::Forum);
		TestTrue(TEXT("the forum destroyed by the collapse"), !IsValid(Forum) || Forum->IsActorBeingDestroyed());
		TestTrue(TEXT("the enemy wins"), State->IsMatchOver() && State->IsWinner(1));
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// V2.2 (D352, V2-Q2 "as a site") on the server's chain: the build order of slot 0's Peasants on its main forum down goes
// through the command queue and the builders' processor (UHOLBuildProcessor, as on a site); the Peasants, born at contact
// with its body (no walk: the test world runs no movement), wait at their places through the ruin, then their number at
// work speeds the rebuilding up (D312); rebuilt, they stop as at the end of a site. Slot 1's Peasant is refused; on the
// forum standing, nothing changes.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryForumBuildersTest, "HeroesOfLiberty.Victory.Forum.Builders", UE::HeroesOfLiberty::Tests::Victory::Flags)
bool FHOLVictoryForumBuildersTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Victory;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>();
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	AHOLPlayerState* Owner = World->SpawnActor<AHOLPlayerState>();
	AHOLPlayerState* Enemy = World->SpawnActor<AHOLPlayerState>();
	AHOLBuilding* Forum = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("commands"), Commands) && TestNotNull(TEXT("units"), Units)
		&& TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("entities"), EntitySubsystem) && TestNotNull(TEXT("player states"), Owner)
		&& TestNotNull(TEXT("player states"), Enemy) && TestNotNull(TEXT("forum"), Forum))
	{
		State->InitVictory(MakeConfig({ { Human, 0 }, { AI, 1 } }));
		Owner->SetMatchSlot(0, 0);
		Enemy->SetMatchSlot(1, 1);
		Forum->Setup(HOLBuildingTypeNames::MainCentre, 0, 0);
		Forum->SetPrimaryForum(true);
		const FHOLForumTuning Tuning = FHOLForumTuning::FromSettings();

		// Two Peasants of slot 0 at contact with the forum's body (east and west), one of slot 1 (north).
		const FBox2D Body = Forum->GetBodyBounds();
		const FVector2D Middle = Body.GetCenter();
		FHOLUnitSpawnRequest Request;
		Request.TypeName = FName(TEXT("Peasant"));
		Request.Count = 1;
		Request.OwnerPlayer = 0;
		Request.Center = FVector(Body.Max.X + 70.0, Middle.Y, 0.0);
		Units->RequestUnits(Request);
		Request.Center = FVector(Body.Min.X - 70.0, Middle.Y, 0.0);
		Units->RequestUnits(Request);
		Request.OwnerPlayer = 1;
		Request.Center = FVector(Middle.X, Body.Max.Y + 70.0, 0.0);
		Units->RequestUnits(Request);
		Units->ServeBirthRequests();
		TArray<uint32> Ids;
		Units->GetAllUnitIds(Ids);
		TArray<uint32> OwnIds;
		TArray<uint32> EnemyIds;
		for (const uint32 Id : Ids)
		{
			FHOLUnitView View;
			if (Units->GetUnitView(Id, View))
			{
				(View.OwnerPlayer == 0 ? OwnIds : EnemyIds).Add(Id);
				TestTrue(TEXT("a Peasant born at contact with the forum"), FMath::Abs(HOLConstruction::GetDistance(Body, FVector2D(View.Location)) - 70.0) < 5.0);
			}
		}
		TestEqual(TEXT("2 Peasants of slot 0"), OwnIds.Num(), 2);
		TestEqual(TEXT("1 Peasant of slot 1"), EnemyIds.Num(), 1);

		// The builders' processor of the server's step, run by the test between the step's start (the forums' phases)
		// and its end, as UHOLSimulationSubsystem::RunStep.
		TSharedPtr<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
		UHOLBuildProcessor* Builders = NewObject<UHOLBuildProcessor>(Simulation);
		Builders->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		int64 Step = 0;
		const auto RunStep = [Simulation, Builders, &EntityManager, StepSeconds, &Step]
		{
			++Step;
			Simulation->GetOnStepStarted().Broadcast(Step);
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Builders, ProcessingContext);
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		const auto CountBuilding = [Units](const TArray<uint32>& Of)
		{
			int32 Count = 0;
			for (const uint32 Id : Of)
			{
				FHOLUnitView View;
				Count += Units->GetUnitView(Id, View) && View.Activity == EHOLUnitActivity::Building ? 1 : 0;
			}
			return Count;
		};
		const auto Order = [Commands, Forum](const TArray<uint32>& Of, const int32 Slot)
		{
			FHOLBuildOrder BuildOrder;
			BuildOrder.UnitIds = Of;
			BuildOrder.Building = Forum;
			BuildOrder.PlayerIndex = Slot;
			Commands->IssueBuildOrder(BuildOrder);
		};

		// The forum standing: the order is refused as on any finished building (unchanged).
		Order(OwnIds, 0);
		RunStep();
		RunStep();
		TestEqual(TEXT("standing: nobody builds the forum"), CountBuilding(OwnIds), 0);
		TestEqual(TEXT("standing: no builder at work"), Forum->GetBuildersAtWork(), 0);

		int32 Hits = 0;
		while (Forum->GetHealth() > 0 && Hits++ < 1000)
		{
			UHOLConstructionSubsystem::DamageBuilding(*Forum, 500, EHOLAttackType::Melee);
		}
		TestTrue(TEXT("the forum fell into ruin"), State->GetForumState(0).Phase == EHOLForumPhase::Ruin);
		// V2.3b: its block as the real chain gives it (game state, settings): the whole ruin left, a life less.
		const int32 RuinSeconds = FMath::CeilToInt(Tuning.RuinSteps * Tuning.StepSeconds - UE_KINDA_SMALL_NUMBER);
		const FHOLForumBlock RuinBlock = Forum->GetForumBlock(false);
		TestTrue(TEXT("ruin block: shown, health hidden"), RuinBlock.bShown && RuinBlock.bHideHealth);
		TestEqual(TEXT("ruin block: described"), RuinBlock.Shown, FString::Printf(TEXT(", in ruin %d s, lives 1"), RuinSeconds));

		// Slot 0's order accepted, slot 1's refused (another player's forum, allies included).
		AddExpectedMessage(TEXT("Build order of slot 0: 2 units to"), EAutomationExpectedMessageFlags::Contains, 1);
		AddExpectedMessage(TEXT("Build order of slot 1 ignored"), EAutomationExpectedMessageFlags::Contains, 1);
		Order(OwnIds, 0);
		Order(EnemyIds, 1);
		int32 RuinSteps = 0;
		int32 MaxAtWorkInRuin = 0;
		bool bProgressInRuin = false;
		while (State->GetForumState(0).Phase == EHOLForumPhase::Ruin && RuinSteps < 100000)
		{
			RunStep();
			++RuinSteps;
			// The step that turns it to rebuilding counts them at once.
			if (State->GetForumState(0).Phase == EHOLForumPhase::Ruin)
			{
				MaxAtWorkInRuin = FMath::Max(MaxAtWorkInRuin, Forum->GetBuildersAtWork());
				bProgressInRuin |= State->GetForumRebuildProgress(0) > 0.f;
			}
			if (RuinSteps == 10)
			{
				TestEqual(TEXT("in ruin: slot 0's Peasants at their places"), CountBuilding(OwnIds), 2);
				TestEqual(TEXT("slot 1's Peasant refused"), CountBuilding(EnemyIds), 0);
			}
		}
		TestEqual(TEXT("in ruin: no builder at work (they wait)"), MaxAtWorkInRuin, 0);
		TestFalse(TEXT("in ruin: no progress"), bProgressInRuin);
		TestTrue(TEXT("rebuilding"), State->GetForumState(0).Phase == EHOLForumPhase::Rebuilding);

		// The rebuilding with 2 Peasants at work: D312's pace (1 + 2^0.63 times the rebuilding alone).
		int32 RebuildSteps = 0;
		int32 MaxAtWork = 0;
		while (State->GetForumState(0).Phase == EHOLForumPhase::Rebuilding && RebuildSteps < 100000)
		{
			RunStep();
			++RebuildSteps;
			MaxAtWork = FMath::Max(MaxAtWork, Forum->GetBuildersAtWork());
			if (RebuildSteps == 10)
			{
				// V2.3b: its owner sees its 2 builders, another player only its progress.
				const int32 Percent = HOLForumRules::GetRebuildPercent(State->GetForumState(0), Tuning);
				TestEqual(TEXT("rebuilding block, own"), Forum->GetForumBlock(true).Shown, FString::Printf(TEXT(", rebuilding %d%%, builders 2, lives 1"), Percent));
				TestEqual(TEXT("rebuilding block, another's"), Forum->GetForumBlock(false).Shown, FString::Printf(TEXT(", rebuilding %d%%, lives 1"), Percent));
			}
		}
		const int32 Alone = HOLForumRules::GetStepsToRebuild(Tuning, 0);
		const int32 WithTwo = HOLForumRules::GetStepsToRebuild(Tuning, 2);
		TestEqual(TEXT("rebuilding: the 2 Peasants shown at work"), MaxAtWork, 2);
		TestTrue(FString::Printf(TEXT("rebuilding faster than alone (%d steps, alone %d)"), RebuildSteps, Alone), RebuildSteps < Alone);
		TestTrue(FString::Printf(TEXT("rebuilding at 2 Peasants' pace (%d steps, expected %d)"), RebuildSteps, WithTwo), FMath::Abs(RebuildSteps - WithTwo) <= 3);
		TestTrue(TEXT("standing again"), State->GetForumState(0).Phase == EHOLForumPhase::Standing);

		// Rebuilt: they stop as at the end of a site (no site nor source near: idle), no builder shown.
		RunStep();
		RunStep();
		TestEqual(TEXT("rebuilt: nobody builds any more"), CountBuilding(OwnIds), 0);
		TestEqual(TEXT("rebuilt: no builder at work shown"), Forum->GetBuildersAtWork(), 0);
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// V2.3b, pure: a main forum's selection block (D353: its lives public), standing, in ruin, rising again, its builders
// shown to its owner alone; a secondary centre unchanged.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryForumBlockTest, "HeroesOfLiberty.Victory.Forum.Block", UE::HeroesOfLiberty::Tests::Victory::Flags)
bool FHOLVictoryForumBlockTest::RunTest(const FString& Parameters)
{
	FHOLForumTuning Tuning;
	Tuning.Tier = 0;
	Tuning.RuinSteps = 300;
	Tuning.RebuildSteps = 1200;
	Tuning.StepSeconds = 0.1f;

	// A secondary centre (no forum state): nothing changes.
	const FHOLForumBlock Secondary = HOLForumRules::GetSelectionBlock(nullptr, Tuning, true, 3);
	TestFalse(TEXT("secondary centre: no forum block"), Secondary.bShown);
	TestFalse(TEXT("secondary centre: health kept"), Secondary.bHideHealth);
	TestEqual(TEXT("secondary centre: nothing described"), Secondary.Shown, FString());

	// Standing: its health as before, its lives under it, to anyone.
	FHOLForumState State;
	for (const bool bOwn : { true, false })
	{
		const FHOLForumBlock Standing = HOLForumRules::GetSelectionBlock(&State, Tuning, bOwn, 0);
		TestTrue(TEXT("standing: shown"), Standing.bShown);
		TestFalse(TEXT("standing: health kept"), Standing.bHideHealth);
		TestEqual(TEXT("standing: described"), Standing.Shown, FString(TEXT(", lives 2")));
		// V2.3d: its lives on a row of their own (label and value as the health's), no line of text.
		TestEqual(TEXT("standing: its lives"), Standing.Lives, 2);
		TestTrue(TEXT("standing: no line"), Standing.Text.IsEmpty());
	}

	// In ruin, 25 s left (24.1 s rounds up): the seconds in place of the health, then its lives.
	TestEqual(TEXT("fell: 1 life left"), HOLForumRules::Fall(State, Tuning), 1);
	State.RuinStepsLeft = 250;
	TestEqual(TEXT("ruin: 25 s left"), HOLForumRules::GetRuinSecondsLeft(State, Tuning), 25);
	State.RuinStepsLeft = 241;
	TestEqual(TEXT("ruin: 24.1 s shows 25 s"), HOLForumRules::GetRuinSecondsLeft(State, Tuning), 25);
	for (const bool bOwn : { true, false })
	{
		const FHOLForumBlock Ruin = HOLForumRules::GetSelectionBlock(&State, Tuning, bOwn, 2);
		TestTrue(TEXT("ruin: shown"), Ruin.bShown);
		TestTrue(TEXT("ruin: health hidden"), Ruin.bHideHealth);
		TestEqual(TEXT("ruin: described"), Ruin.Shown, FString(TEXT(", in ruin 25 s, lives 1")));
		TestEqual(TEXT("ruin: its lines"), Ruin.Text.BuildSourceString(), FString(TEXT("En ruine \u2014 25 s")));
		TestEqual(TEXT("ruin: its lives"), Ruin.Lives, 1);
	}

	// Rising again at 40 %: its builders to its owner alone (as a site's, D313).
	State.Phase = EHOLForumPhase::Rebuilding;
	State.RuinStepsLeft = 0;
	State.RebuildWork = HOLForumRules::GetRebuildTotalWork(Tuning) * 40 / 100;
	TestEqual(TEXT("rebuilding: 40 %"), HOLForumRules::GetRebuildPercent(State, Tuning), 40);
	TestEqual(TEXT("rebuilding: no ruin left"), HOLForumRules::GetRuinSecondsLeft(State, Tuning), 0);
	const FHOLForumBlock OwnRebuilding = HOLForumRules::GetSelectionBlock(&State, Tuning, true, 3);
	TestTrue(TEXT("rebuilding: health hidden"), OwnRebuilding.bHideHealth);
	TestEqual(TEXT("rebuilding, own: described"), OwnRebuilding.Shown, FString(TEXT(", rebuilding 40%, builders 3, lives 1")));
	TestEqual(TEXT("rebuilding, own: its lines"), OwnRebuilding.Text.BuildSourceString(),
		FString(TEXT("Reconstruction \u2014 40 %\nB\u00e2tisseurs : 3")));
	TestEqual(TEXT("rebuilding, own: its lives"), OwnRebuilding.Lives, 1);
	const FHOLForumBlock OtherRebuilding = HOLForumRules::GetSelectionBlock(&State, Tuning, false, 3);
	TestTrue(TEXT("rebuilding, another's: health hidden"), OtherRebuilding.bHideHealth);
	TestEqual(TEXT("rebuilding, another's: described"), OtherRebuilding.Shown, FString(TEXT(", rebuilding 40%, lives 1")));
	TestEqual(TEXT("rebuilding, another's: no builders"), OtherRebuilding.Text.BuildSourceString(),
		FString(TEXT("Reconstruction \u2014 40 %")));
	TestEqual(TEXT("rebuilding, another's: its lives"), OtherRebuilding.Lives, 1);
	return true;
}

// V2.3a (V2-Q1), pure: what a main forum shows in each phase (itself, rubble, a site at the rebuilding's progress), its
// Peasants standing without a hammer through the ruin only, and its grid empty while down.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryForumLookTest, "HeroesOfLiberty.Victory.Forum.Look", UE::HeroesOfLiberty::Tests::Victory::Flags)
bool FHOLVictoryForumLookTest::RunTest(const FString& Parameters)
{
	FHOLForumTuning Tuning;
	Tuning.RuinSteps = 30;
	Tuning.RebuildSteps = 100;
	FHOLForumState State;
	FHOLForumLookChoice Look = HOLForumRules::GetLook(State, Tuning);
	TestTrue(TEXT("standing: itself"), Look.Look == EHOLForumLook::Standing);
	TestEqual(TEXT("standing: whole"), Look.Progress, 1.f);
	TestFalse(TEXT("standing: nobody waits"), HOLForumRules::BuildersWait(State));

	TestEqual(TEXT("the fall"), HOLForumRules::Fall(State, Tuning), 1);
	Look = HOLForumRules::GetLook(State, Tuning);
	TestTrue(TEXT("ruin: rubble"), Look.Look == EHOLForumLook::Rubble);
	TestEqual(TEXT("ruin: nothing risen"), Look.Progress, 0.f);
	TestTrue(TEXT("ruin: its Peasants wait"), HOLForumRules::BuildersWait(State));

	State.Phase = EHOLForumPhase::Rebuilding;
	State.RebuildWork = HOLForumRules::GetRebuildTotalWork(Tuning) / 4;
	Look = HOLForumRules::GetLook(State, Tuning);
	TestTrue(TEXT("rebuilding: a site"), Look.Look == EHOLForumLook::Site);
	TestEqual(TEXT("rebuilding: at its progress"), Look.Progress, 0.25f, 0.001f);
	TestFalse(TEXT("rebuilding: its Peasants strike"), HOLForumRules::BuildersWait(State));

	// The hammer at a site, not at a forum in ruin (standing: the idle sequence), never idle for the HUD (still Building).
	FHOLUnitTypeConstants Peasant;
	Peasant.Speed = 600.f;
	Peasant.IdleSequence = 0;
	Peasant.WalkSequence = 1;
	Peasant.BuildSequence = 11;
	Peasant.BuildPlayRate = 1.f;
	FHOLUnitAnimationInput Input;
	Input.Activity = EHOLUnitActivity::Building;
	TestEqual(TEXT("at a site: the hammer"), HOLUnitAnimation::Choose(Peasant, Input).Sequence, 11);
	Input.bSiteWaits = true;
	TestEqual(TEXT("at a forum in ruin: standing"), HOLUnitAnimation::Choose(Peasant, Input).Sequence, 0);
	TestFalse(TEXT("at a forum in ruin: not turned to strike"), HOLUnitAnimation::Choose(Peasant, Input).bFacesSite);

	// The grid of a forum down: no production, no Annuler.
	FHOLGridContext Grid;
	Grid.bOwnBuilding = true;
	Grid.bBuildingFinished = true;
	Grid.Produces = { FName(TEXT("Peasant")) };
	TestTrue(TEXT("a forum standing produces"), HOLCommandGrid::GetCommand(Grid, 0).Command == EHOLGridCommand::Produce);
	Grid.bForumDown = true;
	TestTrue(TEXT("a forum down: no production"), HOLCommandGrid::GetCommand(Grid, 0).IsNone());
	TestTrue(TEXT("a forum down: nothing to cancel"), HOLCommandGrid::GetCommand(Grid, HOLCommandGrid::CancelCell).IsNone());
	return true;
}

// V2.3a (V2-Q1) on the server's chain: slot 0's main forum hit to 0 health by DamageBuilding shows rubble at its size
// through the ruin (still selectable: its pieces answer the selection's trace), then the look of a site rising with the
// rebuilding's progress (AHOLGameState, not its own work), then itself; a rally point of another of its buildings may be
// set on it while it is down (its Peasants rebuild it).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryForumAppearanceTest, "HeroesOfLiberty.Victory.Forum.Appearance", UE::HeroesOfLiberty::Tests::Victory::Flags)
bool FHOLVictoryForumAppearanceTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Victory;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>();
	UHOLProductionSubsystem* ProductionSystem = World->GetSubsystem<UHOLProductionSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	AHOLPlayerState* Owner = World->SpawnActor<AHOLPlayerState>();
	AHOLPlayerState* Enemy = World->SpawnActor<AHOLPlayerState>();
	AHOLBuilding* Forum = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* Secondary = World->SpawnActor<AHOLBuilding>(FVector(5000.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* EnemyForum = World->SpawnActor<AHOLBuilding>(FVector(-5000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("commands"), Commands) && TestNotNull(TEXT("production"), ProductionSystem)
		&& TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("player states"), Owner) && TestNotNull(TEXT("player states"), Enemy)
		&& TestNotNull(TEXT("buildings"), Forum) && TestNotNull(TEXT("buildings"), Secondary) && TestNotNull(TEXT("buildings"), EnemyForum))
	{
		State->InitVictory(MakeConfig({ { Human, 0 }, { AI, 0 } }));
		Owner->SetMatchSlot(0, 0);
		Enemy->SetMatchSlot(1, 1);
		Owner->SetResources(FHOLResources(1000, 1000, 1000, 1000));
		Forum->Setup(HOLBuildingTypeNames::MainCentre, 0, 0);
		Forum->SetPrimaryForum(true);
		Secondary->Setup(HOLBuildingTypeNames::MainCentre, 0, 0);
		EnemyForum->Setup(HOLBuildingTypeNames::MainCentre, 1, 1);
		EnemyForum->SetPrimaryForum(true);
		ProductionSystem->OnWorldBeginPlay(*World);
		int64 Step = 0;
		const auto RunStep = [Simulation, &Step]
		{
			++Step;
			Simulation->GetOnStepStarted().Broadcast(Step);
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		// What the frame after shows (D325).
		const auto Shown = [Forum]
		{
			Forum->ApplyPendingAppearance();
			return Forum->GetShownLook();
		};
		const auto Selectable = [Forum]
		{
			TArray<UStaticMeshComponent*> Pieces;
			Forum->GetComponents(Pieces);
			return Pieces.ContainsByPredicate([](const UStaticMeshComponent* Piece)
			{
				return Piece->IsRegistered() && Piece->GetCollisionResponseToChannel(HOLCollision::Selection) == ECR_Block;
			});
		};

		TestTrue(TEXT("standing: itself"), Shown() == EHOLForumLook::Standing);
		TestEqual(TEXT("standing: whole"), Forum->GetShownRise(), 1.f);

		AddExpectedMessage(TEXT("Victory: forum of slot 0 fell, lives 1"), EAutomationExpectedMessageFlags::Contains, 1);
		int32 Hits = 0;
		while (Forum->GetHealth() > 0 && Hits++ < 1000)
		{
			UHOLConstructionSubsystem::DamageBuilding(*Forum, 500, EHOLAttackType::Melee);
		}
		TestTrue(TEXT("in ruin"), State->GetForumState(0).Phase == EHOLForumPhase::Ruin);
		TestTrue(TEXT("ruin: rubble"), Shown() == EHOLForumLook::Rubble);
		TestTrue(TEXT("ruin: still selectable"), Selectable());
		TestTrue(TEXT("ruin: its footprint still blocks the navmesh"), Forum->BlocksNavigation());
		RunStep();
		TestTrue(TEXT("ruin: still rubble a step later"), Shown() == EHOLForumLook::Rubble);

		// A rally point of its other centre on it while it is down: kept as on a site.
		FHOLProductionCommand Rally;
		Rally.Kind = FHOLProductionCommand::EKind::Rally;
		Rally.PlayerIndex = 0;
		Rally.Building = Secondary;
		Rally.Location = Forum->GetActorLocation() + FVector(10.0, 0.0, 0.0);
		Rally.RallySite = Forum;
		Commands->IssueProductionOrder(Rally);
		RunStep();
		TestTrue(TEXT("a rally point on the forum down"), Secondary->GetRallySite() == Forum);

		AddExpectedMessage(TEXT("Victory: forum of slot 0 ruin -> rebuilding"), EAutomationExpectedMessageFlags::Contains, 1);
		int32 Guard = 0;
		while (State->GetForumState(0).Phase == EHOLForumPhase::Ruin && Guard++ < 100000)
		{
			RunStep();
		}
		TestTrue(TEXT("rebuilding: a site"), Shown() == EHOLForumLook::Site);
		TestTrue(TEXT("rebuilding: selectable"), Selectable());
		TestTrue(TEXT("rebuilding: low at first"), Forum->GetShownRise() <= AHOLBuilding::MinRiseRatio + AHOLBuilding::RiseStepRatio);
		while (State->GetForumRebuildProgress(0) < 0.5f && Guard++ < 100000)
		{
			RunStep();
		}
		TestTrue(TEXT("rebuilding: still a site"), Shown() == EHOLForumLook::Site);
		TestEqual(TEXT("rebuilding: risen at the rebuilding's progress"), Forum->GetShownRise(), State->GetForumRebuildProgress(0),
			AHOLBuilding::RiseStepRatio + 0.001f);

		AddExpectedMessage(TEXT("Victory: forum of slot 0 rebuilt"), EAutomationExpectedMessageFlags::Contains, 1);
		while (State->GetForumState(0).Phase == EHOLForumPhase::Rebuilding && Guard++ < 100000)
		{
			RunStep();
		}
		TestTrue(TEXT("rebuilt: itself"), Shown() == EHOLForumLook::Standing);
		TestEqual(TEXT("rebuilt: whole"), Forum->GetShownRise(), 1.f);
		EnemyForum->ApplyPendingAppearance();
		TestTrue(TEXT("the enemy's forum untouched"), EnemyForum->GetShownLook() == EHOLForumLook::Standing);
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// V2.4 (V2-Q4), pure: the players' table, top right (D353: every slot's lives and forum public): a row per occupied slot,
// grouped by team, Vous on one's own, the forum's phase, a defeated slot greyed.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryPlayersTableTest, "HeroesOfLiberty.Victory.Forum.PlayersTable", UE::HeroesOfLiberty::Tests::Victory::Flags)
bool FHOLVictoryPlayersTableTest::RunTest(const FString& Parameters)
{
	FHOLForumTuning Tuning;
	Tuning.Tier = 0;
	Tuning.RuinSteps = 300;
	Tuning.RebuildSteps = 1200;
	const auto Describe = [](const TArray<FHOLPlayersTableRow>& Rows)
	{
		return FString::JoinBy(Rows, TEXT(" | "), [](const FHOLPlayersTableRow& Row) { return Row.Shown; });
	};

	// Two players, no team, slot 0 local: slot order, Vous then Joueur 2, 2 lives each, both standing.
	FHOLMatchConfig Duel;
	Duel.Normalize();
	Duel.Slots[0].State = EHOLSlotState::Human;
	Duel.Slots[0].ColorIndex = 3;
	Duel.Slots[1].State = EHOLSlotState::AI;
	Duel.Slots[1].ColorIndex = 5;
	TArray<FHOLForumState> Forums;
	Forums.Init(FHOLForumState(), FHOLMatchConfig::MaxSlots);
	TArray<FHOLPlayersTableRow> Rows = HOLForumRules::GetPlayersTable(Duel, Forums, 0, 0, Tuning);
	TestEqual(TEXT("duel: two rows"), Rows.Num(), 2);
	TestEqual(TEXT("duel: described"), Describe(Rows), FString(TEXT("slot 0 you colour 3 lives 2 standing | slot 1 colour 5 lives 2 standing")));
	if (Rows.Num() == 2)
	{
		TestEqual(TEXT("duel: Vous"), Rows[0].Name.BuildSourceString(), FString(TEXT("Vous")));
		TestEqual(TEXT("duel: Joueur 2"), Rows[1].Name.BuildSourceString(), FString(TEXT("Joueur 2")));
		TestEqual(TEXT("duel: the lives"), Rows[0].Lives.BuildSourceString(), FString(TEXT("2 vies")));
		TestTrue(TEXT("duel: nothing on a standing forum"), Rows[0].State.IsEmpty());
		TestFalse(TEXT("duel: not greyed"), Rows[1].bGreyed);
		TestEqual(TEXT("duel: its colour"), Rows[1].ColorIndex, 5);
	}
	// Seen from slot 1 (a client): Vous moves.
	Rows = HOLForumRules::GetPlayersTable(Duel, Forums, 0, 1, Tuning);
	TestEqual(TEXT("duel from slot 1: described"), Describe(Rows), FString(TEXT("slot 0 colour 3 lives 2 standing | slot 1 you colour 5 lives 2 standing")));

	// Four players in two teams, slots 0 and 2 closed between them: grouped by team, closed slots absent. Slot 1's forum in
	// ruin (1 life), slot 3's rising again at 40 %, slot 5 defeated (greyed, Vaincu).
	FHOLMatchConfig Teams;
	Teams.Normalize();
	const int32 Occupied[] = { 1, 3, 4, 5 };
	const int32 TeamOf[] = { 2, 1, 2, 1 };
	for (int32 Index = 0; Index < 4; ++Index)
	{
		FHOLMatchSlot& Slot = Teams.Slots[Occupied[Index]];
		Slot.State = Index == 0 ? EHOLSlotState::Human : EHOLSlotState::AI;
		Slot.ColorIndex = Occupied[Index];
		Slot.Team = TeamOf[Index];
	}
	TestEqual(TEXT("ruin: one life left"), HOLForumRules::Fall(Forums[1], Tuning), 1);
	TestEqual(TEXT("rebuilding: one life left"), HOLForumRules::Fall(Forums[3], Tuning), 1);
	Forums[3].Phase = EHOLForumPhase::Rebuilding;
	Forums[3].RuinStepsLeft = 0;
	Forums[3].RebuildWork = HOLForumRules::GetRebuildTotalWork(Tuning) * 40 / 100;
	Forums[5].Falls = 2;
	Rows = HOLForumRules::GetPlayersTable(Teams, Forums, 1 << 5, 1, Tuning);
	TestEqual(TEXT("teams: four rows"), Rows.Num(), 4);
	TestEqual(TEXT("teams: grouped by team, slot order within"), Describe(Rows), FString(TEXT(
		"slot 3 team 1 colour 3 lives 1 rebuilding 40% | slot 5 team 1 colour 5 lives 0 defeated greyed"
		" | slot 1 you team 2 colour 1 lives 1 ruin | slot 4 team 2 colour 4 lives 2 standing")));
	if (Rows.Num() == 4)
	{
		TestEqual(TEXT("teams: Reconstruction 40 %"), Rows[0].State.BuildSourceString(), FString(TEXT("Reconstruction 40 %")));
		TestEqual(TEXT("teams: one life"), Rows[0].Lives.BuildSourceString(), FString(TEXT("1 vie")));
		TestEqual(TEXT("teams: Vaincu"), Rows[1].State.BuildSourceString(), FString(TEXT("Vaincu")));
		TestTrue(TEXT("teams: the defeated greyed"), Rows[1].bGreyed);
		TestEqual(TEXT("teams: the defeated's name"), Rows[1].Name.BuildSourceString(), FString(TEXT("Joueur 6")));
		// V2.3d: a defeated slot's lives no longer shown, "Vaincu" alone.
		TestTrue(TEXT("teams: the defeated's lives not shown"), Rows[1].Lives.IsEmpty());
		TestEqual(TEXT("teams: Ruine"), Rows[2].State.BuildSourceString(), FString(TEXT("Ruine")));
		TestEqual(TEXT("teams: Vous on one's row"), Rows[2].Name.BuildSourceString(), FString(TEXT("Vous")));
		TestTrue(TEXT("teams: a new team starts a group"), Rows[2].bNewGroup);
		TestFalse(TEXT("teams: the first row starts none"), Rows[0].bNewGroup);
		TestFalse(TEXT("teams: same team, same group"), Rows[1].bNewGroup);
	}
	// V2.3d: no life left but not defeated yet: the plural for 0, the singular for exactly 1.
	Rows = HOLForumRules::GetPlayersTable(Teams, Forums, 0, 1, Tuning);
	TestEqual(TEXT("no life left: 0 vies"), Rows.IsValidIndex(1) ? Rows[1].Lives.BuildSourceString() : FString(), FString(TEXT("0 vies")));
	TestEqual(TEXT("one life left: 1 vie"), Rows.IsValidIndex(0) ? Rows[0].Lives.BuildSourceString() : FString(), FString(TEXT("1 vie")));
	// The spectator (slot 1 defeated): its own row Vous, greyed, Vaincu.
	Rows = HOLForumRules::GetPlayersTable(Teams, Forums, (1 << 5) | (1 << 1), 1, Tuning);
	TestEqual(TEXT("spectator: its row"), Rows.IsValidIndex(2) ? Rows[2].Shown : FString(), FString(TEXT("slot 1 you team 2 colour 1 lives 1 defeated greyed")));
	return true;
}

#endif
