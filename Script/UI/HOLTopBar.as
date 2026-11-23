// NOT COMPILED YET (ELROND-1119a, H-3 of the HUD plan): this file calls bindings that do not exist before the next C++ lot (UHOLScriptScreen::GetEconomyNow,
// GetIdleWorkerCount, GetStatsButtonShown, SelectNextIdleWorker, OpenGameMenu, OpenStats, GetResourceColor; UHOLQueueItem::SetLabel as a UFUNCTION). The recipe
// is in Docs\Rapports\ELROND-1119a.md; until then the whole script module would fail to compile: do NOT build with this file present and without the lot.
//
// The top bar of the game HUD (resources, population, idle workers' counter) and the Menu / Statistiques buttons, in script. Top left: for each resource its
// marker, name, amount and "(gatherers)", then the population "used / cap", then the "Inactifs : N" button (a click selects the next idle worker, as the "."
// key); top right: "Statistiques" (a spectator's during the match, everyone's after it) left of "Menu". The rules are pure (namespace HOLTopBar: the numbers'
// formats, the texts, the idle counter, the tooltips, the journal and report lines); UHOLTopBarPanel builds the same widgets as the C++ screen (same names, same
// places, same styles) and refreshes them from the economy the screen reads (GetEconomyNow). The widget names are the C++ screen's: TopBar, Resources,
// <Food|Wood|Stone|Gold>Marker/Label/Amount/Gatherers, PopulationLabel, PopulationAmount, IdleWorkersBox, IdleWorkers, MenuPanel, MenuRow, StatsButtonBox,
// StatsButton, MenuButtonBox, MenuButton (hol.HUD.Click, hol.Mouse.Hover, hol.UI.Focus find them by name).

namespace HOLTopBarText
{
	// Same namespace and keys as the C++ LOCTEXT of UHOLGameScreen ("HOLGameScreen,<key>"): the translations stay.
	FString Replace1(FText Pattern, FString Value)
	{
		return Pattern.ToString().Replace("{0}", Value);
	}

	FText Food() { return NSLOCTEXT("HOLGameScreen", "Food", "Nourriture"); }
	FText Wood() { return NSLOCTEXT("HOLGameScreen", "Wood", "Bois"); }
	FText Stone() { return NSLOCTEXT("HOLGameScreen", "Stone", "Pierre"); }
	FText Gold() { return NSLOCTEXT("HOLGameScreen", "Gold", "Or"); }
	FText Population() { return NSLOCTEXT("HOLGameScreen", "Population", "Population"); }
	FText Gatherers() { return NSLOCTEXT("HOLGameScreen", "GathererCount", "({0})"); }
	FText IdleWorkers() { return NSLOCTEXT("HOLGameScreen", "IdleWorkers", "Inactifs : {0}"); }
	FText Menu() { return NSLOCTEXT("HOLGameScreen", "Menu", "Menu"); }
	FText Stats() { return NSLOCTEXT("HOLGameScreen", "Stats", "Statistiques"); }
	FText IncomeTip() { return NSLOCTEXT("HOLGameScreen", "IncomeTip", "+{0} {1}/min"); }
	// The resources' words of the tribute's message (same keys as HOLForumMessages).
	FText TributeFood() { return NSLOCTEXT("HOLGameScreen", "TributeFood", "nourriture"); }
	FText TributeWood() { return NSLOCTEXT("HOLGameScreen", "TributeWood", "bois"); }
	FText TributeStone() { return NSLOCTEXT("HOLGameScreen", "TributeStone", "pierre"); }
	FText TributeGold() { return NSLOCTEXT("HOLGameScreen", "TributeGold", "or"); }
}

namespace HOLTopBar
{
	// REVUE-1122 minor 11: the game screen calls SetBannerRect on every Refresh. The tooltip drops only change when the banner rectangle changes, so the panel applies
	// the rectangle only when it differs from the last one it fully applied. STUB (red step): always applies.
	bool BannerRectNeedsApply(bool bHasApplied, float LastLeft, float LastRight, float LastBottom, float Left, float Right, float Bottom)
	{
		return true;
	}

	// The four resources, in the bar's order (the C++ screen's AddResource order).
	int Food() { return 0; }
	int Wood() { return 1; }
	int Stone() { return 2; }
	int Gold() { return 3; }

	// Functions, not constants: a constant of a namespace is not visible from another file (the test).
	// The C++ constants of the screen (HOLGameScreen.cpp:250-254).
	float ScreenMargin() { return 16.0; }
	float QueueItemHeight() { return 36.0; }
	float MarkerSize() { return 12.0; }
	float IdleWorkersWidth() { return 130.0; }

	// Amounts without thousands separators, as in AoE ("1200"): FormatAmount of the C++ screen (FText::AsNumber, DefaultNoGrouping).
	FString FormatAmount(int Value)
	{
		return f"{Value}";
	}

	// "12 / 50" (FormatRatio: INVTEXT("{0} / {1}")).
	FString FormatRatio(int Value, int Max)
	{
		return FormatAmount(Value) + " / " + FormatAmount(Max);
	}

	// A resource's gatherers, "(3)" (LOCTEXT GathererCount "({0})").
	FString GatherersText(int Count)
	{
		return HOLTopBarText::Replace1(HOLTopBarText::Gatherers(), FormatAmount(Count));
	}

	// The tooltip over an amount and over its gatherers: "+90 bois/min" (the income a minute, every source of it).
	FString IncomeTip(int PerMinute, FString ResourceWord)
	{
		return HOLTopBarText::IncomeTip().ToString().Replace("{0}", FormatAmount(PerMinute)).Replace("{1}", ResourceWord);
	}

	// D814 (Q-income-tooltip-taille = B): the income tooltip as a HUD panel (dark fill, gold border), not the engine's plain tooltip.
	struct FIncomeTipView
	{
		FString Text;
		FLinearColor Fill;
		FLinearColor Border;
		float BorderWidth = 0.0;
		float Padding = 0.0;
	}

	FIncomeTipView IncomeTipView(int PerMinute, int Resource)
	{
		FIncomeTipView View;
		View.Text = IncomeTip(PerMinute, ResourceWord(Resource));
		View.Fill = FLinearColor(0.03, 0.035, 0.045, 0.96);
		View.Border = FLinearColor(0.85, 0.6, 0.08, 1.0);
		View.BorderWidth = 1.0;
		View.Padding = 6.0;
		return View;
	}

	// RECETTE-1121sd: in French the gold's income tooltip covered the "Scénario" banner (centred under the bar). The tooltip's window is placed by the engine at
	// the cursor, so only a transparent drop above the visible panel moves it: the drop that puts the tooltip's top under the banner's bottom when the two share
	// some width (no banner: 0, the position is unchanged).
	float IncomeTipGap() { return 6.0; }

	// Where the engine puts a tooltip raised from the bar's amounts (under the cursor, in the bar): its top.
	float IncomeTipNaturalTop() { return ScreenMargin() + QueueItemHeight() + 4.0; }

	// BannerBottom <= 0: no banner shown, no drop. A tooltip wholly left or right of the banner is not moved either.
	float IncomeTipDrop(float TipLeft, float TipTop, float TipWidth, float BannerLeft, float BannerRight, float BannerBottom)
	{
		if (BannerBottom <= 0.0)
			return 0.0;
		if (TipLeft + TipWidth <= BannerLeft || TipLeft >= BannerRight)
			return 0.0;
		return Math::Max(0.0, BannerBottom + IncomeTipGap() - TipTop);
	}

	// GALADRIEL-1122d: the drop of the tooltip raised from a cell whose cursor point is CellCentre (the tooltip's left edge, the engine puts it at the cursor)
	// and whose width is TipWidth: its OWN rectangle against the banner, so only a tooltip crossing the banner horizontally goes under it.
	float IncomeTipDropForCell(float CellCentre, float TipWidth, float BannerLeft, float BannerRight, float BannerBottom)
	{
		return IncomeTipDrop(CellCentre, IncomeTipNaturalTop(), TipWidth, BannerLeft, BannerRight, BannerBottom);
	}

	// The tooltip's width in the HUD font (a rule without a renderer: an average glyph width plus the two paddings).
	float IncomeTipWidth(FString Text, float Padding)
	{
		return float(Text.Len()) * 8.0 + Padding * 2.0;
	}

	FString ResourceWord(int Resource)
	{
		if (Resource == Food())
			return HOLTopBarText::TributeFood().ToString();
		if (Resource == Wood())
			return HOLTopBarText::TributeWood().ToString();
		if (Resource == Stone())
			return HOLTopBarText::TributeStone().ToString();
		return HOLTopBarText::TributeGold().ToString();
	}

	FText ResourceLabel(int Resource)
	{
		if (Resource == Food())
			return HOLTopBarText::Food();
		if (Resource == Wood())
			return HOLTopBarText::Wood();
		if (Resource == Stone())
			return HOLTopBarText::Stone();
		return HOLTopBarText::Gold();
	}

	// The widget names of a resource's four widgets: FoodMarker, FoodLabel, FoodAmount, FoodGatherers (the C++ screen's Name + "Marker" ...).
	FString ResourceName(int Resource)
	{
		if (Resource == Food())
			return "Food";
		if (Resource == Wood())
			return "Wood";
		if (Resource == Stone())
			return "Stone";
		return "Gold";
	}

	// The amount of a resource in a stock (FHOLResources).
	int AmountOf(FHOLResources Stock, int Resource)
	{
		if (Resource == Food())
			return Stock.Food;
		if (Resource == Wood())
			return Stock.Wood;
		if (Resource == Stone())
			return Stock.Stone;
		return Stock.Gold;
	}

	// "F 0 W 0 S 0 G 0": FHOLResources::ToString, as the report line and the tests of quick_check read it.
	FString ResourcesLine(FHOLResources Stock)
	{
		return f"F {Stock.Food} W {Stock.Wood} S {Stock.Stone} G {Stock.Gold}";
	}

	// The bar is on the screen as soon as the player state has reached this machine, hidden before.
	bool BarShown(bool bHasPlayerState)
	{
		return bHasPlayerState;
	}

	// The report's economy line (hol.HUD.Report, first words of the HUD line): "F 120 W 85 S 0 G 30, population 12/50" or "none" before the player state.
	FString EconomyLine(bool bHasPlayerState, FHOLResources Stock, int PopulationUsed, int PopulationCap)
	{
		if (!bHasPlayerState)
			return "none";
		return ResourcesLine(Stock) + f", population {PopulationUsed}/{PopulationCap}";
	}

	// The report's last part: ", gatherers F 0 W 1 S 0 G 0, idle 2" (none before the player state; the idle count never under 0).
	FString WorkersLine(bool bHasPlayerState, FHOLResources Gatherers, int ShownIdleWorkers)
	{
		if (!bHasPlayerState)
			return "";
		return ", gatherers " + ResourcesLine(Gatherers) + f", idle {Math::Max(ShownIdleWorkers, 0)}";
	}

	// The idle workers' button: the label "Inactifs : N", enabled only with at least one idle worker (a click on a disabled one is refused).
	FString IdleLabel(int Count)
	{
		return HOLTopBarText::Replace1(HOLTopBarText::IdleWorkers(), f"{Count}");
	}

	// The label shown differs from the one the current language gives for this count (a live language switch with the count unchanged).
	bool IdleLabelChanged(FString ShownLabel, int Count)
	{
		return ShownLabel != IdleLabel(Count);
	}

	bool IdleEnabled(int Count)
	{
		return Count > 0;
	}

	// The counter is applied only when it changes (-1: never shown yet).
	bool IdleChanged(int ShownCount, int Count)
	{
		return Count != ShownCount;
	}

	// The journal line of a click on the idle workers' counter (the C++ HandleIdleWorkersClicked; the controller then logs "Next idle worker: ...").
	FString IdleClickLine()
	{
		return "HUD: idle workers clicked";
	}

	// The tooltip text in the current language ("+0 bois/min", "+0 wood/min"...): it changes with a live language switch, so the tooltips are re-set then too.
	FString TipLanguageKey()
	{
		return IncomeTip(0, ResourceWord(0));
	}

	// The income is set again on the tooltips only when it changes.
	bool IncomeChanged(FHOLResources Shown, FHOLResources Income)
	{
		return Shown.Food != Income.Food || Shown.Wood != Income.Wood || Shown.Stone != Income.Stone || Shown.Gold != Income.Gold;
	}
}

// The top bar and the Menu / Statistiques panel. Built once in the script screen's OnInitialized (Build), refreshed in its Refresh (Refresh).
class UHOLTopBarPanel : UObject
{
	UHOLScriptScreen Screen;

	UBorder TopBar;
	UCommonTextBlock FoodAmount;
	UCommonTextBlock WoodAmount;
	UCommonTextBlock StoneAmount;
	UCommonTextBlock GoldAmount;
	UCommonTextBlock FoodGatherers;
	UCommonTextBlock WoodGatherers;
	UCommonTextBlock StoneGatherers;
	UCommonTextBlock GoldGatherers;
	UCommonTextBlock PopulationAmount;
	// D814 (B): the income tooltips, 8 HUD panels (the amount's and the gatherers' of each resource: a Slate widget has one parent), built at the first SetIncomeTips.
	TArray<UBorder> IncomeTipPanels;
	TArray<UBorder> IncomeTipHosts;
	TArray<UCommonTextBlock> IncomeTipTexts;
	UHOLQueueItem IdleWorkers;
	UBorder MenuPanel;
	UHOLQueueItem MenuButton;
	UHOLQueueItem StatsButton;
	USizeBox StatsBox;

	// What is shown, for the report and the tests (a script test cannot read a widget): the player state reached us, the economy as drawn, the idle count
	// (-1: never shown), the income the tooltips carry.
	bool bEconomyShown = false;
	FHOLResources ShownStock;
	FHOLResources ShownGatherers;
	FHOLResources ShownIncome;
	int ShownPopulationUsed = 0;
	int ShownPopulationCap = 0;
	int ShownIdleWorkers = -1;
	FString ShownIdleLabel;
	bool bIncomeKnown = false;
	FString ShownTipKey;
	bool bStatsShown = false;

	UCommonTextBlock AmountOf(int Resource)
	{
		if (Resource == HOLTopBar::Food())
			return FoodAmount;
		if (Resource == HOLTopBar::Wood())
			return WoodAmount;
		if (Resource == HOLTopBar::Stone())
			return StoneAmount;
		return GoldAmount;
	}

	UCommonTextBlock GatherersOf(int Resource)
	{
		if (Resource == HOLTopBar::Food())
			return FoodGatherers;
		if (Resource == HOLTopBar::Wood())
			return WoodGatherers;
		if (Resource == HOLTopBar::Stone())
			return StoneGatherers;
		return GoldGatherers;
	}

	// AddToRow of the C++ screen: centred vertically, a right padding.
	void AddToRow(UHorizontalBox Row, UWidget Widget, float RightPadding)
	{
		UHorizontalBoxSlot Slot = Row.AddChildToHorizontalBox(Widget);
		Slot.SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
		Slot.SetPadding(FMargin(0.0, 0.0, RightPadding, 0.0));
	}

	// MakeSquare of the C++ screen: a flat coloured image of Size x Size.
	UImage MakeSquare(FName Name, FLinearColor Color, float Size)
	{
		UImage Square = Cast<UImage>(Screen.ConstructWidget(UImage, Name));
		Square.SetColorAndOpacity(Color);
		Square.SetDesiredSizeOverride(FVector2D(Size, Size));
		return Square;
	}

	// AddResource of the C++ screen: marker (not for the population), name, amount, gatherers (resources only; the amount and the gatherers are Visible so that
	// their hover shows the income tooltip, D788). Returns the amount.
	UCommonTextBlock AddResource(UHorizontalBox Resources, FString Name, int Resource, FText Label, bool bHasGatherers)
	{
		if (bHasGatherers)
			AddToRow(Resources, MakeSquare(FName(Name + "Marker"), UHOLScriptScreen::GetResourceColor(Resource), HOLTopBar::MarkerSize()), 6.0);
		UCommonTextBlock LabelText = Screen.MakeText(FName(Name + "Label"), UHOLTextStyleHUDLabel);
		LabelText.SetText(Label);
		AddToRow(Resources, LabelText, 6.0);
		UCommonTextBlock Value = Screen.MakeText(FName(Name + "Amount"), UHOLTextStyleHUD);
		AddToRow(Resources, Value, bHasGatherers ? 4.0 : 20.0);
		if (bHasGatherers)
		{
			Value.SetVisibility(ESlateVisibility::Visible);
			UCommonTextBlock Count = Screen.MakeText(FName(Name + "Gatherers"), UHOLTextStyleHUDLabel);
			Count.SetVisibility(ESlateVisibility::Visible);
			AddToRow(Resources, Count, 20.0);
			if (Resource == HOLTopBar::Food())
				FoodGatherers = Count;
			else if (Resource == HOLTopBar::Wood())
				WoodGatherers = Count;
			else if (Resource == HOLTopBar::Stone())
				StoneGatherers = Count;
			else
				GoldGatherers = Count;
		}
		return Value;
	}

	UHOLQueueItem MakeQueueButton(FName Name, FText Label)
	{
		UHOLQueueItem Button = Cast<UHOLQueueItem>(Screen.ConstructWidget(UHOLQueueItem, Name));
		Button.SetLabel(Label);
		Button.OnButtonBaseClicked.AddUFunction(this, FName(Name.ToString() + "Clicked"));
		return Button;
	}

	void Build(UHOLScriptScreen InScreen, UOverlay Frame)
	{
		Screen = InScreen;
		const float Margin = HOLTopBar::ScreenMargin();
		const float ButtonHeight = HOLTopBar::QueueItemHeight() * 0.8;

		// Top bar, top left.
		TopBar = Screen.MakePanel(n"TopBar");
		UOverlaySlot TopBarSlot = Frame.AddChildToOverlay(TopBar);
		TopBarSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);
		TopBarSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Top);
		TopBarSlot.SetPadding(FMargin(Margin));
		UHorizontalBox Resources = Cast<UHorizontalBox>(Screen.ConstructWidget(UHorizontalBox, n"Resources"));
		TopBar.SetContent(Resources);
		FoodAmount = AddResource(Resources, "Food", HOLTopBar::Food(), HOLTopBarText::Food(), true);
		WoodAmount = AddResource(Resources, "Wood", HOLTopBar::Wood(), HOLTopBarText::Wood(), true);
		StoneAmount = AddResource(Resources, "Stone", HOLTopBar::Stone(), HOLTopBarText::Stone(), true);
		GoldAmount = AddResource(Resources, "Gold", HOLTopBar::Gold(), HOLTopBarText::Gold(), true);
		PopulationAmount = AddResource(Resources, "Population", -1, HOLTopBarText::Population(), false);

		// The idle workers' counter, a button (J5b, D298): a click selects the next one, as ".".
		USizeBox IdleBox = Cast<USizeBox>(Screen.ConstructWidget(USizeBox, n"IdleWorkersBox"));
		IdleBox.SetWidthOverride(HOLTopBar::IdleWorkersWidth());
		IdleBox.SetHeightOverride(ButtonHeight);
		IdleWorkers = MakeQueueButton(n"IdleWorkers", FText::FromString(""));
		IdleBox.SetContent(IdleWorkers);
		AddToRow(Resources, IdleBox, 0.0);

		// The in-game menu's button (D356), top right, « Statistiques » left of it (collapsed unless a spectator's or after the match).
		MenuPanel = Screen.MakePanel(n"MenuPanel");
		MenuPanel.SetPadding(FMargin(6.0, 4.0));
		UOverlaySlot MenuPanelSlot = Frame.AddChildToOverlay(MenuPanel);
		MenuPanelSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Right);
		MenuPanelSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Top);
		MenuPanelSlot.SetPadding(FMargin(Margin));
		USizeBox MenuBox = Cast<USizeBox>(Screen.ConstructWidget(USizeBox, n"MenuButtonBox"));
		MenuBox.SetWidthOverride(HOLTopBar::IdleWorkersWidth() * 0.75);
		MenuBox.SetHeightOverride(ButtonHeight);
		MenuButton = MakeQueueButton(n"MenuButton", HOLTopBarText::Menu());
		MenuBox.SetContent(MenuButton);
		UHorizontalBox MenuRow = Cast<UHorizontalBox>(Screen.ConstructWidget(UHorizontalBox, n"MenuRow"));
		StatsBox = Cast<USizeBox>(Screen.ConstructWidget(USizeBox, n"StatsButtonBox"));
		StatsBox.SetWidthOverride(HOLTopBar::IdleWorkersWidth() * 1.25);
		StatsBox.SetHeightOverride(ButtonHeight);
		StatsButton = MakeQueueButton(n"StatsButton", HOLTopBarText::Stats());
		StatsButton.SetVisibility(ESlateVisibility::Collapsed);
		StatsBox.SetContent(StatsButton);
		UHorizontalBoxSlot StatsSlot = MenuRow.AddChildToHorizontalBox(StatsBox);
		StatsSlot.SetPadding(FMargin(0.0, 0.0, 6.0, 0.0));
		MenuRow.AddChildToHorizontalBox(MenuBox);
		MenuPanel.SetContent(MenuRow);
	}

	UFUNCTION()
	void IdleWorkersClicked(UCommonButtonBase Button)
	{
		ClickIdleWorkers();
	}

	UFUNCTION()
	void MenuButtonClicked(UCommonButtonBase Button)
	{
		Screen.OpenGameMenu();
	}

	UFUNCTION()
	void StatsButtonClicked(UCommonButtonBase Button)
	{
		ClickStats();
	}

	// The clicks, mouse or hol.HUD.Click (ClickWidget): false when the C++ screen refused it (a disabled idle counter, a collapsed « Statistiques »).
	bool ClickIdleWorkers()
	{
		if (IdleWorkers == nullptr || !IdleWorkers.GetIsEnabled())
			return false;
		UHOLGameMenuScreen::MenuLog(HOLTopBar::IdleClickLine());
		Screen.SelectNextIdleWorker();
		return true;
	}

	bool ClickMenu()
	{
		if (MenuButton == nullptr)
			return false;
		Screen.OpenGameMenu();
		return true;
	}

	bool ClickStats()
	{
		if (StatsButton == nullptr || StatsButton.GetVisibility() == ESlateVisibility::Collapsed)
			return false;
		Screen.OpenStats();
		return true;
	}

	// hol.HUD.Click's names: the three buttons.
	bool ClickName(FName WidgetName)
	{
		if (WidgetName == n"MenuButton")
			return ClickMenu();
		if (WidgetName == n"StatsButton")
			return ClickStats();
		if (WidgetName == n"IdleWorkers")
			return ClickIdleWorkers();
		return false;
	}

	// The report's two pieces (hol.HUD.Report's HUD line): "F 1 W 2 S 3 G 4, population 5/10" and ", gatherers F 0 W 0 S 0 G 0, idle 1".
	FString DescribeEconomy() const
	{
		return HOLTopBar::EconomyLine(bEconomyShown, ShownStock, ShownPopulationUsed, ShownPopulationCap);
	}

	FString DescribeWorkers() const
	{
		return HOLTopBar::WorkersLine(bEconomyShown, ShownGatherers, ShownIdleWorkers);
	}

	void EnsureIncomeTipPanels()
	{
		if (IncomeTipPanels.Num() > 0)
			return;
		for (int Index = 0; Index < 8; Index++)
		{
			UBorder Panel = Screen.MakePanel(FName(f"IncomeTipPanel{Index}"));
			UCommonTextBlock Text = Screen.MakeText(FName(f"IncomeTipText{Index}"), UHOLTextStyleHUD);
			Text.SetVisibility(ESlateVisibility::HitTestInvisible);
			Panel.SetContent(Text);
			Panel.SetVisibility(ESlateVisibility::HitTestInvisible);
			IncomeTipPanels.Add(Panel);
			IncomeTipTexts.Add(Text);
			// RECETTE-1121sd: a transparent host around the panel, whose top padding is the drop under the "Scénario" banner (0 without a banner: same look).
			UBorder Host = Cast<UBorder>(Screen.ConstructWidget(UBorder, FName(f"IncomeTipHost{Index}")));
			Host.SetBrushColor(FLinearColor(0.0, 0.0, 0.0, 0.0));
			Host.SetPadding(FMargin(0.0));
			Host.SetVisibility(ESlateVisibility::HitTestInvisible);
			Host.SetContent(Panel);
			IncomeTipHosts.Add(Host);
			if (Index % 2 == 0)
				AmountOf(Index / 2).SetToolTip(Host);
			else
				GatherersOf(Index / 2).SetToolTip(Host);
		}
	}

	// The game screen tells the bar where the "Scénario" banner is (its left, right and bottom edge on the screen, Bottom 0 when it is not shown): the income
	// tooltips are dropped under it. Called by HOLGameScreenScript.as (not wired by this commit: the screen file is not this chantier's).
	void SetBannerRect(float Left, float Right, float Bottom)
	{
		EnsureIncomeTipPanels();
		// One drop per tooltip, from ITS cell's position (the cursor point is the cell's centre) and the width of its text: only the tooltip that crosses the
		// banner goes under it; a cell whose box is not known yet keeps the natural place.
		for (int Index = 0; Index < IncomeTipHosts.Num(); Index++)
		{
			UWidget Cell = Index % 2 == 0 ? Cast<UWidget>(AmountOf(Index / 2)) : Cast<UWidget>(GatherersOf(Index / 2));
			FVector2D CellLow;
			FVector2D CellHigh;
			float Drop = 0.0;
			if (Screen.GetWidgetBox(Cell, CellLow, CellHigh))
			{
				const float TipWidth = HOLTopBar::IncomeTipWidth(IncomeTipTexts[Index].GetText().ToString(), HOLTopBar::IncomeTipView(0, Index / 2).Padding);
				Drop = HOLTopBar::IncomeTipDropForCell((CellLow.X + CellHigh.X) * 0.5, TipWidth, Left, Right, Bottom);
			}
			IncomeTipHosts[Index].SetPadding(FMargin(0.0, Drop, 0.0, 0.0));
		}
	}

	void SetIncomeTips(FHOLResources Income)
	{
		EnsureIncomeTipPanels();
		for (int Resource = 0; Resource < 4; Resource++)
		{
			const HOLTopBar::FIncomeTipView View = HOLTopBar::IncomeTipView(HOLTopBar::AmountOf(Income, Resource), Resource);
			for (int Side = 0; Side < 2; Side++)
			{
				const int Index = Resource * 2 + Side;
				FSlateBrush Brush;
				Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
				Brush.TintColor = FSlateColor(View.Fill);
				Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
				Brush.OutlineSettings.CornerRadii = FVector4(4.0, 4.0, 4.0, 4.0);
				Brush.OutlineSettings.Color = FSlateColor(View.Border);
				Brush.OutlineSettings.Width = View.BorderWidth;
				IncomeTipPanels[Index].SetBrush(Brush);
				IncomeTipPanels[Index].SetBrushColor(FLinearColor(1.0, 1.0, 1.0, 1.0));
				IncomeTipPanels[Index].SetPadding(FMargin(View.Padding));
				IncomeTipTexts[Index].SetText(FText::FromString(View.Text));
			}
		}
	}
	// RefreshTopBar + RefreshIdleWorkers + RefreshStatsButton of the C++ screen, in one: the bar from the economy of the player the screen shows (its
	// BoundPlayerState: a spectator's followed player), the idle counter from the controller, « Statistiques » from the match's state.
	void Refresh()
	{
		if (TopBar == nullptr)
			return;
		FHOLResources Stock;
		FHOLResources Gatherers;
		FHOLResources Income;
		int Used = 0;
		int Cap = 0;
		bEconomyShown = HOLTopBar::BarShown(Screen.GetEconomyNow(Stock, Used, Cap, Gatherers, Income));
		TopBar.SetVisibility(bEconomyShown ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		if (bEconomyShown)
		{
			ShownStock = Stock;
			ShownGatherers = Gatherers;
			ShownPopulationUsed = Used;
			ShownPopulationCap = Cap;
			for (int Resource = 0; Resource < 4; Resource++)
			{
				AmountOf(Resource).SetText(FText::FromString(HOLTopBar::FormatAmount(HOLTopBar::AmountOf(Stock, Resource))));
				GatherersOf(Resource).SetText(FText::FromString(HOLTopBar::GatherersText(HOLTopBar::AmountOf(Gatherers, Resource))));
			}
			if (!bIncomeKnown || HOLTopBar::IncomeChanged(ShownIncome, Income) || ShownTipKey != HOLTopBar::TipLanguageKey())
			{
				bIncomeKnown = true;
				ShownTipKey = HOLTopBar::TipLanguageKey();
				ShownIncome = Income;
				SetIncomeTips(Income);
			}
			PopulationAmount.SetText(FText::FromString(HOLTopBar::FormatRatio(Used, Cap)));
		}

		const int Count = Screen.GetIdleWorkerCount();
		if (HOLTopBar::IdleChanged(ShownIdleWorkers, Count) || HOLTopBar::IdleLabelChanged(ShownIdleLabel, Count))
		{
			ShownIdleWorkers = Count;
			ShownIdleLabel = HOLTopBar::IdleLabel(Count);
			IdleWorkers.SetLabel(FText::FromString(ShownIdleLabel));
			IdleWorkers.SetIsEnabled(HOLTopBar::IdleEnabled(Count));
		}

		// The box too: collapsed, « Menu » keeps its place alone.
		bStatsShown = Screen.GetStatsButtonShown();
		StatsButton.SetVisibility(bStatsShown ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		StatsBox.SetVisibility(bStatsShown ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}
