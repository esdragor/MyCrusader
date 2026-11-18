// The statistics screen's widget in script (D851, ST-2, as HOLSettingsScreen.as): the screen UHOLStatsScreen::Open pushes (once ST-3 switches it) is the class
// FindScriptClass finds, the one of this file. Its column, 4 tabs (Summary, Military, Economy, Timeline), the 3 tables, the timeline's 6 series buttons and its
// graph are built here (BP_BuildStats, in place of BuildLayout) with the same widget names as the C++ screen had (the scenarios and hol.UI.Click aim at
// them); the tables are filled again at each OnStatsUpdated or refusal (BP_RefreshStats, in place of Refresh) and the graph at each series (BP_RefreshTimeline,
// in place of RefreshTimeline). The base keeps the opening, the subscriptions, the 10 s request, the navigation (SelectTab, SelectSeries, Close), the
// localized texts (TextOf, HeadersOf, MakeMarkLabel) and the painting of the graph. What a row shows is HOLStatsRules (Script/UI/HOLStatsRules.as).
class UHOLStatsScreenScript : UHOLStatsScreen
{
	UFUNCTION(BlueprintOverride)
	void BP_BuildStats()
	{
		UVerticalBox Column = MakeStatsColumn();
		AddStatsText(Column, n"Heading", UHOLStatsScreen::TextOf(n"Heading"), 0);

		UHorizontalBox TabRow = AddStatsRow(Column);
		SummaryTab = AddStatsButton(TabRow, n"SummaryTab", UHOLStatsScreen::TextOf(n"Summary"));
		MilitaryTab = AddStatsButton(TabRow, n"MilitaryTab", UHOLStatsScreen::TextOf(n"Military"));
		EconomyTab = AddStatsButton(TabRow, n"EconomyTab", UHOLStatsScreen::TextOf(n"Economy"));
		TimelineTab = AddStatsButton(TabRow, n"TimelineTab", UHOLStatsScreen::TextOf(n"Timeline"));

		Status = AddStatsText(Column, n"Status", FText::FromString(""), 1);
		if (Status != nullptr)
			Status.SetVisibility(ESlateVisibility::Collapsed);

		TabPages = AddStatsPages(Column);
		for (int Tab = 0; Tab < HOLStatsRules::TableCount(); Tab++)
			AddStatsGridPage();

		// C5: the series' buttons over the curves (Series<X>, in the order of EHOLMatchStatsSeries), then the graph.
		UVerticalBox TimelinePage = AddStatsTimelinePage();
		UHorizontalBox SeriesRow = AddStatsSeriesRow(TimelinePage);
		for (int Series = 0; Series < HOLStatsRules::SeriesCount(); Series++)
		{
			FName ButtonName = FName("Series" + HOLStatsRules::SeriesName(Series));
			AddStatsSeriesButton(SeriesRow, ButtonName, UHOLStatsScreen::TextOf(ButtonName));
		}
		Graph = AddStatsGraph(TimelinePage);

		BackButton = AddStatsButton(AddStatsRow(Column), n"BackButton", UHOLStatsScreen::TextOf(n"Back"));
	}

	// A figure as the C++ screen showed it (FText::AsNumber, grouped by the culture), the dash of a missing team, or a text of the screen.
	// MANQUE: the dash is a string here (the C++ had FText::AsCultureInvariant(TEXT("—"))); a base text Key would hold it.
	FText CellText(const FHOLStatsCell& Cell)
	{
		if (Cell.bDash)
			return FText::FromString("—");
		if (!Cell.Key.IsNone())
			return UHOLStatsScreen::TextOf(Cell.Key);
		return FText::AsNumber(float(Cell.Value));
	}

	UFUNCTION(BlueprintOverride)
	void BP_RefreshStats(bool bRefusedNow)
	{
		const bool bHasStats = HOLMatchStatsRead::HasWorldStats(this);

		// M6-Q11: a player gets them once the match is over; meanwhile the server says no, or has not answered yet.
		const FName StatusKey = HOLStatsRules::StatusKey(bRefusedNow, bHasStats);
		if (Status != nullptr)
		{
			if (StatusKey.IsNone())
			{
				Status.SetText(FText::FromString(""));
				Status.SetVisibility(ESlateVisibility::Collapsed);
			}
			else
			{
				Status.SetText(UHOLStatsScreen::TextOf(StatusKey));
				Status.SetVisibility(ESlateVisibility::HitTestInvisible);
			}
		}
		if (TabPages != nullptr)
		{
			if (bHasStats && !bRefusedNow)
				TabPages.SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			else
				TabPages.SetVisibility(ESlateVisibility::Collapsed);
		}
		if (!bHasStats || Grids.Num() < HOLStatsRules::TableCount())
			return;

		FHOLMatchStatsState Stats = HOLMatchStatsRead::GetWorldStats(this);
		TArray<FHOLStatsRowState> Rows = HOLMatchStatsRead::GetWorldStatsRows(this);

		// The headers of each table (row 0), the names' column included.
		for (int Tab = 0; Tab < HOLStatsRules::TableCount(); Tab++)
		{
			Grids[Tab].ClearChildren();
			TArray<FText> Headers = UHOLStatsScreen::HeadersOf(Tab);
			for (int Column = 0; Column < Headers.Num(); Column++)
				AddStatsCell(Grids[Tab], 0, Column, Headers[Column], true, false, FLinearColor::White);
		}

		// One row a slot held by a human or an AI, in the slots' order (GetWorldStatsRows leaves the closed ones out).
		for (int Index = 0; Index < Rows.Num(); Index++)
		{
			const int GridRow = Index + 1;
			FHOLPlayerMatchStatsState Player;
			if (Stats.Players.IsValidIndex(Rows[Index].SlotIndex))
				Player = Stats.Players[Rows[Index].SlotIndex];
			for (int Tab = 0; Tab < HOLStatsRules::TableCount(); Tab++)
				AddStatsCell(Grids[Tab], GridRow, 0, Rows[Index].Name, false, true, Rows[Index].Color);
			for (int Tab = 0; Tab < HOLStatsRules::TableCount(); Tab++)
			{
				TArray<FText> Headers = UHOLStatsScreen::HeadersOf(Tab);
				TArray<FHOLStatsCell> Cells = HOLStatsRules::TableRow(Tab, Rows[Index].Team, Rows[Index].Result, Player);
				for (int Column = 1; Column < Headers.Num() && Column - 1 < Cells.Num(); Column++)
					AddStatsCell(Grids[Tab], GridRow, Column, CellText(Cells[Column - 1]), false, false, FLinearColor::White);
			}
		}
		BP_RefreshTimeline(GetActiveSeries());
	}

	UFUNCTION(BlueprintOverride)
	void BP_RefreshTimeline(int Series)
	{
		// Called from NativeOnInitialized before any statistic: nothing to draw without a graph or without rows.
		if (Graph == nullptr)
			return;
		TArray<FHOLStatsRowState> Rows = HOLMatchStatsRead::GetWorldStatsRows(this);
		if (Rows.Num() == 0)
			return;
		FHOLMatchStatsState Stats = HOLMatchStatsRead::GetWorldStats(this);
		TArray<FHOLStatsGraphCurve> Curves = HOLStatsRules::BuildCurves(Series, Rows, Stats);
		TArray<FHOLStatsGraphMark> Marks = HOLStatsRules::BuildMarks(Rows, Stats);
		const int NumCurves = Curves.Num();
		const int NumMarks = Marks.Num();
		Graph.SetGraphData(Curves, Marks);
		Log(n"LogHOLUI", f"Stats screen: timeline {HOLStatsRules::SeriesName(HOLStatsRules::ClampSeries(Series))}, {NumCurves} curves, {NumMarks} marks, {Graph.GetAxesDescription()}");
	}
}
