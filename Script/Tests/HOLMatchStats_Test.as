// Port of HOLMatchStatsTests.cpp (HeroesOfLiberty.Match.Stats.Rules, .Send, .Scores, .Curves) against UHOLMatchStatsReadLibrary (L17-11, ELROND-1117c).
// Same vectors, names adapted. NOT ported (the library does not expose them): RecordUnitDeath/RecordUnitTrained/RecordBuildingLost/RecordEvent/
// AddSample, MakeChunk/MergeChunk/GetNextFromSample (FHOLMatchStats, FHOLPlayerMatchStats, FHOLMatchStatsChunk), MakeCurve (FVector2D points).
// So the Rules test keeps its Record* half, the Send test its chunk half, the Curves test its MakeCurve half in C++.

FHOLResources MSResources(int Food, int Wood, int Stone, int Gold)
{
	FHOLResources Result;
	Result.Food = Food;
	Result.Wood = Wood;
	Result.Stone = Stone;
	Result.Gold = Gold;
	return Result;
}

// Sampling: 10 s at 0.05 s a step is every 200 steps, step 0 included; never less than one step.
void Test_MatchStats_Sampling(FUnitTest& T)
{
	T.AssertEquals(200, HOLMatchStatsRead::GetSampleSteps(0.05f, 10), "10 s at 20 steps a second");
	T.AssertEquals(1, HOLMatchStatsRead::GetSampleSteps(0.0f, 10), "no step length: every step");
	T.AssertEquals(1, HOLMatchStatsRead::GetSampleSteps(5.0f, 1), "an interval under a step: every step");
	T.AssertTrue(HOLMatchStatsRead::IsSampleStep(0, 200), "step 0 sampled");
	T.AssertFalse(HOLMatchStatsRead::IsSampleStep(199, 200), "step 199 not");
	T.AssertTrue(HOLMatchStatsRead::IsSampleStep(400, 200), "step 400 sampled");
}

// Who gets the statistics (M6-Q11).
void Test_MatchStats_CanReceive(FUnitTest& T)
{
	T.AssertFalse(HOLMatchStatsRead::CanReceiveStats(false, false), "a player during the match: refused");
	T.AssertTrue(HOLMatchStatsRead::CanReceiveStats(true, false), "a spectator during the match");
	T.AssertTrue(HOLMatchStatsRead::CanReceiveStats(false, true), "a player after the end");
	T.AssertTrue(HOLMatchStatsRead::CanReceiveStats(true, true), "a spectator after the end");
}

// The scores of the statistics screen (the library takes the counts, not the player table).
void Test_MatchStats_Scores(FUnitTest& T)
{
	T.AssertEquals(0, HOLMatchStatsRead::GetMilitaryScore(0), "nothing done: no military score");
	T.AssertEquals(0, HOLMatchStatsRead::GetEconomyScore(FHOLResources(), 0), "nothing done: no economy score");
	T.AssertEquals(70, HOLMatchStatsRead::GetMilitaryScore(7), "10 a kill, losses not counted");
	T.AssertEquals(39, HOLMatchStatsRead::GetEconomyScore(MSResources(120, 85, 30, 9), 3), "244 gathered is 24, 3 trained 15");
	T.AssertEquals(70, HOLMatchStatsRead::GetMilitaryScore(7), "military unchanged by the economy");
}

// The curves of the Chronologie: a sample values (series 0 Workers, 1 Military, 2 Population, 3 Buildings, 4 Gathered, 5 Stock).
void Test_MatchStats_SeriesValues(FUnitTest& T)
{
	FHOLResources Gathered = MSResources(120, 85, 30, 9);
	FHOLResources Stock = MSResources(50, 10, 0, 5);
	T.AssertEquals(7, HOLMatchStatsRead::GetSeriesValue(7, 3, 11, 4, Gathered, Stock, 0), "workers");
	T.AssertEquals(3, HOLMatchStatsRead::GetSeriesValue(7, 3, 11, 4, Gathered, Stock, 1), "military");
	T.AssertEquals(11, HOLMatchStatsRead::GetSeriesValue(7, 3, 11, 4, Gathered, Stock, 2), "population");
	T.AssertEquals(4, HOLMatchStatsRead::GetSeriesValue(7, 3, 11, 4, Gathered, Stock, 3), "buildings");
	T.AssertEquals(244, HOLMatchStatsRead::GetSeriesValue(7, 3, 11, 4, Gathered, Stock, 4), "gathered: the four together");
	T.AssertEquals(65, HOLMatchStatsRead::GetSeriesValue(7, 3, 11, 4, Gathered, Stock, 5), "stock: the four together");
}

// Round steps: 1, 2, 5 x 10^n, MaxTicks at most over the range; time steps a clock reads; the axis end; the clock text.
void Test_MatchStats_Axes(FUnitTest& T)
{
	T.AssertEquals(2, HOLMatchStatsRead::GetNiceStep(7, 5, false), "0..7 in 5 ticks: 2");
	T.AssertEquals(2, HOLMatchStatsRead::GetNiceStep(10, 5, false), "0..10 in 5 ticks: 2");
	T.AssertEquals(1, HOLMatchStatsRead::GetNiceStep(3, 5, false), "0..3 in 5 ticks: 1");
	T.AssertEquals(1, HOLMatchStatsRead::GetNiceStep(0, 5, false), "nothing: 1");
	T.AssertEquals(50, HOLMatchStatsRead::GetNiceStep(244, 5, false), "0..244 in 5 ticks: 50");
	T.AssertEquals(500, HOLMatchStatsRead::GetNiceStep(1300, 5, false), "0..1300 in 5 ticks: 500");
	T.AssertEquals(50000, HOLMatchStatsRead::GetNiceStep(90000, 4, false), "0..90000 in 4 ticks: 50000");
	T.AssertEquals(10, HOLMatchStatsRead::GetNiceStep(35, 6, true), "35 s in 6 ticks: 10 s");
	T.AssertEquals(15, HOLMatchStatsRead::GetNiceStep(80, 6, true), "80 s in 6 ticks: 15 s");
	T.AssertEquals(60, HOLMatchStatsRead::GetNiceStep(300, 6, true), "5 min in 6 ticks: 1 min");
	T.AssertEquals(600, HOLMatchStatsRead::GetNiceStep(2400, 6, true), "40 min in 6 ticks: 10 min");
	T.AssertEquals(1800, HOLMatchStatsRead::GetNiceStep(10800, 6, true), "3 h in 6 ticks: 30 min");
	T.AssertEquals(1, HOLMatchStatsRead::GetNiceStep(5, 6, true), "5 s in 6 ticks: 1 s");

	T.AssertEquals(8, HOLMatchStatsRead::GetAxisEnd(7, 2, false), "7 by 2: 8");
	T.AssertEquals(10, HOLMatchStatsRead::GetAxisEnd(10, 2, false), "10 by 2: 10");
	T.AssertEquals(50, HOLMatchStatsRead::GetAxisEnd(0, 50, false), "nothing: one step");
	T.AssertEquals(12, HOLMatchStatsRead::GetAxisEnd(10, 2, true), "10 by 2, headroom: 12");
	T.AssertEquals(8, HOLMatchStatsRead::GetAxisEnd(7, 2, true), "7 by 2, headroom: 8");
	T.AssertEquals(300, HOLMatchStatsRead::GetAxisEnd(250, 50, true), "250 by 50, headroom: 300");
	T.AssertEquals(2, HOLMatchStatsRead::GetAxisEnd(1, 1, true), "1 by 1, headroom: 2");
	T.AssertEquals(50, HOLMatchStatsRead::GetAxisEnd(0, 50, true), "nothing, headroom: one step");

	T.AssertTrue(HOLMatchStatsRead::FormatMatchTime(0) == "0:00", "0:00");
	T.AssertTrue(HOLMatchStatsRead::FormatMatchTime(65) == "1:05", "1:05");
	T.AssertTrue(HOLMatchStatsRead::FormatMatchTime(750) == "12:30", "12:30");
}
