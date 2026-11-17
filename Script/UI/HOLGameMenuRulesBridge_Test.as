// The bridge the C++ callers read the menu's rules through (HOLGameMenuRulesBridge.as): its class is found, and what it answers is the rules'
// vectors (HOLGameMenuRules_Test.as, HOLEndBannerScript_Test.as), here by the C++ way in (UHOLGameMenuRulesBridge::Get).

void Test_GameMenuBridgeClass(FUnitTest& T)
{
	UHOLGameMenuRulesBridge Bridge = UHOLGameMenuRulesBridge::Get();
	T.AssertTrue(Bridge != nullptr, "the C++ callers have a script class to read the menu's rules through");
	if (Bridge != nullptr)
		T.AssertEquals(Bridge.Class.Name.ToString(), "HOLGameMenuRulesScript", "the class is HOLGameMenuRulesBridge.as's");
}

void Test_GameMenuBridgeAnswers(FUnitTest& T)
{
	// Typed as the script class: the C++ statics of the base share the overrides' names (not callable from script), the script's own methods are.
	UHOLGameMenuRulesScript Bridge = Cast<UHOLGameMenuRulesScript>(UHOLGameMenuRulesBridge::Get());
	if (Bridge == nullptr)
	{
		T.AssertTrue(false, "no bridge");
		return;
	}
	T.AssertEquals(Bridge.GetEscapeAction(true, true, true), int(EHOLGameMenuEscape::CloseMenu), "menu open: Escape closes it");
	T.AssertEquals(Bridge.GetEscapeAction(false, true, true), int(EHOLGameMenuEscape::CancelMode), "mode armed: Escape leaves it");
	T.AssertEquals(Bridge.GetEscapeAction(false, false, true), int(EHOLGameMenuEscape::Deselect), "selection: Escape deselects");
	T.AssertEquals(Bridge.GetEscapeAction(false, false, false), int(EHOLGameMenuEscape::OpenMenu), "nothing: Escape opens the menu");
	T.AssertTrue(Bridge.CanSurrender(false, false), "standing: Abandonner");
	T.AssertFalse(Bridge.CanSurrender(true, false), "defeated: no Abandonner");
	T.AssertEquals(Bridge.GetEndBanner(0, 2, true, 1), int(EHOLGameMenuBanner::Victory), "my camp wins: victory");
	T.AssertEquals(Bridge.GetEndBanner(-1, 0, true, 1), int(EHOLGameMenuBanner::MatchOver), "spectator, match over: match over");
	T.AssertTrue(Bridge.QuitsToList(int(EHOLGameMenuOrigin::Lobby)), "a lobby's match quits to the list");
	T.AssertFalse(Bridge.QuitsToList(int(EHOLGameMenuOrigin::Local)), "a local match quits to the menu");
	T.AssertTrue(Bridge.OffersReturnToLobby(int(EHOLGameMenuBanner::Victory), int(EHOLGameMenuOrigin::Lobby)), "lobby player, victory: back to the lobby");
	T.AssertFalse(Bridge.ShouldOpenEndBanner(int(EHOLGameMenuBanner::Defeat), int(EHOLGameMenuBanner::Defeat), 0), "already shown: not opened again");
	T.AssertTrue(Bridge.KeepsMatchMenuInput(true, false), "opening: input kept");
	T.AssertTrue(Bridge.ShowsStatsButton(true, false), "spectator: statistics");
	T.AssertFalse(Bridge.CanQuitToMenu(false, false), "standing: no quit to menu");
}
