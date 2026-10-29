// The game screen's part in script (J8a.5 loop 2): made by UHOLGameScreen (UHOLScriptScreen::FindScriptClass) over its
// whole frame; the HUD's blocks move here one by one. First of them, the hero's block, bottom left above the selection's
// block and the control groups' bar (J8a.5 loop 5): his portrait (the emblem over the player's colour, loop 7; a click: as F4), his level, his HP's bar, his XP's bar
// and three abilities' boxes (greyed until they exist); dead the delay, ready the return's button (a click buys it);
// read from his public sheet (HOLHero::, every machine). A mouse click and hol.HUD.Click (ClickWidget) go through the
// same functions; a hidden or greyed widget is not clicked. The buttons stop their clicks, the rest lets them through.
// Hidden for a spectator, a defeated player and a slot without a hero (state None). Refreshed 4 times a second (Refresh).
// Its texts are not localised yet (FText::FromString): a string table will hold them.
class UHOLGameScreenScript : UHOLScriptScreen
{
	UBorder HeroPanel;
	UButton HeroPortrait;
	UBorder HeroPortraitFill;
	UImage HeroPortraitEmblem;
	UCommonTextBlock HeroLevelText;
	UProgressBar HeroHealthBar;
	UCommonTextBlock HeroHealthText;
	UProgressBar HeroXPBar;
	TArray<UBorder> HeroAbilities;
	// The delay ("Prêt dans 0:23", "…").
	UCommonTextBlock HeroStatusText;
	// The return's button: its label and its price (red without the gold), its tooltip when it cannot be bought.
	UButton HeroReturnButton;
	UCommonTextBlock HeroReturnLabel;
	UCommonTextBlock HeroReturnPrice;

	// What the hero's block shows (the view applied last); Describe writes its line.
	FHOLHeroBlockView HeroView;
	// The texts its widgets show, in the current language (Describe's second line, "hero texts: ...").
	FString HeroTexts = "";

	// Sizes of the block (px at 1080p, the HUD's scale).
	const float PortraitSize = 64.0;
	// The column's width: the panel as wide as the selection's block under it, the return's button fitting in it.
	const float BarWidth = 240.0;
	const float AbilitySize = 26.0;

	UFUNCTION(BlueprintOverride)
	void OnInitialized()
	{
		UOverlay Frame = Cast<UOverlay>(ConstructWidget(UOverlay, n"ScriptFrame"));
		SetRootWidget(Frame);
		BuildHeroBlock(Frame);
	}

	// The hero's block: a panel bottom left (UHOLScriptScreen::GetBottomLeftPlace), the portrait left of a column (level,
	// HP's bar and text, XP's bar, abilities, the delay or the return's button).
	void BuildHeroBlock(UOverlay Frame)
	{
		HeroPanel = MakePanel(n"HeroPanel");
		HeroPanel.SetVisibility(ESlateVisibility::Collapsed);
		UOverlaySlot PanelSlot = Frame.AddChildToOverlay(HeroPanel);
		PanelSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);
		PanelSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Bottom);
		PanelSlot.SetPadding(GetBottomLeftPlace());

		UHorizontalBox Row = Cast<UHorizontalBox>(ConstructWidget(UHorizontalBox, n"HeroRow"));
		HeroPanel.SetContent(Row);

		// The portrait: a square button, the player's colour with the hero's emblem over it (loop 7, one texture for all
		// the players: its room around the shield is transparent).
		USizeBox PortraitBox = Cast<USizeBox>(ConstructWidget(USizeBox, n"HeroPortraitBox"));
		PortraitBox.SetWidthOverride(PortraitSize);
		PortraitBox.SetHeightOverride(PortraitSize);
		UHorizontalBoxSlot PortraitSlot = Row.AddChildToHorizontalBox(PortraitBox);
		PortraitSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Top);
		PortraitSlot.SetPadding(FMargin(0.0, 0.0, 10.0, 0.0));
		HeroPortrait = MakeButton(n"HeroPortrait");
		HeroPortrait.OnClicked.AddUFunction(this, n"HandlePortraitClicked");
		PortraitBox.SetContent(HeroPortrait);
		HeroPortraitFill = Cast<UBorder>(ConstructWidget(UBorder, n"HeroPortraitFill"));
		HeroPortraitFill.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
		HeroPortraitFill.SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
		HeroPortraitFill.SetPadding(FMargin(2.0));
		HeroPortraitFill.SetVisibility(ESlateVisibility::HitTestInvisible);
		UButtonSlot FillSlot = Cast<UButtonSlot>(HeroPortrait.AddChild(HeroPortraitFill));
		FillSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
		FillSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
		FillSlot.SetPadding(FMargin(0.0));
		HeroPortraitEmblem = Cast<UImage>(ConstructWidget(UImage, n"HeroPortraitEmblem"));
		HeroPortraitEmblem.SetVisibility(ESlateVisibility::HitTestInvisible);
		UTexture2D Emblem = HOLHeroBlock::LoadEmblem();
		if (Emblem != nullptr)
			HeroPortraitEmblem.SetBrushFromTexture(Emblem, false);
		else
			Warning(f"Hero block: no emblem at {HOLHeroBlock::EmblemPath}");
		HeroPortraitFill.SetContent(HeroPortraitEmblem);

		UVerticalBox Column = Cast<UVerticalBox>(ConstructWidget(UVerticalBox, n"HeroColumn"));
		Row.AddChildToHorizontalBox(Column);
		HeroLevelText = MakeText(n"HeroLevel", UHOLTextStyleHUD);
		Column.AddChildToVerticalBox(HeroLevelText);
		HeroHealthBar = MakeBar(n"HeroHealthBar", FLinearColor(0.25, 0.7, 0.2, 1.0));
		Column.AddChildToVerticalBox(SizedBar(n"HeroHealthBarBox", HeroHealthBar, 8.0)).SetPadding(FMargin(0.0, 3.0, 0.0, 1.0));
		HeroHealthText = MakeText(n"HeroHealth", UHOLTextStyleHUDLabel);
		Column.AddChildToVerticalBox(HeroHealthText);
		// The XP (J8b.4): gold, filled within the level, its numbers on hover; greyed while unknown.
		HeroXPBar = MakeBar(n"HeroXPBar", FLinearColor(0.85, 0.65, 0.15, 1.0));
		HeroXPBar.SetRenderOpacity(0.5);
		Column.AddChildToVerticalBox(SizedBar(n"HeroXPBarBox", HeroXPBar, 5.0)).SetPadding(FMargin(0.0, 3.0, 0.0, 3.0));
		// Three abilities' boxes, greyed, no text yet.
		UHorizontalBox Abilities = Cast<UHorizontalBox>(ConstructWidget(UHorizontalBox, n"HeroAbilities"));
		Column.AddChildToVerticalBox(Abilities);
		for (int Index = 0; Index < 3; ++Index)
		{
			USizeBox AbilityBox = Cast<USizeBox>(ConstructWidget(USizeBox, FName(f"HeroAbilityBox{Index}")));
			AbilityBox.SetWidthOverride(AbilitySize);
			AbilityBox.SetHeightOverride(AbilitySize);
			UBorder Ability = Cast<UBorder>(ConstructWidget(UBorder, FName(f"HeroAbility{Index}")));
			Ability.SetBrushColor(FLinearColor(0.09, 0.09, 0.11, 1.0));
			AbilityBox.SetContent(Ability);
			Abilities.AddChildToHorizontalBox(AbilityBox).SetPadding(FMargin(0.0, 0.0, 4.0, 0.0));
			HeroAbilities.Add(Ability);
		}
		HeroStatusText = MakeText(n"HeroStatus", UHOLTextStyleHUDLabel);
		HeroStatusText.SetVisibility(ESlateVisibility::Collapsed);
		Column.AddChildToVerticalBox(HeroStatusText).SetPadding(FMargin(0.0, 4.0, 0.0, 0.0));

		// The return's button, under the abilities (ready only).
		HeroReturnButton = MakeButton(n"HeroReturnButton");
		HeroReturnButton.SetVisibility(ESlateVisibility::Collapsed);
		HeroReturnButton.OnClicked.AddUFunction(this, n"HandleReturnClicked");
		Column.AddChildToVerticalBox(HeroReturnButton).SetPadding(FMargin(0.0, 4.0, 0.0, 0.0));
		UHorizontalBox ReturnRow = Cast<UHorizontalBox>(ConstructWidget(UHorizontalBox, n"HeroReturnRow"));
		ReturnRow.SetVisibility(ESlateVisibility::HitTestInvisible);
		HeroReturnButton.AddChild(ReturnRow);
		HeroReturnLabel = MakeText(n"HeroReturnLabel", UHOLTextStyleHUDLabel);
		ReturnRow.AddChildToHorizontalBox(HeroReturnLabel);
		HeroReturnPrice = MakeText(n"HeroReturnPrice", UHOLTextStyleHUDLabel);
		ReturnRow.AddChildToHorizontalBox(HeroReturnPrice);
	}

	// A bar in a box of the block's width and of Height.
	USizeBox SizedBar(FName BoxName, UProgressBar Bar, float Height)
	{
		USizeBox Box = Cast<USizeBox>(ConstructWidget(USizeBox, BoxName));
		Box.SetWidthOverride(BarWidth);
		Box.SetHeightOverride(Height);
		Box.SetContent(Bar);
		return Box;
	}

	UFUNCTION(BlueprintOverride)
	void Refresh()
	{
		RefreshHero();
	}

	// The view (HOLHeroBlock::Make) from the public sheet, the slot and the return's quote for an order sent now.
	void RefreshHero()
	{
		const int PlayerSlot = GetLocalSlot();
		FHOLHeroBlockView View;
		if (PlayerSlot < 0)
			View = HOLHeroBlock::Make(true, false, EHOLHeroState::None, 0, 0, 0, -1, FHOLHeroReturnQuote());
		else
			View = HOLHeroBlock::Make(false, HOLHero::IsSlotDefeated(PlayerSlot), HOLHero::GetHeroState(PlayerSlot), HOLHero::GetHeroLevel(PlayerSlot),
				HOLHero::GetHeroHealth(PlayerSlot), HOLHero::GetHeroMaxHealth(PlayerSlot), HOLHero::GetHeroXP(PlayerSlot), HOLHeroReturnQuote::ReadNow(PlayerSlot));
		ShowHero(View);
	}

	// The view applied to the block's widgets.
	void ShowHero(FHOLHeroBlockView View)
	{
		HeroView = View;
		HeroTexts = "";
		HeroPanel.SetVisibility(View.bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		if (!View.bVisible)
			return;
		// The portrait: the player's colour and the emblem, both darkened while dead (its click does nothing then).
		const FLinearColor Colour = GetLocalColor();
		const float Shade = View.bPortraitDark ? 0.3 : 1.0;
		HeroPortraitFill.SetBrushColor(FLinearColor(Colour.R * Shade, Colour.G * Shade, Colour.B * Shade, 1.0));
		HeroPortraitEmblem.SetColorAndOpacity(FLinearColor(Shade, Shade, Shade, 1.0));
		UHOLScriptScreen::SetButtonGreyed(HeroPortrait, View.bPortraitDark);

		HeroLevelText.SetText(FText::FromString(HOLHeroBlockText::LevelText(View)));
		HeroHealthBar.SetPercent(View.HealthFraction);
		HeroHealthText.SetText(FText::FromString(HOLHeroBlockText::HealthText(View)));
		HeroXPBar.SetPercent(View.XPFraction);
		HeroXPBar.SetRenderOpacity(View.bXPGreyed ? 0.5 : 1.0);
		HeroXPBar.SetToolTipText(FText::FromString(View.XPText));
		for (int Index = 0; Index < HeroAbilities.Num(); ++Index)
		{
			HeroAbilities[Index].SetVisibility(Index < View.NumAbilities ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
			HeroAbilities[Index].SetRenderOpacity(View.bAbilitiesGreyed ? 0.6 : 1.0);
		}

		const FString Delay = HOLHeroBlockText::DelayText(View);
		HeroStatusText.SetText(FText::FromString(Delay));
		HeroStatusText.SetVisibility(Delay.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);

		HeroReturnButton.SetVisibility(View.bButtonVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		HeroTexts = HOLHeroBlockText::LevelText(View) + " | " + HOLHeroBlockText::HealthText(View) + " | " + Delay;
		if (!View.bButtonVisible)
			return;
		// "Ramener le héros — {0} or": the label before the price; the price and what follows it in their own text.
		FString Before;
		FString After;
		FString PriceText;
		if (HOLHeroBlockText::Pattern(View.ButtonKey).Split("{0}", Before, After))
			PriceText = f"{View.Price}" + After;
		else
			Before = HOLHeroBlockText::ButtonText(View);
		HeroReturnLabel.SetText(FText::FromString(Before));
		HeroReturnPrice.SetText(FText::FromString(PriceText));
		HeroReturnLabel.SetColorAndOpacity(View.bButtonEnabled ? UHOLScriptScreen::GetTextColor() : UHOLScriptScreen::GetMutedColor());
		HeroReturnPrice.SetColorAndOpacity(View.bPriceRed ? UHOLScriptScreen::GetMissingColor() : (View.bButtonEnabled ? UHOLScriptScreen::GetTextColor() : UHOLScriptScreen::GetMutedColor()));
		UHOLScriptScreen::SetButtonGreyed(HeroReturnButton, !View.bButtonEnabled);
		HeroReturnButton.SetToolTipText(FText::FromString(HOLHeroBlockText::TooltipText(View)));
		HeroTexts += " | " + Before + PriceText + " | " + HOLHeroBlockText::TooltipText(View);
	}

	// A click on the portrait (the mouse's or hol.HUD.Click): as F4, the hero selected, again the camera on him. Hidden
	// or dead: nothing (false).
	bool ClickPortrait()
	{
		if (!HeroView.bVisible || HeroView.bPortraitDark)
			return false;
		AHOLPlayerController Controller = Cast<AHOLPlayerController>(GetOwningPlayer());
		if (Controller == nullptr)
			return false;
		Controller.SelectHero();
		return true;
	}

	// A click on the return's button: the order sent (the server prices it at the next step). Hidden or greyed: nothing.
	bool ClickReturn()
	{
		if (!HeroView.bVisible || !HeroView.bButtonVisible || !HeroView.bButtonEnabled)
			return false;
		AHOLPlayerController Controller = Cast<AHOLPlayerController>(GetOwningPlayer());
		if (Controller == nullptr)
			return false;
		Controller.RequestReturnHero();
		return true;
	}

	UFUNCTION()
	void HandlePortraitClicked()
	{
		ClickPortrait();
	}

	UFUNCTION()
	void HandleReturnClicked()
	{
		ClickReturn();
	}

	UFUNCTION(BlueprintOverride)
	bool ClickWidget(FName WidgetName)
	{
		// The view of now, as the mouse would see it (the tests' waits draw no frame).
		RefreshNow();
		if (WidgetName == n"HeroPortrait")
			return ClickPortrait();
		if (WidgetName == n"HeroReturnButton")
			return ClickReturn();
		return false;
	}

	UFUNCTION(BlueprintOverride)
	FString Describe() const
	{
		return "hero: " + HeroView.Describe + (HeroTexts.IsEmpty() ? "" : "\nhero texts: " + HeroTexts)
			+ (HeroView.bVisible ? "\nhero xp: " + HeroView.XPDescribe + f" bar {Math::RoundToInt(HeroView.XPFraction * 100.0)}%" : "");
	}
}
