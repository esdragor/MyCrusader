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
#include "MassCommonFragments.h"
#include "Match/HOLForumPresence.h"
#include "Match/HOLForumRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerState.h"
#include "Misc/AutomationTest.h"
#include "Components/StaticMeshComponent.h"
#include "HOLCollision.h"
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

	/** V3.2: a unit of Type born for Owner at Location (no walk: the test world runs no movement); its identifier, 0 if none. */
	uint32 PlaceUnit(UHOLUnitSubsystem& Units, const FName Type, const int32 Owner, const FVector& Location)
	{
		TArray<uint32> Before;
		Units.GetAllUnitIds(Before);
		FHOLUnitSpawnRequest Request;
		Request.TypeName = Type;
		Request.Count = 1;
		Request.OwnerPlayer = static_cast<uint8>(Owner);
		Request.Center = Location;
		Units.RequestUnits(Request);
		Units.ServeBirthRequests();
		TArray<uint32> After;
		Units.GetAllUnitIds(After);
		for (const uint32 Id : After)
		{
			if (!Before.Contains(Id))
			{
				return Id;
			}
		}
		return 0;
	}

	/** V3.2: the unit put at Location, as if it had walked there. */
	void MoveUnit(UWorld& World, const UHOLUnitSubsystem& Units, const uint32 Id, const FVector& Location)
	{
		FMassEntityManager& EntityManager = World.GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();
		if (FTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FTransformFragment>(Units.FindEntity(Id)))
		{
			Transform->GetMutableTransform().SetLocation(Location);
		}
	}

	/** V3.2: the unit's ground distance to the body's edge. */
	double DistanceToEdge(const UHOLUnitSubsystem& Units, const uint32 Id, const FBox2D& Body)
	{
		FHOLUnitView View;
		return Units.GetUnitView(Id, View) ? HOLConstruction::GetDistance(Body, FVector2D(View.Location)) : -1.0;
	}

	constexpr EHOLSlotState Human = EHOLSlotState::Human;
	constexpr EHOLSlotState AI = EHOLSlotState::AI;
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

// V3.1 on the server's chain (V2's fall, real steps): slot 0's forum hit to 0 health by DamageBuilding falls; attackers fixed
// in its zone (the override, as hol.Victory.HoldPresence) hold it, a contest by the replaceable provider freezes it, nobody
// lets it recede, then held to ForumHoldTime its foundations are razed: its last life, the defeat at the end of the step.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryForumHoldChainTest, "HeroesOfLiberty.Victory.Forum.Hold.Chain", UE::HeroesOfLiberty::Tests::Victory::Flags)
bool FHOLVictoryForumHoldChainTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Victory;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	AHOLPlayerState* Owner = World->SpawnActor<AHOLPlayerState>();
	AHOLPlayerState* Enemy = World->SpawnActor<AHOLPlayerState>();
	AHOLBuilding* Forum = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* EnemyForum = World->SpawnActor<AHOLBuilding>(FVector(-5000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("player states"), Owner)
		&& TestNotNull(TEXT("player states"), Enemy) && TestNotNull(TEXT("buildings"), Forum) && TestNotNull(TEXT("buildings"), EnemyForum))
	{
		State->InitVictory(MakeConfig({ { Human, 0 }, { AI, 0 } }));
		Owner->SetMatchSlot(0, 0);
		Enemy->SetMatchSlot(1, 1);
		Forum->Setup(HOLBuildingTypeNames::MainCentre, 0, 0);
		Forum->SetPrimaryForum(true);
		EnemyForum->Setup(HOLBuildingTypeNames::MainCentre, 1, 1);
		EnemyForum->SetPrimaryForum(true);
		const FHOLForumTuning Tuning = FHOLForumTuning::FromSettings();
		int64 Step = 0;
		const auto RunSteps = [Simulation, &Step](const int32 Count)
		{
			for (int32 Index = 0; Index < Count; ++Index)
			{
				++Step;
				Simulation->GetOnStepStarted().Broadcast(Step);
				Simulation->GetOnStepEnded().Broadcast(Step);
			}
		};

		AddExpectedMessage(TEXT("Victory: forum of slot 0 fell, lives 1"), EAutomationExpectedMessageFlags::Contains, 1);
		int32 Hits = 0;
		while (Forum->GetHealth() > 0 && Hits++ < 1000)
		{
			UHOLConstructionSubsystem::DamageBuilding(*Forum, 500, EHOLAttackType::Melee);
		}
		TestTrue(TEXT("fallen: in ruin"), State->GetForumState(0).Phase == EHOLForumPhase::Ruin);
		TestEqual(TEXT("fallen: 1 life"), State->GetLives(0), 1);

		// Attackers fixed in the zone, no attacker slot given: the first slot of the other camp holds.
		AddExpectedMessage(TEXT("Victory: forum of slot 0 held by slot 1"), EAutomationExpectedMessageFlags::Contains, 2);
		FHOLForumPresence Attackers;
		Attackers.bAttackers = true;
		State->SetForumPresenceOverride(0, Attackers);
		RunSteps(50);
		TestEqual(TEXT("held 50 steps"), State->GetForumState(0).HoldSteps, 50);
		TestTrue(TEXT("holding"), State->GetForumState(0).Hold == EHOLForumHold::Holding);
		TestEqual(TEXT("held by slot 1"), static_cast<int32>(State->GetForumState(0).HolderSlot), 1);
		TestEqual(TEXT("the enemy's forum untouched"), State->GetForumState(1).HoldSteps, 0);

		// A contest by the replaceable provider (V3.2's way): the count frozen.
		AddExpectedMessage(TEXT("Victory: forum of slot 0 contested"), EAutomationExpectedMessageFlags::Contains, 1);
		State->ClearForumPresenceOverride(0);
		State->SetForumPresenceProvider([](const AHOLGameState&, const int32 Slot)
		{
			FHOLForumPresence Presence;
			Presence.bAttackers = Slot == 0;
			Presence.bDefenders = Slot == 0;
			Presence.AttackerSlot = Slot == 0 ? 1 : INDEX_NONE;
			return Presence;
		});
		RunSteps(10);
		TestEqual(TEXT("contested: still 50 steps"), State->GetForumState(0).HoldSteps, 50);
		TestTrue(TEXT("contested"), State->GetForumState(0).IsContested());

		// Nobody: it recedes 2 steps a step.
		AddExpectedMessage(TEXT("Victory: forum of slot 0 released"), EAutomationExpectedMessageFlags::Contains, 1);
		State->SetForumPresenceProvider(nullptr);
		RunSteps(5);
		TestEqual(TEXT("nobody: 10 steps given back"), State->GetForumState(0).HoldSteps, 40);

		// Held again to ForumHoldTime: the foundations razed, the last life, the defeat at the end of that step.
		AddExpectedMessage(TEXT("Victory: forum of slot 0 razed, lives 0"), EAutomationExpectedMessageFlags::Contains, 1);
		AddExpectedMessage(TEXT("Victory: slot 0 defeated \\(Forum\\)"), EAutomationExpectedMessageFlags::Contains, 1);
		State->SetForumPresenceOverride(0, FHOLForumPresence::Attackers(1));
		FHOLForumState BeforeRaze = State->GetForumState(0);
		int32 HeldSteps = 0;
		while (State->GetForumState(0).Falls < 2 && HeldSteps < 100000)
		{
			BeforeRaze = State->GetForumState(0);
			++Step;
			++HeldSteps;
			Simulation->GetOnStepStarted().Broadcast(Step);
			if (State->GetForumState(0).Falls >= 2)
			{
				break;
			}
			Simulation->GetOnStepEnded().Broadcast(Step);
		}
		TestEqual(TEXT("razed after the hold's steps left (~60 s in all)"), HeldSteps, Tuning.HoldSteps - 40);
		TestTrue(TEXT("held through the ruin's end: rebuilding before the razing"), BeforeRaze.Phase == EHOLForumPhase::Rebuilding);
		TestEqual(TEXT("the rebuilding frozen while held"), BeforeRaze.RebuildWork, static_cast<int64>(0));
		TestEqual(TEXT("razed: 0 life"), State->GetLives(0), 0);
		TestFalse(TEXT("the defeat waits for the end of the step"), State->IsSlotDefeated(0));
		Simulation->GetOnStepEnded().Broadcast(Step);
		TestTrue(TEXT("slot 0 defeated by its forum"), State->IsSlotDefeated(0) && State->GetDefeatReason(0) == EHOLDefeatReason::Forum);
		TestTrue(TEXT("the enemy wins"), State->IsMatchOver() && State->IsWinner(1));
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// V3.2 on the server's chain: the presence given at the launch (InitVictory) counts the real units near slot 0's forum down,
// by team, from its edge (12 m). Standing, no walk of the units; fallen by DamageBuilding and rebuilding, 3 enemy soldiers
// 5 m from its edge hold it (the rebuilding frozen), a Peasant of the defender contests it, 4 soldiers of slot 2 take the
// ring (the most), dead (Dying) they no longer count, the enemies 20 m off let it recede and the rebuilding go on.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryForumHoldPresenceTest, "HeroesOfLiberty.Victory.Forum.Hold.Presence", UE::HeroesOfLiberty::Tests::Victory::Flags)
bool FHOLVictoryForumHoldPresenceTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Victory;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	AHOLBuilding* Forum = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation)
		&& TestNotNull(TEXT("forum"), Forum))
	{
		State->InitVictory(MakeConfig({ { Human, 0 }, { AI, 0 }, { AI, 0 } }));
		Forum->Setup(HOLBuildingTypeNames::MainCentre, 0, 0);
		Forum->SetPrimaryForum(true);
		const FHOLForumTuning Tuning = FHOLForumTuning::FromSettings();
		int64 Step = 0;
		const auto RunSteps = [Simulation, &Step](const int32 Count)
		{
			for (int32 Index = 0; Index < Count; ++Index)
			{
				++Step;
				Simulation->GetOnStepStarted().Broadcast(Step);
				Simulation->GetOnStepEnded().Broadcast(Step);
			}
		};
		const FBox2D Body = Forum->GetBodyBounds();
		const FVector2D Middle = Body.GetCenter();
		const auto East = [&Body, &Middle](const double FromEdge, const double Along) { return FVector(Body.Max.X + FromEdge, Middle.Y + Along, 0.0); };
		const auto North = [&Body, &Middle](const double FromEdge, const double Along) { return FVector(Middle.X + Along, Body.Max.Y + FromEdge, 0.0); };
		const auto West = [&Body, &Middle](const double FromEdge, const double Along) { return FVector(Body.Min.X - FromEdge, Middle.Y + Along, 0.0); };
		const FHOLForumPresenceCounter* Counter = State->GetForumPresenceCounter();
		TestNotNull(TEXT("the server counts the units near a forum down"), Counter);
		const auto Walks = [Counter] { return Counter ? Counter->GetWalks() : -1; };

		// 3 enemy soldiers 5 m from its edge, the forum standing: no walk of the units, nobody holds.
		TArray<uint32> Enemies;
		for (int32 Index = 0; Index < 3; ++Index)
		{
			Enemies.Add(PlaceUnit(*Units, FName(TEXT("Spearman")), 1, East(500.0, (Index - 1) * 150.0)));
			TestTrue(TEXT("an enemy soldier placed 5 m from the edge"), Enemies.Last() != 0 && FMath::Abs(DistanceToEdge(*Units, Enemies.Last(), Body) - 500.0) < 5.0);
		}
		RunSteps(10);
		TestEqual(TEXT("standing: nobody holds"), State->GetForumState(0).HoldSteps, 0);
		TestEqual(TEXT("standing: no walk of the units"), Walks(), 0);

		// Fallen (DamageBuilding), its ruin skipped, 2 builders at work: the 3 enemies hold, the rebuilding frozen.
		AddExpectedMessage(TEXT("Victory: forum of slot 0 fell, lives 1"), EAutomationExpectedMessageFlags::Contains, 1);
		int32 Hits = 0;
		while (Forum->GetHealth() > 0 && Hits++ < 1000)
		{
			UHOLConstructionSubsystem::DamageBuilding(*Forum, 500, EHOLAttackType::Melee);
		}
		TestTrue(TEXT("fallen"), State->GetForumState(0).IsDown());
		TestTrue(TEXT("its ruin skipped: rebuilding"), State->SkipForumPhase(0) && State->GetForumState(0).Phase == EHOLForumPhase::Rebuilding);
		State->SetForumBuilders(0, 2);
		AddExpectedMessage(TEXT("Victory: forum of slot 0 held by slot 1"), EAutomationExpectedMessageFlags::Contains, 0);
		RunSteps(20);
		TestEqual(TEXT("1) 3 enemies 5 m off: held 20 steps"), State->GetForumState(0).HoldSteps, 20);
		TestTrue(TEXT("1) holding"), State->GetForumState(0).Hold == EHOLForumHold::Holding);
		TestEqual(TEXT("1) held by slot 1"), static_cast<int32>(State->GetForumState(0).HolderSlot), 1);
		TestEqual(TEXT("1) the rebuilding frozen"), State->GetForumState(0).RebuildWork, static_cast<int64>(0));
		TestEqual(TEXT("1) one walk of the units a step"), Walks(), 20);

		// 2) A Peasant of the defender 5 m off: contested, the count frozen.
		AddExpectedMessage(TEXT("Victory: forum of slot 0 contested"), EAutomationExpectedMessageFlags::Contains, 0);
		const uint32 Defender = PlaceUnit(*Units, FName(TEXT("Peasant")), 0, North(500.0, 0.0));
		RunSteps(10);
		TestTrue(TEXT("2) contested"), State->GetForumState(0).IsContested());
		TestEqual(TEXT("2) the count frozen"), State->GetForumState(0).HoldSteps, 20);
		TestEqual(TEXT("2) the rebuilding still frozen"), State->GetForumState(0).RebuildWork, static_cast<int64>(0));

		// 6) The Peasant 20 m off, 4 soldiers of slot 2 5 m off the west edge: slot 2 has the most, its colour.
		MoveUnit(*World, *Units, Defender, North(2000.0, 0.0));
		AddExpectedMessage(TEXT("Victory: forum of slot 0 held by slot 2"), EAutomationExpectedMessageFlags::Contains, 0);
		for (int32 Index = 0; Index < 4; ++Index)
		{
			TestTrue(TEXT("a soldier of slot 2 placed"), PlaceUnit(*Units, FName(TEXT("Spearman")), 2, West(500.0, (Index - 2) * 150.0)) != 0);
		}
		RunSteps(5);
		TestTrue(TEXT("6) holding again"), State->GetForumState(0).Hold == EHOLForumHold::Holding);
		TestEqual(TEXT("6) the most numerous holds: slot 2"), static_cast<int32>(State->GetForumState(0).HolderSlot), 2);
		TestEqual(TEXT("6) 25 steps held"), State->GetForumState(0).HoldSteps, 25);

		// 5) Slot 2's soldiers dead (Dying, KillUnitsOfOwner): no longer counted, slot 1 holds again.
		TestEqual(TEXT("5) slot 2's soldiers killed"), Units->KillUnitsOfOwner(2), 4);
		RunSteps(5);
		TestEqual(TEXT("5) the dying do not count: slot 1"), static_cast<int32>(State->GetForumState(0).HolderSlot), 1);
		TestEqual(TEXT("5) 30 steps held"), State->GetForumState(0).HoldSteps, 30);

		// 3) The enemies 20 m off the edge (out of the zone): it recedes, the rebuilding goes on.
		AddExpectedMessage(TEXT("Victory: forum of slot 0 released"), EAutomationExpectedMessageFlags::Contains, 0);
		for (const uint32 Id : Enemies)
		{
			MoveUnit(*World, *Units, Id, FVector(Body.Max.X + 2000.0, Middle.Y, 0.0));
		}
		RunSteps(5);
		TestTrue(TEXT("3) receding"), State->GetForumState(0).Hold == EHOLForumHold::Receding);
		TestEqual(TEXT("3) the count goes back down"), State->GetForumState(0).HoldSteps, FMath::Max(30 - 5 * Tuning.HoldDecaySteps, 0));
		TestTrue(TEXT("3) the rebuilding goes on"), State->GetForumState(0).RebuildWork > 0);
		TestEqual(TEXT("cost: one walk a step while down (45 steps)"), Walks(), 45);
		if (Counter)
		{
			AddInfo(FString::Printf(TEXT("Cost: %d walks, %lld units walked (%d units)"), Counter->GetWalks(), Counter->GetUnitsWalked(), Units->GetNumUnits()));
		}
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// V3.2, teams (V3-Q2): slot 0 and slot 1 in team 1, slot 2 in team 2. Slot 2's soldiers hold slot 0's forum down; an ally
// (slot 1) in the zone contests it as the defender's own would; an owner out of the launch's slots (Neutral, D401) attacks.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryForumHoldPresenceTeamsTest, "HeroesOfLiberty.Victory.Forum.Hold.PresenceTeams", UE::HeroesOfLiberty::Tests::Victory::Flags)
bool FHOLVictoryForumHoldPresenceTeamsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Victory;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	AHOLBuilding* Forum = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation)
		&& TestNotNull(TEXT("forum"), Forum))
	{
		State->InitVictory(MakeConfig({ { Human, 1 }, { AI, 1 }, { AI, 2 } }));
		Forum->Setup(HOLBuildingTypeNames::MainCentre, 0, 0);
		Forum->SetPrimaryForum(true);
		int64 Step = 0;
		const auto RunSteps = [Simulation, &Step](const int32 Count)
		{
			for (int32 Index = 0; Index < Count; ++Index)
			{
				++Step;
				Simulation->GetOnStepStarted().Broadcast(Step);
				Simulation->GetOnStepEnded().Broadcast(Step);
			}
		};
		const FBox2D Body = Forum->GetBodyBounds();
		const FVector2D Middle = Body.GetCenter();

		AddExpectedMessage(TEXT("Victory: forum of slot 0 fell, lives 1"), EAutomationExpectedMessageFlags::Contains, 1);
		int32 Hits = 0;
		while (Forum->GetHealth() > 0 && Hits++ < 1000)
		{
			UHOLConstructionSubsystem::DamageBuilding(*Forum, 500, EHOLAttackType::Melee);
		}
		TestTrue(TEXT("fallen"), State->GetForumState(0).IsDown());

		// Slot 2 (the other team) holds.
		AddExpectedMessage(TEXT("Victory: forum of slot 0 held by slot 2"), EAutomationExpectedMessageFlags::Contains, 0);
		PlaceUnit(*Units, FName(TEXT("Spearman")), 2, FVector(Body.Max.X + 500.0, Middle.Y, 0.0));
		PlaceUnit(*Units, FName(TEXT("Spearman")), 2, FVector(Body.Max.X + 500.0, Middle.Y + 150.0, 0.0));
		RunSteps(5);
		TestTrue(TEXT("the other team holds"), State->GetForumState(0).Hold == EHOLForumHold::Holding && State->GetForumState(0).HolderSlot == 2);
		TestEqual(TEXT("5 steps held"), State->GetForumState(0).HoldSteps, 5);

		// 4) An ally (slot 1, same team) 5 m off: contested, the count frozen.
		AddExpectedMessage(TEXT("Victory: forum of slot 0 contested"), EAutomationExpectedMessageFlags::Contains, 0);
		const uint32 Ally = PlaceUnit(*Units, FName(TEXT("Spearman")), 1, FVector(Middle.X, Body.Min.Y - 500.0, 0.0));
		RunSteps(5);
		TestTrue(TEXT("4) an ally contests"), State->GetForumState(0).IsContested());
		TestEqual(TEXT("4) the count frozen"), State->GetForumState(0).HoldSteps, 5);

		// Slot 2 dead, the ally gone 20 m off, a Neutral unit (owner 5, a closed slot) 5 m off: it holds (D401).
		Units->KillUnitsOfOwner(2);
		MoveUnit(*World, *Units, Ally, FVector(Middle.X, Body.Min.Y - 2000.0, 0.0));
		PlaceUnit(*Units, FName(TEXT("Spearman")), 5, FVector(Body.Min.X - 500.0, Middle.Y, 0.0));
		RunSteps(5);
		TestTrue(TEXT("a Neutral unit holds"), State->GetForumState(0).Hold == EHOLForumHold::Holding && State->GetForumState(0).HolderSlot == 5);
		TestEqual(TEXT("10 steps held"), State->GetForumState(0).HoldSteps, 10);
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
				// V3.2: an enemy standing at the forum would freeze its rebuilding (D348); it dies here (Dying, not counted).
				TestEqual(TEXT("slot 1's Peasant killed"), Units->KillUnitsOfOwner(1), 1);
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

// V3.3 (V3-Q5), pure, what script cannot read: the ring's colours, the players' table rows and the zone's outline. The hold's lines,
// the block's text and the ring are tested in script: HOLForumRules_Test.as Test_ForumHoldDisplay.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryForumHoldDisplayTest, "HeroesOfLiberty.Victory.Forum.Hold.Display", UE::HeroesOfLiberty::Tests::Victory::Flags)
bool FHOLVictoryForumHoldDisplayTest::RunTest(const FString& Parameters)
{
	FHOLForumTuning Tuning;
	Tuning.Tier = 0;
	Tuning.RuinSteps = 300;
	Tuning.RebuildSteps = 1200;
	Tuning.StepSeconds = 0.1f;
	Tuning.HoldSteps = 600;

	// The forum state the players' table below reads (rest of the display: script, HOLForumRules_Test.as Test_ForumHoldDisplay).
	FHOLForumState State;
	HOLForumRules::Fall(State, Tuning);
	State.Phase = EHOLForumPhase::Rebuilding;
	State.RuinStepsLeft = 0;
	State.RebuildWork = HOLForumRules::GetRebuildTotalWork(Tuning) * 40 / 100;
	State.Hold = EHOLForumHold::Holding;
	State.HoldSteps = 300;
	State.HolderSlot = INDEX_NONE;

	// The colours (V3.4b): the holder's own, a neutral dark grey contested (it stands out on the light ground), halfway to
	// a dark grey receding (darkened, not paled, and still apart from the contested grey).
	const FLinearColor Red(1.f, 0.f, 0.f);
	TestTrue(TEXT("colour: the holder's"), HOLForumRules::GetRingColor(EHOLForumRingTint::Holder, Red).Equals(Red));
	const FLinearColor Grey = HOLForumRules::GetRingColor(EHOLForumRingTint::Grey, Red);
	TestTrue(TEXT("colour: grey is neutral and dark"), FMath::IsNearlyEqual(Grey.R, Grey.G) && FMath::IsNearlyEqual(Grey.G, Grey.B)
		&& Grey.R >= 0.15f && Grey.R <= 0.2f);
	TestTrue(TEXT("colour: faded halfway to a dark grey"), HOLForumRules::GetRingColor(EHOLForumRingTint::Faded, Red).Equals(FLinearColor(0.55f, 0.05f, 0.05f), 0.01f));

	// The players' table: slot 0's forum in ruin held by slot 1, slot 1's rising again contested.
	FHOLMatchConfig Duel;
	Duel.Normalize();
	Duel.Slots[0].State = EHOLSlotState::Human;
	Duel.Slots[0].ColorIndex = 3;
	Duel.Slots[1].State = EHOLSlotState::AI;
	Duel.Slots[1].ColorIndex = 5;
	TArray<FHOLForumState> Forums;
	Forums.Init(FHOLForumState(), FHOLMatchConfig::MaxSlots);
	HOLForumRules::Fall(Forums[0], Tuning);
	Forums[0].Hold = EHOLForumHold::Holding;
	Forums[0].HoldSteps = 350;
	Forums[0].HolderSlot = 1;
	Forums[1] = State;
	Forums[1].Hold = EHOLForumHold::Contested;
	Forums[1].HolderSlot = 0;
	const TArray<FHOLPlayersTableRow> Rows = HOLForumRules::GetPlayersTable(Duel, Forums, 0, 0, Tuning);
	TestEqual(TEXT("table: two rows"), Rows.Num(), 2);
	if (Rows.Num() == 2)
	{
		TestEqual(TEXT("table: held, described"), Rows[0].Shown, FString(TEXT("slot 0 you colour 3 lives 1 ruin hold 35 s holding by slot 1")));
		TestEqual(TEXT("table: held, its state"), Rows[0].State.BuildSourceString(), FString(TEXT("Ruine · tenue 35 s")));
		TestEqual(TEXT("table: contested, described"), Rows[1].Shown, FString(TEXT("slot 1 enemy colour 5 lives 1 rebuilding 40% hold 30 s contested by slot 0")));
		TestEqual(TEXT("table: contested, its state"), Rows[1].State.BuildSourceString(), FString(TEXT("Reconstruction 40 % · contestée 30 s")));
	}

	// The zone's outline: 1200 cm around a 10 m x 6 m footprint, from the top's middle, clockwise (towards +Y first).
	const FBox2D Body(FVector2D(-500.0, -300.0), FVector2D(500.0, 300.0));
	TestTrue(TEXT("outline: starts at the top's middle"), HOLForumRules::GetZoneOutlinePoint(Body, 1200.0, 0.0).Equals(FVector2D(1700.0, 0.0), 0.5));
	TestTrue(TEXT("outline: ends there"), HOLForumRules::GetZoneOutlinePoint(Body, 1200.0, 1.0).Equals(FVector2D(1700.0, 0.0), 0.5));
	TestTrue(TEXT("outline: a quarter on the right side's middle"), HOLForumRules::GetZoneOutlinePoint(Body, 1200.0, 0.25).Equals(FVector2D(0.0, 1500.0), 0.5));
	TestTrue(TEXT("outline: half at the bottom's middle"), HOLForumRules::GetZoneOutlinePoint(Body, 1200.0, 0.5).Equals(FVector2D(-1700.0, 0.0), 0.5));
	TestTrue(TEXT("outline: clockwise"), HOLForumRules::GetZoneOutlinePoint(Body, 1200.0, 0.05).Y > 0.0);
	for (int32 Index = 0; Index <= 40; ++Index)
	{
		const FVector2D Point = HOLForumRules::GetZoneOutlinePoint(Body, 1200.0, Index / 40.0);
		const double Distance = FVector2D(FMath::Max(FMath::Abs(Point.X) - 500.0, 0.0), FMath::Max(FMath::Abs(Point.Y) - 300.0, 0.0)).Size();
		TestTrue(*FString::Printf(TEXT("outline: point %d at the zone's reach (%.1f)"), Index, Distance), FMath::IsNearlyEqual(Distance, 1200.0, 1.0));
	}
	return true;
}

// V2.3a (V2-Q1), pure: the Peasants' hammer at a site, not at a forum in ruin (HOLUnitAnimation::Choose, no script binding).
// The forum's look (itself, rubble, a site) is tested in script: HOLForumRules_Test.as Test_ForumLook.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryForumLookTest, "HeroesOfLiberty.Victory.Forum.Look", UE::HeroesOfLiberty::Tests::Victory::Flags)
bool FHOLVictoryForumLookTest::RunTest(const FString& Parameters)
{
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
	TestEqual(TEXT("duel: described"), Describe(Rows), FString(TEXT("slot 0 you colour 3 lives 2 standing | slot 1 enemy colour 5 lives 2 standing")));
	if (Rows.Num() == 2)
	{
		TestEqual(TEXT("duel: Vous"), Rows[0].Name.BuildSourceString(), FString(TEXT("Vous")));
		TestEqual(TEXT("duel: the AI's name (IA0.1-Q1)"), Rows[1].Name.BuildSourceString(), FString(TEXT("IA Normal 2")));
		TestEqual(TEXT("duel: the lives"), Rows[0].Lives.BuildSourceString(), FString(TEXT("2 vies")));
		TestTrue(TEXT("duel: nothing on a standing forum"), Rows[0].State.IsEmpty());
		TestFalse(TEXT("duel: not greyed"), Rows[1].bGreyed);
		TestEqual(TEXT("duel: its colour"), Rows[1].ColorIndex, 5);
	}
	// Seen from slot 1 (a client): Vous moves.
	Rows = HOLForumRules::GetPlayersTable(Duel, Forums, 0, 1, Tuning);
	TestEqual(TEXT("duel from slot 1: described"), Describe(Rows), FString(TEXT("slot 0 enemy colour 3 lives 2 standing | slot 1 you colour 5 lives 2 standing")));

	// M6-Q12: a match from a lobby, its humans named: the other's name instead of Joueur N, Vous still on one's row;
	// a spectator (no slot) sees both names. An unnamed slot keeps Joueur N.
	FHOLMatchConfig Named = Duel;
	Named.Slots[0].PlayerName = TEXT("Alice");
	Named.Slots[1].State = EHOLSlotState::Human;
	Named.Slots[1].PlayerName = TEXT("Bob");
	Rows = HOLForumRules::GetPlayersTable(Named, Forums, 0, 1, Tuning);
	TestEqual(TEXT("named: Alice for Bob"), Rows.IsValidIndex(0) ? Rows[0].Name.ToString() : FString(), FString(TEXT("Alice")));
	TestEqual(TEXT("named: Vous for Bob"), Rows.IsValidIndex(1) ? Rows[1].Name.BuildSourceString() : FString(), FString(TEXT("Vous")));
	TestEqual(TEXT("named: described"), Describe(Rows), FString(TEXT("slot 0 enemy 'Alice' colour 3 lives 2 standing | slot 1 you 'Bob' colour 5 lives 2 standing")));
	Rows = HOLForumRules::GetPlayersTable(Named, Forums, 0, INDEX_NONE, Tuning);
	TestEqual(TEXT("named, spectator: Alice"), Rows.IsValidIndex(0) ? Rows[0].Name.ToString() : FString(), FString(TEXT("Alice")));
	TestEqual(TEXT("named, spectator: Bob"), Rows.IsValidIndex(1) ? Rows[1].Name.ToString() : FString(), FString(TEXT("Bob")));
	Named.Slots[1].PlayerName = TEXT("  ");
	Rows = HOLForumRules::GetPlayersTable(Named, Forums, 0, 0, Tuning);
	TestEqual(TEXT("unnamed: Joueur 2"), Rows.IsValidIndex(1) ? Rows[1].Name.BuildSourceString() : FString(), FString(TEXT("Joueur 2")));

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
		"slot 3 enemy team 1 colour 3 lives 1 rebuilding 40% | slot 5 enemy team 1 colour 5 lives 0 defeated greyed struck"
		" | slot 1 you team 2 colour 1 lives 1 ruin | slot 4 ally team 2 colour 4 lives 2 standing")));
	if (Rows.Num() == 4)
	{
		TestEqual(TEXT("teams: Reconstruction 40 %"), Rows[0].State.BuildSourceString(), FString(TEXT("Reconstruction 40 %")));
		TestEqual(TEXT("teams: one life"), Rows[0].Lives.BuildSourceString(), FString(TEXT("1 vie")));
		TestEqual(TEXT("teams: Vaincu"), Rows[1].State.BuildSourceString(), FString(TEXT("Vaincu")));
		TestTrue(TEXT("teams: the defeated greyed"), Rows[1].bGreyed);
		TestEqual(TEXT("teams: the defeated's name (an AI, IA0.1-Q1)"), Rows[1].Name.BuildSourceString(), FString(TEXT("IA Normal 6")));
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
	TestEqual(TEXT("spectator: its row"), Rows.IsValidIndex(2) ? Rows[2].Shown : FString(), FString(TEXT("slot 1 you team 2 colour 1 lives 1 defeated greyed struck")));

	// E4-1, E4-2 (the user's network trial, Alice and Bob allied against Carol): slots 0 and 1 in one team, slot 2 in
	// another. Seen by slot 0: itself Self, slot 1 an ally, slot 2 an enemy; slot 2 defeated: its name struck.
	FHOLMatchConfig Allies;
	Allies.Normalize();
	for (int32 Slot = 0; Slot < 3; ++Slot)
	{
		Allies.Slots[Slot].State = EHOLSlotState::Human;
		Allies.Slots[Slot].ColorIndex = Slot;
		Allies.Slots[Slot].Team = Slot < 2 ? 1 : 2;
	}
	Forums.Init(FHOLForumState(), FHOLMatchConfig::MaxSlots);
	Forums[2].Falls = 2;
	Rows = HOLForumRules::GetPlayersTable(Allies, Forums, 1 << 2, 0, Tuning);
	TestEqual(TEXT("allies: three rows"), Rows.Num(), 3);
	TestEqual(TEXT("allies: described"), Describe(Rows), FString(TEXT(
		"slot 0 you team 1 colour 0 lives 2 standing | slot 1 ally team 1 colour 1 lives 2 standing"
		" | slot 2 enemy team 2 colour 2 lives 0 defeated greyed struck")));
	if (Rows.Num() == 3)
	{
		TestTrue(TEXT("allies: slot 0 Self"), Rows[0].Relation == EHOLPlayerRelation::Self);
		TestTrue(TEXT("allies: slot 1 Ally"), Rows[1].Relation == EHOLPlayerRelation::Ally);
		TestTrue(TEXT("allies: slot 2 Enemy"), Rows[2].Relation == EHOLPlayerRelation::Enemy);
		TestFalse(TEXT("allies: slot 1 standing, not struck"), Rows[1].bStruck);
		TestTrue(TEXT("allies: slot 2 defeated, struck"), Rows[2].bStruck);
		TestTrue(TEXT("allies: struck as greyed"), Rows[2].bStruck == Rows[2].bGreyed);
	}
	// Seen by slot 2 (the defeated): both others enemies, its own row Self and struck.
	Rows = HOLForumRules::GetPlayersTable(Allies, Forums, 1 << 2, 2, Tuning);
	TestEqual(TEXT("allies from slot 2: described"), Describe(Rows), FString(TEXT(
		"slot 0 enemy team 1 colour 0 lives 2 standing | slot 1 enemy team 1 colour 1 lives 2 standing"
		" | slot 2 you team 2 colour 2 lives 0 defeated greyed struck")));
	// A spectator (no slot): no relation (no icon), the defeated still struck.
	Rows = HOLForumRules::GetPlayersTable(Allies, Forums, 1 << 2, INDEX_NONE, Tuning);
	TestEqual(TEXT("allies, spectator: described"), Describe(Rows), FString(TEXT(
		"slot 0 team 1 colour 0 lives 2 standing | slot 1 team 1 colour 1 lives 2 standing"
		" | slot 2 team 2 colour 2 lives 0 defeated greyed struck")));
	for (const FHOLPlayersTableRow& Row : Rows)
	{
		TestTrue(*FString::Printf(TEXT("allies, spectator: slot %d no relation"), Row.Slot), Row.Relation == EHOLPlayerRelation::None);
	}
	TestTrue(TEXT("allies, spectator: slot 2 struck"), Rows.IsValidIndex(2) && Rows[2].bStruck);
	return true;
}

#endif
