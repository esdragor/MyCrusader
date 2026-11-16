// The in-game menu's widget in script (MIG-CPP bascule GameMenu): UHOLGameMenuScreen::Open (Escape, « Menu », hol.UI.Open GameMenu) pushes
// the class FindScriptClass finds, the one of Script/UI/HOLGameMenuScreen.as; without a match, ReadFacts knows nothing and the view
// made from it is « Reprendre » alone, as the C++ menu without a game or player state.

void Test_GameMenuScreenScriptClass(FUnitTest& T)
{
	const TSubclassOf<UHOLGameMenuScreen> Found = UHOLGameMenuScreen::FindScriptClass();
	T.AssertTrue(Found.IsValid(), "the menu opened by Escape has a script class");
	if (Found.IsValid())
		T.AssertEquals(Found.Get().GetName().ToString(), "HOLGameMenuScreenScript", "the menu opened by Escape is HOLGameMenuScreen.as's");
}

void Test_GameMenuScreenNoMatch(FUnitTest& T)
{
	const FHOLGameMenuFacts Facts = UHOLGameMenuScreen::ReadFacts(nullptr);
	T.AssertTrue(!Facts.bKnown, "no player: nothing known of the match");
	T.AssertEquals(Facts.Origin, 0, "no game instance: a local match");
	const FHOLGameMenuView View = HOLGameMenuScript::Make(Facts.bKnown, Facts.MySlot, Facts.bDefeated, Facts.bMatchOver, Facts.bPaused,
		Facts.bPauseFree, Facts.PausesLeft, Facts.bCanPauseNow, EHOLGameMenuBanner::None, EHOLGameMenuOrigin(Facts.Origin));
	T.AssertEquals(View.Describe, "menu resume", "without a match: Reprendre alone");
}
