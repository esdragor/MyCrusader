// The end banner in script (HOLEndBannerScript.as, MIG-P2d): the vectors of Victory.EndBanner.Which and .Closed
// (Source/HeroesOfLiberty/Tests/HOLGameMenuTests.cpp), the same as long as the C++ twin lives, and the banner's view.

int B(EHOLGameMenuBanner Banner)
{
	return int(Banner);
}

void Test_EndBannerWhich(FUnitTest& T)
{
	// D357, V1-Q2: slot 0 is this player; masks bit Slot.
	T.AssertEquals(B(HOLEndBannerScript::GetEndBanner(0, 2, false, 0)), B(EHOLGameMenuBanner::None), "standing, match going on: none");
	T.AssertEquals(B(HOLEndBannerScript::GetEndBanner(0, 1, false, 0)), B(EHOLGameMenuBanner::Defeat), "defeated, match going on: defeat");
	T.AssertEquals(B(HOLEndBannerScript::GetEndBanner(0, 2, true, 1)), B(EHOLGameMenuBanner::Victory), "match over, my camp wins: victory");
	T.AssertEquals(B(HOLEndBannerScript::GetEndBanner(0, 3, true, 5)), B(EHOLGameMenuBanner::Victory), "match over, my camp wins though I fell: victory");
	T.AssertEquals(B(HOLEndBannerScript::GetEndBanner(0, 1, true, 2)), B(EHOLGameMenuBanner::Defeat), "match over, my camp loses: defeat");
	T.AssertEquals(B(HOLEndBannerScript::GetEndBanner(0, 3, true, 0)), B(EHOLGameMenuBanner::Defeat), "match over, no winner: defeat");
	// M6-Q10: a spectator (no slot) gets the neutral banner once the match is over, none before.
	T.AssertEquals(B(HOLEndBannerScript::GetEndBanner(-1, 1, false, 0)), B(EHOLGameMenuBanner::None), "no slot, match going on: none");
	T.AssertEquals(B(HOLEndBannerScript::GetEndBanner(-1, 3, true, 1)), B(EHOLGameMenuBanner::MatchOver), "no slot, match over: match over");
}

void Test_EndBannerClosed(FUnitTest& T)
{
	const EHOLGameMenuBanner None = EHOLGameMenuBanner::None;
	const EHOLGameMenuBanner Defeat = EHOLGameMenuBanner::Defeat;
	const EHOLGameMenuBanner Victory = EHOLGameMenuBanner::Victory;
	T.AssertFalse(HOLEndBannerScript::ShouldOpenEndBanner(None, None, None), "nothing due: closed");
	T.AssertTrue(HOLEndBannerScript::ShouldOpenEndBanner(Defeat, None, None), "defeat due, never shown: open");
	T.AssertFalse(HOLEndBannerScript::ShouldOpenEndBanner(Defeat, Defeat, None), "defeat already shown: not again");
	T.AssertFalse(HOLEndBannerScript::ShouldOpenEndBanner(Defeat, None, Defeat), "defeat closed by Continue watching: not again");
	T.AssertTrue(HOLEndBannerScript::ShouldOpenEndBanner(Victory, None, Defeat), "defeat closed, then the team's victory: open");
	T.AssertTrue(HOLEndBannerScript::ShouldOpenEndBanner(Victory, Defeat, None), "defeat shown, then the team's victory: replaced");
	T.AssertFalse(HOLEndBannerScript::ShouldOpenEndBanner(Victory, None, Victory), "victory closed: not again");
}

void Test_EndBannerScriptView(FUnitTest& T)
{
	// A lobby's winner: « Continuer », the lobby offered, the quit to the list.
	const FHOLEndBannerView Won = HOLEndBannerScript::Make(HOLEndBannerScript::GetEndBanner(0, 2, true, 1), EHOLGameMenuOrigin::Lobby);
	T.AssertEquals(Won.Describe, "victory continue return quit list stats", "a lobby's winner");
	T.AssertEquals(HOLEndBannerScript::Label(Won, n"Title"), "Victoire", "title");
	T.AssertEquals(HOLEndBannerScript::Label(Won, n"ContinueButton"), "Continuer", "a winner continues");
	T.AssertEquals(int(HOLEndBannerScript::Click(Won, n"ReturnButton")), int(EHOLEndBannerAction::ReturnToLobby), "Retour au salon");
	// A local loser: « Continuer à regarder », no lobby, the quit to the menu.
	const FHOLEndBannerView Lost = HOLEndBannerScript::Make(HOLEndBannerScript::GetEndBanner(0, 1, false, 0), EHOLGameMenuOrigin::Local);
	T.AssertEquals(Lost.Describe, "defeat continue quit menu stats", "a local loser");
	T.AssertEquals(HOLEndBannerScript::Label(Lost, n"ContinueButton"), "Continuer à regarder", "a loser watches on");
	T.AssertEquals(int(HOLEndBannerScript::Click(Lost, n"ReturnButton")), int(EHOLEndBannerAction::None), "no lobby: the hidden Retour au salon does nothing");
	T.AssertEquals(int(HOLEndBannerScript::Click(Lost, n"QuitButton")), int(EHOLEndBannerAction::QuitToMenu), "local: to the menu");
	// M6-Q10: the spectator of « Regarder »: the neutral title, the list.
	const FHOLEndBannerView Watched = HOLEndBannerScript::Make(HOLEndBannerScript::GetEndBanner(-1, 3, true, 1), EHOLGameMenuOrigin::Manager);
	T.AssertEquals(Watched.Describe, "over continue quit list stats", "a spectator from the list");
	T.AssertEquals(HOLEndBannerScript::Label(Watched, n"Title"), "Partie terminée", "neutral title");
	T.AssertEquals(int(HOLEndBannerScript::Click(Watched, n"StatsButton")), int(EHOLEndBannerAction::Stats), "Statistiques for everyone");
	// Nothing due: no banner.
	const FHOLEndBannerView Hidden = HOLEndBannerScript::Make(EHOLGameMenuBanner::None, EHOLGameMenuOrigin::Local);
	T.AssertEquals(Hidden.Describe, "hidden", "nothing due");
	T.AssertEquals(int(HOLEndBannerScript::Click(Hidden, n"ContinueButton")), int(EHOLEndBannerAction::None), "hidden: nothing");
}
