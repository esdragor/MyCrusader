// The statistics screen in script (D851, ST-2): UHOLStatsScreen::FindScriptClass finds the class of Script/UI/HOLStatsScreen.as; the figures a table row shows
// and the curves a series gives the graph are the pure rules of Script/UI/HOLStatsRules.as, tested here without a screen from values written out in full.
// (Scores: 10 points an enemy unit killed; economy a point every 10 resources gathered and 5 points a unit trained: HOLMatchStats::GetMilitaryScore / GetEconomyScore.)

FHOLStatsRowState STS_MakeRow(int SlotIndex, FString Name, int Team, int Result)
{
	FHOLStatsRowState Row;
	Row.SlotIndex = SlotIndex;
	Row.Name = FText::FromString(Name);
	Row.Color = FLinearColor(1.0, 0.0, 0.0, 1.0);
	Row.Team = Team;
	Row.Result = Result;
	return Row;
}

// A player with one sample at 10 s: 7 workers, 3 military, 12 population, 5 buildings.
FHOLPlayerMatchStatsState STS_MakePlayer(int PlayerIndex)
{
	FHOLPlayerMatchStatsState Player;
	Player.PlayerIndex = PlayerIndex;
	FHOLMatchStatsSampleState Sample;
	Sample.TimeSeconds = 10;
	Sample.Workers = 7;
	Sample.Military = 3;
	Sample.Population = 12;
	Sample.Buildings = 5;
	Player.Samples.Add(Sample);
	return Player;
}

void Test_StatsScriptClass(FUnitTest& T)
{
	const TSubclassOf<UHOLStatsScreen> Found = UHOLStatsScreen::FindScriptClass();
	T.AssertTrue(Found.IsValid(), "the statistics screen has a script class");
	if (Found.IsValid())
		T.AssertEquals(Found.Get().GetName().ToString(), "HOLStatsScreenScript", "the statistics screen's script class is HOLStatsScreen.as's");
}

void Test_StatsSummaryRow(FUnitTest& T)
{
	// 4 enemy units killed, 200 resources gathered (100 food, 50 wood, 30 stone, 20 gold), 3 units trained, in team 2, winner.
	FHOLPlayerMatchStatsState Player;
	Player.UnitsKilled = 4;
	Player.UnitsTrained = 3;
	Player.Gathered.Food = 100;
	Player.Gathered.Wood = 50;
	Player.Gathered.Stone = 30;
	Player.Gathered.Gold = 20;
	TArray<FHOLStatsCell> Cells = HOLStatsRules::SummaryRow(2, 1, Player);
	T.AssertEquals(Cells.Num(), 5, "the summary shows 5 columns after the name: team, result, military score, economy score, total");
	if (Cells.Num() != 5)
		return;
	const int Team = Cells[0].Value;
	const bool bDash = Cells[0].bDash;
	const FName ResultKey = Cells[1].Key;
	const int MilitaryScore = Cells[2].Value;
	const int EconomyScore = Cells[3].Value;
	const int Total = Cells[4].Value;
	T.AssertEquals(Team, 2, "the team is shown");
	T.AssertTrue(!bDash, "a player in a team has no dash");
	T.AssertTrue(ResultKey == n"Winner", "result 1 shows the winner's text");
	T.AssertEquals(MilitaryScore, 40, "4 units killed make 40 military points");
	T.AssertEquals(EconomyScore, 35, "200 resources gathered and 3 units trained make 20 + 15 economy points");
	T.AssertEquals(Total, 75, "the total adds the two scores");

	// No team: a dash; a defeated player and a player still in the game name their result.
	TArray<FHOLStatsCell> NoTeam = HOLStatsRules::SummaryRow(0, 2, Player);
	const bool bNoTeamDash = NoTeam[0].bDash;
	const FName DefeatedKey = NoTeam[1].Key;
	T.AssertTrue(bNoTeamDash, "team 0 shows a dash");
	T.AssertTrue(DefeatedKey == n"Defeated", "result 2 shows the defeated text");
	T.AssertTrue(HOLStatsRules::ResultKey(0) == n"Playing", "result 0 shows the in-game text");

	// The two other tables: 4 military columns, 7 economy columns, the economy one in the order food, wood, stone, gold.
	Player.UnitsLost = 9;
	Player.BuildingsLost = 2;
	Player.MaxMilitary = 30;
	Player.MaxWorkers = 15;
	Player.MaxPopulation = 60;
	TArray<FHOLStatsCell> Military = HOLStatsRules::TableRow(1, 2, 1, Player);
	TArray<FHOLStatsCell> Economy = HOLStatsRules::TableRow(2, 2, 1, Player);
	T.AssertEquals(Military.Num(), 4, "the military table shows 4 columns after the name");
	T.AssertEquals(Economy.Num(), 7, "the economy table shows 7 columns after the name");
	if (Military.Num() == 4 && Economy.Num() == 7)
	{
		const int Killed = Military[0].Value;
		const int Lost = Military[1].Value;
		const int MaxMilitary = Military[3].Value;
		const int Wood = Economy[1].Value;
		const int Gold = Economy[3].Value;
		const int MaxPopulation = Economy[6].Value;
		T.AssertEquals(Killed, 4, "military column 1 is the units killed");
		T.AssertEquals(Lost, 9, "military column 2 is the units lost");
		T.AssertEquals(MaxMilitary, 30, "military column 4 is the army max");
		T.AssertEquals(Wood, 50, "economy column 2 is the wood gathered");
		T.AssertEquals(Gold, 20, "economy column 4 is the gold gathered");
		T.AssertEquals(MaxPopulation, 60, "economy column 7 is the population max");
	}
	TArray<FHOLStatsCell> Third = HOLStatsRules::TableRow(3, 2, 1, Player);
	T.AssertEquals(Third.Num(), 0, "the timeline page has no table row");
}

void Test_StatsTimelineSeries(FUnitTest& T)
{
	// Two rows (slots 0 and 2, slot 1 closed) and a third whose slot has no statistics yet.
	TArray<FHOLStatsRowState> Rows;
	Rows.Add(STS_MakeRow(0, "Ada", 1, 0));
	Rows.Add(STS_MakeRow(2, "Bob", 2, 0));
	Rows.Add(STS_MakeRow(5, "Cyd", 0, 0));
	FHOLMatchStatsState Stats;
	Stats.Players.Add(STS_MakePlayer(0));
	Stats.Players.Add(STS_MakePlayer(1));
	Stats.Players.Add(STS_MakePlayer(2));

	// The series picked decides the points of every curve, a curve a row in the rows' order.
	TArray<FHOLStatsGraphCurve> Workers = HOLStatsRules::BuildCurves(0, Rows, Stats);
	TArray<FHOLStatsGraphCurve> Military = HOLStatsRules::BuildCurves(1, Rows, Stats);
	T.AssertEquals(Workers.Num(), 3, "the workers series gives a curve a row");
	T.AssertEquals(Military.Num(), 3, "the military series gives a curve a row");
	if (Workers.Num() != 3 || Military.Num() != 3 || Workers[0].Points.Num() == 0 || Military[0].Points.Num() == 0)
	{
		T.AssertTrue(false, "the first curve has points");
		return;
	}
	const double WorkersLast = Workers[0].Points[Workers[0].Points.Num() - 1].Y;
	const double MilitaryLast = Military[0].Points[Military[0].Points.Num() - 1].Y;
	T.AssertEquals(Math::RoundToInt(WorkersLast), 7, "the workers series draws the 7 workers of the sample");
	T.AssertEquals(Math::RoundToInt(MilitaryLast), 3, "the military series draws the 3 military of the sample");
	const FString SecondName = Workers[1].Name.ToString();
	T.AssertEquals(SecondName, "Bob", "the curves follow the rows' order and carry the players' names");
	// Slot 2's statistics are the third of the table: the row of slot 2 draws the player 2's curve.
	const int SecondPoints = Workers[1].Points.Num();
	T.AssertEquals(SecondPoints, Workers[0].Points.Num(), "slot 2 has its own statistics, as many points as slot 0's");
	// Slot 5 has no statistics yet: its curve is there, empty.
	const int ThirdPoints = Workers[2].Points.Num();
	T.AssertEquals(ThirdPoints, 0, "a slot with no statistics yet gets an empty curve");
	// Out of range: the series is kept in 0..5 (the stock series is the last).
	TArray<FHOLStatsGraphCurve> Past = HOLStatsRules::BuildCurves(99, Rows, Stats);
	TArray<FHOLStatsGraphCurve> Last = HOLStatsRules::BuildCurves(5, Rows, Stats);
	const int PastNum = Past.Num();
	T.AssertEquals(PastNum, Last.Num(), "a series past the last one is the last one");
	T.AssertEquals(HOLStatsRules::ClampSeries(-3), 0, "a negative series is the first one");

	// The dated marks: a defeat of slot 0, labelled and in the colour of its row.
	Stats.Events.Add(FHOLMatchStatsEventState());
	Stats.Events[0].TimeSeconds = 90;
	Stats.Events[0].PlayerIndex = 0;
	Stats.Events[0].Kind = 0;
	TArray<FHOLStatsGraphMark> Marks = HOLStatsRules::BuildMarks(Rows, Stats);
	T.AssertEquals(Marks.Num(), 1, "an event is a mark");
	if (Marks.Num() == 1)
	{
		const int MarkTime = Marks[0].TimeSeconds;
		const float32 MarkRed = Marks[0].Color.R;
		T.AssertEquals(MarkTime, 90, "the mark is at the event's time");
		T.AssertEquals(Math::RoundToInt(MarkRed), 1, "the mark takes the colour of the player's row");
	}
}
