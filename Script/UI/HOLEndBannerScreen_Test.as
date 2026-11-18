// The end banner's widget in script (MIG-CPP bascule EndBanner): UHOLEndBannerScreen::Open (the HUD, at each defeat or the match's end) pushes
// the class FindScriptClass finds, the one of Script/UI/HOLEndBannerScreen.as. It is not the in-game menu's class (UHOLGameMenuScreen::FindScriptClass
// finds its own, each base has its own).

void Test_EndBannerScreenScriptClass(FUnitTest& T)
{
	const TSubclassOf<UHOLEndBannerScreen> Found = UHOLEndBannerScreen::FindScriptClass();
	T.AssertTrue(Found.IsValid(), "the end banner opened by the HUD has a script class");
	if (Found.IsValid())
		T.AssertEquals(Found.Get().GetName().ToString(), "HOLEndBannerScreenScript", "the end banner opened by the HUD is HOLEndBannerScreen.as's");
	const TSubclassOf<UHOLGameMenuScreen> Menu = UHOLGameMenuScreen::FindScriptClass();
	if (Menu.IsValid())
		T.AssertEquals(Menu.Get().GetName().ToString(), "HOLGameMenuScreenScript", "the menu's class stays its own, not the banner's");
}
