// The end banner's widget in script (MIG-CPP bascule EndBanner, D830/D835): UHOLEndBannerScreen::Open (AHOLGameHUD::RefreshEndBanner, at each
// defeat or the match's end) pushes the class FindScriptClass finds, the one of this file, over the match in the menu layer. Its column, title,
// detail and buttons are built here (BuildBanner, in place of BuildLayout), its view refreshed on opening and at each SetResult (RefreshBanner)
// from HOLEndBannerScript::Make over the result and the origin the base reads; a click goes through HOLEndBannerScript::Click to the C++ actions
// kept in the base (Continue and its delegate, the ways out, the statistics). Logs the lines UHOLEndBanner's C++ logged (« End banner: ... »).
class UHOLEndBannerScreenScript : UHOLEndBannerScreen
{
	UHOLButton ContinueButton;
	UHOLButton QuitButton;
	UHOLButton ReturnButton;
	UHOLButton StatsButton;
	// The view shown (HOLEndBannerScript::Make), what the clicks are judged on.
	FHOLEndBannerView View;
	FString LastLogged = "";

	UFUNCTION(BlueprintOverride)
	void BuildBanner()
	{
		UVerticalBox Column = MakeBannerColumn();
		Title = AddBannerText(Column, n"Title", FText::GetEmpty(), true);
		// Under the title: the spectator's winner (M6-Q10).
		Detail = AddBannerText(Column, n"Detail", FText::GetEmpty(), false);
		if (Detail != nullptr)
			Detail.SetVisibility(ESlateVisibility::Collapsed);
		UHorizontalBox Buttons = AddBannerRow(Column);
		ContinueButton = AddBannerButtonNamed(Buttons, n"ContinueButton");
		QuitButton = AddBannerButtonNamed(Buttons, n"QuitButton");
		ReturnButton = AddBannerButtonNamed(Buttons, n"ReturnButton");
		StatsButton = AddBannerButtonNamed(Buttons, n"StatsButton");
		FocusTarget = ContinueButton;
	}

	UHOLButton AddBannerButtonNamed(UHorizontalBox Row, FName Name)
	{
		UHOLButton Button = AddBannerButton(Row, Name, FText::FromString(HOLEndBannerScript::Label(View, Name)));
		if (Button != nullptr)
			Button.OnButtonBaseClicked.AddUFunction(this, n"HandleButtonClicked");
		return Button;
	}

	UFUNCTION(BlueprintOverride)
	void RefreshBanner()
	{
		View = HOLEndBannerScript::Make(EHOLGameMenuBanner(GetResultValue()), EHOLGameMenuOrigin(GetOriginValue()));
		if (!View.bVisible)
			return;
		if (Title != nullptr)
			Title.SetText(FText::FromString(HOLEndBannerScript::Label(View, n"Title")));
		if (ContinueButton != nullptr)
			UHOLGameMenuScreen::SetButtonLabel(ContinueButton, FText::FromString(HOLEndBannerScript::Label(View, n"ContinueButton")));
		if (ReturnButton != nullptr)
			ReturnButton.SetVisibility(View.bReturn ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		if (QuitButton != nullptr)
			UHOLGameMenuScreen::SetButtonLabel(QuitButton, FText::FromString(HOLEndBannerScript::Label(View, n"QuitButton")));
		if (View.Describe == LastLogged)
			return;
		LastLogged = View.Describe;
		const FString Which = View.Result == EHOLGameMenuBanner::Victory ? "Victory" : (View.Result == EHOLGameMenuBanner::MatchOver ? "MatchOver" : "Defeat");
		UHOLGameMenuScreen::MenuLog("End banner: " + Which);
		UHOLGameMenuScreen::MenuLog("End banner: way out " + (View.bQuitToList ? "list" : "menu") + " (origin " + GetOriginValue() + ")");
		if (View.bReturn)
			UHOLGameMenuScreen::MenuLog("End banner: lobby match, back to lobby offered, no countdown");
	}

	UFUNCTION()
	void HandleButtonClicked(UCommonButtonBase Button)
	{
		if (Button == nullptr)
			return;
		const EHOLEndBannerAction Action = HOLEndBannerScript::Click(View, Button.GetName());
		if (Action == EHOLEndBannerAction::Continue)
			Continue();
		else if (Action == EHOLEndBannerAction::QuitToMenu)
			QuitToMenu();
		else if (Action == EHOLEndBannerAction::QuitToList)
			QuitToList();
		else if (Action == EHOLEndBannerAction::ReturnToLobby)
			ReturnToLobby();
		else if (Action == EHOLEndBannerAction::Stats)
			ShowStats();
	}
}
