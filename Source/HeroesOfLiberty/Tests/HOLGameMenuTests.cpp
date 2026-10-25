// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Player/HOLGameMenuRules.h"

#if WITH_DEV_AUTOMATION_TESTS

// V1.3a checks (D356, V1-Q1): what Escape does in a match, when « Abandonner » is offered.
namespace UE::HeroesOfLiberty::Tests::GameMenu
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGameMenuEscapeTest, "HeroesOfLiberty.Victory.GameMenu.Escape",
	UE::HeroesOfLiberty::Tests::GameMenu::Flags)

bool FHOLGameMenuEscapeTest::RunTest(const FString& Parameters)
{
	using HOLGameMenuRules::GetEscapeAction;
	// The menu open: Escape closes it, whatever lies below.
	TestTrue(TEXT("menu open, nothing selected: close"), GetEscapeAction(true, false, false) == EHOLEscapeAction::CloseMenu);
	TestTrue(TEXT("menu open, units selected: close"), GetEscapeAction(true, false, true) == EHOLEscapeAction::CloseMenu);
	TestTrue(TEXT("menu open, a mode armed: close"), GetEscapeAction(true, true, true) == EHOLEscapeAction::CloseMenu);
	// A mode armed is left first, the selection kept (D313).
	TestTrue(TEXT("mode armed, units selected: cancel the mode"), GetEscapeAction(false, true, true) == EHOLEscapeAction::CancelMode);
	TestTrue(TEXT("mode armed, nothing selected: cancel the mode"), GetEscapeAction(false, true, false) == EHOLEscapeAction::CancelMode);
	// A selection is dropped, as before V1.3.
	TestTrue(TEXT("units or a building selected: deselect"), GetEscapeAction(false, false, true) == EHOLEscapeAction::Deselect);
	// Nothing to undo: the menu.
	TestTrue(TEXT("nothing selected: open the menu"), GetEscapeAction(false, false, false) == EHOLEscapeAction::OpenMenu);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGameMenuSurrenderTest, "HeroesOfLiberty.Victory.GameMenu.Surrender",
	UE::HeroesOfLiberty::Tests::GameMenu::Flags)

bool FHOLGameMenuSurrenderTest::RunTest(const FString& Parameters)
{
	using HOLGameMenuRules::CanSurrender;
	TestTrue(TEXT("standing, match going on: offered"), CanSurrender(false, false));
	TestFalse(TEXT("defeated: not offered"), CanSurrender(true, false));
	TestFalse(TEXT("match over: not offered"), CanSurrender(false, true));
	TestFalse(TEXT("defeated, match over: not offered"), CanSurrender(true, true));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGameMenuQuitToMenuTest, "HeroesOfLiberty.Victory.GameMenu.QuitToMenu",
	UE::HeroesOfLiberty::Tests::GameMenu::Flags)

bool FHOLGameMenuQuitToMenuTest::RunTest(const FString& Parameters)
{
	using HOLGameMenuRules::CanQuitToMenu;
	// User's trial of V1: a spectator needs a way back to the menu in the in-game menu.
	TestFalse(TEXT("standing, match going on: not offered (Abandonner is)"), CanQuitToMenu(false, false));
	TestTrue(TEXT("defeated: offered"), CanQuitToMenu(true, false));
	TestTrue(TEXT("match over: offered"), CanQuitToMenu(false, true));
	TestTrue(TEXT("defeated, match over: offered"), CanQuitToMenu(true, true));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLEndBannerWhichTest, "HeroesOfLiberty.Victory.EndBanner.Which",
	UE::HeroesOfLiberty::Tests::GameMenu::Flags)

bool FHOLEndBannerWhichTest::RunTest(const FString& Parameters)
{
	using HOLGameMenuRules::GetEndBanner;
	// D357, V1-Q2: slot 0 is this player; masks bit Slot.
	TestTrue(TEXT("standing, match going on: none"), GetEndBanner(0, 0b10, false, 0) == EHOLEndBanner::None);
	TestTrue(TEXT("defeated, match going on: defeat"), GetEndBanner(0, 0b01, false, 0) == EHOLEndBanner::Defeat);
	TestTrue(TEXT("match over, my camp wins: victory"), GetEndBanner(0, 0b10, true, 0b01) == EHOLEndBanner::Victory);
	TestTrue(TEXT("match over, my camp wins though I fell: victory"), GetEndBanner(0, 0b11, true, 0b101) == EHOLEndBanner::Victory);
	TestTrue(TEXT("match over, my camp loses: defeat"), GetEndBanner(0, 0b01, true, 0b10) == EHOLEndBanner::Defeat);
	TestTrue(TEXT("match over, no winner: defeat"), GetEndBanner(0, 0b11, true, 0) == EHOLEndBanner::Defeat);
	// M6-Q10: a spectator (no slot) gets the neutral banner once the match is over, none before.
	TestTrue(TEXT("no slot, match going on: none"), GetEndBanner(INDEX_NONE, 0b01, false, 0) == EHOLEndBanner::None);
	TestTrue(TEXT("no slot, match over: match over"), GetEndBanner(INDEX_NONE, 0b11, true, 0b01) == EHOLEndBanner::MatchOver);
	// M6-Q7 revised, M6-Q10: « Retour au salon » is a lobby's players' alone (no countdown: they leave when they choose).
	using HOLGameMenuRules::OffersReturnToLobby;
	using HOLGameMenuRules::QuitsToList;
	TestTrue(TEXT("lobby match, defeat: back to the lobby offered"), OffersReturnToLobby(EHOLEndBanner::Defeat, EHOLMatchOrigin::Lobby));
	TestTrue(TEXT("lobby match, victory: back to the lobby offered"), OffersReturnToLobby(EHOLEndBanner::Victory, EHOLMatchOrigin::Lobby));
	TestFalse(TEXT("lobby match, no slot: not offered"), OffersReturnToLobby(EHOLEndBanner::MatchOver, EHOLMatchOrigin::Lobby));
	TestFalse(TEXT("watched from the list: not offered"), OffersReturnToLobby(EHOLEndBanner::MatchOver, EHOLMatchOrigin::Manager));
	TestFalse(TEXT("watched from the list, a result anyway: not offered"), OffersReturnToLobby(EHOLEndBanner::Victory, EHOLMatchOrigin::Manager));
	TestFalse(TEXT("no lobby: not offered"), OffersReturnToLobby(EHOLEndBanner::Victory, EHOLMatchOrigin::Local));
	TestFalse(TEXT("match going on: not offered"), OffersReturnToLobby(EHOLEndBanner::None, EHOLMatchOrigin::Lobby));
	// R4.8, M6-Q10: whoever came from the manager quits to its list, the spectator of « Regarder » too.
	TestTrue(TEXT("lobby match: quit to the list"), QuitsToList(EHOLMatchOrigin::Lobby));
	TestTrue(TEXT("watched from the list: quit to the list"), QuitsToList(EHOLMatchOrigin::Manager));
	TestFalse(TEXT("local match: quit to the menu"), QuitsToList(EHOLMatchOrigin::Local));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLEndBannerClosedTest, "HeroesOfLiberty.Victory.EndBanner.Closed",
	UE::HeroesOfLiberty::Tests::GameMenu::Flags)

bool FHOLEndBannerClosedTest::RunTest(const FString& Parameters)
{
	using HOLGameMenuRules::ShouldOpenEndBanner;
	constexpr EHOLEndBanner None = EHOLEndBanner::None;
	constexpr EHOLEndBanner Defeat = EHOLEndBanner::Defeat;
	constexpr EHOLEndBanner Victory = EHOLEndBanner::Victory;
	TestFalse(TEXT("nothing due: closed"), ShouldOpenEndBanner(None, None, None));
	TestTrue(TEXT("defeat due, never shown: open"), ShouldOpenEndBanner(Defeat, None, None));
	TestFalse(TEXT("defeat already shown: not again"), ShouldOpenEndBanner(Defeat, Defeat, None));
	TestFalse(TEXT("defeat closed by Continue watching: not again"), ShouldOpenEndBanner(Defeat, None, Defeat));
	TestTrue(TEXT("defeat closed, then the team's victory: open"), ShouldOpenEndBanner(Victory, None, Defeat));
	TestTrue(TEXT("defeat shown, then the team's victory: replaced"), ShouldOpenEndBanner(Victory, Defeat, None));
	TestFalse(TEXT("victory closed: not again"), ShouldOpenEndBanner(Victory, None, Victory));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGameMenuInputTest, "HeroesOfLiberty.Victory.GameMenu.Input",
	UE::HeroesOfLiberty::Tests::GameMenu::Flags)

bool FHOLGameMenuInputTest::RunTest(const FString& Parameters)
{
	using HOLGameMenuRules::KeepsMatchMenuInput;
	TestTrue(TEXT("a match screen opens: menu input on"), KeepsMatchMenuInput(true, false));
	TestTrue(TEXT("the menu opens over the banner: on"), KeepsMatchMenuInput(true, true));
	TestFalse(TEXT("the last match screen closes: off, Escape back to the match"), KeepsMatchMenuInput(false, false));
	// V1.4a: the menu closed over the end banner, Escape must still close the banner.
	TestTrue(TEXT("the menu closes, the banner still open: kept"), KeepsMatchMenuInput(false, true));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGameMenuStatsButtonTest, "HeroesOfLiberty.Victory.GameMenu.StatsButton",
	UE::HeroesOfLiberty::Tests::GameMenu::Flags)

bool FHOLGameMenuStatsButtonTest::RunTest(const FString& Parameters)
{
	using HOLGameMenuRules::ShowsStatsButton;
	// M6-Q10: the spectator follows the statistics during the match; a player waits for the end. M6-Q7 revised: after
	// it, everyone stays as long as he likes, the end banner closed: the button is everyone's.
	TestTrue(TEXT("a spectator during the match: shown"), ShowsStatsButton(true, false));
	TestFalse(TEXT("a player during the match: hidden"), ShowsStatsButton(false, false));
	TestTrue(TEXT("a spectator after the end: shown"), ShowsStatsButton(true, true));
	TestTrue(TEXT("a player after the end: shown"), ShowsStatsButton(false, true));
	return true;
}

#endif
