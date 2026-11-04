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
// TQ4 = A: the talent tree window (5 rows x 3 cells, Guardian / Captain / Champion, names and effects on hover), opened by the reminder, the icons
// of the talents taken or the T key; a click on a pickable cell sends RequestHeroTalent. TQ5 = A (D618): one icon per talent taken under the list, its
// hover the talent's name and effect. The views are HOLTalentTree's.
UBorder TalentWindow;
UCommonTextBlock TalentWindowTitle;
UCommonTextBlock TalentWindowPoints;
UButton TalentWindowClose;
UCommonTextBlock TalentWindowCloseLabel;
UButton TalentWindowAdvised;
UCommonTextBlock TalentWindowAdvisedLabel;
TArray<UCommonTextBlock> TalentWindowFamilyHeads;
TArray<UCommonTextBlock> TalentWindowRowLabels;
TArray<UButton> TalentCellButtons;
TArray<UCommonTextBlock> TalentCellLabels;
UHorizontalBox HeroTalentIcons;
TArray<UButton> TalentIconButtons;
TArray<UCommonTextBlock> TalentIconLabels;
TArray<FHOLTalentCellView> TalentCells;
bool bTalentWindowOpen = false;
FString TalentWindowTexts = "";
FString TalentIconTexts = "";
const float TalentCellWidth = 170.0;
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

	// The AI level line (EV-Q1 = B, D620; AIInProgress.md), top left under the top bar: one line per AI slot, "IA 3 : Évolutive, joue Normal".
	// A trial tool: shown only while the times of the Evolving AI are set (hol.AI.SetEvolveTimes), so that a trial reads the stage on the screen.
	UBorder AILevelPanel;
	UOverlaySlot AILevelSlot;
	UCommonTextBlock AILevelText;
	// Where the line sits: under the cheats' panel (C++ widget, over this one: folded or open, its height changes), never behind it
	// (the line was hidden by it, ai_level_line scenario and the "HUD overlap" check). Top margin and gap in px at 1080p.
	const float AILevelTopMin = 72.0;
	const float AILevelGap = 6.0;
	float AILevelTop = 72.0;
	FString AILevelTexts = "";

	// The volcano's lines (V10, HOLVolcanoBanner.as), top left under the cheats' panel and the AI level line (the top right holds the players' lives):
	// the alert's countdown, the lava, the altar being held. Hidden when no site speaks (no Catastrophe, a client, a site that is only armed).
	UBorder VolcanoPanel;
	UOverlaySlot VolcanoSlot;
	UCommonTextBlock VolcanoText;
	FString VolcanoTexts = "";
	// The AI line's height with its gap (px at 1080p), when it is shown.
	const float VolcanoAfterAILine = 34.0;
	float VolcanoTop = 72.0;

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

	// The Diplomacy panel (D7 of DiplomacyInProgress.md, GDD D387): a button at the top, Ctrl + D, and a panel over the game, a line per player (colour,
	// name, faction, lives; stance; vision; tribute; private message). Three presentations behind hol.UI.DiplomacyLayout 0|1|2 (the rows are the
	// same, HOLDiplomacyPanel.as; only the places of the controls differ): 0 stance in three buttons, the panel wide and centred; 1 the stance
	// cycled on a coloured chip, the panel narrow at the right under the players' table; 2 the stance in a drop-down, the panel compact bottom
	// left over the hero's block. The discussion itself (D385) is another chantier: « Message » only notes whom it is for.
	const int DipMaxRows = 8;
	const int DipKindMenu = 4;
	const int DipKindVision = 5;
	const int DipKindTribute = 6;
	const int DipKindMessage = 7;
	const int DipKindChip = 3;
	UBorder DipButtonPanel;
	UOverlaySlot DipButtonSlot;
	UButton DipOpenButton;
	UCommonTextBlock DipOpenLabel;
	USizeBox DipPanelBox;
	UBorder DipPanel;
	UOverlaySlot DipPanelSlot;
	UCommonTextBlock DipTitleText;
	UCommonTextBlock DipLockedText;
	UButton DipCloseButton;
	UCommonTextBlock DipCloseLabel;
	UVerticalBox DipRowsBox;
	TArray<UVerticalBox> DipRowBoxes;
	TArray<UHorizontalBox> DipLines;
	TArray<UHorizontalBox> DipMenuBoxes;
	TArray<UBorder> DipSwatches;
	TArray<USizeBox> DipSwatchBoxes;
	TArray<UCommonTextBlock> DipNames;
	TArray<UCommonTextBlock> DipFactions;
	TArray<UCommonTextBlock> DipInfos;
	TArray<UButton> DipStanceButtons;
	TArray<UButton> DipChips;
	TArray<UButton> DipMenuButtons;
	TArray<UCommonTextBlock> DipPendings;
	TArray<UButton> DipVisionButtons;
	TArray<UButton> DipTributeButtons;
	TArray<UButton> DipMessageButtons;
	TArray<UHOLDiplomacyClick> DipHandlers;
	TArray<FHOLDiplomacyRowView> DipRows;
	bool bDipOpen = false;
	// The match is in the Diplomacy mode (set by RefreshDiplomacy): outside it the panel keeps the players, tribute and message but has no
	// posture control and no vision box (the user's answer of 2026-11-04: they are not there, not greyed).
	bool bDipModeNow = false;
	int DipLayout = -1;
	// The line whose drop-down is open (layout 2), -1 none; the player the last « Message » was for.
	int DipMenuSlot = -1;
	int DipMessageSlot = -1;
	FString DipTexts = "";
	float DipPanelTop = -1.0;
	float DipPanelBottom = -1.0;
	float DipButtonRight = -1.0;

	UFUNCTION(BlueprintOverride)
	void OnInitialized()
	{
		UOverlay Frame = Cast<UOverlay>(ConstructWidget(UOverlay, n"ScriptFrame"));
		SetRootWidget(Frame);
		BuildHeroBlock(Frame);
		BuildDiplomacyPanel(Frame);
		BuildDuelWindow(Frame);
		BuildAILevelLine(Frame);
		BuildVolcanoBanner(Frame);
		BuildMinimap(Frame);
		BuildTalentWindow(Frame);
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

	// ---- The Diplomacy panel (see the members above) ----

	// A button with its label (a child text); the label is found again by DipLabelOf.
	UButton MakeDipButton(FName Name)
	{
		UButton Button = MakeButton(Name);
		UCommonTextBlock Label = MakeText(FName(f"{Name}Label"), UHOLTextStyleHUDLabel);
		Label.SetVisibility(ESlateVisibility::HitTestInvisible);
		Button.AddChild(Label);
		return Button;
	}

	UCommonTextBlock DipLabelOf(UButton Button)
	{
		return Cast<UCommonTextBlock>(Button.GetChildAt(0));
	}

	// One object a control: its clicks come back to PanelClick with the line's slot and the control's kind.
	UButton BindDipButton(UButton Button, int Slot, int Kind)
	{
		UHOLDiplomacyClick Handler = Cast<UHOLDiplomacyClick>(NewObject(this, UHOLDiplomacyClick));
		Handler.Screen = this;
		Handler.Slot = Slot;
		Handler.Kind = Kind;
		DipHandlers.Add(Handler);
		Button.OnClicked.AddUFunction(Handler, n"Clicked");
		return Button;
	}

	void BuildDiplomacyPanel(UOverlay Frame)
	{
		// The button at the top, left of the menu's (the place follows the menu's box, RefreshDiplomacy).
		DipButtonPanel = MakePanel(n"DiplomacyButtonPanel");
		DipButtonPanel.SetPadding(FMargin(4.0, 2.0));
		DipButtonPanel.SetVisibility(ESlateVisibility::Collapsed);
		DipButtonSlot = Frame.AddChildToOverlay(DipButtonPanel);
		DipButtonSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Right);
		DipButtonSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Top);
		DipButtonSlot.SetPadding(FMargin(0.0, 16.0, 300.0, 0.0));
		DipOpenButton = MakeDipButton(n"DiplomacyButton");
		BindDipButton(DipOpenButton, -1, 9);
		DipOpenLabel = DipLabelOf(DipOpenButton);
		DipButtonPanel.SetContent(DipOpenButton);

		// The panel itself: a box limiting its height (the rows scroll beyond it), the title and the close button, the locked note, the rows.
		DipPanelBox = Cast<USizeBox>(ConstructWidget(USizeBox, n"DiplomacyPanelBox"));
		DipPanelBox.SetVisibility(ESlateVisibility::Collapsed);
		DipPanelSlot = Frame.AddChildToOverlay(DipPanelBox);
		DipPanel = MakePanel(n"DiplomacyPanel");
		DipPanelBox.SetContent(DipPanel);
		UVerticalBox Column = Cast<UVerticalBox>(ConstructWidget(UVerticalBox, n"DiplomacyColumn"));
		DipPanel.SetContent(Column);
		UHorizontalBox Head = Cast<UHorizontalBox>(ConstructWidget(UHorizontalBox, n"DiplomacyHead"));
		Column.AddChildToVerticalBox(Head);
		DipTitleText = MakeText(n"DiplomacyTitle", UHOLTextStyleHUD);
		Head.AddChildToHorizontalBox(DipTitleText).SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
		DipLockedText = MakeText(n"DiplomacyLocked", UHOLTextStyleHUDLabel);
		UHorizontalBoxSlot LockedSlot = Head.AddChildToHorizontalBox(DipLockedText);
		LockedSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
		LockedSlot.SetPadding(FMargin(16.0, 0.0, 16.0, 0.0));
		DipCloseButton = MakeDipButton(n"DiplomacyClose");
		BindDipButton(DipCloseButton, -1, 8);
		DipCloseLabel = DipLabelOf(DipCloseButton);
		Head.AddChildToHorizontalBox(DipCloseButton);

		UScrollBox Scroll = Cast<UScrollBox>(ConstructWidget(UScrollBox, n"DiplomacyScroll"));
		Column.AddChildToVerticalBox(Scroll).SetPadding(FMargin(0.0, 8.0, 0.0, 0.0));
		DipRowsBox = Cast<UVerticalBox>(ConstructWidget(UVerticalBox, n"DiplomacyRows"));
		Scroll.AddChild(DipRowsBox);

		for (int Slot = 0; Slot < DipMaxRows; ++Slot)
		{
			UVerticalBox RowBox = Cast<UVerticalBox>(ConstructWidget(UVerticalBox, FName(f"DipRow_{Slot}")));
			DipRowsBox.AddChildToVerticalBox(RowBox).SetPadding(FMargin(0.0, 0.0, 0.0, 6.0));
			DipRowBoxes.Add(RowBox);
			for (int Line = 0; Line < 3; ++Line)
			{
				UHorizontalBox LineBox = Cast<UHorizontalBox>(ConstructWidget(UHorizontalBox, FName(f"DipLine_{Slot}_{Line}")));
				RowBox.AddChildToVerticalBox(LineBox).SetPadding(FMargin(0.0, Line > 0 ? 3.0 : 0.0, 0.0, 0.0));
				DipLines.Add(LineBox);
			}
			UHorizontalBox MenuBox = Cast<UHorizontalBox>(ConstructWidget(UHorizontalBox, FName(f"DipMenuBox_{Slot}")));
			RowBox.AddChildToVerticalBox(MenuBox).SetPadding(FMargin(0.0, 3.0, 0.0, 0.0));
			DipMenuBoxes.Add(MenuBox);

			USizeBox SwatchBox = Cast<USizeBox>(ConstructWidget(USizeBox, FName(f"DipSwatchBox_{Slot}")));
			SwatchBox.SetWidthOverride(14.0);
			SwatchBox.SetHeightOverride(14.0);
			UBorder Swatch = Cast<UBorder>(ConstructWidget(UBorder, FName(f"DipSwatch_{Slot}")));
			SwatchBox.SetContent(Swatch);
			DipSwatchBoxes.Add(SwatchBox);
			DipSwatches.Add(Swatch);
			DipNames.Add(MakeText(FName(f"DipName_{Slot}"), UHOLTextStyleHUD));
			DipFactions.Add(MakeText(FName(f"DipFaction_{Slot}"), UHOLTextStyleHUDLabel));
			DipInfos.Add(MakeText(FName(f"DipInfo_{Slot}"), UHOLTextStyleHUDLabel));
			DipPendings.Add(MakeText(FName(f"DipPending_{Slot}"), UHOLTextStyleHUDLabel));
			// The three stance buttons (layouts 0 and 2), the chip (1) and the drop-down's head (2): Enemy 0, Neutral 1, Ally 2 as the stances.
			DipStanceButtons.Add(BindDipButton(MakeDipButton(FName(f"DipEnemy_{Slot}")), Slot, HOLDiplomacy::Enemy));
			DipStanceButtons.Add(BindDipButton(MakeDipButton(FName(f"DipNeutral_{Slot}")), Slot, HOLDiplomacy::Neutral));
			DipStanceButtons.Add(BindDipButton(MakeDipButton(FName(f"DipAlly_{Slot}")), Slot, HOLDiplomacy::Ally));
			DipChips.Add(BindDipButton(MakeDipButton(FName(f"DipChip_{Slot}")), Slot, DipKindChip));
			DipMenuButtons.Add(BindDipButton(MakeDipButton(FName(f"DipMenu_{Slot}")), Slot, DipKindMenu));
			DipVisionButtons.Add(BindDipButton(MakeDipButton(FName(f"DipVision_{Slot}")), Slot, DipKindVision));
			DipTributeButtons.Add(BindDipButton(MakeDipButton(FName(f"DipTribute_{Slot}")), Slot, DipKindTribute));
			DipMessageButtons.Add(BindDipButton(MakeDipButton(FName(f"DipMessage_{Slot}")), Slot, DipKindMessage));
		}
		for (int Slot = 0; Slot < DipMaxRows; ++Slot)
			DipRows.Add(FHOLDiplomacyRowView());
	}

	// A control of a line: added to a line with its gap.
	void DipAdd(UHorizontalBox Line, UWidget Widget, float Gap = 8.0)
	{
		UHorizontalBoxSlot Added = Line.AddChildToHorizontalBox(Widget);
		Added.SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
		Added.SetPadding(FMargin(0.0, 0.0, Gap, 0.0));
	}

	USizeBox DipWide(UWidget Widget, float Width, FName BoxName)
	{
		USizeBox Box = Cast<USizeBox>(ConstructWidget(USizeBox, BoxName));
		Box.SetMinDesiredWidth(Width);
		Box.SetContent(Widget);
		return Box;
	}

	// Where the controls of a line go, by layout (the same widgets every time: ClearChildren only detaches them).
	void PlaceDipRow(int Slot, int Layout)
	{
		for (int Line = 0; Line < 3; ++Line)
			DipLines[Slot * 3 + Line].ClearChildren();
		DipMenuBoxes[Slot].ClearChildren();
		UHorizontalBox First = DipLines[Slot * 3];
		UHorizontalBox Second = DipLines[Slot * 3 + 1];
		UHorizontalBox Third = DipLines[Slot * 3 + 2];
		DipAdd(First, DipSwatchBoxes[Slot], 8.0);
		if (Layout == 0)
		{
			// One wide line: colour, name, faction, lives | three buttons, the wait | vision, tribute, message.
			DipAdd(First, DipWide(DipNames[Slot], 110.0, FName(f"DipNameBox_{Slot}")), 10.0);
			DipAdd(First, DipWide(DipFactions[Slot], 150.0, FName(f"DipFactionBox_{Slot}")), 10.0);
			DipAdd(First, DipWide(DipInfos[Slot], 60.0, FName(f"DipInfoBox_{Slot}")), 14.0);
			DipAdd(First, DipStanceButtons[Slot * 3 + 2], 4.0);
			DipAdd(First, DipStanceButtons[Slot * 3 + 1], 4.0);
			DipAdd(First, DipStanceButtons[Slot * 3 + 0], 8.0);
			DipAdd(First, DipWide(DipPendings[Slot], 170.0, FName(f"DipPendingBox_{Slot}")), 10.0);
			DipAdd(First, DipVisionButtons[Slot], 6.0);
			DipAdd(First, DipTributeButtons[Slot], 6.0);
			DipAdd(First, DipMessageButtons[Slot], 0.0);
		}
		else if (Layout == 1)
		{
			// Narrow: colour, name, faction, lives; then the chip, the wait; then vision, tribute, message under.
			DipAdd(First, DipWide(DipNames[Slot], 80.0, FName(f"DipNameBox_{Slot}")), 8.0);
			DipAdd(First, DipFactions[Slot], 8.0);
			DipAdd(First, DipInfos[Slot], 0.0);
			DipAdd(Second, DipChips[Slot], 8.0);
			DipAdd(Second, DipPendings[Slot], 0.0);
			DipAdd(Third, DipVisionButtons[Slot], 6.0);
			DipAdd(Third, DipTributeButtons[Slot], 6.0);
			DipAdd(Third, DipMessageButtons[Slot], 0.0);
		}
		else
		{
			// Compact: colour, name, faction, lives; then the drop-down's head, the wait, vision, tribute, message; the drop-down opens under.
			DipAdd(First, DipWide(DipNames[Slot], 80.0, FName(f"DipNameBox_{Slot}")), 8.0);
			DipAdd(First, DipFactions[Slot], 8.0);
			DipAdd(First, DipInfos[Slot], 0.0);
			DipAdd(Second, DipMenuButtons[Slot], 8.0);
			DipAdd(Second, DipPendings[Slot], 8.0);
			DipAdd(Second, DipVisionButtons[Slot], 6.0);
			DipAdd(Second, DipTributeButtons[Slot], 6.0);
			DipAdd(Second, DipMessageButtons[Slot], 0.0);
			DipAdd(DipMenuBoxes[Slot], DipStanceButtons[Slot * 3 + 2], 4.0);
			DipAdd(DipMenuBoxes[Slot], DipStanceButtons[Slot * 3 + 1], 4.0);
			DipAdd(DipMenuBoxes[Slot], DipStanceButtons[Slot * 3 + 0], 0.0);
		}
	}

	// Tints over the buttons' dark: a stance's colour on its active button or chip (the brush is nearly black, so the factors are large).
	FLinearColor DipStanceTint(int Stance) const
	{
		if (Stance == HOLDiplomacy::Ally)
			return FLinearColor(2.6, 8.0, 2.8, 1.0);
		if (Stance == HOLDiplomacy::Neutral)
			return FLinearColor(8.5, 7.1, 1.7, 1.0);
		return FLinearColor(8.5, 2.1, 1.7, 1.0);
	}

	// A control shown or not (collapsed: no room), greyed or not (its click ignored), its label and tooltip.
	void DipSetButton(UButton Button, bool bShown, bool bEnabled, FText Label, FText Tip, FLinearColor Tint)
	{
		Button.SetVisibility(bShown ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		if (!bShown)
			return;
		UHOLScriptScreen::SetButtonGreyed(Button, !bEnabled);
		// Greyed: the whole control dimmed (the muted label alone was too close to the normal one to tell them apart).
		Button.SetRenderOpacity(bEnabled ? 1.0 : 0.45);
		Button.SetBackgroundColor(Tint);
		UCommonTextBlock LabelText = DipLabelOf(Button);
		LabelText.SetText(Label);
		LabelText.SetColorAndOpacity(bEnabled ? UHOLScriptScreen::GetTextColor() : UHOLScriptScreen::GetMutedColor());
		Button.SetToolTipText(Tip);
	}

	void RefreshDiplomacy()
	{
		const int Viewer = GetLocalSlot();
		const bool bPlays = Viewer >= 0;
		DipTexts = "";
		DipButtonPanel.SetVisibility(bPlays ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		DipOpenLabel.SetText(HOLDiplomacyPanelText::PatternText(HOLDiplomacyPanelText::Button));
		DipOpenButton.SetToolTipText(HOLDiplomacyPanelText::PatternText(HOLDiplomacyPanelText::ButtonTip));
		DipOpenLabel.SetColorAndOpacity(bDipOpen ? UHOLScriptScreen::GetMutedColor() : UHOLScriptScreen::GetTextColor());
		// Left of the menu's buttons (the C++ screen's box), at their height.
		FVector2D MenuLow;
		FVector2D MenuHigh;
		const FVector2D Room = GetViewportSlateSize();
		if (GetGameScreenBox(n"MenuPanel", MenuLow, MenuHigh))
		{
			const float Right = Math::Max(0.0, Room.X - MenuLow.X + 6.0);
			if (Math::Abs(Right - DipButtonRight) > 0.5 || Math::Abs(MenuLow.Y - DipPanelTop) > 0.5)
			{
				DipButtonRight = Right;
				DipPanelTop = MenuLow.Y;
				DipButtonSlot.SetPadding(FMargin(0.0, Math::Max(4.0, MenuLow.Y), Right, 0.0));
			}
		}
		if (!bPlays)
			bDipOpen = false;
		DipPanelBox.SetVisibility(bDipOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		if (!bDipOpen)
			return;

		const int Layout = HOLDiplomacyRead::GetPanelLayout();
		const bool bDipMode = HOLDiplomacyRead::IsDiplomacyMode();
		bDipModeNow = bDipMode;
		if (Layout != DipLayout)
		{
			DipLayout = Layout;
			DipMenuSlot = -1;
			for (int Slot = 0; Slot < DipMaxRows; ++Slot)
				PlaceDipRow(Slot, Layout);
		}
		PlaceDipPanel(Layout, Room);
		DipTitleText.SetText(HOLDiplomacyPanelText::PatternText(HOLDiplomacyPanelText::Title));
		DipLockedText.SetText(FText());
		DipLockedText.SetColorAndOpacity(UHOLScriptScreen::GetMutedColor());
		DipCloseLabel.SetText(HOLDiplomacyPanelText::PatternText(HOLDiplomacyPanelText::Close));

		const bool bViewerDefeated = HOLHero::IsSlotDefeated(Viewer);
		const bool bMarket = HOLDiplomacyRead::HasFinishedMarket(Viewer);
		const int Tax = HOLDiplomacyRead::GetTributeTaxPercent();
		for (int Slot = 0; Slot < DipMaxRows; ++Slot)
		{
			const bool bThere = HOLDiplomacyRead::IsSlotInMatch(Slot);
			DipRowBoxes[Slot].SetVisibility(bThere ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
			if (!bThere)
			{
				DipRows[Slot] = FHOLDiplomacyRowView();
				continue;
			}
			FHOLDiplomacyRowView Row = HOLDiplomacyPanel::MakeRow(bDipMode, Viewer, bViewerDefeated, Slot, HOLDiplomacyRead::IsSlotAI(Slot),
				HOLHero::IsSlotDefeated(Slot), HOLDiplomacyRead::GetStance(Viewer, Slot), HOLDiplomacyRead::GetStance(Slot, Viewer),
				HOLDiplomacyRead::GetVision(Viewer, Slot), bMarket, Tax);
			DipRows[Slot] = Row;
			ShowDipRow(Slot, Row, Layout, Viewer);
		}
	}

	// The panel's place and room by layout, from where the other blocks are now (px at the HUD's scale).
	void PlaceDipPanel(int Layout, FVector2D Room)
	{
		float Top = 150.0;
		float MaxHeight = 400.0;
		EHorizontalAlignment Horizontal = EHorizontalAlignment::HAlign_Center;
		EVerticalAlignment Vertical = EVerticalAlignment::VAlign_Top;
		FMargin Margin = FMargin(0.0);
		if (Layout == 0)
		{
			// Centred, high: clear of the top bar, the table at the right and the bottom bar (330 px kept below).
			MaxHeight = Math::Max(160.0, Room.Y - Top - 330.0);
			Margin = FMargin(0.0, Top, 0.0, 0.0);
		}
		else if (Layout == 1)
		{
			// At the right, under the players' table and the thread's five lines (they fade; the place stays), over the minimap's.
			FVector2D TableLow;
			FVector2D TableHigh;
			float Under = 72.0;
			if (GetGameScreenBox(n"PlayersPanel", TableLow, TableHigh))
				Under = TableHigh.Y;
			Top = Under + 6.0 + 5.0 * 26.0 + 6.0;
			MaxHeight = Math::Max(160.0, Room.Y - Top - 272.0);
			Horizontal = EHorizontalAlignment::HAlign_Right;
			Margin = FMargin(0.0, Top, 16.0, 0.0);
		}
		else
		{
			// Bottom left, over the hero's block (its top edge, or the bottom bar's place when it is hidden), under the cheats' panel.
			FVector2D HeroLow;
			FVector2D HeroHigh;
			float Limit = Room.Y - GetBottomLeftPlace().Bottom;
			if (GetWidgetBox(HeroPanel, HeroLow, HeroHigh))
				Limit = HeroLow.Y;
			Top = Math::Max(GetCheatPanelBottom() + 8.0, 80.0);
			MaxHeight = Math::Max(120.0, Limit - 8.0 - Top);
			Horizontal = EHorizontalAlignment::HAlign_Left;
			Vertical = EVerticalAlignment::VAlign_Bottom;
			Margin = FMargin(16.0, 0.0, 0.0, Math::Max(0.0, Room.Y - Limit + 8.0));
		}
		DipPanelSlot.SetHorizontalAlignment(Horizontal);
		DipPanelSlot.SetVerticalAlignment(Vertical);
		DipPanelSlot.SetPadding(Margin);
		DipPanelBox.SetMaxDesiredHeight(MaxHeight);
	}

	// One line from its row.
	void ShowDipRow(int Slot, FHOLDiplomacyRowView Row, int Layout, int Viewer)
	{
		DipRowBoxes[Slot].SetRenderOpacity(Row.bDefeated ? 0.45 : 1.0);
		DipSwatches[Slot].SetBrushColor(HOLDiplomacyRead::GetSlotColor(Slot));
		DipNames[Slot].SetText(FText::FromString(HOLDiplomacyRead::GetSlotNameText(Viewer, Slot)));
		DipNames[Slot].SetColorAndOpacity(HOLDiplomacyRead::GetSlotColor(Slot));
		DipFactions[Slot].SetText(FText::FromString(HOLDiplomacyRead::GetSlotFactionText(Slot)));
		DipFactions[Slot].SetColorAndOpacity(UHOLScriptScreen::GetMutedColor());
		FString Info = HOLDiplomacyRead::GetSlotLivesText(Viewer, Slot);
		if (Info.IsEmpty())
			Info = HOLDiplomacyRead::GetSlotStateText(Viewer, Slot);
		DipInfos[Slot].SetText(FText::FromString(Info));
		DipInfos[Slot].SetColorAndOpacity(UHOLScriptScreen::GetMutedColor());

		// The posture controls: none on the viewer's own line; greyed when locked (the standard modes, an AI, a defeated player).
		const bool bControls = !Row.bSelf;
		const bool bStances = bControls && bDipModeNow;
		for (int Stance = HOLDiplomacy::Enemy; Stance <= HOLDiplomacy::Ally; ++Stance)
		{
			const bool bActive = Row.MyStance == Stance;
			DipSetButton(DipStanceButtons[Slot * 3 + Stance], bStances && (Layout == 0 || (Layout == 2 && DipMenuSlot == Slot)), Row.bStanceEditable,
				HOLDiplomacyPanelText::StanceLabel(Stance), FText(), bActive ? DipStanceTint(Stance) : FLinearColor(1.0, 1.0, 1.0, 1.0));
		}
		DipSetButton(DipChips[Slot], bStances && Layout == 1, Row.bStanceEditable, HOLDiplomacyPanelText::StanceLabel(Row.MyStance), FText(), DipStanceTint(Row.MyStance));
		DipSetButton(DipMenuButtons[Slot], bStances && Layout == 2, Row.bStanceEditable,
			FText::FromString(HOLDiplomacyPanelText::StanceLabel(Row.MyStance).ToString() + " v"), FText(), DipStanceTint(Row.MyStance));
		DipPendings[Slot].SetVisibility(Row.bPending ? ESlateVisibility::HitTestInvisible : (Layout == 0 && bStances ? ESlateVisibility::Hidden : ESlateVisibility::Collapsed));
		DipPendings[Slot].SetText(HOLDiplomacyPanelText::PatternText(HOLDiplomacyPanelText::Pending));
		DipPendings[Slot].SetColorAndOpacity(FLinearColor(1.0, 0.82, 0.25, 1.0));

		// Vision (checked: [x]), tribute (with the tax, greyed without a Market), private message.
		const FString Mark = Row.bVisionChecked ? "[x] " : "[ ] ";
		DipSetButton(DipVisionButtons[Slot], bStances, Row.bVisionEnabled, FText::FromString(Mark + HOLDiplomacyPanelText::Format(HOLDiplomacyPanelText::Vision)),
			HOLDiplomacyPanelText::PatternText(HOLDiplomacyPanelText::VisionTip), Row.bVisionChecked ? DipStanceTint(HOLDiplomacy::Ally) : FLinearColor(1.0, 1.0, 1.0, 1.0));
		DipSetButton(DipTributeButtons[Slot], bControls, Row.bTributeEnabled, FText::FromString(HOLDiplomacyPanelText::Format(HOLDiplomacyPanelText::Tribute, f"{Row.TaxPercent}")),
			Row.bNoMarket ? HOLDiplomacyPanelText::PatternText(HOLDiplomacyPanelText::TributeNoMarket) : FText::FromString(HOLDiplomacyPanelText::Format(HOLDiplomacyPanelText::TributeTip, f"{Row.TaxPercent}")),
			FLinearColor(1.0, 1.0, 1.0, 1.0));
		DipSetButton(DipMessageButtons[Slot], bControls, Row.bMessageEnabled, HOLDiplomacyPanelText::PatternText(HOLDiplomacyPanelText::Message),
			HOLDiplomacyPanelText::PatternText(HOLDiplomacyPanelText::MessageTip), FLinearColor(1.0, 1.0, 1.0, 1.0));
		DipMenuBoxes[Slot].SetVisibility(Layout == 2 && DipMenuSlot == Slot ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		DipTexts += (DipTexts.IsEmpty() ? "" : " || ") + HOLDiplomacyRead::GetSlotNameText(Viewer, Slot) + " (" + HOLDiplomacyRead::GetSlotFactionText(Slot) + ") "
			+ Info + " | " + HOLDiplomacyPanelText::StanceLabel(Row.MyStance).ToString() + (Row.bPending ? " | " + HOLDiplomacyPanelText::Format(HOLDiplomacyPanelText::Pending) : "")
			+ " | " + Row.Describe;
	}

	// A click on a control of the panel (the mouse's, through UHOLDiplomacyClick, or hol.HUD.Click by the control's name): Kind 0..2 a stance
	// (Enemy, Neutral, Ally), 3 the chip (the next stance), 4 the drop-down, 5 vision, 6 tribute, 7 message, 8 close, 9 the top button.
	// A hidden or greyed control does nothing (false). The server decides the rest (the order is taken at the next step).
	bool PanelClick(int Slot, int Kind)
	{
		if (Kind == 9)
			return ToggleDiplomacy();
		if (!bDipOpen)
			return false;
		if (Kind == 8)
		{
			bDipOpen = false;
			DipPanelBox.SetVisibility(ESlateVisibility::Collapsed);
			return true;
		}
		if (Slot < 0 || Slot >= DipMaxRows)
			return false;
		const FHOLDiplomacyRowView Row = DipRows[Slot];
		AHOLPlayerController Controller = Cast<AHOLPlayerController>(GetOwningPlayer());
		if (Row.Slot != Slot || Row.bSelf || Controller == nullptr)
			return false;
		if (Kind >= HOLDiplomacy::Enemy && Kind <= HOLDiplomacy::Ally)
		{
			if (!Row.bStanceEditable || !DipStanceButtons[Slot * 3 + Kind].IsVisible())
				return false;
			Controller.RequestStance(Slot, Kind);
			DipMenuSlot = -1;
			return true;
		}
		if (Kind == DipKindChip)
		{
			if (!Row.bStanceEditable || !DipChips[Slot].IsVisible())
				return false;
			Controller.RequestStance(Slot, HOLDiplomacyPanel::NextStance(Row.MyStance));
			return true;
		}
		if (Kind == DipKindMenu)
		{
			if (!Row.bStanceEditable || !DipMenuButtons[Slot].IsVisible())
				return false;
			DipMenuSlot = DipMenuSlot == Slot ? -1 : Slot;
			return true;
		}
		if (Kind == DipKindVision)
		{
			if (!Row.bVisionEnabled || !DipVisionButtons[Slot].IsVisible())
				return false;
			Controller.RequestShareVision(Slot, !Row.bVisionChecked);
			return true;
		}
		if (Kind == DipKindTribute)
		{
			if (!Row.bTributeEnabled)
				return false;
			Controller.RequestPanelTribute(Slot, 3, 100);
			return true;
		}
		if (Kind == DipKindMessage)
		{
			if (!Row.bMessageEnabled)
				return false;
			DipMessageSlot = Slot;
			return true;
		}
		return false;
	}

	bool ToggleDiplomacy()
	{
		if (GetLocalSlot() < 0)
			return false;
		bDipOpen = !bDipOpen;
		DipMenuSlot = -1;
		DipPanelBox.SetVisibility(bDipOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		return true;
	}

	// hol.HUD.Click names: DiplomacyButton, DiplomacyClose, Dip<Enemy|Neutral|Ally|Chip|Menu|Vision|Tribute|Message>_<slot>, DiplomacyKey (Ctrl + D).
	bool ClickDiplomacyName(FName WidgetName)
	{
		if (WidgetName == n"DiplomacyButton" || WidgetName == n"DiplomacyKey")
			return ToggleDiplomacy();
		if (WidgetName == n"DiplomacyClose")
			return PanelClick(-1, 8);
		for (int Slot = 0; Slot < DipMaxRows; ++Slot)
		{
			if (WidgetName == FName(f"DipEnemy_{Slot}"))
				return PanelClick(Slot, HOLDiplomacy::Enemy);
			if (WidgetName == FName(f"DipNeutral_{Slot}"))
				return PanelClick(Slot, HOLDiplomacy::Neutral);
			if (WidgetName == FName(f"DipAlly_{Slot}"))
				return PanelClick(Slot, HOLDiplomacy::Ally);
			if (WidgetName == FName(f"DipChip_{Slot}"))
				return PanelClick(Slot, DipKindChip);
			if (WidgetName == FName(f"DipMenu_{Slot}"))
				return PanelClick(Slot, DipKindMenu);
			if (WidgetName == FName(f"DipVision_{Slot}"))
				return PanelClick(Slot, DipKindVision);
			if (WidgetName == FName(f"DipTribute_{Slot}"))
				return PanelClick(Slot, DipKindTribute);
			if (WidgetName == FName(f"DipMessage_{Slot}"))
				return PanelClick(Slot, DipKindMessage);
		}
		return false;
	}

	void BuildVolcanoBanner(UOverlay Frame)
	{
		VolcanoPanel = MakePanel(n"VolcanoPanel");
		VolcanoPanel.SetVisibility(ESlateVisibility::Collapsed);
		VolcanoSlot = Frame.AddChildToOverlay(VolcanoPanel);
		VolcanoSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);
		VolcanoSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Top);
		VolcanoSlot.SetPadding(FMargin(16.0, VolcanoTop, 0.0, 0.0));
		VolcanoText = MakeText(n"VolcanoText", UHOLTextStyleHUDLabel);
		VolcanoPanel.SetContent(VolcanoText);
	}

	// One line per site that has something to read (HOLVolcanoBanner::Make, from the library's views: the alert, the lava, the altar).
	void RefreshVolcano()
	{
		TArray<FHOLVolcanoSiteView> Sites;
		const int Count = HOLVolcano::GetSiteCount();
		for (int Index = 0; Index < Count; Index++)
		{
			Sites.Add(HOLVolcano::GetSiteView(Index));
		}
		const FHOLVolcanoBannerView View = HOLVolcanoBanner::Make(Sites);
		VolcanoTexts = View.Text.Replace("\n", " / ");
		// Under the cheats' panel (folded or open) and under the AI level line when it is there.
		const float Top = Math::Max(AILevelTopMin, GetCheatPanelBottom() + AILevelGap) + (AILevelPanel.GetVisibility() == ESlateVisibility::Collapsed ? 0.0 : VolcanoAfterAILine);
		if (Math::Abs(Top - VolcanoTop) > 0.5)
		{
			VolcanoTop = Top;
			VolcanoSlot.SetPadding(FMargin(16.0, VolcanoTop, 0.0, 0.0));
		}
		VolcanoPanel.SetVisibility(View.bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		VolcanoText.SetText(FText::FromString(View.Text));
	}

	void BuildAILevelLine(UOverlay Frame)
	{
		AILevelPanel = MakePanel(n"AILevelPanel");
		AILevelPanel.SetVisibility(ESlateVisibility::Collapsed);
		AILevelSlot = Frame.AddChildToOverlay(AILevelPanel);
		AILevelSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);
		AILevelSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Top);
		AILevelSlot.SetPadding(FMargin(16.0, AILevelTop, 0.0, 0.0));
		AILevelText = MakeText(n"AILevelText", UHOLTextStyleHUDLabel);
		AILevelPanel.SetContent(AILevelText);
	}

	// One line per AI slot: its level, and for an Evolving one the stage it plays now (HOLAILives::Played, the brain's own rule, from the match's
	// clock). Hidden unless the trial tool set the times, and when the match has no AI.
	void RefreshAILevel()
	{
		FString Text = "";
		if (HOLAITools::HasEvolveTimes())
		{
			const float Now = float(Math::Max(0, HOLHero::GetServerStep())) / float(Math::Max(1, HOLHero::GetSimulationRateHz()));
			const FHOLAILivesData Times;
			for (int Slot = 0; Slot < 8; Slot++)
			{
				const int Code = HOLAITools::GetSlotLevel(Slot);
				if (Code < 0)
					continue;
				const EHOLAIDifficulty Configured = EHOLAIDifficulty(Code);
				Text += (Text.IsEmpty() ? "" : "\n") + HOLAILives::LevelLine(Slot, Configured, HOLAILives::Played(Configured, Now, Times));
			}
		}
		AILevelTexts = Text.Replace("\n", " / ");
		// Under the cheats' panel, as tall as it is now (folded or open).
		const float Top = Math::Max(AILevelTopMin, GetCheatPanelBottom() + AILevelGap);
		if (Math::Abs(Top - AILevelTop) > 0.5)
		{
			AILevelTop = Top;
			AILevelSlot.SetPadding(FMargin(16.0, AILevelTop, 0.0, 0.0));
		}
		AILevelPanel.SetVisibility(Text.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
		AILevelText.SetText(FText::FromString(Text));
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

		// The notice and the list wrap at the block's width (a long effect would otherwise widen the whole block when a talent is taken).
		HeroTalentNoticeLabel = MakeText(n"HeroTalentNoticeLabel", UHOLTextStyleHUDLabel);
		HeroTalentNoticeLabel.SetVisibility(ESlateVisibility::Collapsed);
		HeroTalentNoticeLabel.SetWrapTextAt(BarWidth);
		Column.AddChildToVerticalBox(WrappedBox(n"HeroTalentNoticeBox", HeroTalentNoticeLabel)).SetPadding(FMargin(0.0, 4.0, 0.0, 0.0));
		HeroTalentListLabel = MakeText(n"HeroTalentListLabel", UHOLTextStyleHUDLabel);
		HeroTalentListLabel.SetVisibility(ESlateVisibility::Collapsed);
		HeroTalentListLabel.SetWrapTextAt(BarWidth);
		Column.AddChildToVerticalBox(WrappedBox(n"HeroTalentListBox", HeroTalentListLabel)).SetPadding(FMargin(0.0, 2.0, 0.0, 0.0));
		// TQ5 = A: one icon per talent taken (a click opens the tree), its hover the talent's name and effect.
		HeroTalentIcons = Cast<UHorizontalBox>(ConstructWidget(UHorizontalBox, n"HeroTalentIcons"));
		HeroTalentIcons.SetVisibility(ESlateVisibility::Collapsed);
		Column.AddChildToVerticalBox(HeroTalentIcons).SetPadding(FMargin(0.0, 3.0, 0.0, 0.0));
		const int IconCount = HOLTalent::PointsEarned(HOLHero::GetMaxLevel());
		for (int Index = 0; Index < IconCount; ++Index)
		{
			USizeBox IconBox = Cast<USizeBox>(ConstructWidget(USizeBox, FName(f"HeroTalentIconBox{Index}")));
			IconBox.SetWidthOverride(AbilitySize);
			IconBox.SetHeightOverride(AbilitySize);
			UButton Icon = MakeButton(FName(f"HeroTalentIcon{Index}"));
			Icon.OnClicked.AddUFunction(this, n"HandleTalentClicked");
			Icon.SetVisibility(ESlateVisibility::Collapsed);
			IconBox.SetContent(Icon);
			UCommonTextBlock Glyph = MakeText(FName(f"HeroTalentIconGlyph{Index}"), UHOLTextStyleHUD);
			Glyph.SetVisibility(ESlateVisibility::HitTestInvisible);
			Icon.AddChild(Glyph);
			HeroTalentIcons.AddChildToHorizontalBox(IconBox).SetPadding(FMargin(0.0, 0.0, 4.0, 0.0));
			TalentIconButtons.Add(Icon);
			TalentIconLabels.Add(Glyph);
		}
	}

	// The tree window (TQ4 = A): a panel in the middle of the screen, no dim layer (the game goes on under it, only the panel takes the clicks). A
	// title and a close button, the points kept, the three family heads, then 5 rows of 3 cells (the row's number at the left); each cell a button
	// whose label is the talent's name and whose hover is its name and effect; the advised choice (D105) at the foot.
	void BuildTalentWindow(UOverlay Frame)
	{
		TalentWindow = MakePanel(n"TalentWindow");
		TalentWindow.SetVisibility(ESlateVisibility::Collapsed);
		UOverlaySlot WindowSlot = Frame.AddChildToOverlay(TalentWindow);
		WindowSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
		WindowSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
		UVerticalBox Column = Cast<UVerticalBox>(ConstructWidget(UVerticalBox, n"TalentWindowColumn"));
		TalentWindow.SetContent(Column);

		UHorizontalBox Head = Cast<UHorizontalBox>(ConstructWidget(UHorizontalBox, n"TalentWindowHead"));
		Column.AddChildToVerticalBox(Head);
		TalentWindowTitle = MakeText(n"TalentWindowTitle", UHOLTextStyleHUD);
		Head.AddChildToHorizontalBox(TalentWindowTitle).SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
		TalentWindowPoints = MakeText(n"TalentWindowPoints", UHOLTextStyleHUDLabel);
		UHorizontalBoxSlot PointsSlot = Head.AddChildToHorizontalBox(TalentWindowPoints);
		PointsSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
		PointsSlot.SetPadding(FMargin(24.0, 0.0, 24.0, 0.0));
		TalentWindowClose = MakeButton(n"TalentWindowClose");
		TalentWindowClose.OnClicked.AddUFunction(this, n"HandleTalentWindowClose");
		Head.AddChildToHorizontalBox(TalentWindowClose);
		TalentWindowCloseLabel = MakeText(n"TalentWindowCloseLabel", UHOLTextStyleHUDLabel);
		TalentWindowCloseLabel.SetVisibility(ESlateVisibility::HitTestInvisible);
		TalentWindowClose.AddChild(TalentWindowCloseLabel);

		// The family heads, over the columns (after the width of the row label).
		UHorizontalBox Heads = Cast<UHorizontalBox>(ConstructWidget(UHorizontalBox, n"TalentWindowHeads"));
		Column.AddChildToVerticalBox(Heads).SetPadding(FMargin(0.0, 10.0, 0.0, 2.0));
		Heads.AddChildToHorizontalBox(TalentRowLabelBox(n"TalentWindowHeadsPad", nullptr));
		for (int Family = 0; Family < HOLTalent::FamilyCount; ++Family)
		{
			UCommonTextBlock FamilyHead = MakeText(FName(f"TalentWindowFamily{Family}"), UHOLTextStyleHUD);
			FamilyHead.SetJustification(ETextJustify::Center);
			USizeBox HeadBox = Cast<USizeBox>(ConstructWidget(USizeBox, FName(f"TalentWindowFamilyBox{Family}")));
			HeadBox.SetWidthOverride(TalentCellWidth);
			HeadBox.SetContent(FamilyHead);
			Heads.AddChildToHorizontalBox(HeadBox).SetPadding(FMargin(0.0, 0.0, 4.0, 0.0));
			TalentWindowFamilyHeads.Add(FamilyHead);
		}

		// All the game's rows (2 to 9): the ones past the prototype's maximum level are drawn greyed "plus tard" (HOLTalentTree::MakeWithLater).
		const int Rows = HOLTalent::RowCount;
		for (int Index = 0; Index < Rows; ++Index)
		{
			const int Row = HOLTalent::FirstRow + Index;
			UHorizontalBox RowBox = Cast<UHorizontalBox>(ConstructWidget(UHorizontalBox, FName(f"TalentWindowRow{Row}")));
			Column.AddChildToVerticalBox(RowBox).SetPadding(FMargin(0.0, 3.0, 0.0, 0.0));
			UCommonTextBlock RowLabel = MakeText(FName(f"TalentWindowRowLabel{Row}"), UHOLTextStyleHUDLabel);
			RowBox.AddChildToHorizontalBox(TalentRowLabelBox(FName(f"TalentWindowRowLabelBox{Row}"), RowLabel)).SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
			TalentWindowRowLabels.Add(RowLabel);
			for (int Family = 0; Family < HOLTalent::FamilyCount; ++Family)
			{
				USizeBox CellBox = Cast<USizeBox>(ConstructWidget(USizeBox, FName(f"HeroTalentCellBox_{Row}_{Family}")));
				CellBox.SetWidthOverride(TalentCellWidth);
				CellBox.SetMinDesiredHeight(40.0);
				UButton Cell = MakeButton(TalentCellName(Row, Family));
				// One handler a cell (a button's click does not say which): HandleCell0 .. HandleCell23, the row and the column from the number.
				Cell.OnClicked.AddUFunction(this, FName(f"HandleCell{Index * HOLTalent::FamilyCount + Family}"));
				CellBox.SetContent(Cell);
				UCommonTextBlock CellLabel = MakeText(FName(f"HeroTalentCellLabel_{Row}_{Family}"), UHOLTextStyleHUDLabel);
				CellLabel.SetVisibility(ESlateVisibility::HitTestInvisible);
				CellLabel.SetJustification(ETextJustify::Center);
				CellLabel.SetWrapTextAt(TalentCellWidth - 20.0);
				Cell.AddChild(CellLabel);
				RowBox.AddChildToHorizontalBox(CellBox).SetPadding(FMargin(0.0, 0.0, 4.0, 0.0));
				TalentCellButtons.Add(Cell);
				TalentCellLabels.Add(CellLabel);
			}
		}

		TalentWindowAdvised = MakeButton(n"TalentWindowAdvised");
		TalentWindowAdvised.OnClicked.AddUFunction(this, n"HandleTalentAdvisedClicked");
		UVerticalBoxSlot AdvisedSlot = Column.AddChildToVerticalBox(TalentWindowAdvised);
		AdvisedSlot.SetPadding(FMargin(0.0, 10.0, 0.0, 0.0));
		AdvisedSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
		TalentWindowAdvisedLabel = MakeText(n"TalentWindowAdvisedLabel", UHOLTextStyleHUDLabel);
		TalentWindowAdvisedLabel.SetVisibility(ESlateVisibility::HitTestInvisible);
		TalentWindowAdvised.AddChild(TalentWindowAdvisedLabel);
	}

	// The room at the left of a row (its number) and of the family heads: the same width.
	USizeBox TalentRowLabelBox(FName BoxName, UCommonTextBlock Label)
	{
		USizeBox Box = Cast<USizeBox>(ConstructWidget(USizeBox, BoxName));
		Box.SetWidthOverride(80.0);
		if (Label != nullptr)
			Box.SetContent(Label);
		return Box;
	}

	// The name of a cell's button (hol.HUD.Click): HeroTalentCell_<row>_<family>.
	FName TalentCellName(int Row, int Family) const
	{
		return FName(f"HeroTalentCell_{Row}_{Family}");
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
		RefreshAILevel();
		RefreshVolcano();
		RefreshDiplomacy();
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
		HOLMinimap::Age(MinimapAlerts, MinimapLastTime < 0.0 ? 0.0 : Math::Max(0.0, Now - MinimapLastTime), Minimap.GetAlertSeconds());
		MinimapLastTime = Now;
		HOLMinimap::Detect(MinimapSeenBefore, Seen, MinimapAlerts);
		MinimapSeenBefore = Seen;
		TArray<FVector2D> Lit;
		TArray<double> LitAges;
		for (int Index = 0; Index < MinimapAlerts.Num(); Index++)
		{
			if (HOLMinimap::IsLit(MinimapAlerts[Index], Minimap.GetAlertBlink()))
			{
				Lit.Add(HOLMinimap::ToPicture(MinimapFrame, FVector2D(MinimapAlerts[Index].X, MinimapAlerts[Index].Y)));
				LitAges.Add(MinimapAlerts[Index].Age);
			}
		}
		MinimapLitAlerts = Lit.Num();
		Minimap.SetLayers(Marks, Lit, LitAges);
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
	// nothing); a button opening the tree while a point is kept (D105, TQ4 = A), the tree window and the icons of the talents taken (TQ5 = A).
	void ShowTalents(int PlayerSlot, FHOLHeroBlockView View)
	{
		TalentView = FHOLTalentPanelView();
		TalentTexts = "";
		TalentListText = "";
		TalentWindowTexts = "";
		TalentIconTexts = "";
		const int Count = PlayerSlot >= 0 ? HOLHero::GetHeroTalentCount(PlayerSlot) : -1;
		if (!View.bVisible || Count < 0)
		{
			HeroTalentButton.SetVisibility(ESlateVisibility::Collapsed);
			HeroTalentListLabel.SetVisibility(ESlateVisibility::Collapsed);
			HeroTalentNoticeLabel.SetVisibility(ESlateVisibility::Collapsed);
			HeroTalentIcons.SetVisibility(ESlateVisibility::Collapsed);
			bTalentWindowOpen = false;
			TalentWindow.SetVisibility(ESlateVisibility::Collapsed);
			return;
		}
		TArray<int> Picks;
		for (int Index = 0; Index < Count; ++Index)
			Picks.Add(HOLHero::GetHeroTalentFamily(PlayerSlot, Index));
		ShowTalentFeedback(PlayerSlot, Count, Picks);
		TalentView = HOLTalentPanel::Make(View.Level, Picks);
		TalentCells = HOLTalentTree::MakeWithLater(HOLHero::GetMaxLevel(), View.Level, Picks);
		ShowTalentIcons(View.Level, Picks);
		ShowTalentWindow();
		HeroTalentButton.SetVisibility(TalentView.bReminder ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		if (!TalentView.bReminder)
			return;
		TalentTexts = HOLTalentText::ReminderText(TalentView);
		HeroTalentLabel.SetText(FText::FromString(TalentTexts));
		HeroTalentLabel.SetColorAndOpacity(UHOLScriptScreen::GetTextColor());
		UHOLScriptScreen::SetButtonGreyed(HeroTalentButton, false);
		HeroTalentButton.SetToolTipText(HOLTalentText::OpenTooltip());
	}

	// TQ5 = A (D618): one icon per talent taken, in the order of the rows; the glyph over the family's colour, the hover the name and effect.
	void ShowTalentIcons(int Level, const TArray<int>& Picks)
	{
		const TArray<FHOLTalentCellView> Taken = HOLTalentTree::TakenEntries(Level, Picks);
		HeroTalentIcons.SetVisibility(Taken.Num() > 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		for (int Index = 0; Index < TalentIconButtons.Num(); ++Index)
		{
			const bool bShown = Index < Taken.Num();
			TalentIconButtons[Index].SetVisibility(bShown ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
			if (!bShown)
				continue;
			TalentIconLabels[Index].SetText(FText::FromString(Taken[Index].Glyph));
			TalentIconLabels[Index].SetColorAndOpacity(TalentFamilyColour(Taken[Index].Family));
			TalentIconButtons[Index].SetBackgroundColor(TalentFamilyColour(Taken[Index].Family));
			TalentIconButtons[Index].SetToolTipText(FText::FromString(Taken[Index].Tooltip));
			TalentIconTexts += (Index > 0 ? " || " : "") + Taken[Index].Glyph + " = " + Taken[Index].Tooltip;
		}
	}

	// A colour for each family column (Guardian, Captain, Champion), over the buttons' dark: icons and taken cells.
	FLinearColor TalentFamilyColour(int Family) const
	{
		if (Family == 0)
			return FLinearColor(0.35, 0.6, 1.0, 1.0);
		if (Family == 1)
			return FLinearColor(1.0, 0.8, 0.3, 1.0);
		return FLinearColor(1.0, 0.45, 0.4, 1.0);
	}

	// The tree window as the cells say (HOLTalentTree::Make): shown while open (the hero shown), every cell its name, its colour by state, its hover.
	void ShowTalentWindow()
	{
		TalentWindow.SetVisibility(bTalentWindowOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		if (!bTalentWindowOpen)
			return;
		TalentWindowTitle.SetText(HOLTalentText::WindowTitle());
		TalentWindowPoints.SetText(FText::FromString(HOLTalentText::ReminderText(TalentView)));
		TalentWindowPoints.SetColorAndOpacity(UHOLScriptScreen::GetMutedColor());
		TalentWindowCloseLabel.SetText(HOLTalentText::CloseLabel());
		TalentWindowCloseLabel.SetColorAndOpacity(UHOLScriptScreen::GetTextColor());
		for (int Family = 0; Family < TalentWindowFamilyHeads.Num(); ++Family)
		{
			TalentWindowFamilyHeads[Family].SetText(FText::FromString(HOLTalentText::FamilyName(Family)));
			TalentWindowFamilyHeads[Family].SetColorAndOpacity(TalentFamilyColour(Family));
		}
		for (int Index = 0; Index < TalentWindowRowLabels.Num(); ++Index)
		{
			TalentWindowRowLabels[Index].SetText(FText::FromString(HOLTalentText::RowLabel(HOLTalent::FirstRow + Index)));
			TalentWindowRowLabels[Index].SetColorAndOpacity(UHOLScriptScreen::GetMutedColor());
		}
		for (int Index = 0; Index < TalentCellButtons.Num() && Index < TalentCells.Num(); ++Index)
		{
			const FHOLTalentCellView Cell = TalentCells[Index];
			const bool bTaken = Cell.State == EHOLTalentCellState::Taken;
			TalentCellLabels[Index].SetText(FText::FromString(Cell.Name));
			TalentCellLabels[Index].SetColorAndOpacity(bTaken || Cell.bPickable ? UHOLScriptScreen::GetTextColor() : UHOLScriptScreen::GetMutedColor());
			if (Cell.State == EHOLTalentCellState::Locked)
			{
				// "Plus tard": fainter than the rows still to come in this game (their hover says the level).
				FLinearColor Faint = UHOLScriptScreen::GetMutedColor();
				Faint.A = 0.45;
				TalentCellLabels[Index].SetColorAndOpacity(Faint);
			}
			TalentCellButtons[Index].SetBackgroundColor(bTaken ? TalentFamilyColour(Cell.Family) : FLinearColor(1.0, 1.0, 1.0, 1.0));
			UHOLScriptScreen::SetButtonGreyed(TalentCellButtons[Index], !bTaken && !Cell.bPickable);
			TalentCellButtons[Index].SetToolTipText(FText::FromString(Cell.Tooltip));
		}
		TalentWindowAdvisedLabel.SetText(HOLTalentText::AdvisedLabel());
		TalentWindowAdvisedLabel.SetColorAndOpacity(TalentView.bAdviceEnabled ? UHOLScriptScreen::GetTextColor() : UHOLScriptScreen::GetMutedColor());
		UHOLScriptScreen::SetButtonGreyed(TalentWindowAdvised, !TalentView.bAdviceEnabled);
		TalentWindowAdvised.SetToolTipText(FText::FromString(HOLTalentText::TooltipText(TalentView)));
		TalentWindowTexts = HOLTalentText::ReminderText(TalentView) + " | " + HOLTalentTree::Describe(TalentCells);
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
		// A low window: the cheats' panel folds when this block opens (Q-heros-triches-600 = A).
		SetCheatsHeroBlockShown(View.bVisible);
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

	// A click on the talent line, an icon of a talent taken, or the T key (TQ4 = A): the tree opens, or closes if it was open. Nothing without a hero.
	bool ClickTalent()
	{
		if (!HeroView.bVisible || TalentSeenCount < 0)
			return false;
		bTalentWindowOpen = !bTalentWindowOpen;
		ShowTalentWindow();
		return true;
	}

	UFUNCTION()
	void HandleTalentClicked()
	{
		ClickTalent();
	}

	bool ClickTalentClose()
	{
		if (!bTalentWindowOpen)
			return false;
		bTalentWindowOpen = false;
		ShowTalentWindow();
		return true;
	}

	UFUNCTION()
	void HandleTalentWindowClose()
	{
		ClickTalentClose();
	}

	// A click on the cell (Row, Family): only a pickable cell asks, for its own family (HOLTalentTree::ClickedFamily); the server checks it.
	bool ClickTalentCell(int Row, int Family)
	{
		if (!bTalentWindowOpen)
			return false;
		const int Asked = HOLTalentTree::ClickedFamily(TalentCells, Row, Family);
		if (Asked == HOLTalent::FamilyNone)
			return false;
		AHOLPlayerController Controller = Cast<AHOLPlayerController>(GetOwningPlayer());
		if (Controller == nullptr)
			return false;
		Controller.RequestHeroTalent(Asked);
		return true;
	}

	UFUNCTION()
	void HandleCell0()
	{
		ClickTalentCell(HOLTalent::FirstRow + 0, 0);
	}

	UFUNCTION()
	void HandleCell1()
	{
		ClickTalentCell(HOLTalent::FirstRow + 0, 1);
	}

	UFUNCTION()
	void HandleCell2()
	{
		ClickTalentCell(HOLTalent::FirstRow + 0, 2);
	}

	UFUNCTION()
	void HandleCell3()
	{
		ClickTalentCell(HOLTalent::FirstRow + 1, 0);
	}

	UFUNCTION()
	void HandleCell4()
	{
		ClickTalentCell(HOLTalent::FirstRow + 1, 1);
	}

	UFUNCTION()
	void HandleCell5()
	{
		ClickTalentCell(HOLTalent::FirstRow + 1, 2);
	}

	UFUNCTION()
	void HandleCell6()
	{
		ClickTalentCell(HOLTalent::FirstRow + 2, 0);
	}

	UFUNCTION()
	void HandleCell7()
	{
		ClickTalentCell(HOLTalent::FirstRow + 2, 1);
	}

	UFUNCTION()
	void HandleCell8()
	{
		ClickTalentCell(HOLTalent::FirstRow + 2, 2);
	}

	UFUNCTION()
	void HandleCell9()
	{
		ClickTalentCell(HOLTalent::FirstRow + 3, 0);
	}

	UFUNCTION()
	void HandleCell10()
	{
		ClickTalentCell(HOLTalent::FirstRow + 3, 1);
	}

	UFUNCTION()
	void HandleCell11()
	{
		ClickTalentCell(HOLTalent::FirstRow + 3, 2);
	}

	UFUNCTION()
	void HandleCell12()
	{
		ClickTalentCell(HOLTalent::FirstRow + 4, 0);
	}

	UFUNCTION()
	void HandleCell13()
	{
		ClickTalentCell(HOLTalent::FirstRow + 4, 1);
	}

	UFUNCTION()
	void HandleCell14()
	{
		ClickTalentCell(HOLTalent::FirstRow + 4, 2);
	}

	UFUNCTION()
	void HandleCell15()
	{
		ClickTalentCell(HOLTalent::FirstRow + 5, 0);
	}

	UFUNCTION()
	void HandleCell16()
	{
		ClickTalentCell(HOLTalent::FirstRow + 5, 1);
	}

	UFUNCTION()
	void HandleCell17()
	{
		ClickTalentCell(HOLTalent::FirstRow + 5, 2);
	}

	UFUNCTION()
	void HandleCell18()
	{
		ClickTalentCell(HOLTalent::FirstRow + 6, 0);
	}

	UFUNCTION()
	void HandleCell19()
	{
		ClickTalentCell(HOLTalent::FirstRow + 6, 1);
	}

	UFUNCTION()
	void HandleCell20()
	{
		ClickTalentCell(HOLTalent::FirstRow + 6, 2);
	}

	UFUNCTION()
	void HandleCell21()
	{
		ClickTalentCell(HOLTalent::FirstRow + 7, 0);
	}

	UFUNCTION()
	void HandleCell22()
	{
		ClickTalentCell(HOLTalent::FirstRow + 7, 1);
	}

	UFUNCTION()
	void HandleCell23()
	{
		ClickTalentCell(HOLTalent::FirstRow + 7, 2);
	}

	// The advised choice (D105), now at the foot of the window: the family most filled, the order sent. Greyed or closed: nothing.
	bool ClickTalentAdvised()
	{
		if (!bTalentWindowOpen || !TalentView.bAdviceEnabled)
			return false;
		AHOLPlayerController Controller = Cast<AHOLPlayerController>(GetOwningPlayer());
		if (Controller == nullptr)
			return false;
		Controller.RequestHeroTalent(TalentView.AdvisedFamily);
		return true;
	}

	UFUNCTION()
	void HandleTalentAdvisedClicked()
	{
		ClickTalentAdvised();
	}

	// The T key (read in UHOLScriptScreen::NativeTick).
	UFUNCTION(BlueprintOverride)
	void KeyPressed(FName KeyName)
	{
		if (KeyName == n"T")
			ClickTalent();
		if (KeyName == n"CtrlD")
			ToggleDiplomacy();
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
		if (WidgetName.ToString().StartsWith("Dip"))
			return ClickDiplomacyName(WidgetName);
		if (WidgetName == n"HeroTalentButton" || WidgetName == n"TalentKey")
			return ClickTalent();
		if (WidgetName == n"TalentWindowClose")
			return ClickTalentClose();
		if (WidgetName == n"TalentWindowAdvised")
			return ClickTalentAdvised();
		for (int Index = 0; Index < TalentIconButtons.Num(); ++Index)
		{
			if (WidgetName == FName(f"HeroTalentIcon{Index}"))
				return TalentIconButtons[Index].IsVisible() && ClickTalent();
		}
		for (int Row = HOLTalent::FirstRow; Row < HOLTalent::FirstRow + TalentWindowRowLabels.Num(); ++Row)
		{
			for (int Family = 0; Family < HOLTalent::FamilyCount; ++Family)
			{
				if (WidgetName == TalentCellName(Row, Family))
					return ClickTalentCell(Row, Family);
			}
		}
		if (WidgetName == n"DuelAcceptButton")
			return ClickDuelAnswer(true);
		if (WidgetName == n"DuelRefuseButton")
			return ClickDuelAnswer(false);
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
			+ (HeroView.bVisible && TalentView.Rows.Num() > 0 ? "\nhero talents: " + TalentView.Describe + (TalentTexts.IsEmpty() ? "" : " | " + TalentTexts) + f" | next row {TalentView.NextRow}" : "")
			+ (HeroView.bVisible && TalentSeenCount >= 0 ? "\nhero talent texts: list " + TalentListText + " | notice " + TalentNoticeText + " | icons " + TalentIconTexts : "")
			+ (HeroView.bVisible && TalentSeenCount >= 0 ? "\ntalent window: " + (bTalentWindowOpen ? "open | " + TalentWindowTexts : "closed") : "")
			+ (DuelView.Kind != EHOLDuelWindowKind::None ? "\nduel window: " + DuelTexts : "")
			+ (AILevelTexts.IsEmpty() ? "" : "\nai level: " + AILevelTexts)
			+ (VolcanoTexts.IsEmpty() ? "" : "\nvolcano: " + VolcanoTexts)
			+ "\ndiplomacy: " + (bDipOpen ? f"open layout {DipLayout}" + (DipMenuSlot >= 0 ? f" menu {DipMenuSlot}" : "") + (DipMessageSlot >= 0 ? f" message to {DipMessageSlot}" : "") + " | " + DipTexts : "closed")
			+ "\npause: " + PauseView.Describe + (PauseTexts.IsEmpty() ? "" : "\npause texts: " + PauseTexts);
	}
}
