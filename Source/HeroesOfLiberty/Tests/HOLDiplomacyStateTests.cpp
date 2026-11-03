// Copyright Epic Games, Inc. All Rights Reserved.

#include "Commands/HOLCommandSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLDiplomacyRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLSimulationSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// D5 (DiplomacyInProgress.md; GDD 14.3, D369, D370, D371, D367): the stances of a match through the real chain: an order of the player's
// command queue, taken at the start of the next simulation step by the game state, decided by HOLDiplomacy::SetStance / SetVision,
// stored in the arrays the clients receive (GetDiplomacyBook rebuilds the book from them, as a client does). No stance typed in by hand.
// The replication itself to a second machine is the "net diplomacy" case of quick_check.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDiplomacyStateTest, "HeroesOfLiberty.Diplomacy.OrdersInMatch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Slots 0, 1, 2 human, 3 an AI, the others closed; the Diplomacy mode:
// - at the launch every stance is the default (Enemy, DQ-1 provisional): everybody fires at everybody;
// - an order waits for the next step; "Ally" from one side only does not make an alliance (D369), the second makes it, announced once;
// - the vision shared by one ally is read by the other, and the break (one side says Enemy) cuts it at that very step (D370, D371);
// - Neutral on both sides: no automatic fire but still an adversary (D401); the most hostile of the two stances applies (D375);
// - an AI never makes an alliance (D367); a closed slot, oneself and a stance out of range are refused, nothing changes;
// - outside the Diplomacy mode every order is refused and the relations are the fixed teams (D357).
bool FHOLDiplomacyStateTest::RunTest(const FString& Parameters)
{
	using namespace HOLDiplomacy;
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
	World->SetGameState(GameState);
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>();
	if (TestNotNull(TEXT("game state"), GameState) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("commands"), Commands))
	{
		const auto Launch = [GameState](const EHOLMatchMode Mode, const int32 TeamOfSlot0, const int32 TeamOfSlot1)
		{
			FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
			Config.Mode = Mode;
			Config.Slots.SetNum(FHOLMatchConfig::MaxSlots);
			for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
			{
				Config.Slots[Slot].State = Slot < 3 ? EHOLSlotState::Human : Slot == 3 ? EHOLSlotState::AI : EHOLSlotState::Closed;
				Config.Slots[Slot].Team = Slot == 0 ? TeamOfSlot0 : Slot == 1 ? TeamOfSlot1 : 0;
			}
			GameState->InitVictory(Config);
		};
		int64 Step = 0;
		int32 Announced = 0;
		int32 Formed = 0;
		int32 Broken = 0;
		GameState->OnDiplomacyAnnounced.AddLambda([&](const FStanceChange& Change)
		{
			++Announced;
			Formed += Change.bFormed ? 1 : 0;
			Broken += Change.bBroke ? 1 : 0;
		});
		const auto StartStep = [Simulation, &Step] { Simulation->GetOnStepStarted().Broadcast(++Step); };
		const auto Say = [&](const int32 From, const int32 To, const int32 Stance)
		{
			Commands->IssueDiplomacyOrder(From, EHOLDiplomacyOrderKind::Stance, To, Stance);
			StartStep();
		};
		const auto Share = [&](const int32 From, const int32 To, const bool bShare)
		{
			Commands->IssueDiplomacyOrder(From, EHOLDiplomacyOrderKind::Vision, To, bShare ? 1 : 0);
			StartStep();
		};
		const auto Relation = [GameState](const int32 A, const int32 B) { return GameState->GetDiplomacyRelation(A, B); };

		Launch(EHOLMatchMode::Diplomacy, 0, 0);
		TestTrue(TEXT("the Diplomacy mode"), GameState->IsDiplomacyMode());
		TestEqual(TEXT("at the launch, 0 toward 1: the default stance"), StanceOf(GameState->GetDiplomacyBook(), 0, 1), DefaultStance);
		TestEqual(TEXT("... the relation of every pair is Enemy"), Relation(0, 1) + Relation(0, 2) + Relation(1, 2), 3 * Enemy);
		TestTrue(TEXT("... and they fire at once"), FiresAtOnce(GameState->GetDiplomacyBook(), 0, 1));

		// An order waits for the step.
		Commands->IssueDiplomacyOrder(0, EHOLDiplomacyOrderKind::Stance, 1, Ally);
		TestEqual(TEXT("an order queued is not applied before the step"), StanceOf(GameState->GetDiplomacyBook(), 0, 1), Enemy);
		StartStep();
		TestEqual(TEXT("... applied at the step's start"), StanceOf(GameState->GetDiplomacyBook(), 0, 1), Ally);
		TestEqual(TEXT("one side's Ally is no alliance (D369): the relation stays Enemy"), Relation(0, 1), Enemy);
		TestEqual(TEXT("... announced"), Announced, 1);
		Say(1, 0, Ally);
		TestEqual(TEXT("both say Ally: allies"), Relation(0, 1), Ally);
		TestEqual(TEXT("... an alliance formed, announced"), Formed * 100 + Announced, 102);
		Say(1, 0, Ally);
		TestEqual(TEXT("the same stance again: nothing announced"), Announced, 2);

		// The shared vision, cut by the break.
		Share(0, 1, true);
		TestTrue(TEXT("0 shares its vision with ally 1: 1 reads it"), SharesVision(GameState->GetDiplomacyBook(), 0, 1));
		TestFalse(TEXT("... not the other way"), SharesVision(GameState->GetDiplomacyBook(), 1, 0));
		Share(0, 2, true);
		TestFalse(TEXT("vision toward a player who is not an ally: refused"), SharesVision(GameState->GetDiplomacyBook(), 0, 2));
		Say(1, 0, Enemy);
		TestEqual(TEXT("the break is immediate: Enemy at that step"), Relation(0, 1), Enemy);
		TestEqual(TEXT("... announced as a break"), Broken, 1);
		TestFalse(TEXT("... the shared vision is cut at that second (D371)"), SharesVision(GameState->GetDiplomacyBook(), 0, 1));
		Say(1, 0, Ally);
		TestEqual(TEXT("the alliance formed again"), Relation(0, 1), Ally);
		TestFalse(TEXT("... the vision is not given back by itself"), SharesVision(GameState->GetDiplomacyBook(), 0, 1));

		// Neutral: no automatic fire, still an adversary; the most hostile applies.
		Say(0, 2, Neutral);
		TestEqual(TEXT("0 Neutral toward 2, 2 still Enemy: Enemy (D375)"), Relation(0, 2), Enemy);
		Say(2, 0, Neutral);
		TestEqual(TEXT("both Neutral"), Relation(0, 2), Neutral);
		TestFalse(TEXT("... no automatic fire"), FiresAtOnce(GameState->GetDiplomacyBook(), 0, 2));
		TestTrue(TEXT("... still an adversary for the other rules (D401)"), IsAdversary(GameState->GetDiplomacyBook(), 0, 2));
		Say(2, 0, Ally);
		TestEqual(TEXT("2 says Ally, 0 Neutral: Neutral (the most hostile)"), Relation(0, 2), Neutral);

		// Refusals: nothing changes.
		const int32 AnnouncedBefore = Announced;
		Say(0, 3, Ally);
		TestEqual(TEXT("toward an AI: the stance is kept but the AI is Enemy to everybody (D367)"), Relation(0, 3), Enemy);
		Say(3, 0, Ally);
		TestEqual(TEXT("an AI says nothing (refused)"), StanceOf(GameState->GetDiplomacyBook(), 3, 0), Enemy);
		Say(0, 5, Ally);
		TestEqual(TEXT("toward a closed slot: refused"), StanceOf(GameState->GetDiplomacyBook(), 0, 5), Enemy);
		Say(0, 0, Ally);
		Say(0, 2, 7);
		Say(5, 0, Ally);
		TestEqual(TEXT("oneself, a stance out of range, a closed slot's order: refused"), Announced, AnnouncedBefore + 1);
		Commands->IssueDiplomacyOrder(INDEX_NONE, EHOLDiplomacyOrderKind::Stance, 1, Ally);
		Commands->IssueDiplomacyOrder(FHOLMatchConfig::MaxSlots, EHOLDiplomacyOrderKind::Stance, 1, Ally);
		TArray<FHOLDiplomacyOrder> Left;
		Commands->ConsumeDiplomacyOrders(Left);
		TestEqual(TEXT("the orders of no player's slot are dropped"), Left.Num(), 0);

		// A new match: every stance back to the default.
		Launch(EHOLMatchMode::Diplomacy, 0, 0);
		TestEqual(TEXT("a new launch resets the stances"), Relation(0, 1) + StanceOf(GameState->GetDiplomacyBook(), 0, 1), Enemy);

		// Outside the Diplomacy mode: the fixed teams, no order obeyed.
		Launch(EHOLMatchMode::Standard, 1, 1);
		TestFalse(TEXT("Standard is not the Diplomacy mode"), GameState->IsDiplomacyMode());
		TestEqual(TEXT("the same team: allies"), Relation(0, 1), Ally);
		TestEqual(TEXT("another slot, no team: Enemy"), Relation(0, 2), Enemy);
		const int32 AnnouncedStandard = Announced;
		Say(0, 1, Enemy);
		Say(0, 2, Ally);
		TestEqual(TEXT("orders refused outside the Diplomacy mode: the teams stay"), Relation(0, 1) * 10 + Relation(0, 2), Ally * 10 + Enemy);
		TestEqual(TEXT("... nothing announced"), Announced, AnnouncedStandard);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
