// The settings screen's widget in script (D840 lot 2): UHOLSettingsScreen::Open (the title's Settings button, hol.UI.Open Settings) pushes the
// class FindScriptClass finds, the one of Script/UI/HOLSettingsScreen.as. The values its rotators offer come from the base (OptionsOf), the way
// a change reaches the settings from UHOLGameUserSettings (the rotator's handler calls the same functions; nothing is saved here).

void Test_SettingsScreenScriptClass(FUnitTest& T)
{
	const TSubclassOf<UHOLSettingsScreen> Found = UHOLSettingsScreen::FindScriptClass();
	T.AssertTrue(Found.IsValid(), "the settings screen opened by the title has a script class");
	if (Found.IsValid())
		T.AssertEquals(Found.Get().GetName().ToString(), "HOLSettingsScreenScript", "the settings screen opened by the title is HOLSettingsScreen.as's");
}

void Test_SettingsOptions(FUnitTest& T)
{
	T.AssertEquals(UHOLSettingsScreen::OptionsOf(n"Level").Num(), 5, "5 quality levels per group");
	T.AssertEquals(UHOLSettingsScreen::OptionsOf(n"Quality").Num(), 6, "the overall quality adds Custom to the 5 levels");
	T.AssertEquals(UHOLSettingsScreen::OptionsOf(n"Volume").Num(), 11, "a volume goes from 0 to 100 % in 10 steps");
	T.AssertEquals(UHOLSettingsScreen::OptionsOf(n"WindowMode").Num(), 3, "fullscreen, borderless window, window");
	T.AssertEquals(UHOLSettingsScreen::OptionsOf(n"Language").Num(), 3, "French, English, Japanese");
	T.AssertEquals(UHOLSettingsScreen::OptionsOf(n"FrameRate").Num(), 6, "5 frame rate limits and unlimited");
	T.AssertEquals(UHOLSettingsScreen::OptionsOf(n"RenderScale").Num(), 6, "6 render scales from 50 to 100 %");
	T.AssertTrue(!UHOLSettingsScreen::TextOf(n"Heading").IsEmpty(), "the screen's heading has a text");
	T.AssertTrue(!UHOLSettingsScreen::TextOf(n"Volume_Music").IsEmpty(), "a volume's label comes from its name");
}

void Test_SettingsVolumeEffect(FUnitTest& T)
{
	UHOLGameUserSettings Settings = UHOLGameUserSettings::Get();
	if (Settings == nullptr)
		return;
	const float Before = Settings.GetVolumeAt(1);
	// What the Music rotator's handler does at the step 3 of 10 (SettingChanged: SetVolumeAt(1, 3 / 10)), without saving.
	Settings.SetVolumeAt(1, float(3) / 10.0);
	T.AssertEquals(Math::RoundToInt(Settings.GetVolumeAt(1) * 10.0), 3, "the music volume read back is the step chosen");
	Settings.SetVolumeAt(1, Before);
	T.AssertEquals(Math::RoundToInt(Settings.GetVolumeAt(1) * 10.0), Math::RoundToInt(Before * 10.0), "the volume is put back");
}

void Test_SettingsQualityChoice(FUnitTest& T)
{
	T.AssertEquals(UHOLSettingsScreen::ResolveQualityChoice(2), 2, "a level of the 5 is taken as it is");
	const int Chosen = UHOLSettingsScreen::ResolveQualityChoice(5);
	T.AssertTrue(Chosen >= 0 && Chosen < 5, "Custom cannot be chosen: the rotator passes over it");
}
