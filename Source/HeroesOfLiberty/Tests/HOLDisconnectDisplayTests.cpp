// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLDisconnectRules.h"
#include "Match/HOLForumMessages.h"
#include "Match/HOLForumRules.h"
#include "Match/HOLMatchConfig.h"
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
	const FHOLForumEvent Left = FHOLForumEvent::Make(EHOLForumEventKind::Left, 1);
	TestEqual(TEXT("seen by another"), HOLForumMessages::GetMessage(Left, Config, 0, false).Text.ToString(), FString(TEXT("Bob a quitt\u00e9 la partie")));
	TestEqual(TEXT("seen by himself"), HOLForumMessages::GetMessage(Left, Config, 1, false).Text.ToString(), FString(TEXT("Vous avez quitt\u00e9 la partie")));
	TestFalse(TEXT("not the plain defeat"), HOLForumMessages::GetMessage(Left, Config, 0, false).Text.ToString().Contains(TEXT("vaincu")));
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
	return true;
}

#endif