// The in-game menu (D356, V1-Q1) in script (MIG-CPP bascule GameMenu, D830/D835): pushed by UHOLGameMenuScreen::Open (its FindScriptClass),
// over the match in the menu layer. Its column and buttons are built here (BuildMenu, in place of BuildLayout), their state refreshed
// each frame (RefreshMenu: a defeat or the match's end may come while it is open) from HOLGameMenuScript::Make over what
// UHOLGameMenuScreen::ReadFacts reads of the match; a click goes through HOLGameMenuScript::Click to the C++ actions kept in the base
// (the surrender's confirmation, the pause, the ways out, the end banner again). Logs « Game menu: view <Describe> » on each change.
class UHOLGameMenuScreenScript : UHOLGameMenuScreen
{
	UHOLButton ResumeButton;
	UHOLButton PauseButton;
	UHOLButton SurrenderButton;
	UHOLButton ReturnButton;
	UHOLButton QuitButton;
	UHOLButton ResultButton;
	// The view shown (HOLGameMenuScript::Make), what the clicks are judged on.
	FHOLGameMenuView View;
	FString PauseLabel = "";

	UFUNCTION(BlueprintOverride)
	void BuildMenu()
	{
		UVerticalBox Column = MakeMenuColumn();
		AddMenuTitle(Column, n"Title", FText::FromString(HOLGameMenuScript::Label(View, n"Title")));
		ResumeButton = AddMenuButtonNamed(Column, n"ResumeButton");
		PauseButton = AddMenuButtonNamed(Column, n"PauseButton");
		SurrenderButton = AddMenuButtonNamed(Column, n"SurrenderButton");
		// The end banner's texts (same keys, already translated).
		ReturnButton = AddMenuButtonNamed(Column, n"ReturnButton");
		QuitButton = AddMenuButtonNamed(Column, n"QuitButton");
		ResultButton = AddMenuButtonNamed(Column, n"ResultButton");
		// Offered once the match is over alone (RefreshMenu).
		Show(PauseButton, false);
		Show(ReturnButton, false);
		Show(ResultButton, false);
		FocusTarget = ResumeButton;
	}

	UHOLButton AddMenuButtonNamed(UVerticalBox Column, FName Name)
	{
		UHOLButton Button = AddMenuButton(Column, Name, FText::FromString(HOLGameMenuScript::Label(View, Name)));
		if (Button != nullptr)
			Button.OnButtonBaseClicked.AddUFunction(this, n"HandleButtonClicked");
		return Button;
	}

	// True when the button's visibility changed.
	bool Show(UHOLButton Button, bool bShown)
	{
		const ESlateVisibility Wanted = bShown ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
		if (Button == nullptr || Button.GetVisibility() == Wanted)
			return false;
		Button.SetVisibility(Wanted);
		return true;
	}

	UFUNCTION(BlueprintOverride)
	void RefreshMenu(bool bOpening)
	{
		const FHOLGameMenuFacts Facts = UHOLGameMenuScreen::ReadFacts(GetOwningPlayer());
		const EHOLGameMenuBanner Banner = Facts.bKnown
			? HOLEndBannerScript::GetEndBanner(Facts.MySlot, Facts.DefeatedMask, Facts.bMatchOver, Facts.WinnerMask) : EHOLGameMenuBanner::None;
		const FString Before = View.Describe;
		View = HOLGameMenuScript::Make(Facts.bKnown, Facts.MySlot, Facts.bDefeated, Facts.bMatchOver, Facts.bPaused, Facts.bPauseFree,
			Facts.PausesLeft, Facts.bCanPauseNow, Banner, EHOLGameMenuOrigin(Facts.Origin));
		// D513: « Pause » (« Pause (2) » networked), greyed with none left.
		Show(PauseButton, View.bPause);
		if (View.bPause && PauseButton != nullptr)
		{
			const FString Wanted = HOLGameMenuScript::Label(View, n"PauseButton");
			if (Wanted != PauseLabel)
			{
				PauseLabel = Wanted;
				UHOLGameMenuScreen::SetButtonLabel(PauseButton, FText::FromString(Wanted));
			}
			PauseButton.SetIsEnabled(View.bPauseEnabled);
		}
		// D357: « Abandonner » or the ways out; once over, the end banner's (M6-Q7 revised).
		Show(SurrenderButton, View.bSurrender);
		Show(QuitButton, View.bQuit);
		const bool bReturnChanged = Show(ReturnButton, View.bReturn);
		const bool bResultChanged = Show(ResultButton, View.bResult);
		// The quit's way follows the match's origin, the same all the match long.
		if (bOpening && QuitButton != nullptr)
			UHOLGameMenuScreen::SetButtonLabel(QuitButton, FText::FromString(HOLGameMenuScript::Label(View, n"QuitButton")));
		if ((bReturnChanged || bResultChanged) && Facts.bMatchOver)
			UHOLGameMenuScreen::MenuLog("Game menu: match over, ways out" + (View.bReturn ? " lobby," : "") + " "
				+ (View.bQuitToList ? "list" : "menu") + ", result");
		if (bOpening || View.Describe != Before)
			UHOLGameMenuScreen::MenuLog("Game menu: view " + View.Describe);
	}

	UFUNCTION()
	void HandleButtonClicked(UCommonButtonBase Button)
	{
		if (Button == nullptr)
			return;
		const EHOLGameMenuAction Action = HOLGameMenuScript::Click(View, Button.GetName());
		if (Action == EHOLGameMenuAction::Resume)
			DeactivateWidget();
		else if (Action == EHOLGameMenuAction::Pause)
			PauseMatch();
		else if (Action == EHOLGameMenuAction::Surrender)
			AskSurrender();
		else if (Action == EHOLGameMenuAction::QuitToMenu || Action == EHOLGameMenuAction::QuitToList)
			QuitMatch();
		else if (Action == EHOLGameMenuAction::ReturnToLobby)
			ReturnToLobby();
		else if (Action == EHOLGameMenuAction::ShowResult)
			ShowResult();
	}
}
