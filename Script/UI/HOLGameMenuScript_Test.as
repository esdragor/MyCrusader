// The in-game menu as a view (HOLGameMenuScript.as, MIG-P2b): its buttons as UHOLGameMenu shows them (RefreshSurrender, RefreshPause, SetOpen),
// the clicks on hidden or greyed ones refused, the labels by the match's origin.

FHOLGameMenuView StandingNetworked(int PausesLeft, bool bCanPauseNow)
{
	return HOLGameMenuScript::Make(true, 0, false, false, false, false, PausesLeft, bCanPauseNow, EHOLGameMenuBanner::None, EHOLGameMenuOrigin::Local);
}

void Test_GameMenuScriptButtons(FUnitTest& T)
{
	const FHOLGameMenuView Standing = StandingNetworked(2, true);
	T.AssertTrue(Standing.bSurrender, "standing, match going on: Abandonner shown");
	T.AssertEquals(Standing.Describe, "menu resume pause 2 surrender", "standing player's line");
	T.AssertEquals(int(HOLGameMenuScript::Click(Standing, n"SurrenderButton")), int(EHOLGameMenuAction::Surrender), "Abandonner asks to surrender");

	// D357: the defeated player has no « Abandonner », but the way out.
	const FHOLGameMenuView Defeated = HOLGameMenuScript::Make(true, 0, true, false, false, false, 2, false, EHOLGameMenuBanner::Defeat, EHOLGameMenuOrigin::Local);
	T.AssertFalse(Defeated.bSurrender, "defeated: Abandonner absent");
	T.AssertTrue(Defeated.bQuit, "defeated: Quitter vers le menu in its place");
	T.AssertEquals(Defeated.Describe, "menu resume quit menu", "defeated player's line: no pause, no result before the end");
	T.AssertEquals(int(HOLGameMenuScript::Click(Defeated, n"SurrenderButton")), int(EHOLGameMenuAction::None), "a click on the hidden Abandonner does nothing");

	// M6-Q7 revised: once over, a lobby's player is offered the lobby, the list and the result.
	const FHOLGameMenuView LobbyOver = HOLGameMenuScript::Make(true, 0, false, true, false, false, 2, false, EHOLGameMenuBanner::Victory, EHOLGameMenuOrigin::Lobby);
	T.AssertEquals(LobbyOver.Describe, "menu resume return quit list result", "lobby match over");
	T.AssertEquals(int(HOLGameMenuScript::Click(LobbyOver, n"ReturnButton")), int(EHOLGameMenuAction::ReturnToLobby), "Retour au salon");
	T.AssertEquals(int(HOLGameMenuScript::Click(LobbyOver, n"QuitButton")), int(EHOLGameMenuAction::QuitToList), "from a lobby, the quit goes to the list");
	T.AssertEquals(int(HOLGameMenuScript::Click(LobbyOver, n"ResultButton")), int(EHOLGameMenuAction::ShowResult), "Résultat");

	// M6-Q10: the spectator of « Regarder », the match over: the list and the result, no lobby.
	const FHOLGameMenuView Watched = HOLGameMenuScript::Make(true, -1, false, true, false, false, 2, false, EHOLGameMenuBanner::MatchOver, EHOLGameMenuOrigin::Manager);
	T.AssertEquals(Watched.Describe, "menu resume quit list result", "watched match over");

	// No game or player state yet: Reprendre alone.
	const FHOLGameMenuView Unknown = HOLGameMenuScript::Make(false, 0, false, false, false, false, 2, true, EHOLGameMenuBanner::None, EHOLGameMenuOrigin::Local);
	T.AssertEquals(Unknown.Describe, "menu resume", "nothing known: resume alone");
	T.AssertEquals(int(HOLGameMenuScript::Click(Unknown, n"ResumeButton")), int(EHOLGameMenuAction::Resume), "Reprendre always closes");
}

void Test_GameMenuScriptPause(FUnitTest& T)
{
	// D513: the pauses left shown in a networked match, greyed with none left.
	const FHOLGameMenuView NoneLeft = StandingNetworked(0, false);
	T.AssertEquals(NoneLeft.Describe, "menu resume pause 0 greyed surrender", "no pause left: greyed");
	T.AssertEquals(int(HOLGameMenuScript::Click(NoneLeft, n"PauseButton")), int(EHOLGameMenuAction::None), "a greyed Pause does nothing");
	T.AssertEquals(HOLGameMenuScript::Label(StandingNetworked(2, true), n"PauseButton"), "Pause (2)", "networked: the count in the label");
	T.AssertEquals(int(HOLGameMenuScript::Click(StandingNetworked(2, true), n"PauseButton")), int(EHOLGameMenuAction::Pause), "Pause asks the server");
	// Solo pauses freely: no count.
	const FHOLGameMenuView Solo = HOLGameMenuScript::Make(true, 0, false, false, false, true, 0, true, EHOLGameMenuBanner::None, EHOLGameMenuOrigin::Local);
	T.AssertEquals(Solo.Describe, "menu resume pause surrender", "solo: a free pause");
	T.AssertEquals(HOLGameMenuScript::Label(Solo, n"PauseButton"), "Pause", "solo: no count");
	// Already paused, or a spectator: no Pause.
	const FHOLGameMenuView Paused = HOLGameMenuScript::Make(true, 0, false, false, true, false, 1, false, EHOLGameMenuBanner::None, EHOLGameMenuOrigin::Local);
	T.AssertFalse(Paused.bPause, "the match paused: no Pause");
	const FHOLGameMenuView Spectator = HOLGameMenuScript::Make(true, -1, false, false, false, false, 2, false, EHOLGameMenuBanner::None, EHOLGameMenuOrigin::Manager);
	T.AssertFalse(Spectator.bPause, "a spectator: no Pause");
}

void Test_GameMenuScriptLabels(FUnitTest& T)
{
	const FHOLGameMenuView Local = HOLGameMenuScript::Make(true, 0, true, false, false, false, 2, false, EHOLGameMenuBanner::Defeat, EHOLGameMenuOrigin::Local);
	const FHOLGameMenuView Lobby = HOLGameMenuScript::Make(true, 0, true, false, false, false, 2, false, EHOLGameMenuBanner::Defeat, EHOLGameMenuOrigin::Lobby);
	T.AssertEquals(HOLGameMenuScript::Label(Local, n"QuitButton"), "Quitter vers le menu", "local match: to the menu");
	T.AssertEquals(HOLGameMenuScript::Label(Lobby, n"QuitButton"), "Quitter vers la liste", "from the manager: to the list");
	T.AssertEquals(int(HOLGameMenuScript::Click(Local, n"QuitButton")), int(EHOLGameMenuAction::QuitToMenu), "local match: quit to the menu");
	T.AssertEquals(HOLGameMenuScript::Label(Local, n"ResumeButton"), "Reprendre", "Reprendre");
	T.AssertEquals(HOLGameMenuScript::Label(Local, n"Nothing"), "", "not the menu's");
	T.AssertEquals(int(HOLGameMenuScript::Click(Local, n"Nothing")), int(EHOLGameMenuAction::None), "not the menu's: nothing");
}
