// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLDisconnectRules.h"
#include "Match/HOLForumMessages.h"
#include "Match/HOLForumRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLMatchStats.h"
#include "Match/HOLMatchStatsSubsystem.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	FHOLMatchConfig MakeFourHumans()
	{
		FHOLMatchConfig Config;
		Config.Slots.SetNum(FHOLMatchConfig::MaxSlots);
		const TCHAR* Names[] = { TEXT("Alice"), TEXT("Bob"), TEXT("Cleo"), TEXT("Dan") };
		for (int32 Slot = 0; Slot < 4; ++Slot)
		{
			Config.Slots[Slot].State = EHOLSlotState::Human;
			Config.Slots[Slot].PlayerName = Names[Slot];
			Config.Slots[Slot].ColorIndex = Slot;
		}
		return Config;
	}
}

// T8 (D722): the players' table keeps the name and adds a pastille: « Connexion perdue » while the window runs, « IA » once the AI keeps the seat.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDisconnectTableTest, "HeroesOfLiberty.Match.DisconnectTable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLDisconnectTableTest::RunTest(const FString& Parameters)
{
	const FHOLMatchConfig Config = MakeFourHumans();
	const TArray<FHOLForumState> Forums;
	// Bob (1) away, Cleo (2) held by the AI, Dan (3) defeated while away (no pastille), Alice (0) present.
	const TArray<FHOLPlayersTableRow> Rows = HOLForumRules::GetPlayersTable(Config, Forums, 1 << 3, 0, FHOLForumTuning(), {}, nullptr,
		(1 << 1) | (1 << 3), 1 << 2);
	TestEqual(TEXT("one row per seat"), Rows.Num(), 4);
	if (Rows.Num() != 4)
	{
		return false;
	}
	TestFalse(TEXT("present: no pastille"), Rows[0].bAway || Rows[0].bAIHeld);
	TestTrue(TEXT("away: pastille"), Rows[1].bAway && !Rows[1].bAIHeld);
	TestEqual(TEXT("away: the name stays"), Rows[1].Name.ToString(), FString(TEXT("Bob")));
	TestEqual(TEXT("away: the state says so"), Rows[1].State.ToString(), FString(TEXT("Connexion perdue")));
	TestTrue(TEXT("away: in the report"), Rows[1].Shown.Contains(TEXT(" away")));
	TestTrue(TEXT("AI holds: pastille"), Rows[2].bAIHeld && !Rows[2].bAway);
	TestEqual(TEXT("AI holds: the state says IA"), Rows[2].State.ToString(), FString(TEXT("IA")));
	TestTrue(TEXT("AI holds: in the report"), Rows[2].Shown.Contains(TEXT(" ai-held")));
	TestFalse(TEXT("defeated: no pastille"), Rows[3].bAway || Rows[3].bAIHeld);
	TestEqual(TEXT("defeated: Vaincu"), Rows[3].State.ToString(), FString(TEXT("Vaincu")));
	return true;
}

// T8 (D722): the defeat of a player who left and did not come back has its own text, not « vaincu ».
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDisconnectMessageTest, "HeroesOfLiberty.Match.DisconnectMessage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLDisconnectMessageTest::RunTest(const FString& Parameters)
{
	const FHOLMatchConfig Config = MakeFourHumans();
	// Answer B (Q-disconnect-left): one kind, Defeated, and a Reason that picks the text.
	const FHOLForumEvent Left = FHOLForumEvent::Make(EHOLForumEventKind::Defeated, 1, INDEX_NONE, 0, EHOLForumEventReason::Disconnected);
	TestEqual(TEXT("seen by another"), HOLForumMessages::GetMessage(Left, Config, 0, false).Text.ToString(), FString(TEXT("Bob a quitt\u00e9 la partie")));
	TestEqual(TEXT("seen by himself"), HOLForumMessages::GetMessage(Left, Config, 1, false).Text.ToString(), FString(TEXT("Vous avez quitt\u00e9 la partie")));
	TestFalse(TEXT("not the plain defeat"), HOLForumMessages::GetMessage(Left, Config, 0, false).Text.ToString().Contains(TEXT("vaincu")));
	TestFalse(TEXT("his own leaving is no alert for himself"), HOLForumMessages::GetMessage(Left, Config, 1, false).bAlert);
	const FHOLForumEvent Beaten = FHOLForumEvent::Make(EHOLForumEventKind::Defeated, 1);
	TestEqual(TEXT("no reason: the plain defeat"), HOLForumMessages::GetMessage(Beaten, Config, 0, false).Text.ToString(), FString(TEXT("Bob est vaincu")));
	return true;
}

// Answer B on the real chain: DefeatSlot(Disconnected) sends ONE Defeated event carrying the reason, the text written for the
// viewers says \u00ab a quitt\u00e9 la partie \u00bb, and the statistics count it as the seat's one defeat (D508: one line per seat).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDisconnectLeftIsDefeatTest, "HeroesOfLiberty.Match.DisconnectLeftIsDefeat",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLDisconnectLeftIsDefeatTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	UHOLMatchStatsSubsystem* Stats = World->GetSubsystem<UHOLMatchStatsSubsystem>();
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("stats subsystem"), Stats))
	{
		FHOLMatchConfig Config = MakeFourHumans();
		Config.Slots[0].Team = 1;
		Config.Slots[1].Team = 2;
		Config.Slots[2].State = EHOLSlotState::AI;
		Config.Slots[2].Team = 2;
		Config.Slots[3].State = EHOLSlotState::Closed;
		State->InitVictory(Config);
		TArray<FHOLForumEvent> Events;
		State->OnForumEvent.AddLambda([&Events](const FHOLForumEvent& Event) { Events.Add(Event); });

		TestTrue(TEXT("Bob's seat is defeated (Disconnected)"), State->DefeatSlot(1, EHOLDefeatReason::Disconnected));
		TestEqual(TEXT("one event"), Events.Num(), 1);
		if (Events.Num() == 1)
		{
			TestTrue(TEXT("the kind is Defeated"), Events[0].Kind == EHOLForumEventKind::Defeated);
			TestTrue(TEXT("it carries the reason"), Events[0].Reason == EHOLForumEventReason::Disconnected);
			TestEqual(TEXT("the text Alice reads"), HOLForumMessages::GetMessage(Events[0], Config, 0, false).Text.ToString(), FString(TEXT("Bob a quitt\u00e9 la partie")));
		}
		int32 Defeats = 0;
		for (const FHOLMatchStatsEventEntry& Entry : Stats->GetStats().Events)
		{
			Defeats += (Entry.Kind == EHOLMatchStatsEvent::Defeated && Entry.PlayerIndex == 1) ? 1 : 0;
		}
		TestEqual(TEXT("the timeline counts one defeat for Bob's seat"), Defeats, 1);

		// A surrender stays \u00ab vaincu \u00bb and counts the same way.
		TestTrue(TEXT("Alice gives up"), State->DefeatSlot(0, EHOLDefeatReason::Surrender));
		TestTrue(TEXT("... the plain defeat, no reason"), Events.Num() == 2 && Events[1].Kind == EHOLForumEventKind::Defeated && Events[1].Reason == EHOLForumEventReason::None);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

// T9: the lines of hol.Disconnect.Report.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDisconnectReportTest, "HeroesOfLiberty.Match.DisconnectReport",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLDisconnectReportTest::RunTest(const FString& Parameters)
{
	using ESeat = FHOLDisconnectRules::ESeatState;
	TestEqual(TEXT("present"), FHOLDisconnectRules::FormatSeatReport(0, TEXT("Alice"), ESeat::Present, 0.), FString(TEXT("slot 0 (Alice) present")));
	TestEqual(TEXT("away, AI holds"), FHOLDisconnectRules::FormatSeatReport(1, TEXT("Bob"), ESeat::AbsentAIHolds, 161.2), FString(TEXT("slot 1 (Bob) away, AI holds, 162 s left")));
	TestEqual(TEXT("window over"), FHOLDisconnectRules::FormatSeatReport(2, TEXT("Cleo"), ESeat::AIHolds, 0.), FString(TEXT("slot 2 (Cleo) away, window over, the AI keeps the seat")));
	TestEqual(TEXT("defeated"), FHOLDisconnectRules::FormatSeatReport(3, TEXT("Dan"), ESeat::Defeated, 0.), FString(TEXT("slot 3 (Dan) defeated")));

	// The AI's mode ends the line of a seat it keeps (the report said who holds the seat, not how the AI plays it).
	using EMode = FHOLDisconnectRules::EAIMode;
	TestEqual(TEXT("away, hold mode"), FHOLDisconnectRules::FormatSeatReport(1, TEXT("Bob"), ESeat::AbsentAIHolds, 161.2, EMode::Hold),
		FString(TEXT("slot 1 (Bob) away, AI holds, 162 s left, AI mode hold")));
	TestEqual(TEXT("window over, full brain"), FHOLDisconnectRules::FormatSeatReport(2, TEXT("Cleo"), ESeat::AIHolds, 0., EMode::Full),
		FString(TEXT("slot 2 (Cleo) away, window over, the AI keeps the seat, AI mode full brain")));
	TestEqual(TEXT("present: no mode"), FHOLDisconnectRules::FormatSeatReport(0, TEXT("Alice"), ESeat::Present, 0., EMode::None), FString(TEXT("slot 0 (Alice) present")));
	return true;
}

#endif