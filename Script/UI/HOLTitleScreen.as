// The title screen's widget in script (ECR-1, D840/D839): the menu's controller pushes the class UHOLTitleScreen::FindScriptClass finds, the one
// of this file, as the root of the menu layer. Its column (the game's name, Play, Multiplayer, Settings, Quit) and the version are built here
// (BuildTitle, in place of BuildLayout), with the same widget names the scenarios and hol.UI.Click / hol.Mouse.Click use; a click goes to the
// actions kept in the base (the New game, Multiplayer and Settings screens, the quit confirmation). The triple click of the faction cheat, the
// focus and the localised labels stay in the base (C++).
class UHOLTitleScreenScript : UHOLTitleScreen
{
	UFUNCTION(BlueprintOverride)
	void BuildTitle()
	{
		UVerticalBox Column = MakeTitleColumn();
		// The game's name is not translated.
		Title = AddTitleText(Column, n"Title", FText::FromString("Heroes of Liberty"), true);
		// Hit-testable, so that a click on the title reaches the screen (the faction cheat, the base's NativeOnMouseButtonDown).
		if (Title != nullptr)
			Title.SetVisibility(ESlateVisibility::Visible);
		PlayButton = AddTitleButtonNamed(Column, n"PlayButton");
		MultiplayerButton = AddTitleButtonNamed(Column, n"MultiplayerButton");
		SettingsButton = AddTitleButtonNamed(Column, n"SettingsButton");
		QuitButton = AddTitleButtonNamed(Column, n"QuitButton");
		// The version (R1.7), small in the bottom right corner.
		AddVersionText();
	}

	UHOLButton AddTitleButtonNamed(UVerticalBox Column, FName Name)
	{
		UHOLButton Button = AddTitleButton(Column, Name, UHOLTitleScreen::ButtonLabel(Name));
		if (Button != nullptr)
			Button.OnButtonBaseClicked.AddUFunction(this, n"HandleButtonClicked");
		return Button;
	}

	UFUNCTION()
	void HandleButtonClicked(UCommonButtonBase Button)
	{
		if (Button == nullptr)
			return;
		if (Button == PlayButton)
			OpenNewGame();
		else if (Button == MultiplayerButton)
			OpenMultiplayer();
		else if (Button == SettingsButton)
			OpenSettings();
		else if (Button == QuitButton)
			AskQuit();
	}
}
