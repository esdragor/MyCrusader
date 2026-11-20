// NOT COMPILED YET (HALDIR-1119p, H-9 lot A of the HUD plan): this file calls bindings that do not exist before the next C++ lot (HOLSelectionRead::GetSelectionKind,
// GetBuildingSelectionView, GetSourceSelectionView, GetOwnerName: Source\HeroesOfLibertyUI\HUD\HOLSelectionReadLibrary.h, and UHOLScriptScreen::SetSelectionReport). Do NOT
// build with this file present and without that lot.
//
// CROCHET : what the switch pastes into HOLGameScreenScript.as (nobody edits it before; HOLSelection.as is the only Build of the block, lots B and C get their hosts from it):
//   member   : UHOLSelectionPanel Selection;                                                    (beside AgePanels, line ~131)
//   Build    : Selection = Cast<UHOLSelectionPanel>(NewObject(this, UHOLSelectionPanel)); Selection.Build(this, <the bottom bar, a UHorizontalBox>);
//              in OnInitialized, after AgePanels.Build (line ~279); the bar is the C++ BottomBar: the switch must hand it over (a GetSelectionHost() like GetAgePanelsHost, NOT
//              made here), the panel goes first in it (AddToBar(SelectionPlace, 0), bottom aligned). Then lots B and C build into Selection.StatsHost / Selection.ChipsHost.
//   Refresh  : Selection.Refresh();                       in Refresh() (~1404), every refresh (a change, then each 250 ms): the path, kinds 0, 1 and 4 drawn here.
//   per frame: Selection.FrameTick();                     once per image from the script screen's tick while Selection.IsFrameLive(): a selected source's remaining amount.
//   Describe : UHOLGameScreen::Describe reads Screen.GetSelectionReportKind / Activity / Health / Forum (the copies SetSelectionReport posed), not its own fields.
//   Clicks   : none (the block has no button). Lots B and C call SetPanelShown(true) when they draw kinds 2 and 3 (this class hides the progress and lives rows for them).
//
// The selection block of the game HUD, bottom left, in script: its panel, the name, the owner with its colour's swatch, the health, a site's progress, the activity line and a main
// forum's lives; one path of the four of UHOLGameScreen::Refresh drawn here per call (a building alone, a source), the others (units, a group of buildings) belong to lots B and C,
// which fill the two empty hosts (SelectionChipsHost between the name and the owner, SelectionStatsHost between the health and the site's progress) and write the name, owner, health and
// activity rows through this class's setters. ShownSelection, ShownActivity and ShownHealth (the line hol.HUD.Report reads, quick_check) have ONE writer: this class pushes them to C++
// (SetSelectionReport). The widget names are the C++ screen's (hol.HUD.Click, hol.Mouse.Hover, DescribeRects find them by name).

namespace HOLSelectionText
{
	// Same namespace and keys as the C++ LOCTEXT of UHOLGameScreen ("HOLGameScreen,<key>"): the translations stay.
	FString Replace1(FText Pattern, FString Value)
	{
		return Pattern.ToString().Replace("{0}", Value);
	}

	FText FoundationName() { return NSLOCTEXT("HOLGameScreen", "FoundationName", "Fondation : {0}"); }
	FText SiteName() { return NSLOCTEXT("HOLGameScreen", "SiteName", "Chantier : {0}"); }
	FText ProgressPercent() { return NSLOCTEXT("HOLGameScreen", "ProgressPercent", "{0} %"); }
	FText Builders() { return NSLOCTEXT("HOLGameScreen", "Builders", "Bâtisseurs : {0}"); }
	FText Health() { return NSLOCTEXT("HOLGameScreen", "Health", "Points de vie"); }
	FText LivesLabel() { return NSLOCTEXT("HOLGameScreen", "ForumLivesLabel", "Vies"); }
	FText SourceRemaining() { return NSLOCTEXT("HOLGameScreen", "SourceRemaining", "Reste : {0}"); }
}

// What the block shows of a line under the health (SetDetailLine of the C++ screen): the report's text, the line's text, and whether the row is on the screen.
struct FHOLSelectionDetail
{
	FString Shown;
	FString Text;
	bool bVisible = false;
}

namespace HOLSelectionRules
{
	// Functions, not constants: a constant of a namespace is not visible from another file (the test). The C++ constants of the screen (HOLGameScreen.cpp:251-261).
	float SelectionWidth() { return 340.0; }
	float BarHeight() { return 252.0; }
	float SwatchSize() { return 14.0; }
	float SiteBarWidth() { return 200.0; }

	// Amounts without thousands separators ("1840"): FormatAmount of the C++ screen.
	FString FormatAmount(int Value)
	{
		return f"{Value}";
	}

	// "250 / 250" (FormatRatio: INVTEXT("{0} / {1}")): a building's health.
	FString HealthText(int Health, int MaxHealth)
	{
		return FormatAmount(Health) + " / " + FormatAmount(MaxHealth);
	}

	// A unit's ShownHealth: ", hp 250/250", ", hp 40/100 (share)" for a client's hero known by a share; empty when its maximum is unknown (RefreshUnitHealth).
	FString HealthShown(int Health, int MaxHealth, bool bExact)
	{
		if (MaxHealth <= 0)
			return "";
		return f", hp {Health}/{MaxHealth}" + (bExact ? "" : " (share)");
	}

	// The site's percentage as the C++ screen floors it (FloorToInt(GetProgress() * 100.f), in float32).
	int PercentOf(float32 Progress)
	{
		return Math::FloorToInt(Progress * float32(100.0));
	}

	// The name line: a finished building's name; a foundation's "Fondation : X" (J6a.3); a site's "Chantier : X" (J6a.5, D313).
	FString NameText(FString Name, bool bFinished, bool bFoundation)
	{
		if (bFinished)
			return Name;
		if (bFoundation)
			return HOLSelectionText::Replace1(HOLSelectionText::FoundationName(), Name);
		return HOLSelectionText::Replace1(HOLSelectionText::SiteName(), Name);
	}

	// The site's percentage text, "40 %".
	FString ProgressText(int Percent)
	{
		return HOLSelectionText::Replace1(HOLSelectionText::ProgressPercent(), f"{Percent}");
	}

	// ShownSelection of a building alone: "MainCentre (own)", "House (other), foundation", "House (own), under construction 40%" (the line quick_check reads).
	FString BuildingShownKind(FString TypeKey, bool bOwn, bool bFinished, bool bFoundation, int Percent)
	{
		FString Own = bOwn ? "own" : "other";
		FString Tail = "";
		if (bFoundation)
			Tail = ", foundation";
		else if (!bFinished)
			Tail = f", under construction {Percent}%";
		return f"{TypeKey} ({Own}){Tail}";
	}

	// ShownSelection of a source: "Tree source 12" (the key, then its index).
	FString SourceShownKind(FString TypeKey, int Index)
	{
		return f"{TypeKey} source {Index}";
	}

	FString NoneShownKind()
	{
		return "none";
	}

	// A site's builders, which only its owner receives (D313): ", builders 2" and "Bâtisseurs : 2".
	FString BuildersShown(int Count)
	{
		return f", builders {Count}";
	}

	FString BuildersText(int Count)
	{
		return HOLSelectionText::Replace1(HOLSelectionText::Builders(), f"{Count}");
	}

	// A building's detail line, in the C++ order (RefreshSelection): a main forum's line (its text, hidden when it stands: its lives alone, the report keeps them), then a site's builders
	// for its owner, which wins over it. Nothing else: no line.
	FHOLSelectionDetail BuildingDetail(bool bFinished, bool bOwn, int Builders, bool bForumShown, FString ForumShown, FString ForumText)
	{
		FHOLSelectionDetail Detail;
		if (bForumShown)
		{
			Detail.Shown = ForumShown;
			Detail.Text = ForumText;
			Detail.bVisible = ForumText != "";
		}
		if (!bFinished && bOwn)
		{
			Detail.Shown = BuildersShown(Builders);
			Detail.Text = BuildersText(Builders);
			Detail.bVisible = true;
		}
		return Detail;
	}

	// A source's detail line: ", 1840 left" and "Reste : 1840" (D298).
	FHOLSelectionDetail SourceDetail(int Remaining)
	{
		FHOLSelectionDetail Detail;
		Detail.Shown = f", {Remaining} left";
		Detail.Text = HOLSelectionText::Replace1(HOLSelectionText::SourceRemaining(), FormatAmount(Remaining));
		Detail.bVisible = true;
		return Detail;
	}

	// The report's ShownActivity of a worker or a unit that strikes (RefreshUnitActivity): ", Gathering Wood 4/10" with its load known, ", Attacking" otherwise; empty without a unit.
	// Activity and Resource are the enum names as UEnum::GetValueAsString gives them: the C++ writes that whole string, so the caller passes it whole (this rule only joins).
	FString WorkerActivityShown(bool bHasUnit, FString Activity, bool bLoadShown, FString Resource, int Amount, int Capacity)
	{
		if (!bHasUnit)
			return "";
		if (bLoadShown)
			return f", {Activity} {Resource} {Amount}/{Capacity}";
		return f", {Activity}";
	}

	// A main forum's report text (ShownForum): its Shown line when it is a forum, empty otherwise.
	FString ForumReport(bool bForumShown, FString ForumShown)
	{
		return bForumShown ? ForumShown : "";
	}

	// The lives row: shown for a forum only; its value is set again only when it changes (-1: never set).
	bool LivesChanged(int ShownLives, int Lives)
	{
		return Lives != ShownLives;
	}
}

// The selection block. Built once in the script screen's OnInitialized (Build), refreshed in its Refresh (Refresh) and, for a source, each image (FrameTick).
class UHOLSelectionPanel : UObject
{
	UHOLScriptScreen Screen;

	// The two hosts of lots B and C: empty vertical boxes, collapsed until they put something in them.
	UVerticalBox ChipsHost;
	UVerticalBox StatsHost;
	// The garrison row of a building alone (D813, HOLGarrisonFrame.as): built under the forum's lives, refreshed by RefreshBuilding, hidden on every other path.
	UHOLGarrisonFrame Garrison;

	UOverlay SelectionPlace;
	UBorder SelectionPanel;
	UCommonTextBlock SelectionName;
	UHorizontalBox OwnerRow;
	UImage OwnerSwatch;
	UCommonTextBlock OwnerText;
	UHorizontalBox HealthRow;
	UCommonTextBlock HealthText;
	UHorizontalBox ProgressRow;
	UProgressBar SiteProgress;
	UCommonTextBlock ProgressText;
	UCommonTextBlock ActivityText;
	UHorizontalBox LivesRow;
	UCommonTextBlock LivesText;

	// What is shown, for the report and the tests (a script test cannot read a widget). ShownSelection ("none", "King (own)"), ShownActivity (", hp"...), ShownHealth, and the forum's line.
	FString ShownKind = "none";
	FString ShownActivity = "";
	FString ShownHealth = "";
	FString ShownForum = "";
	int ShownLives = -1;
	// The path drawn at the last Refresh (0 nothing, 1 a building, 2 units, 3 a group, 4 a source): FrameTick follows a source only.
	int ShownPathKind = 0;

	// AddToRow of the C++ screen: centred vertically, a right padding.
	void AddToRow(UHorizontalBox Row, UWidget Widget, float RightPadding)
	{
		UHorizontalBoxSlot Slot = Row.AddChildToHorizontalBox(Widget);
		Slot.SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
		Slot.SetPadding(FMargin(0.0, 0.0, RightPadding, 0.0));
	}

	UVerticalBoxSlot AddToColumn(UVerticalBox Column, UWidget Widget, float Top, float Bottom)
	{
		UVerticalBoxSlot Slot = Column.AddChildToVerticalBox(Widget);
		Slot.SetPadding(FMargin(0.0, Top, 0.0, Bottom));
		return Slot;
	}

	// SetShown of the C++ screen: touches the widget only when its visibility changes.
	void SetShown(UWidget Widget, bool bShown)
	{
		if (Widget == nullptr)
			return;
		const ESlateVisibility Wanted = bShown ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
		if (Widget.GetVisibility() != Wanted)
			Widget.SetVisibility(Wanted);
	}

	UHorizontalBox MakeRow(FName Name)
	{
		return Cast<UHorizontalBox>(Screen.ConstructWidget(UHorizontalBox, Name));
	}

	// BuildLayout's selection block (HOLGameScreen.cpp:2685-2778), bottom left of the bar: a fixed width and the bar's height (a spacer holds the place while the panel is collapsed).
	void Build(UHOLScriptScreen InScreen, UHorizontalBox Bar)
	{
		Screen = InScreen;

		SelectionPlace = Cast<UOverlay>(Screen.ConstructWidget(UOverlay, n"SelectionPlace"));
		SelectionPlace.SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		UHorizontalBoxSlot BarSlot = Bar.AddChildToHorizontalBox(SelectionPlace);
		BarSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Bottom);
		BarSlot.SetPadding(FMargin(0.0, 0.0, 0.0, 0.0));
		USpacer Spacer = Cast<USpacer>(Screen.ConstructWidget(USpacer, n"SelectionSpacer"));
		Spacer.SetSize(FVector2D(HOLSelectionRules::SelectionWidth(), HOLSelectionRules::BarHeight()));
		SelectionPlace.AddChildToOverlay(Spacer);
		SelectionPanel = Screen.MakePanel(n"SelectionPanel");
		UOverlaySlot PanelSlot = SelectionPlace.AddChildToOverlay(SelectionPanel);
		PanelSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
		PanelSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
		USizeBox Box = Cast<USizeBox>(Screen.ConstructWidget(USizeBox, n"SelectionBox"));
		Box.SetWidthOverride(HOLSelectionRules::SelectionWidth() - 24.0);
		SelectionPanel.SetContent(Box);
		UVerticalBox Column = Cast<UVerticalBox>(Screen.ConstructWidget(UVerticalBox, n"SelectionColumn"));
		Box.SetContent(Column);

		SelectionName = Screen.MakeText(n"SelectionName", UHOLTextStyleHUDHeading);
		AddToColumn(Column, SelectionName, 0.0, 4.0);

		// Lot C's chips go here (between the name and the owner), empty and collapsed.
		ChipsHost = Cast<UVerticalBox>(Screen.ConstructWidget(UVerticalBox, n"SelectionChipsHost"));
		ChipsHost.SetVisibility(ESlateVisibility::Collapsed);
		Column.AddChildToVerticalBox(ChipsHost);

		OwnerRow = MakeRow(n"SelectionOwnerRow");
		AddToColumn(Column, OwnerRow, 0.0, 2.0);
		OwnerSwatch = Cast<UImage>(Screen.ConstructWidget(UImage, n"SelectionSwatch"));
		OwnerSwatch.SetColorAndOpacity(FLinearColor(1.0, 1.0, 1.0, 1.0));
		OwnerSwatch.SetDesiredSizeOverride(FVector2D(HOLSelectionRules::SwatchSize(), HOLSelectionRules::SwatchSize()));
		AddToRow(OwnerRow, OwnerSwatch, 8.0);
		OwnerText = Screen.MakeText(n"SelectionOwner", UHOLTextStyleHUD);
		AddToRow(OwnerRow, OwnerText, 0.0);

		HealthRow = MakeRow(n"SelectionHealthRow");
		Column.AddChildToVerticalBox(HealthRow);
		UCommonTextBlock HealthLabel = Screen.MakeText(n"SelectionHealthLabel", UHOLTextStyleHUDLabel);
		HealthLabel.SetText(HOLSelectionText::Health());
		AddToRow(HealthRow, HealthLabel, 8.0);
		HealthText = Screen.MakeText(n"SelectionHealth", UHOLTextStyleHUD);
		AddToRow(HealthRow, HealthText, 0.0);

		// Lot B's six stats go here (between the health and the site's progress), empty and collapsed.
		StatsHost = Cast<UVerticalBox>(Screen.ConstructWidget(UVerticalBox, n"SelectionStatsHost"));
		StatsHost.SetVisibility(ESlateVisibility::Collapsed);
		Column.AddChildToVerticalBox(StatsHost);

		// A site's progress (J6a.5, D313): a bar and its percentage.
		ProgressRow = MakeRow(n"SelectionProgressRow");
		AddToColumn(Column, ProgressRow, 4.0, 0.0);
		SiteProgress = Screen.MakeBar(n"SelectionProgress", UHOLScriptScreen::GetAccentColor());
		USizeBox BarBox = Cast<USizeBox>(Screen.ConstructWidget(USizeBox, n"SelectionProgressBox"));
		BarBox.SetWidthOverride(HOLSelectionRules::SiteBarWidth());
		BarBox.SetHeightOverride(8.0);
		BarBox.SetContent(SiteProgress);
		AddToRow(ProgressRow, BarBox, 8.0);
		ProgressText = Screen.MakeText(n"SelectionProgressText", UHOLTextStyleHUD);
		AddToRow(ProgressRow, ProgressText, 0.0);
		ProgressRow.SetVisibility(ESlateVisibility::Collapsed);

		ActivityText = Screen.MakeText(n"SelectionActivity", UHOLTextStyleHUD);
		ActivityText.AutoWrapText = true;
		AddToColumn(Column, ActivityText, 2.0, 0.0);

		// A main forum's lives (V2.3d): a row as the health's, under the ruin's or the rebuilding's lines.
		LivesRow = MakeRow(n"SelectionLivesRow");
		AddToColumn(Column, LivesRow, 2.0, 0.0);
		UCommonTextBlock LivesLabel = Screen.MakeText(n"SelectionLivesLabel", UHOLTextStyleHUDLabel);
		LivesLabel.SetText(HOLSelectionText::LivesLabel());
		AddToRow(LivesRow, LivesLabel, 8.0);
		LivesText = Screen.MakeText(n"SelectionLives", UHOLTextStyleHUD);
		AddToRow(LivesRow, LivesText, 0.0);
		LivesRow.SetVisibility(ESlateVisibility::Collapsed);

		// A garrison building's occupants (D813): the counter, one icon button per occupant, Sortir tous.
		Garrison = Cast<UHOLGarrisonFrame>(NewObject(this, UHOLGarrisonFrame));
		Garrison.Build(Screen, Column);

		// Nothing selected yet.
		SelectionPanel.SetVisibility(ESlateVisibility::Collapsed);
		PushReport();
	}

	// Contract of lots B and C ---------------------------------------------------------------------------------------------------------------------------------------------

	// The panel itself: on the screen (Visible, it takes the mouse as the C++ one) or collapsed (its spacer keeps the place).
	void SetPanelShown(bool bShown)
	{
		if (SelectionPanel == nullptr)
			return;
		const ESlateVisibility Wanted = bShown ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
		if (SelectionPanel.GetVisibility() != Wanted)
			SelectionPanel.SetVisibility(Wanted);
	}

	void SetName(FText Text, bool bShown)
	{
		if (SelectionName == nullptr)
			return;
		SelectionName.SetText(Text);
		SetShown(SelectionName, bShown);
	}

	void SetOwner(FText Text, FLinearColor Colour, bool bShown)
	{
		if (OwnerRow == nullptr)
			return;
		if (bShown)
		{
			OwnerSwatch.SetColorAndOpacity(Colour);
			OwnerText.SetText(Text);
		}
		SetShown(OwnerRow, bShown);
	}

	void SetHealth(FText Text, bool bShown)
	{
		if (HealthRow == nullptr)
			return;
		if (bShown)
			HealthText.SetText(Text);
		SetShown(HealthRow, bShown);
	}

	void SetActivity(FText Text, bool bShown)
	{
		if (ActivityText == nullptr)
			return;
		if (bShown)
			ActivityText.SetText(Text);
		SetShown(ActivityText, bShown);
	}

	// The three report fields have this class as their only writer; each pushes the four to C++ when it changes.
	void SetShownKind(FString Kind)
	{
		if (Kind == ShownKind)
			return;
		ShownKind = Kind;
		PushReport();
	}

	void SetShownActivity(FString Activity)
	{
		if (Activity == ShownActivity)
			return;
		ShownActivity = Activity;
		PushReport();
	}

	void SetShownHealth(FString Health)
	{
		if (Health == ShownHealth)
			return;
		ShownHealth = Health;
		PushReport();
	}

	void PushReport()
	{
		if (Screen != nullptr)
			Screen.SetSelectionReport(ShownKind, ShownActivity, ShownHealth, ShownForum);
	}

	// The detail line under the health (SetDetailLine of the C++ screen): the text is set only when the report's text changes.
	void SetDetail(FHOLSelectionDetail Detail)
	{
		if (Detail.Shown != ShownActivity)
		{
			SetShownActivity(Detail.Shown);
			SetShown(ActivityText, Detail.Shown != "");
			if (Detail.Shown != "")
				ActivityText.SetText(FText::FromString(Detail.Text));
		}
	}

	// Refresh -------------------------------------------------------------------------------------------------------------------------------------------------------------

	// One refresh: the path the controller's selection takes (UHOLGameScreen::Refresh's order), the building alone and the source drawn here.
	void Refresh()
	{
		if (SelectionPanel == nullptr)
			return;
		ShownPathKind = HOLSelectionRead::GetSelectionKind();
		// A main forum's lives row is for a building alone (V2.3d): every other path hides it, and a site's progress too.
		if (ShownPathKind != 1)
		{
			SetShown(LivesRow, false);
			if (Garrison != nullptr)
				Garrison.Hide();
		}
		if (ShownPathKind == 1)
			RefreshBuilding();
		else if (ShownPathKind == 4)
			RefreshSource();
		else if (ShownPathKind == 0)
			RefreshNothing();
		else
			SetShown(ProgressRow, false);
	}

	// A source's remaining amount moves each image while someone gathers it (D298): FrameTick follows it, nothing else.
	bool IsFrameLive() const
	{
		return ShownPathKind == 4;
	}

	void FrameTick()
	{
		if (ShownPathKind == 4)
			RefreshSource();
	}

	// RefreshSelection(nullptr): the panel gone, the report "none".
	void RefreshNothing()
	{
		SetPanelShown(false);
		SetShown(ProgressRow, false);
		SetShownKind(HOLSelectionRules::NoneShownKind());
		SetShownHealth("");
		SetDetail(FHOLSelectionDetail());
	}

	// RefreshSelection: a building alone, finished, foundation or site.
	void RefreshBuilding()
	{
		FHOLBuildingSelView View;
		if (!HOLSelectionRead::GetBuildingSelectionView(View))
		{
			RefreshNothing();
			return;
		}
		SetPanelShown(true);
		const bool bSite = !View.bFinished;
		SetShown(ProgressRow, bSite);
		const int Percent = HOLSelectionRules::PercentOf(View.Progress);
		SetShownKind(HOLSelectionRules::BuildingShownKind(View.TypeKey, View.bOwn, View.bFinished, View.bFoundation, Percent));
		SetName(FText::FromString(HOLSelectionRules::NameText(View.Name.ToString(), View.bFinished, View.bFoundation)), true);
		if (bSite)
		{
			SiteProgress.SetPercent(View.Progress);
			ProgressText.SetText(FText::FromString(HOLSelectionRules::ProgressText(Percent)));
		}
		SetOwner(View.Owner, View.Colour, true);
		SetShownHealth("");
		// A main forum: its lives to anyone (D353); down, its ruin or its rebuilding in place of its health. Reported on the "HUD forum" line, not in Describe.
		ShownForum = HOLSelectionRules::ForumReport(View.bForumShown, View.ForumShown);
		SetHealth(FText::FromString(HOLSelectionRules::HealthText(View.Health, View.MaxHealth)), !View.bForumHideHealth);
		SetShown(LivesRow, View.bForumShown);
		if (View.bForumShown && HOLSelectionRules::LivesChanged(ShownLives, View.ForumLives))
		{
			ShownLives = View.ForumLives;
			LivesText.SetText(FText::FromString(HOLSelectionRules::FormatAmount(View.ForumLives)));
		}
		const FHOLSelectionDetail Detail = HOLSelectionRules::BuildingDetail(View.bFinished, View.bOwn, View.Builders, View.bForumShown, View.ForumShown, View.ForumText.ToString());
		SetDetail(Detail);
		// The garrison of an own finished building with places (HOLGarrisonFrameRules::ViewFor draws nothing for any other).
		if (Garrison != nullptr)
			Garrison.Refresh(View.GarrisonOccupants, View.GarrisonOccupantTypes, HOLGarrison::SlotsOf(FName(View.TypeKey)), View.bOwn, View.bFinished);
		// The row follows the forum's text, each refresh, as the C++ screen's SetShown after SetDetailLine.
		if (View.bForumShown && !(bSite && View.bOwn))
			SetShown(ActivityText, Detail.bVisible);
		PushReport();
	}

	// RefreshSourceSelection: a tree or a vein, nobody's: its name and what it still holds (D298).
	void RefreshSource()
	{
		FHOLSourceSelView View;
		if (!HOLSelectionRead::GetSourceSelectionView(View))
		{
			RefreshNothing();
			return;
		}
		SetPanelShown(true);
		SetShown(ProgressRow, false);
		SetShown(OwnerRow, false);
		SetShown(HealthRow, false);
		SetShownHealth("");
		const FString Kind = HOLSelectionRules::SourceShownKind(View.TypeKey, View.Index);
		if (Kind != ShownKind)
		{
			SetShownKind(Kind);
			SetName(View.Name, true);
		}
		SetDetail(HOLSelectionRules::SourceDetail(View.Remaining));
	}
}
