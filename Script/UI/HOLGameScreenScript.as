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
	// Its place (EtapeDefini T): read anew on every refresh, the bottom bar's scale may change with the window.
	UOverlaySlot HeroPanelSlot;
	UButton HeroPortrait;
	UBorder HeroPortraitFill;
	UImage HeroPortraitEmblem;
	UCommonTextBlock HeroLevelText;
	UProgressBar HeroHealthBar;
	UCommonTextBlock HeroHealthText;
	UProgressBar HeroXPBar;
	TArray<UBorder> HeroAbilities;
	TArray<UImage> HeroAbilityIcons;
	TArray<USizeBox> HeroAbilityCurtains;
	TArray<UCommonTextBlock> HeroAbilitySeconds;
	// The delay ("Prêt dans 0:23", "…").
	UCommonTextBlock HeroStatusText;
	// The return's button: its label and its price (red without the gold), its tooltip when it cannot be bought.
	UButton HeroReturnButton;
	UCommonTextBlock HeroReturnLabel;
	UCommonTextBlock HeroReturnPrice;
// T5: the reminder of a talent point not spent, a button taking the advised choice (D105).
UButton HeroTalentButton;
UCommonTextBlock HeroTalentLabel;
// What the talent line shows (HOLTalentPanel::Make), and its texts for hol.HUD.Report.
FHOLTalentPanelView TalentView;
FString TalentTexts = "";
// TQ4 (PROVISIONAL, hol.UI.TalentFirstChoice, option B): three family buttons under the reminder while a point is kept.
UHorizontalBox HeroFamilyRow;
TArray<UButton> HeroFamilyButtons;
TArray<UCommonTextBlock> HeroFamilyLabels;
// T5 (the trial of 2026-10-31: a point spent changed nothing the eye could see): the talents taken by name, and a notice in the accent colour
// for a few seconds when one is taken (with a sound), both from HOLTalentNames' one table.
UCommonTextBlock HeroTalentListLabel;
UCommonTextBlock HeroTalentNoticeLabel;
// The talents the sheet showed at the last refresh (-1: not seen yet, a hero seen for the first time raises no notice), the server step the
// notice lasts until, and the texts of both lines and of the family buttons' tooltips, for hol.HUD.Report.
int TalentSeenCount = -1;
int TalentNoticeUntilStep = 0;
FString TalentListText = "";
FString TalentNoticeText = "";
FString TalentTipTexts = "";

	// What the hero's block shows (the view applied last); Describe writes its line.
	FHOLHeroBlockView HeroView;
	// The texts its widgets show, in the current language (Describe's second line, "hero texts: ...").
	FString HeroTexts = "";

	// The pause screen (P5, D513): a dim layer over the whole match (it takes the clicks: nothing reaches the game or the
	// HUD under it) and a panel in the middle: the title, who paused, the timer, the pauses left, « Reprendre ».
	UBorder PauseLayer;
	UCommonTextBlock PauseTitleText;
	UCommonTextBlock PauseByText;
	UCommonTextBlock PauseTimerText;
	UCommonTextBlock PauseLeftText;
	UButton PauseResumeButton;
	UCommonTextBlock PauseResumeLabel;
	FHOLPauseScreenView PauseView;
	FString PauseTexts = "";

	// The duel's challenge window (D8, D117; DuelInProgress.md): top centre, no dim layer, the game goes on under it. A challenge received shows the
	// challenger, the countdown (a bar and the seconds, flashing in the last instants) and « Accepter » / « Refuser »; a challenge sent shows
	// « Défi envoyé » and the same countdown, with no button (the challenger cannot cancel, D80). Read from the local hero's public sheet; the
	// rules are HOLDuel::SheetWindowView. Its texts are not localised yet (provisional, like the hero block's).
	UBorder DuelPanel;
	UCommonTextBlock DuelTitleText;
	UCommonTextBlock DuelTimerText;
	UProgressBar DuelBar;
	UHorizontalBox DuelButtons;
	UButton DuelAcceptButton;
	UButton DuelRefuseButton;
	FHOLDuelWindowView DuelView;
	FString DuelTexts = "";

	// Sizes of the block (px at 1080p, the HUD's scale).
	const float PortraitSize = 64.0;
	// The column's width: the panel as wide as the selection's block under it, the return's button fitting in it.
	const float BarWidth = 240.0;
	const float AbilitySize = 26.0;

	// The minimap (InterfaceInProgress.md, the prototype's, AoE4's manner): bottom right in the place MinimapPlace keeps in the
	// C++ screen. The rules are HOLMinimapRules.as; UHOLMinimapView reads the game, acts and paints.
	UHOLMinimapView Minimap;
	FHOLMinimapFrame MinimapFrame;
	bool bMinimapFrame = false;
	// The own things as the last refresh saw them (the alerts compare two readings), the alerts, the time of the last refresh.
	TArray<FHOLMinimapSeen> MinimapSeenBefore;
	TArray<FHOLMinimapAlert> MinimapAlerts;
	// The camera's footprint on the ground at the last refresh and the focus then.
	TArray<FVector2D> MinimapQuad;
	FVector2D MinimapQuadFocus;
	float MinimapLastTime = -1.0;
	int MinimapTerrainWait = 0;
	int MinimapLitAlerts = 0;
	FString MinimapLastClick = "none";
	// The picture's side (px at 1080p) and the margin of the screen, as the C++ screen's MinimapPlace.
	const float MinimapSize = 240.0;
	const float MinimapMargin = 16.0;

	UFUNCTION(BlueprintOverride)
	void OnInitialized()
	{
		UOverlay Frame = Cast<UOverlay>(ConstructWidget(UOverlay, n"ScriptFrame"));
		SetRootWidget(Frame);
		BuildHeroBlock(Frame);
		BuildDuelWindow(Frame);
		BuildMinimap(Frame);
		// Last: over the other blocks.
		BuildPauseScreen(Frame);
	}

	void BuildDuelWindow(UOverlay Frame)
	{
		DuelPanel = MakePanel(n"DuelPanel");
		DuelPanel.SetVisibility(ESlateVisibility::Collapsed);
		UOverlaySlot PanelSlot = Frame.AddChildToOverlay(DuelPanel);
		PanelSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
		PanelSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Top);
		PanelSlot.SetPadding(FMargin(0.0, 140.0, 0.0, 0.0));
		UVerticalBox Column = Cast<UVerticalBox>(ConstructWidget(UVerticalBox, n"DuelColumn"));
		DuelPanel.SetContent(Column);
		DuelTitleText = MakeText(n"DuelTitle", UHOLTextStyleHUD);
		Column.AddChildToVerticalBox(DuelTitleText).SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
		DuelBar = MakeBar(n"DuelBar", FLinearColor(0.85, 0.65, 0.15, 1.0));
		Column.AddChildToVerticalBox(SizedBar(n"DuelBarBox", DuelBar, 10.0)).SetPadding(FMargin(0.0, 8.0, 0.0, 0.0));
		DuelTimerText = MakeText(n"DuelTimer", UHOLTextStyleHUDLabel);
		Column.AddChildToVerticalBox(DuelTimerText).SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
		DuelButtons = Cast<UHorizontalBox>(ConstructWidget(UHorizontalBox, n"DuelButtons"));
		Column.AddChildToVerticalBox(DuelButtons).SetPadding(FMargin(0.0, 8.0, 0.0, 0.0));
		DuelAcceptButton = MakeButton(n"DuelAcceptButton");
		DuelAcceptButton.OnClicked.AddUFunction(this, n"HandleDuelAcceptClicked");
		DuelButtons.AddChildToHorizontalBox(DuelAcceptButton).SetPadding(FMargin(0.0, 0.0, 8.0, 0.0));
		UCommonTextBlock AcceptLabel = MakeText(n"DuelAcceptLabel", UHOLTextStyleHUDLabel);
		AcceptLabel.SetText(FText::FromString("Accepter"));
		DuelAcceptButton.AddChild(AcceptLabel);
		DuelRefuseButton = MakeButton(n"DuelRefuseButton");
		DuelRefuseButton.OnClicked.AddUFunction(this, n"HandleDuelRefuseClicked");
		DuelButtons.AddChildToHorizontalBox(DuelRefuseButton);
		UCommonTextBlock RefuseLabel = MakeText(n"DuelRefuseLabel", UHOLTextStyleHUDLabel);
		RefuseLabel.SetText(FText::FromString("Refuser"));
		DuelRefuseButton.AddChild(RefuseLabel);
	}

	// The window from the local hero's sheet (HOLDuel::SheetWindowView); nothing for a spectator, a defeated player or a hero in no challenge.
	void RefreshDuel()
	{
		const int PlayerSlot = GetLocalSlot();
		FHOLDuelWindowView View;
		int LeftMs = 0;
		if (PlayerSlot >= 0 && !HOLHero::IsSlotDefeated(PlayerSlot))
		{
			const int Rate = Math::Max(1, HOLHero::GetSimulationRateHz());
			LeftMs = Math::Max(0, HOLHero::GetHeroDuelEndStep(PlayerSlot) - HOLHero::GetServerStep()) * 1000 / Rate;
			View = HOLDuel::SheetWindowView(HOLDuel::Preset(), HOLHero::GetHeroDuelCode(PlayerSlot), HOLHero::GetHeroDuelOpponent(PlayerSlot), LeftMs);
		}
		ShowDuel(View, LeftMs);
	}

	void ShowDuel(FHOLDuelWindowView View, int LeftMs)
	{
		DuelView = View;
		DuelTexts = "";
		const bool bShown = View.Kind != EHOLDuelWindowKind::None;
		DuelPanel.SetVisibility(bShown ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		if (!bShown)
			return;
		const FString Who = f"joueur {View.OpponentSlot + 1}";
		const FString Title = View.Kind == EHOLDuelWindowKind::Incoming ? f"Le héros du {Who} te défie en duel" : f"Défi envoyé au héros du {Who}";
		const FString Timer = f"{View.SecondsLeft} s";
		DuelTitleText.SetText(FText::FromString(Title));
		// D117: the last instants flash, so that the window cannot be missed.
		const bool bFlashOff = View.bUrgent && ((LeftMs / 250) % 2) == 1;
		DuelTitleText.SetColorAndOpacity(bFlashOff ? FLinearColor(0.9, 0.2, 0.15, 1.0) : UHOLScriptScreen::GetTextColor());
		DuelTimerText.SetText(FText::FromString(Timer));
		DuelBar.SetPercent(float(View.FillPermille) / 1000.0);
		DuelButtons.SetVisibility(View.bCanAnswer ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		DuelTexts = Title + " | " + Timer + " | " + (View.bCanAnswer ? "Accepter / Refuser" : "sans bouton");
	}

	// A click on « Accepter » or « Refuser »: the answer sent (the server decides, D80: final). No window to answer: nothing.
	bool ClickDuelAnswer(bool bAccept)
	{
		if (DuelView.Kind != EHOLDuelWindowKind::Incoming || !DuelView.bCanAnswer)
			return false;
		AHOLPlayerController Controller = Cast<AHOLPlayerController>(GetOwningPlayer());
		if (Controller == nullptr)
			return false;
		Controller.RequestDuelAnswer(bAccept);
		return true;
	}

	UFUNCTION()
	void HandleDuelAcceptClicked()
	{
		ClickDuelAnswer(true);
	}

	UFUNCTION()
	void HandleDuelRefuseClicked()
	{
		ClickDuelAnswer(false);
	}

	void BuildPauseScreen(UOverlay Frame)
	{
		PauseLayer = Cast<UBorder>(ConstructWidget(UBorder, n"PauseLayer"));
		PauseLayer.SetBrushColor(FLinearColor(0.0, 0.0, 0.0, 0.45));
		PauseLayer.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
		PauseLayer.SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
		PauseLayer.SetVisibility(ESlateVisibility::Collapsed);
		UOverlaySlot LayerSlot = Frame.AddChildToOverlay(PauseLayer);
		LayerSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
		LayerSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);

		UBorder Panel = MakePanel(n"PausePanel");
		PauseLayer.SetContent(Panel);
		UVerticalBox Column = Cast<UVerticalBox>(ConstructWidget(UVerticalBox, n"PauseColumn"));
		Panel.SetContent(Column);
		PauseTitleText = MakeText(n"PauseTitle", UHOLTextStyleHUD);
		PauseTitleText.SetText(HOLPauseScreenText::PatternText(HOLPauseScreenText::Title));
		Column.AddChildToVerticalBox(PauseTitleText).SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
		PauseByText = MakeText(n"PauseBy", UHOLTextStyleHUDLabel);
		Column.AddChildToVerticalBox(PauseByText).SetPadding(FMargin(0.0, 8.0, 0.0, 0.0));
		PauseTimerText = MakeText(n"PauseTimer", UHOLTextStyleHUDLabel);
		Column.AddChildToVerticalBox(PauseTimerText).SetPadding(FMargin(0.0, 4.0, 0.0, 0.0));
		PauseLeftText = MakeText(n"PauseLeft", UHOLTextStyleHUDLabel);
		Column.AddChildToVerticalBox(PauseLeftText).SetPadding(FMargin(0.0, 4.0, 0.0, 0.0));
		PauseResumeButton = MakeButton(n"PauseResumeButton");
		PauseResumeButton.OnClicked.AddUFunction(this, n"HandlePauseResumeClicked");
		UVerticalBoxSlot ButtonSlot = Column.AddChildToVerticalBox(PauseResumeButton);
		ButtonSlot.SetPadding(FMargin(0.0, 12.0, 0.0, 0.0));
		ButtonSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
		PauseResumeLabel = MakeText(n"PauseResumeLabel", UHOLTextStyleHUDLabel);
		PauseResumeLabel.SetText(HOLPauseScreenText::PatternText(HOLPauseScreenText::Resume));
		PauseResumeButton.AddChild(PauseResumeLabel);
	}

	// The hero's block: a panel bottom left (UHOLScriptScreen::GetBottomLeftPlace), the portrait left of a column (level,
	// HP's bar and text, XP's bar, abilities, the delay or the return's button).
	void BuildHeroBlock(UOverlay Frame)
	{
		HeroPanel = MakePanel(n"HeroPanel");
		HeroPanel.SetVisibility(ESlateVisibility::Collapsed);
		HeroPanelSlot = Frame.AddChildToOverlay(HeroPanel);
		HeroPanelSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);
		HeroPanelSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Bottom);
		HeroPanelSlot.SetPadding(GetBottomLeftPlace());

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
		// Three abilities' boxes (J8c.4): aura, cry, banner; each its icon, a dark curtain from the top over the share of its
		// cool-down left, the seconds left on it; its tooltip says what it does.
		UHorizontalBox Abilities = Cast<UHorizontalBox>(ConstructWidget(UHorizontalBox, n"HeroAbilities"));
		Column.AddChildToVerticalBox(Abilities);
		for (int Index = 0; Index < 3; ++Index)
		{
			USizeBox AbilityBox = Cast<USizeBox>(ConstructWidget(USizeBox, FName(f"HeroAbilityBox{Index}")));
			AbilityBox.SetWidthOverride(AbilitySize);
			AbilityBox.SetHeightOverride(AbilitySize);
			UBorder Ability = Cast<UBorder>(ConstructWidget(UBorder, FName(f"HeroAbility{Index}")));
			Ability.SetBrushColor(FLinearColor(0.09, 0.09, 0.11, 1.0));
			Ability.SetPadding(FMargin(0.0));
			AbilityBox.SetContent(Ability);
			UOverlay Layers = Cast<UOverlay>(ConstructWidget(UOverlay, FName(f"HeroAbilityLayers{Index}")));
			Ability.SetContent(Layers);
			UImage Icon = Cast<UImage>(ConstructWidget(UImage, FName(f"HeroAbilityIcon{Index}")));
			UTexture2D IconTexture = HOLHeroBlock::LoadAbilityIcon(HOLHeroBlock::AbilityAt(Index));
			if (IconTexture != nullptr)
				Icon.SetBrushFromTexture(IconTexture, false);
			else
				Warning(f"Hero block: no icon for ability {Index}");
			UOverlaySlot IconSlot = Layers.AddChildToOverlay(Icon);
			IconSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
			IconSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
			IconSlot.SetPadding(FMargin(2.0));
			USizeBox Curtain = Cast<USizeBox>(ConstructWidget(USizeBox, FName(f"HeroAbilityCurtain{Index}")));
			Curtain.SetHeightOverride(0.0);
			UBorder CurtainFill = Cast<UBorder>(ConstructWidget(UBorder, FName(f"HeroAbilityCurtainFill{Index}")));
			CurtainFill.SetBrushColor(FLinearColor(0.0, 0.0, 0.0, 0.6));
			Curtain.SetContent(CurtainFill);
			UOverlaySlot CurtainSlot = Layers.AddChildToOverlay(Curtain);
			CurtainSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
			CurtainSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Top);
			UCommonTextBlock Seconds = MakeText(FName(f"HeroAbilitySeconds{Index}"), UHOLTextStyleHUDLabel);
			UOverlaySlot SecondsSlot = Layers.AddChildToOverlay(Seconds);
			SecondsSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
			SecondsSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
			HeroAbilityIcons.Add(Icon);
			HeroAbilityCurtains.Add(Curtain);
			HeroAbilitySeconds.Add(Seconds);
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

		// T5: the talent point's reminder under the abilities, a button while the advised choice exists; hidden with no point.
		HeroTalentButton = MakeButton(n"HeroTalentButton");
		HeroTalentButton.SetVisibility(ESlateVisibility::Collapsed);
		HeroTalentButton.OnClicked.AddUFunction(this, n"HandleTalentClicked");
		Column.AddChildToVerticalBox(HeroTalentButton).SetPadding(FMargin(0.0, 4.0, 0.0, 0.0));
		HeroTalentLabel = MakeText(n"HeroTalentLabel", UHOLTextStyleHUDLabel);
		HeroTalentLabel.SetVisibility(ESlateVisibility::HitTestInvisible);
		HeroTalentButton.AddChild(HeroTalentLabel);

		// TQ4 (option B): the three families of the first choice, hidden unless a point is kept.
		HeroFamilyRow = Cast<UHorizontalBox>(ConstructWidget(UHorizontalBox, n"HeroFamilyRow"));
		HeroFamilyRow.SetVisibility(ESlateVisibility::Collapsed);
		Column.AddChildToVerticalBox(HeroFamilyRow).SetPadding(FMargin(0.0, 4.0, 0.0, 0.0));
		for (int Family = 0; Family < 3; ++Family)
		{
			UButton Button = MakeButton(Family == 0 ? n"HeroTalentFamily0" : Family == 1 ? n"HeroTalentFamily1" : n"HeroTalentFamily2");
			Button.OnClicked.AddUFunction(this, Family == 0 ? n"HandleFamily0" : Family == 1 ? n"HandleFamily1" : n"HandleFamily2");
			UCommonTextBlock Label = MakeText(Family == 0 ? n"HeroTalentFamilyLabel0" : Family == 1 ? n"HeroTalentFamilyLabel1" : n"HeroTalentFamilyLabel2", UHOLTextStyleHUDLabel);
			Label.SetVisibility(ESlateVisibility::HitTestInvisible);
			Button.AddChild(Label);
			HeroFamilyRow.AddChildToHorizontalBox(Button).SetPadding(FMargin(0.0, 0.0, 4.0, 0.0));
			HeroFamilyButtons.Add(Button);
			HeroFamilyLabels.Add(Label);
		}
		// The notice and the list wrap at the block's width (a long effect would otherwise widen the whole block when a talent is taken).
		HeroTalentNoticeLabel = MakeText(n"HeroTalentNoticeLabel", UHOLTextStyleHUDLabel);
		HeroTalentNoticeLabel.SetVisibility(ESlateVisibility::Collapsed);
		HeroTalentNoticeLabel.SetWrapTextAt(BarWidth);
		Column.AddChildToVerticalBox(WrappedBox(n"HeroTalentNoticeBox", HeroTalentNoticeLabel)).SetPadding(FMargin(0.0, 4.0, 0.0, 0.0));
		HeroTalentListLabel = MakeText(n"HeroTalentListLabel", UHOLTextStyleHUDLabel);
		HeroTalentListLabel.SetVisibility(ESlateVisibility::Collapsed);
		HeroTalentListLabel.SetWrapTextAt(BarWidth);
		Column.AddChildToVerticalBox(WrappedBox(n"HeroTalentListBox", HeroTalentListLabel)).SetPadding(FMargin(0.0, 2.0, 0.0, 0.0));
	}

	// A text in a box no wider than the block's bars (BarWidth), so that it wraps there.
	USizeBox WrappedBox(FName BoxName, UCommonTextBlock Text)
	{
		USizeBox Box = Cast<USizeBox>(ConstructWidget(USizeBox, BoxName));
		Box.SetMaxDesiredWidth(BarWidth);
		Box.SetContent(Text);
		return Box;
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
		// The block's place follows the bottom bar's scale (EtapeDefini T).
		if (HeroPanelSlot != nullptr)
			HeroPanelSlot.SetPadding(GetBottomLeftPlace());
		RefreshHero();
		RefreshDuel();
		RefreshPause();
		RefreshMinimap();
	}

	void BuildMinimap(UOverlay Frame)
	{
		Minimap = Cast<UHOLMinimapView>(ConstructWidget(UHOLMinimapView, n"Minimap"));
		Minimap.SetSide(MinimapSize);
		Minimap.SetVisibility(ESlateVisibility::Collapsed);
		Minimap.OnPressed.AddUFunction(this, n"HandleMinimapPressed");
		Minimap.OnTicked.AddUFunction(this, n"HandleMinimapTicked");
		UOverlaySlot MinimapSlot = Frame.AddChildToOverlay(Minimap);
		MinimapSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Right);
		MinimapSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Bottom);
		MinimapSlot.SetPadding(FMargin(MinimapMargin));
	}

	// The picture again from the game: the terrain (the fog, the explored sources) twice a second, the dots, the alerts and the
	// camera's frame at every refresh. Hidden when the map has no bounds.
	void RefreshMinimap()
	{
		if (Minimap == nullptr)
			return;
		FVector2D Low;
		FVector2D High;
		if (!Minimap.GetMapArea(Low, High))
		{
			Minimap.SetVisibility(ESlateVisibility::Collapsed);
			return;
		}
		if (!bMinimapFrame || MinimapFrame.MinX != Low.X || MinimapFrame.MaxX != High.X || MinimapFrame.MinY != Low.Y || MinimapFrame.MaxY != High.Y)
		{
			MinimapFrame = HOLMinimap::MakeFrame(Low.X, Low.Y, High.X, High.Y, MinimapSize);
			Minimap.SetFrame(Low, High);
			bMinimapFrame = true;
			MinimapTerrainWait = 0;
		}
		Minimap.SetVisibility(ESlateVisibility::Visible);
		const int PlayerSlot = GetLocalSlot();
		if (MinimapTerrainWait <= 0)
		{
			Minimap.RefreshTerrain(PlayerSlot);
			MinimapTerrainWait = 2;
		}
		MinimapTerrainWait -= 1;

		TArray<FHOLMinimapEntity> Entities;
		Minimap.GatherEntities(PlayerSlot, Entities);
		TArray<FHOLMinimapMark> Marks;
		TArray<FHOLMinimapSeen> Seen;
		// Buildings first, the units over them.
		for (int Pass = 0; Pass < 2; Pass++)
		{
			for (int Index = 0; Index < Entities.Num(); Index++)
			{
				FHOLMinimapEntity Entity = Entities[Index];
				if (Entity.bBuilding != (Pass == 0))
					continue;
				FHOLMinimapMark Mark;
				Mark.Position = HOLMinimap::ToPicture(MinimapFrame, Entity.Location);
				Mark.Color = Entity.Color;
				Mark.Size = Entity.bBuilding ? 6.0 : 3.0;
				Marks.Add(Mark);
				if (Entity.bOwn)
				{
					FHOLMinimapSeen Own;
					Own.Id = Entity.Id;
					Own.X = Entity.Location.X;
					Own.Y = Entity.Location.Y;
					Own.Health = Entity.Health;
					Seen.Add(Own);
				}
			}
		}
		// The alerts: what lost health since the last reading, aged by the game's clock.
		const float Now = Gameplay::GetTimeSeconds();
		HOLMinimap::Age(MinimapAlerts, MinimapLastTime < 0.0 ? 0.0 : Math::Max(0.0, Now - MinimapLastTime));
		MinimapLastTime = Now;
		HOLMinimap::Detect(MinimapSeenBefore, Seen, MinimapAlerts);
		MinimapSeenBefore = Seen;
		TArray<FVector2D> Lit;
		for (int Index = 0; Index < MinimapAlerts.Num(); Index++)
		{
			if (HOLMinimap::IsLit(MinimapAlerts[Index]))
				Lit.Add(HOLMinimap::ToPicture(MinimapFrame, FVector2D(MinimapAlerts[Index].X, MinimapAlerts[Index].Y)));
		}
		MinimapLitAlerts = Lit.Num();
		Minimap.SetLayers(Marks, Lit);
		// The camera's ground as the view sees it now (the frame then follows the focus every frame, HandleMinimapTicked).
		MinimapQuad.Reset();
		if (Minimap.GetCameraGround(MinimapQuad))
			MinimapQuadFocus = Minimap.GetCameraFocus();
		else
			MinimapQuad.Reset();
		HandleMinimapTicked();
	}

	// Every frame: the camera's frame is the last measured ground quad moved by what the focus has moved since (the camera has no
	// rotation and keeps its height between two refreshes, so its footprint only slides: edge scrolling, a drag, a click).
	UFUNCTION()
	void HandleMinimapTicked()
	{
		if (Minimap == nullptr || !bMinimapFrame || MinimapQuad.Num() != 4)
			return;
		const FVector2D Shift = Minimap.GetCameraFocus() - MinimapQuadFocus;
		TArray<FVector2D> Moved;
		for (int Index = 0; Index < MinimapQuad.Num(); Index++)
			Moved.Add(MinimapQuad[Index] + Shift);
		Minimap.SetCameraFrame(HOLMinimap::ToPictureAll(MinimapFrame, Moved));
	}

	// A press on the picture (Position in px from its upper left): left, the camera goes there (held and dragged, it follows);
	// right, the selected units go there (HOLMinimap::ClickAction). The same function serves hol.HUD.Click (MinimapClick...).
	UFUNCTION()
	void HandleMinimapPressed(FVector2D Position, bool bRight, bool bDrag)
	{
		if (Minimap == nullptr || !bMinimapFrame || (bRight && bDrag))
			return;
		const int Action = HOLMinimap::ClickAction(bRight, Minimap.HasOrderableSelection());
		const FVector2D Ground = HOLMinimap::ToWorld(MinimapFrame, Position);
		if (Action == HOLMinimap::ClickCamera)
		{
			Minimap.FocusCamera(Ground);
			MinimapLastClick = f"camera {Math::RoundToInt(Ground.X)} {Math::RoundToInt(Ground.Y)}";
		}
		else if (Action == HOLMinimap::ClickMove)
		{
			Minimap.OrderMove(Ground);
			MinimapLastClick = f"move {Math::RoundToInt(Ground.X)} {Math::RoundToInt(Ground.Y)}";
		}
		else
			MinimapLastClick = "nothing";
	}

	// A whole number written in digits ("120"); anything else counts as 0.
	float ParseWhole(FString Digits)
	{
		float Value = 0.0;
		for (int Index = 0; Index < Digits.Len(); Index++)
		{
			const int Digit = FString("0123456789").Find(Digits.Mid(Index, 1));
			if (Digit < 0)
				return 0.0;
			Value = Value * 10.0 + Digit;
		}
		return Value;
	}

	// hol.HUD.Click "MinimapClickL120x80" / "MinimapClickR120x80": a press at that picture point (px), the left or the right button.
	bool ClickMinimapName(FName WidgetName)
	{
		FString Text = WidgetName.ToString();
		if (!Text.StartsWith("MinimapClick") || Text.Len() < 15)
			return false;
		const bool bRight = Text.Mid(12, 1) == "R";
		FString Rest = Text.Mid(13);
		int XAt = Rest.Find("x");
		if (XAt <= 0)
			return false;
		const float Px = ParseWhole(Rest.Left(XAt));
		const float Py = ParseWhole(Rest.Mid(XAt + 1));
		HandleMinimapPressed(FVector2D(Px, Py), bRight, false);
		return true;
	}
	// The view (HOLPauseScreen::Make) from the game state's copy of the pause.
	void RefreshPause()
	{
		const int PlayerSlot = GetLocalSlot();
		ShowPause(HOLPauseScreen::Make(HOLPause::IsMatchPaused(), HOLPause::IsPauseFree(), HOLPause::GetPausedByName(),
			HOLPause::GetTimerSecondsLeft(), HOLPause::GetPausesLeft(), HOLPause::CanResumeNow(PlayerSlot)));
	}

	void ShowPause(FHOLPauseScreenView View)
	{
		PauseView = View;
		PauseTexts = "";
		PauseLayer.SetVisibility(View.bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		if (!View.bVisible)
			return;
		const FString By = HOLPauseScreenText::ByText(View);
		const FString Timer = HOLPauseScreenText::TimerText(View);
		const FString Left = HOLPauseScreenText::LeftText(View);
		PauseByText.SetText(FText::FromString(By));
		PauseByText.SetVisibility(By.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
		PauseTimerText.SetText(FText::FromString(Timer));
		PauseTimerText.SetVisibility(Timer.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
		PauseLeftText.SetText(FText::FromString(Left));
		PauseLeftText.SetVisibility(Left.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
		PauseResumeLabel.SetColorAndOpacity(View.bCanResume ? UHOLScriptScreen::GetTextColor() : UHOLScriptScreen::GetMutedColor());
		UHOLScriptScreen::SetButtonGreyed(PauseResumeButton, !View.bCanResume);
		PauseTexts = HOLPauseScreenText::Format(HOLPauseScreenText::Title) + " | " + By + " | " + Timer + " | " + Left + " | "
			+ HOLPauseScreenText::Format(HOLPauseScreenText::Resume);
	}

	// A click on « Reprendre »: asked of the server (it decides). Hidden or greyed: nothing.
	bool ClickPauseResume()
	{
		if (!PauseView.bVisible || !PauseView.bCanResume)
			return false;
		AHOLPlayerController Controller = Cast<AHOLPlayerController>(GetOwningPlayer());
		if (Controller == nullptr)
			return false;
		Controller.RequestResume();
		return true;
	}

	UFUNCTION()
	void HandlePauseResumeClicked()
	{
		ClickPauseResume();
	}

	// The view (HOLHeroBlock::Make) from the public sheet, the slot and the return's quote for an order sent now.
	void RefreshHero()
	{
		const int PlayerSlot = GetLocalSlot();
		FHOLHeroBlockView View;
		if (PlayerSlot < 0)
			View = HOLHeroBlock::Make(true, false, EHOLHeroState::None, 0, 0, 0, -1, FHOLHeroReturnQuote());
		else
		{
			View = HOLHeroBlock::Make(false, HOLHero::IsSlotDefeated(PlayerSlot), HOLHero::GetHeroState(PlayerSlot), HOLHero::GetHeroLevel(PlayerSlot),
				HOLHero::GetHeroHealth(PlayerSlot), HOLHero::GetHeroMaxHealth(PlayerSlot), HOLHero::GetHeroXP(PlayerSlot), HOLHeroReturnQuote::ReadNow(PlayerSlot));
			// J8c.4: the abilities' boxes from the private sheet's ready steps (the owner's, J8-Q5) and the server's step.
			TArray<int> ReadySteps;
			ReadySteps.Add(0);
			ReadySteps.Add(HOLHero::GetHeroAbilityReadyStep(PlayerSlot, EHOLHeroAbility::CloseRanks));
			ReadySteps.Add(HOLHero::GetHeroAbilityReadyStep(PlayerSlot, EHOLHeroAbility::Banner));
			if (View.bVisible)
				HOLHeroBlock::SetAbilities(View, HOLHero::GetHeroState(PlayerSlot) == EHOLHeroState::Alive, View.Level, ReadySteps,
					HOLHero::GetServerStep(), HOLHero::GetSimulationRateHz());
		}
		ShowHero(View);
		ShowTalents(PlayerSlot, View);
	}

	// T5: the talent point's reminder (HOLTalentPanel::Make from the level and the picks the sheet knows: -1 picks, unknown here, shows
	// nothing); a button taking the advised choice while a point is kept and a first talent exists (D105).
	void ShowTalents(int PlayerSlot, FHOLHeroBlockView View)
	{
		TalentView = FHOLTalentPanelView();
		TalentTexts = "";
		TalentListText = "";
		TalentTipTexts = "";
		const int Count = PlayerSlot >= 0 ? HOLHero::GetHeroTalentCount(PlayerSlot) : -1;
		if (!View.bVisible || Count < 0)
		{
			HeroTalentButton.SetVisibility(ESlateVisibility::Collapsed);
			HeroFamilyRow.SetVisibility(ESlateVisibility::Collapsed);
			HeroTalentListLabel.SetVisibility(ESlateVisibility::Collapsed);
			HeroTalentNoticeLabel.SetVisibility(ESlateVisibility::Collapsed);
			return;
		}
		TArray<int> Picks;
		for (int Index = 0; Index < Count; ++Index)
			Picks.Add(HOLHero::GetHeroTalentFamily(PlayerSlot, Index));
		ShowTalentFeedback(PlayerSlot, Count, Picks);
		TalentView = HOLTalentPanel::MakeFor(View.Level, Picks, HOLHero::GetTalentFirstChoiceMode());
		HeroTalentButton.SetVisibility(TalentView.bReminder ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		HeroFamilyRow.SetVisibility(TalentView.bFamilyButtons ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		for (int Family = 0; Family < HeroFamilyButtons.Num(); ++Family)
		{
			HeroFamilyLabels[Family].SetText(FText::FromString(HOLTalentText::FamilyName(Family)));
			HeroFamilyLabels[Family].SetColorAndOpacity(UHOLScriptScreen::GetTextColor());
			HeroFamilyButtons[Family].SetToolTipText(FText::FromString(HOLTalentText::FamilyTooltip(TalentView, Family)));
			TalentTipTexts += (Family > 0 ? " || " : "") + HOLTalentText::FamilyTooltip(TalentView, Family);
		}
		if (!TalentView.bReminder)
			return;
		TalentTexts = HOLTalentText::ReminderText(TalentView);
		HeroTalentLabel.SetText(FText::FromString(TalentTexts));
		HeroTalentLabel.SetColorAndOpacity(TalentView.bAdviceEnabled ? UHOLScriptScreen::GetTextColor() : UHOLScriptScreen::GetMutedColor());
		UHOLScriptScreen::SetButtonGreyed(HeroTalentButton, !TalentView.bAdviceEnabled);
		HeroTalentButton.SetToolTipText(FText::FromString(HOLTalentText::TooltipText(TalentView)));
	}

	// T5: what a spent point shows. The talents taken, by name, stay under the reminder; a talent taken since the last refresh raises a notice
	// "Talent pris : <name> — <effect>" in the accent colour for 8 s and a sound (the hero's own slot only: the sheet of an adversary's picks
	// is not sent). Names and effects: HOLTalentNames, the one table.
	void ShowTalentFeedback(int PlayerSlot, int Count, const TArray<int>& Picks)
	{
		if (TalentSeenCount >= 0 && Count > TalentSeenCount)
		{
			TalentNoticeText = HOLTalentText::NoticeText(HOLTalent::FirstRow + Count - 1, Picks[Count - 1]);
			TalentNoticeUntilStep = HOLHero::GetServerStep() + 8 * HOLHero::GetSimulationRateHz();
			HOLHero::PlayTalentTakenSound();
		}
		TalentSeenCount = Count;
		TalentListText = HOLTalentText::ListText(Picks);
		HeroTalentListLabel.SetVisibility(Count > 0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		HeroTalentListLabel.SetText(FText::FromString(TalentListText));
		HeroTalentListLabel.SetColorAndOpacity(UHOLScriptScreen::GetMutedColor());
		const bool bNotice = !TalentNoticeText.IsEmpty() && HOLHero::GetServerStep() < TalentNoticeUntilStep;
		if (!bNotice)
			TalentNoticeText = "";
		HeroTalentNoticeLabel.SetVisibility(bNotice ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		HeroTalentNoticeLabel.SetText(FText::FromString(TalentNoticeText));
		HeroTalentNoticeLabel.SetColorAndOpacity(FLinearColor(1.0, 0.82, 0.25, 1.0));
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
			// No box known (never set): dark, as a locked one.
			FHOLHeroAbilityBox Box;
			Box.bLocked = true;
			if (View.Abilities.IsValidIndex(Index))
				Box = View.Abilities[Index];
			// Lit: full; cooling down or off: dimmed; locked: faint.
			const float Shade = Box.bLit ? 1.0 : Box.bLocked ? 0.3 : 0.55;
			HeroAbilityIcons[Index].SetColorAndOpacity(FLinearColor(Shade, Shade, Shade, Box.bLocked ? 0.6 : 1.0));
			HeroAbilityCurtains[Index].SetHeightOverride(AbilitySize * Box.CooldownFraction);
			HeroAbilitySeconds[Index].SetText(FText::FromString(Box.SecondsLeft > 0 ? f"{Box.SecondsLeft}" : ""));
			HeroAbilities[Index].SetToolTipText(FText::FromString(View.Abilities.IsValidIndex(Index) ? HOLHeroBlockText::AbilityTooltip(Box) : ""));
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

	// A click on the talent line (D105): the advised choice taken, the order sent (the server checks it). Hidden or greyed: nothing.
	bool ClickTalent()
	{
		if (!TalentView.bReminder || !TalentView.bAdviceEnabled)
			return false;
		AHOLPlayerController Controller = Cast<AHOLPlayerController>(GetOwningPlayer());
		if (Controller == nullptr)
			return false;
		Controller.RequestHeroTalent(TalentView.AdvisedFamily);
		return true;
	}

	UFUNCTION()
	void HandleTalentClicked()
	{
		ClickTalent();
	}

	// A click on one of the three family buttons (TQ4, option B): the first (or any) talent of that family, the order sent (the server checks it).
	bool ClickFamily(int Family)
	{
		if (!TalentView.bFamilyButtons)
			return false;
		AHOLPlayerController Controller = Cast<AHOLPlayerController>(GetOwningPlayer());
		if (Controller == nullptr)
			return false;
		Controller.RequestHeroTalent(Family);
		return true;
	}

	UFUNCTION()
	void HandleFamily0()
	{
		ClickFamily(0);
	}

	UFUNCTION()
	void HandleFamily1()
	{
		ClickFamily(1);
	}

	UFUNCTION()
	void HandleFamily2()
	{
		ClickFamily(2);
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
		if (ClickMinimapName(WidgetName))
			return true;
		if (WidgetName == n"HeroTalentButton")
			return ClickTalent();
		if (WidgetName == n"DuelAcceptButton")
			return ClickDuelAnswer(true);
		if (WidgetName == n"DuelRefuseButton")
			return ClickDuelAnswer(false);
		if (WidgetName == n"HeroTalentFamily0")
			return ClickFamily(0);
		if (WidgetName == n"HeroTalentFamily1")
			return ClickFamily(1);
		if (WidgetName == n"HeroTalentFamily2")
			return ClickFamily(2);
		if (WidgetName == n"PauseResumeButton")
			return ClickPauseResume();
		return false;
	}

	// "aura on, cry 42 s, banner locked 4 | <the cry's tooltip, one line>" (hol.HUD.Report).
	// "focus (-14000, 0), ground [(x, y) x4]": where the camera looks and the ground its four corners reach (the frame's source).
	FString DescribeMinimapCamera() const
	{
		FString Line = f"focus ({Math::RoundToInt(MinimapQuadFocus.X)}, {Math::RoundToInt(MinimapQuadFocus.Y)}), ground";
		for (int Index = 0; Index < MinimapQuad.Num(); Index++)
			Line += f" ({Math::RoundToInt(MinimapQuad[Index].X)}, {Math::RoundToInt(MinimapQuad[Index].Y)})";
		return Line;
	}

	FString DescribeAbilities() const
	{
		FString Line;
		for (int Index = 0; Index < HeroView.Abilities.Num(); ++Index)
			Line += (Index > 0 ? ", " : "") + HeroView.Abilities[Index].Describe;
		if (HeroView.Abilities.Num() > 1)
			Line += " | " + HOLHeroBlockText::AbilityTooltip(HeroView.Abilities[1]).Replace("\n", " / ");
		return Line;
	}

	UFUNCTION(BlueprintOverride)
	FString Describe() const
	{
		return "hero: " + HeroView.Describe + (HeroTexts.IsEmpty() ? "" : "\nhero texts: " + HeroTexts)
			+ (HeroView.bVisible ? "\nhero xp: " + HeroView.XPDescribe + f" bar {Math::RoundToInt(HeroView.XPFraction * 100.0)}%" : "")
			+ (HeroView.Abilities.Num() > 0 ? "\nhero abilities: " + DescribeAbilities() : "")
			+ (Minimap != nullptr ? "\n" + Minimap.Describe() + f", lit {MinimapLitAlerts}, alerts {MinimapAlerts.Num()}, area x {Math::RoundToInt(MinimapFrame.MinX)}..{Math::RoundToInt(MinimapFrame.MaxX)} y {Math::RoundToInt(MinimapFrame.MinY)}..{Math::RoundToInt(MinimapFrame.MaxY)}, last click {MinimapLastClick}, camera " + DescribeMinimapCamera() : "")
			+ (HeroView.bVisible && TalentView.Rows.Num() > 0 ? "\nhero talents: " + TalentView.Describe + (TalentTexts.IsEmpty() ? "" : " | " + TalentTexts) + (TalentView.bFamilyButtons ? f" | family buttons row {TalentView.NextRow}" : "") : "")
			+ (HeroView.bVisible && TalentSeenCount >= 0 ? "\nhero talent texts: list " + TalentListText + " | notice " + TalentNoticeText + " | tips " + TalentTipTexts : "")
			+ (DuelView.Kind != EHOLDuelWindowKind::None ? "\nduel window: " + DuelTexts : "")
			+ "\npause: " + PauseView.Describe + (PauseTexts.IsEmpty() ? "" : "\npause texts: " + PauseTexts);
	}
}
