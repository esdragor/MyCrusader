// The title screen's widget in script (ECR-1): UHOLTitleScreen::FindScriptClass (the class the menu's controller pushes as the root of the
// menu layer) finds Script/UI/HOLTitleScreen.as's class; the button labels are the base's (localised in C++, namespace HOLTitleScreen), the
// same words as before the switch. The screen on top in the running menu ("menu=HOLTitleScreenScript") is checked by quick_check's captures.

void Test_TitleScreenScriptClass(FUnitTest& T)
{
	const TSubclassOf<UHOLTitleScreen> Found = UHOLTitleScreen::FindScriptClass();
	T.AssertTrue(Found.IsValid(), "the title screen pushed by the menu has a script class");
	if (Found.IsValid())
		T.AssertEquals(Found.Get().GetName().ToString(), "HOLTitleScreenScript", "the title screen pushed by the menu is HOLTitleScreen.as's");
}

void Test_TitleScreenLabels(FUnitTest& T)
{
	T.AssertEquals(UHOLTitleScreen::ButtonLabel(n"PlayButton").ToString(), "Jouer", "the Play button's label");
	T.AssertEquals(UHOLTitleScreen::ButtonLabel(n"MultiplayerButton").ToString(), "Multijoueur", "the Multiplayer button's label");
	T.AssertEquals(UHOLTitleScreen::ButtonLabel(n"SettingsButton").ToString(), "Réglages", "the Settings button's label");
	T.AssertEquals(UHOLTitleScreen::ButtonLabel(n"QuitButton").ToString(), "Quitter", "the Quit button's label");
	T.AssertEquals(UHOLTitleScreen::ButtonLabel(n"Title").ToString(), "", "a name that is not a button has no label");
}
