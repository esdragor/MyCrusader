// The statistics screen's widget in script (D851, ST-2, as HOLSettingsScreen.as): the screen UHOLStatsScreen::Open pushes (once ST-3 switches it) is the class
// FindScriptClass finds, the one of this file. Its column, 4 tabs (Summary, Military, Economy, Timeline), the 3 tables, the timeline's 6 series buttons and its
// graph are built here (BuildStats, the script name of BP_BuildStats, in place of BuildLayout) with the same widget names as the C++ screen had (the scenarios and hol.UI.Click aim at
// them); the tables are filled again at each OnStatsUpdated or refusal (RefreshStats, of BP_RefreshStats, in place of Refresh) and the graph at each series (RefreshTimeline, of BP_RefreshTimeline,
// in place of RefreshTimeline). The base keeps the opening, the subscriptions, the 10 s request, the navigation (SelectTab, SelectSeries, Close), the
// localized texts (TextOf, HeadersOf, MakeMarkLabel) and the painting of the graph. What a row shows is HOLStatsRules (Script/UI/HOLStatsRules.as).
class UHOLStatsScreenScript : UHOLStatsScreen
{
	UFUNCTION(BlueprintOverride)
	void BuildStats()
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

	// A figure as the C++ screen showed it (NumberText of the base: FText::AsNumber, thousands grouped by the culture), the dash of a missing team (TextOf Dash), or a text of the screen.
	FText CellText(const FHOLStatsCell& Cell)
	{
		if (Cell.bDash)
			return UHOLStatsScreen::TextOf(n"Dash");
		if (!Cell.Key.IsNone())
			return UHOLStatsScreen::TextOf(Cell.Key);
		return UHOLStatsScreen::NumberText(Cell.Value);
	}

	// The dated marks: one a recorded event, in the colour and the name of its slot, a closed one included (the base's GetWorldSlotColor / GetWorldSlotName; HOLStatsRules::BuildMarks
	// only knew the slots with a row), the label by the base (MakeMarkLabel).
	TArray<FHOLStatsGraphMark> BuildSlotMarks(const FHOLMatchStatsState& Stats)
	{
		TArray<FHOLStatsGraphMark> Marks;
		for (int Index = 0; Index < Stats.Events.Num(); Index++)
		{
			FHOLStatsGraphMark Mark;
			Mark.TimeSeconds = Stats.Events[Index].TimeSeconds;
			Mark.Color = HOLMatchStatsRead::GetWorldSlotColor(Stats.Events[Index].PlayerIndex);
			Mark.Label = UHOLStatsScreen::MakeMarkLabel(Stats.Events[Index].Kind, HOLMatchStatsRead::GetWorldSlotName(Stats.Events[Index].PlayerIndex));
			Marks.Add(Mark);
		}
		return Marks;
	}

	UFUNCTION(BlueprintOverride)
	void RefreshStats(bool bRefusedNow)
	{
		const bool bHasStats = HOLMatchStatsRead::HasWorldStats();

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

		FHOLMatchStatsState Stats = HOLMatchStatsRead::GetWorldStats();
		TArray<FHOLStatsRowState> Rows = HOLMatchStatsRead::GetWorldStatsRows();

		// The headers of each table (row 0), the names' column included.
		for (int Tab = 0; Tab < HOLStatsRules::TableCount(); Tab++)
		{
			UGridPanel Grid = Grids[Tab];
			Grid.ClearChildren();
			TArray<FText> Headers = UHOLStatsScreen::HeadersOf(Tab);
			for (int Column = 0; Column < Headers.Num(); Column++)
				AddStatsCell(Grid, 0, Column, Headers[Column], true, false, FLinearColor::White);
		}

		TArray<UGridPanel> Tables;
		for (int Tab = 0; Tab < HOLStatsRules::TableCount(); Tab++)
			Tables.Add(Grids[Tab]);
		// One row a slot held by a human or an AI, in the slots' order (GetWorldStatsRows leaves the closed ones out).
		for (int Index = 0; Index < Rows.Num(); Index++)
		{
			const int GridRow = Index + 1;
			FHOLPlayerMatchStatsState Player;
			if (Stats.Players.IsValidIndex(Rows[Index].SlotIndex))
				Player = Stats.Players[Rows[Index].SlotIndex];
			for (int Tab = 0; Tab < HOLStatsRules::TableCount(); Tab++)
				AddStatsCell(Tables[Tab], GridRow, 0, Rows[Index].Name, false, true, Rows[Index].Color);
			for (int Tab = 0; Tab < HOLStatsRules::TableCount(); Tab++)
			{
				TArray<FText> Headers = UHOLStatsScreen::HeadersOf(Tab);
				TArray<FHOLStatsCell> Cells = HOLStatsRules::TableRow(Tab, Rows[Index].Team, Rows[Index].Result, Player);
				for (int Column = 1; Column < Headers.Num() && Column - 1 < Cells.Num(); Column++)
					AddStatsCell(Tables[Tab], GridRow, Column, CellText(Cells[Column - 1]), false, false, FLinearColor::White);
			}
			const int MilitaryScore = HOLMatchStatsRead::GetMilitaryScore(Player.UnitsKilled);
			const int EconomyScore = HOLMatchStatsRead::GetEconomyScore(Player.Gathered, Player.UnitsTrained);
			Log(n"LogHOLUI", f"Stats screen: row {Rows[Index].Name.ToString()} (slot {Rows[Index].SlotIndex}) military {MilitaryScore} economy {EconomyScore}, {HOLMatchStatsRead::DescribePlayer(Player)}");
		}
		DrawTimeline(GetActiveSeries());
	}

	UFUNCTION(BlueprintOverride)
	void RefreshTimeline(int Series)
	{
		DrawTimeline(Series);
	}

	// The graph's data; a name of its own because RefreshTimeline is also a C++ non-UFUNCTION of the base (the call would resolve to it).
	void DrawTimeline(int Series)
	{
		// Called from NativeOnInitialized before any statistic: nothing to draw without a graph or without rows.
		if (Graph == nullptr)
			return;
		TArray<FHOLStatsRowState> Rows = HOLMatchStatsRead::GetWorldStatsRows();
		if (Rows.Num() == 0)
			return;
		FHOLMatchStatsState Stats = HOLMatchStatsRead::GetWorldStats();
		TArray<FHOLStatsGraphCurve> Curves = HOLStatsRules::BuildCurves(Series, Rows, Stats);
		TArray<FHOLStatsGraphMark> Marks = BuildSlotMarks(Stats);
		const int NumCurves = Curves.Num();
		const int NumMarks = Marks.Num();
		int Points = 0;
		for (int Index = 0; Index < NumCurves; Index++)
			Points += Curves[Index].Points.Num();
		Graph.SetGraphData(Curves, Marks);
		// The line quick_check parses (curves, points, marks, axes), as the C++ screen wrote it.
		Log(n"LogHOLUI", f"Stats screen: timeline {HOLStatsRules::SeriesName(HOLStatsRules::ClampSeries(Series))}, {NumCurves} curves, {Points} points, {NumMarks} marks, {Graph.GetAxesDescription()}");
	}
}
