// The statistics screen's pure rules (ST-2, D851), as the C++ HOLStatsScreen::Refresh and RefreshTimeline had them inline: what a table row shows in each of
// its columns for a player, the status line's key, the names of the series' buttons, and the curves and marks the graph is given for a series. Nothing here
// touches a widget: the data comes in (the rows of HOLMatchStatsRead::GetWorldStatsRows, the statistics of GetWorldStats), the values go out, so
// Script/UI/HOLStatsScreen_Test.as tests them without a screen.

// One cell of a table row after the name: a figure (Value), the dash of a player without a team (bDash), or a text of the screen by its key (Key, TextOf).
struct FHOLStatsCell
{
	int Value = 0;
	FName Key;
	bool bDash = false;
}

namespace HOLStatsRules
{
	// The tables: 0 Summary, 1 Military, 2 Economy (the page of the timeline is the 4th).
	// (Functions, not constants: a constant of a file is not seen from another file.)
	int TableCount()
	{
		return 3;
	}

	// The timeline's series (EHOLMatchStatsSeries): 0 Workers, 1 Military, 2 Population, 3 Buildings, 4 Gathered, 5 Stock.
	int SeriesCount()
	{
		return 6;
	}

	// The series' name, the end of its button's name (SeriesWorkers...) and of the text key of its label.
	FString SeriesName(int Series)
	{
		if (Series == 0)
			return "Workers";
		if (Series == 1)
			return "Military";
		if (Series == 2)
			return "Population";
		if (Series == 3)
			return "Buildings";
		if (Series == 4)
			return "Gathered";
		return "Stock";
	}

	// The series shown, kept in 0..SeriesCount - 1 (UHOLStatsScreen::SelectSeries clamps the same way).
	int ClampSeries(int Series)
	{
		return Math::Clamp(Series, 0, SeriesCount() - 1);
	}

	// The status line in place of the tables: the refusal (a player during the match), else the wait for the server's copy, else nothing (None).
	FName StatusKey(bool bRefused, bool bHasStats)
	{
		if (bRefused)
			return n"Refused";
		if (!bHasStats)
			return n"Waiting";
		return NAME_None;
	}

	// The result's text key: 1 won, 2 defeated, else still playing (FHOLStatsRowState::Result).
	FName ResultKey(int Result)
	{
		if (Result == 1)
			return n"Winner";
		if (Result == 2)
			return n"Defeated";
		return n"Playing";
	}

	FHOLStatsCell NumberCell(int Value)
	{
		FHOLStatsCell Cell;
		Cell.Value = Value;
		return Cell;
	}

	// Summary after the name: team (a dash when 0), result, military score, economy score, total of both.
	TArray<FHOLStatsCell> SummaryRow(int Team, int Result, const FHOLPlayerMatchStatsState& Player)
	{
		const int MilitaryScore = HOLMatchStatsRead::GetMilitaryScore(Player.UnitsKilled);
		const int EconomyScore = HOLMatchStatsRead::GetEconomyScore(Player.Gathered, Player.UnitsTrained);
		TArray<FHOLStatsCell> Cells;
		FHOLStatsCell TeamCell = NumberCell(Team);
		TeamCell.bDash = Team <= 0;
		Cells.Add(TeamCell);
		FHOLStatsCell ResultCell;
		ResultCell.Key = ResultKey(Result);
		Cells.Add(ResultCell);
		Cells.Add(NumberCell(MilitaryScore));
		Cells.Add(NumberCell(EconomyScore));
		Cells.Add(NumberCell(MilitaryScore + EconomyScore));
		return Cells;
	}

	// Military after the name: killed, lost, buildings lost, army max.
	TArray<FHOLStatsCell> MilitaryRow(const FHOLPlayerMatchStatsState& Player)
	{
		TArray<FHOLStatsCell> Cells;
		Cells.Add(NumberCell(Player.UnitsKilled));
		Cells.Add(NumberCell(Player.UnitsLost));
		Cells.Add(NumberCell(Player.BuildingsLost));
		Cells.Add(NumberCell(Player.MaxMilitary));
		return Cells;
	}

	// Economy after the name: food, wood, stone, gold gathered, units trained, workers max, population max.
	TArray<FHOLStatsCell> EconomyRow(const FHOLPlayerMatchStatsState& Player)
	{
		TArray<FHOLStatsCell> Cells;
		Cells.Add(NumberCell(Player.Gathered.Food));
		Cells.Add(NumberCell(Player.Gathered.Wood));
		Cells.Add(NumberCell(Player.Gathered.Stone));
		Cells.Add(NumberCell(Player.Gathered.Gold));
		Cells.Add(NumberCell(Player.UnitsTrained));
		Cells.Add(NumberCell(Player.MaxWorkers));
		Cells.Add(NumberCell(Player.MaxPopulation));
		return Cells;
	}

	// The cells after the name of the table Tab (0 Summary, 1 Military, 2 Economy); empty out of range.
	TArray<FHOLStatsCell> TableRow(int Tab, int Team, int Result, const FHOLPlayerMatchStatsState& Player)
	{
		if (Tab == 0)
			return SummaryRow(Team, Result, Player);
		if (Tab == 1)
			return MilitaryRow(Player);
		if (Tab == 2)
			return EconomyRow(Player);
		TArray<FHOLStatsCell> NoCells;
		return NoCells;
	}

	// The index in Rows of the row of a slot, -1 when the slot has none (a closed slot).
	int FindRow(const TArray<FHOLStatsRowState>& Rows, int SlotIndex)
	{
		for (int Index = 0; Index < Rows.Num(); Index++)
		{
			if (Rows[Index].SlotIndex == SlotIndex)
				return Index;
		}
		return -1;
	}

	// The curves of a series: one a row (a slot held by a human or an AI), in the rows' order, in the player's name and colour, the points of
	// HOLMatchStatsRead::MakeCurve for that series; a slot with no statistics yet gets an empty curve (as the C++ RefreshTimeline).
	TArray<FHOLStatsGraphCurve> BuildCurves(int Series, const TArray<FHOLStatsRowState>& Rows, const FHOLMatchStatsState& Stats)
	{
		const int Chosen = ClampSeries(Series);
		TArray<FHOLStatsGraphCurve> Curves;
		for (int Index = 0; Index < Rows.Num(); Index++)
		{
			FHOLStatsGraphCurve Curve;
			Curve.Name = Rows[Index].Name;
			Curve.Color = Rows[Index].Color;
			if (Stats.Players.IsValidIndex(Rows[Index].SlotIndex))
			{
				FHOLPlayerMatchStatsState Player = Stats.Players[Rows[Index].SlotIndex];
				Curve.Points = HOLMatchStatsRead::MakeCurve(Player, Chosen);
			}
			Curves.Add(Curve);
		}
		return Curves;
	}

	// The dated marks: one a recorded event (defeat, fallen forum), in the colour of its player and labelled by the base (UHOLStatsScreen::MakeMarkLabel).
	// MANQUE: the C++ named any slot (HOLForumMessages::GetPlayerName); here a slot without a row (closed) gets white and an empty name.
	TArray<FHOLStatsGraphMark> BuildMarks(const TArray<FHOLStatsRowState>& Rows, const FHOLMatchStatsState& Stats)
	{
		TArray<FHOLStatsGraphMark> Marks;
		for (int Index = 0; Index < Stats.Events.Num(); Index++)
		{
			FHOLStatsGraphMark Mark;
			Mark.TimeSeconds = Stats.Events[Index].TimeSeconds;
			FText PlayerName = FText::FromString("");
			Mark.Color = FLinearColor::White;
			const int RowIndex = FindRow(Rows, Stats.Events[Index].PlayerIndex);
			if (RowIndex >= 0)
			{
				PlayerName = Rows[RowIndex].Name;
				Mark.Color = Rows[RowIndex].Color;
			}
			Mark.Label = UHOLStatsScreen::MakeMarkLabel(Stats.Events[Index].Kind, PlayerName);
			Marks.Add(Mark);
		}
		return Marks;
	}
}
