// The in-game menu's rules in script (HOLGameMenuRules.as, MIG-P2a): the vectors of Source/HeroesOfLiberty/Tests/HOLGameMenuTests.cpp
// (Victory.GameMenu.Surrender, .QuitToMenu, .StatsButton, the lobby and list half of Victory.EndBanner.Which), the same as long as the
// C++ twin lives.

void Test_GameMenuCanSurrender(FUnitTest& T)
{
	T.AssertTrue(HOLGameMenuRules::CanSurrender(false, false), "standing, match going on: offered");
	T.AssertFalse(HOLGameMenuRules::CanSurrender(true, false), "defeated: not offered");
	T.AssertFalse(HOLGameMenuRules::CanSurrender(false, true), "match over: not offered");
	T.AssertFalse(HOLGameMenuRules::CanSurrender(true, true), "defeated, match over: not offered");
}

void Test_GameMenuCanQuitToMenu(FUnitTest& T)
{
	// User's trial of V1: a spectator needs a way back to the menu in the in-game menu.
	T.AssertFalse(HOLGameMenuRules::CanQuitToMenu(false, false), "standing, match going on: not offered (Abandonner is)");
	T.AssertTrue(HOLGameMenuRules::CanQuitToMenu(true, false), "defeated: offered");
	T.AssertTrue(HOLGameMenuRules::CanQuitToMenu(false, true), "match over: offered");
	T.AssertTrue(HOLGameMenuRules::CanQuitToMenu(true, true), "defeated, match over: offered");
}

void Test_GameMenuReturnToLobbyAndList(FUnitTest& T)
{
	// M6-Q7 revised, M6-Q10: « Retour au salon » is a lobby's players' alone.
	T.AssertTrue(HOLGameMenuRules::OffersReturnToLobby(EHOLGameMenuBanner::Defeat, EHOLGameMenuOrigin::Lobby), "lobby match, defeat: offered");
	T.AssertTrue(HOLGameMenuRules::OffersReturnToLobby(EHOLGameMenuBanner::Victory, EHOLGameMenuOrigin::Lobby), "lobby match, victory: offered");
	T.AssertFalse(HOLGameMenuRules::OffersReturnToLobby(EHOLGameMenuBanner::MatchOver, EHOLGameMenuOrigin::Lobby), "lobby match, no slot: not offered");
	T.AssertFalse(HOLGameMenuRules::OffersReturnToLobby(EHOLGameMenuBanner::MatchOver, EHOLGameMenuOrigin::Manager), "watched from the list: not offered");
	T.AssertFalse(HOLGameMenuRules::OffersReturnToLobby(EHOLGameMenuBanner::Victory, EHOLGameMenuOrigin::Manager), "watched from the list, a result anyway: not offered");
	T.AssertFalse(HOLGameMenuRules::OffersReturnToLobby(EHOLGameMenuBanner::Victory, EHOLGameMenuOrigin::Local), "no lobby: not offered");
	T.AssertFalse(HOLGameMenuRules::OffersReturnToLobby(EHOLGameMenuBanner::None, EHOLGameMenuOrigin::Lobby), "match going on: not offered");
	// R4.8, M6-Q10: whoever came from the manager quits to its list, the spectator of « Regarder » too.
	T.AssertTrue(HOLGameMenuRules::QuitsToList(EHOLGameMenuOrigin::Lobby), "lobby match: quit to the list");
	T.AssertTrue(HOLGameMenuRules::QuitsToList(EHOLGameMenuOrigin::Manager), "watched from the list: quit to the list");
	T.AssertFalse(HOLGameMenuRules::QuitsToList(EHOLGameMenuOrigin::Local), "local match: quit to the menu");
}

void Test_GameMenuStatsButton(FUnitTest& T)
{
	// M6-Q10: the spectator follows the statistics during the match; a player waits for the end, then everyone's.
	T.AssertTrue(HOLGameMenuRules::ShowsStatsButton(true, false), "a spectator during the match: shown");
	T.AssertFalse(HOLGameMenuRules::ShowsStatsButton(false, false), "a player during the match: hidden");
	T.AssertTrue(HOLGameMenuRules::ShowsStatsButton(true, true), "a spectator after the end: shown");
	T.AssertTrue(HOLGameMenuRules::ShowsStatsButton(false, true), "a player after the end: shown");
}

int Esc(EHOLGameMenuEscape Action)
{
	return int(Action);
}

// HeroesOfLiberty.Victory.GameMenu.Escape: what Escape does in a match.
void Test_GameMenuEscape(FUnitTest& T)
{
	// The menu open: Escape closes it, whatever lies below.
	T.AssertEquals(Esc(HOLGameMenuRules::GetEscapeAction(true, false, false)), Esc(EHOLGameMenuEscape::CloseMenu), "menu open, nothing selected: close");
	T.AssertEquals(Esc(HOLGameMenuRules::GetEscapeAction(true, false, true)), Esc(EHOLGameMenuEscape::CloseMenu), "menu open, units selected: close");
	T.AssertEquals(Esc(HOLGameMenuRules::GetEscapeAction(true, true, true)), Esc(EHOLGameMenuEscape::CloseMenu), "menu open, a mode armed: close");
	// A mode armed is left first, the selection kept (D313).
	T.AssertEquals(Esc(HOLGameMenuRules::GetEscapeAction(false, true, true)), Esc(EHOLGameMenuEscape::CancelMode), "mode armed, units selected: cancel the mode");
	T.AssertEquals(Esc(HOLGameMenuRules::GetEscapeAction(false, true, false)), Esc(EHOLGameMenuEscape::CancelMode), "mode armed, nothing selected: cancel the mode");
	// A selection is dropped, as before V1.3; nothing to undo: the menu.
	T.AssertEquals(Esc(HOLGameMenuRules::GetEscapeAction(false, false, true)), Esc(EHOLGameMenuEscape::Deselect), "units or a building selected: deselect");
	T.AssertEquals(Esc(HOLGameMenuRules::GetEscapeAction(false, false, false)), Esc(EHOLGameMenuEscape::OpenMenu), "nothing selected: open the menu");
}

// HeroesOfLiberty.Victory.GameMenu.Input: menu input while match screens open and close.
void Test_GameMenuKeepsInput(FUnitTest& T)
{
	T.AssertTrue(HOLGameMenuRules::KeepsMatchMenuInput(true, false), "a match screen opens: menu input on");
	T.AssertTrue(HOLGameMenuRules::KeepsMatchMenuInput(true, true), "the menu opens over the banner: on");
	T.AssertFalse(HOLGameMenuRules::KeepsMatchMenuInput(false, false), "the last match screen closes: off, Escape back to the match");
	// V1.4a: the menu closed over the end banner, Escape must still close the banner.
	T.AssertTrue(HOLGameMenuRules::KeepsMatchMenuInput(false, true), "the menu closes, the banner still open: kept");
}