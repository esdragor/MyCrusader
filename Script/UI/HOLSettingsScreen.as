// The settings screen's widget in script (D840, lot 2, as HOLEndBannerScreen.as): UHOLSettingsScreen::Open (the title's Settings button, hol.UI.Open
// Settings) pushes the class FindScriptClass finds, the one of this file. Its column, 4 tabs (Video, Sound, Controls, Language) and rotators are built
// here (BuildSettings, in place of BuildLayout) with the same widget names as the C++ screen had (the scenarios and hol.UI.Click aim at them); each change
// (SettingChanged) goes to UHOLGameUserSettings / the engine's settings, is applied and saved at once, and the values are shown again (RefreshSettings).
// The base keeps the opening, the navigation columns, the localized texts (TextOf, OptionsOf), the window's display (r.SetRes) and the language.
class UHOLSettingsScreenScript : UHOLSettingsScreen
{
	// The names of the quality groups and of the volumes, in the order of the settings (UHOLSettingsScreen::GetGroupLevel, EHOLVolume).
	TArray<FName> GroupNames;
	TArray<FName> VolumeNames;

	UFUNCTION(BlueprintOverride)
	void BuildSettings()
	{
		GroupNames.Reset();
		GroupNames.Add(n"ViewDistance");
		GroupNames.Add(n"AntiAliasing");
		GroupNames.Add(n"GlobalIllumination");
		GroupNames.Add(n"Texture");
		GroupNames.Add(n"VisualEffect");
		GroupNames.Add(n"Foliage");
		GroupNames.Add(n"Shading");
		GroupNames.Add(n"Landscape");
		VolumeNames.Reset();
		VolumeNames.Add(n"Master");
		VolumeNames.Add(n"Music");
		VolumeNames.Add(n"Effects");
		VolumeNames.Add(n"Voice");

		UVerticalBox Column = MakeSettingsColumn();
		AddSettingsText(Column, n"Heading", UHOLSettingsScreen::TextOf(n"Heading"), 0);

		UHorizontalBox TabRow = AddSettingsRow(Column);
		VideoTab = AddSettingsTab(TabRow, n"VideoTab", UHOLSettingsScreen::TextOf(n"Video"));
		SoundTab = AddSettingsTab(TabRow, n"SoundTab", UHOLSettingsScreen::TextOf(n"Sound"));
		ControlsTab = AddSettingsTab(TabRow, n"ControlsTab", UHOLSettingsScreen::TextOf(n"Controls"));
		LanguageTab = AddSettingsTab(TabRow, n"LanguageTab", UHOLSettingsScreen::TextOf(n"Language"));
		TabLinks();

		AddSettingsPages(Column);

		// Video: display on the left, quality on the right, the gamepad hint below.
		UVerticalBox Video = AddSettingsPage();
		UHorizontalBox Pair = AddSettingsPair(Video);
		UVerticalBox Display = AddSettingsPairColumn(Pair, false);
		UVerticalBox Quality = AddSettingsPairColumn(Pair, true);
		ResolutionRotator = AddSettingsRotator(Display, n"ResolutionRotator", UHOLSettingsScreen::TextOf(n"Resolution"), GetResolutionNames(), 0, 1);
		WindowModeRotator = AddSettingsRotator(Display, n"WindowModeRotator", UHOLSettingsScreen::TextOf(n"WindowMode"), UHOLSettingsScreen::OptionsOf(n"WindowMode"), 0, 1);
		VSyncRotator = AddSettingsRotator(Display, n"VSyncRotator", UHOLSettingsScreen::TextOf(n"VSync"), UHOLSettingsScreen::OptionsOf(n"OffOn"), 0, 1);
		FrameRateRotator = AddSettingsRotator(Display, n"FrameRateRotator", UHOLSettingsScreen::TextOf(n"FrameRate"), UHOLSettingsScreen::OptionsOf(n"FrameRate"), 0, 1);
		RenderScaleRotator = AddSettingsRotator(Display, n"RenderScaleRotator", UHOLSettingsScreen::TextOf(n"RenderScale"), UHOLSettingsScreen::OptionsOf(n"RenderScale"), 0, 1);
		QualityRotator = AddSettingsRotator(Quality, n"QualityRotator", UHOLSettingsScreen::TextOf(n"Quality"), UHOLSettingsScreen::OptionsOf(n"Quality"), 0, 2);
		GroupRotators.Reset();
		for (int Group = 0; Group < GroupNames.Num(); Group++)
		{
			UHOLRotator Rotator = AddSettingsRotator(Quality, GroupRotatorName(Group), UHOLSettingsScreen::TextOf(GroupNames[Group]), UHOLSettingsScreen::OptionsOf(n"Level"), 0, 2);
			GroupRotators.Add(Rotator);
		}
		AddSettingsHint(Video);

		UVerticalBox Sound = AddSettingsPage();
		VolumeRotators.Reset();
		for (int Volume = 0; Volume < VolumeNames.Num(); Volume++)
		{
			FName Key = VolumeRotatorName(Volume);
			UHOLRotator Rotator = AddSettingsRotator(Sound, Key, UHOLSettingsScreen::TextOf(Key), UHOLSettingsScreen::OptionsOf(n"Volume"), 10, 0);
			VolumeRotators.Add(Rotator);
		}
		AddSettingsText(Sound, n"SoundNote", UHOLSettingsScreen::TextOf(n"SoundNote"), 1);

		// Controls: greyed until the game has enough actions to remap (D253); camera speed and edge scrolling act since J1.
		UVerticalBox Controls = AddSettingsPage();
		RemapButton = AddSettingsPageButton(Controls, n"RemapButton", UHOLSettingsScreen::TextOf(n"Remap"));
		CameraSpeedRotator = AddSettingsRotator(Controls, n"CameraSpeedRotator", UHOLSettingsScreen::TextOf(n"CameraSpeed"), UHOLSettingsScreen::OptionsOf(n"CameraSpeed"), 1, 0);
		EdgeScrollRotator = AddSettingsRotator(Controls, n"EdgeScrollRotator", UHOLSettingsScreen::TextOf(n"EdgeScroll"), UHOLSettingsScreen::OptionsOf(n"OffOn"), 1, 0);
		AddSettingsText(Controls, n"ControlsNote", UHOLSettingsScreen::TextOf(n"RemapNote"), 1);
		if (RemapButton != nullptr)
			RemapButton.SetIsEnabled(false);

		UVerticalBox Language = AddSettingsPage();
		LanguageRotator = AddSettingsRotator(Language, n"LanguageRotator", UHOLSettingsScreen::TextOf(n"LanguageLabel"), UHOLSettingsScreen::OptionsOf(n"Language"), 0, 0);

		UHorizontalBox Buttons = AddSettingsRow(Column);
		BackButton = AddSettingsButton(Buttons, n"BackButton", UHOLSettingsScreen::TextOf(n"Back"));
		if (BackButton != nullptr)
			BackButton.OnButtonBaseClicked.AddUFunction(this, n"HandleButtonClicked");
	}

	FName GroupRotatorName(int Group)
	{
		return FName("Quality_" + GroupNames[Group].ToString());
	}

	FName VolumeRotatorName(int Volume)
	{
		return FName("Volume_" + VolumeNames[Volume].ToString());
	}

	void TabLinks()
	{
		if (VideoTab != nullptr)
			VideoTab.OnButtonBaseClicked.AddUFunction(this, n"HandleButtonClicked");
		if (SoundTab != nullptr)
			SoundTab.OnButtonBaseClicked.AddUFunction(this, n"HandleButtonClicked");
		if (ControlsTab != nullptr)
			ControlsTab.OnButtonBaseClicked.AddUFunction(this, n"HandleButtonClicked");
		if (LanguageTab != nullptr)
			LanguageTab.OnButtonBaseClicked.AddUFunction(this, n"HandleButtonClicked");
	}

	UFUNCTION()
	void HandleButtonClicked(UCommonButtonBase Button)
	{
		if (Button == nullptr)
			return;
		if (Button == VideoTab)
			SelectTab(0);
		else if (Button == SoundTab)
			SelectTab(1);
		else if (Button == ControlsTab)
			SelectTab(2);
		else if (Button == LanguageTab)
			SelectTab(3);
		else if (Button == BackButton)
			CloseSettings();
	}

	// Shows the values of the settings.
	UFUNCTION(BlueprintOverride)
	void RefreshSettings()
	{
		ShowSettings();
	}

	// The body of RefreshSettings: an override cannot be called from the script, so the changes call this.
	void ShowSettings()
	{
		UHOLGameUserSettings Settings = UHOLGameUserSettings::Get();
		if (Settings == nullptr)
		{
			UHOLGameMenuScreen::MenuLog("Settings: the game user settings are not UHOLGameUserSettings (DefaultEngine.ini, GameUserSettingsClassName)");
			return;
		}
		// The window as it is, which the command line can impose over the saved settings.
		if (ResolutionRotator != nullptr)
			ResolutionRotator.SetSelectedItem(GetShownResolutionIndex());
		if (WindowModeRotator != nullptr)
			WindowModeRotator.SetSelectedItem(UHOLSettingsScreen::GetShownWindowMode());
		if (VSyncRotator != nullptr)
			VSyncRotator.SetSelectedItem(Settings.IsVSyncEnabled() ? 1 : 0);
		if (FrameRateRotator != nullptr)
			FrameRateRotator.SetSelectedItem(UHOLSettingsScreen::GetFrameRateIndex());
		if (RenderScaleRotator != nullptr)
			RenderScaleRotator.SetSelectedItem(UHOLSettingsScreen::GetRenderScaleIndex());
		if (QualityRotator != nullptr)
			QualityRotator.SetSelectedItem(UHOLSettingsScreen::GetQualityIndex());
		for (int Group = 0; Group < GroupRotators.Num(); Group++)
		{
			UHOLRotator GroupRotator = GroupRotators[Group];
			if (GroupRotator != nullptr)
				GroupRotator.SetSelectedItem(UHOLSettingsScreen::GetGroupLevel(Group));
		}
		for (int Volume = 0; Volume < VolumeRotators.Num(); Volume++)
		{
			UHOLRotator VolumeRotator = VolumeRotators[Volume];
			if (VolumeRotator != nullptr)
				VolumeRotator.SetSelectedItem(Math::RoundToInt(Settings.GetVolumeAt(Volume) * 10.0));
		}
		if (LanguageRotator != nullptr)
			LanguageRotator.SetSelectedItem(UHOLSettingsScreen::GetLanguageIndex());
		if (CameraSpeedRotator != nullptr)
			CameraSpeedRotator.SetSelectedItem(Settings.GetCameraSpeedIndex());
		if (EdgeScrollRotator != nullptr)
			EdgeScrollRotator.SetSelectedItem(Settings.IsEdgeScrollEnabled() ? 1 : 0);
	}

	// Applies and saves the settings, then shows them (a change can move others: overall quality and groups). The resolution and the window mode are
	// applied only when they are the ones changed, so that no other change resizes the window to saved values.
	void ApplyAndRefresh(bool bResolutionChanged)
	{
		UHOLGameUserSettings Settings = UHOLGameUserSettings::Get();
		if (Settings != nullptr)
		{
			if (bResolutionChanged)
				Settings.ApplyResolutionSettings(false);
			Settings.ApplyNonResolutionSettings();
			Settings.SaveSettings();
		}
		ShowSettings();
	}

	UFUNCTION(BlueprintOverride)
	void SettingChanged(FName Name, int Value)
	{
		UHOLGameUserSettings Settings = UHOLGameUserSettings::Get();
		if (Name == n"ResolutionRotator")
		{
			ChooseResolution(Value);
			ApplyAndRefresh(true);
			return;
		}
		if (Name == n"WindowModeRotator")
		{
			UHOLSettingsScreen::ChooseWindowMode(Value);
			ApplyAndRefresh(true);
			return;
		}
		if (Name == n"LanguageRotator")
		{
			UHOLSettingsScreen::SetLanguageIndex(Value);
			ShowSettings();
			return;
		}
		if (Settings != nullptr)
		{
			if (Name == n"VSyncRotator")
				Settings.SetVSyncEnabled(Value == 1);
			else if (Name == n"FrameRateRotator")
				UHOLSettingsScreen::SetFrameRateIndex(Value);
			else if (Name == n"RenderScaleRotator")
				UHOLSettingsScreen::SetRenderScaleIndex(Value);
			else if (Name == n"QualityRotator")
				Settings.SetOverallScalabilityLevel(UHOLSettingsScreen::ResolveQualityChoice(Value));
			else if (Name == n"CameraSpeedRotator")
				Settings.SetCameraSpeedIndex(Value);
			else if (Name == n"EdgeScrollRotator")
				Settings.SetEdgeScrollEnabled(Value == 1);
			else
			{
				for (int Group = 0; Group < GroupNames.Num(); Group++)
				{
					if (Name == GroupRotatorName(Group))
						UHOLSettingsScreen::SetGroupLevel(Group, Value);
				}
				for (int Volume = 0; Volume < VolumeNames.Num(); Volume++)
				{
					if (Name == VolumeRotatorName(Volume))
						Settings.SetVolumeAt(Volume, float(Value) / 10.0);
				}
			}
		}
		ApplyAndRefresh(false);
	}
}
