// Port of HOLMatchStatsTests.cpp (HeroesOfLiberty.Match.Stats.Rules, .Send, .Scores, .Curves) against UHOLMatchStatsReadLibrary (L17-11, ELROND-1117c).
// Same vectors, names adapted. The Record*/AddSample/chunk/MakeCurve halves (Test_MatchStats_Record, _Samples, _Send, _Curve) go through the
// FHOLMatchStatsState copies of the library (S1g suite, SAURON-1117g: they compile only after the build that adds them). The whole C++ test
// HeroesOfLiberty.Match.Stats.* can go once these four are green, then red once (a value falsified).

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

// Deaths: a loss for the victim, a kill for another player only; the slots grow; the tables (HOLMatchStatsTests.cpp, Rules).
void Test_MatchStats_Record(FUnitTest& T)
{
	FHOLMatchStatsState Stats;
	HOLMatchStatsRead::RecordUnitDeath(Stats, 1, true, 0);
	HOLMatchStatsRead::RecordUnitDeath(Stats, 1, false, 0);
	HOLMatchStatsRead::RecordUnitDeath(Stats, 0, false, -1);
	HOLMatchStatsRead::RecordUnitDeath(Stats, 0, false, 0);
	T.AssertEquals(2, Stats.Players.Num(), "two slots");
	T.AssertEquals(2, Stats.Players[0].UnitsKilled, "slot 0 killed 2 (not its own unit, not nobody's)");
	T.AssertEquals(2, Stats.Players[0].UnitsLost, "slot 0 lost 2");
	T.AssertEquals(2, Stats.Players[1].UnitsLost, "slot 1 lost 2");
	T.AssertEquals(1, Stats.Players[1].WorkersLost, "slot 1 lost 1 worker");
	T.AssertEquals(0, Stats.Players[1].UnitsKilled, "slot 1 killed none");

	HOLMatchStatsRead::RecordUnitTrained(Stats, 3);
	HOLMatchStatsRead::RecordBuildingLost(Stats, 3);
	T.AssertEquals(4, Stats.Players.Num(), "slot 3 added");
	T.AssertEquals(-1, Stats.Players[2].PlayerIndex, "slot 2 unknown");
	T.AssertEquals(3, Stats.Players[3].PlayerIndex, "slot 3 named");
	T.AssertEquals(1, Stats.Players[3].UnitsTrained, "slot 3 trained 1");
	T.AssertEquals(1, Stats.Players[3].BuildingsLost, "slot 3 lost a building");

	HOLMatchStatsRead::RecordUnitTrained(Stats, -1);
	HOLMatchStatsRead::RecordBuildingLost(Stats, -1);
	T.AssertEquals(4, Stats.Players.Num(), "no slot: nothing recorded");

	HOLMatchStatsRead::RecordEvent(Stats, 1, 0, 125);
	T.AssertEquals(1, Stats.Events.Num(), "one event");
	T.AssertEquals(125, Stats.Events[0].TimeSeconds, "its time");
	T.AssertEquals(1, Stats.Events[0].PlayerIndex, "its player");
	T.AssertEquals(0, Stats.Events[0].Kind, "Defeated");
	HOLMatchStatsRead::RecordEvent(Stats, 1, 1, 130);
	T.AssertEquals(1, Stats.Events[1].Kind, "ForumFell");
	HOLMatchStatsRead::RecordEvent(Stats, 1, 9, 140);
	T.AssertEquals(2, Stats.Events.Num(), "an unknown kind: nothing recorded");
}

// Samples: the curves grow, the maxima keep the highest, the gathered totals are the last sample's.
void Test_MatchStats_Samples(FUnitTest& T)
{
	FHOLMatchStatsState Stats;
	FHOLMatchStatsSampleState First;
	First.TimeSeconds = 0;
	First.Workers = 7;
	First.Military = 1;
	First.Population = 8;
	First.Gathered = MSResources(10, 0, 0, 0);
	FHOLMatchStatsSampleState Second = First;
	Second.TimeSeconds = 10;
	Second.Workers = 5;
	Second.Military = 4;
	Second.Population = 9;
	Second.Gathered = MSResources(30, 20, 0, 0);
	HOLMatchStatsRead::AddSample(Stats, 0, First);
	HOLMatchStatsRead::AddSample(Stats, 0, Second);
	T.AssertEquals(1, Stats.Players.Num(), "one slot");
	T.AssertEquals(2, Stats.Players[0].Samples.Num(), "two samples");
	T.AssertEquals(10, Stats.Players[0].Samples[1].TimeSeconds, "the second sample kept");
	T.AssertEquals(7, Stats.Players[0].MaxWorkers, "max workers");
	T.AssertEquals(4, Stats.Players[0].MaxMilitary, "max military");
	T.AssertEquals(9, Stats.Players[0].MaxPopulation, "max population");
	T.AssertEquals(30, Stats.Players[0].Gathered.Food, "gathered from the last sample: food");
	T.AssertEquals(20, Stats.Players[0].Gathered.Wood, "gathered from the last sample: wood");
	HOLMatchStatsRead::AddSample(Stats, -1, First);
	T.AssertEquals(1, Stats.Players.Num(), "no slot: no sample");
}

// A server state: two players, 5 samples each (Workers = Index + Player), a death, three events.
FHOLMatchStatsState MSServer()
{
	FHOLMatchStatsState Server;
	Server.SampleIntervalSeconds = 10;
	for (int Index = 0; Index < 5; ++Index)
	{
		for (int Player = 0; Player < 2; ++Player)
		{
			FHOLMatchStatsSampleState Sample;
			Sample.TimeSeconds = Index * 10;
			Sample.Workers = Index + Player;
			HOLMatchStatsRead::AddSample(Server, Player, Sample);
		}
	}
	HOLMatchStatsRead::RecordUnitDeath(Server, 1, false, 0);
	HOLMatchStatsRead::RecordEvent(Server, 1, 1, 20);
	HOLMatchStatsRead::RecordEvent(Server, 1, 1, 30);
	HOLMatchStatsRead::RecordEvent(Server, 1, 0, 40);
	return Server;
}

// The chunks a client asks for and merges (HOLMatchStatsTests.cpp, Send): four chunks of 2 samples, the last asked again each time.
void Test_MatchStats_Send(FUnitTest& T)
{
	FHOLMatchStatsState Server = MSServer();
	FHOLMatchStatsState Copy;
	T.AssertEquals(0, HOLMatchStatsRead::GetNextFromSample(Copy), "an empty copy asks from 0");
	FHOLMatchStatsChunkState Chunk = HOLMatchStatsRead::MakeChunk(Server, 0, 0, 2);
	T.AssertFalse(Chunk.bComplete, "first chunk incomplete");
	T.AssertEquals(2, Chunk.Stats.Players.Num(), "first chunk: both players");
	if (Chunk.Stats.Players.Num() != 2)
	{
		return;
	}
	T.AssertEquals(2, Chunk.Stats.Players[1].Samples.Num(), "first chunk: 2 samples");
	T.AssertEquals(1, Chunk.Stats.Players[0].UnitsKilled, "first chunk: the tables");
	T.AssertEquals(3, Chunk.Stats.Events.Num(), "first chunk: every event");
	HOLMatchStatsRead::MergeChunk(Copy, Chunk);
	T.AssertEquals(1, HOLMatchStatsRead::GetNextFromSample(Copy), "then asks again from its last sample");
	int Chunks = 1;
	while (!Chunk.bComplete && Chunks < 10)
	{
		Chunk = HOLMatchStatsRead::MakeChunk(Server, HOLMatchStatsRead::GetNextFromSample(Copy), Copy.Events.Num(), 2);
		HOLMatchStatsRead::MergeChunk(Copy, Chunk);
		++Chunks;
	}
	T.AssertEquals(4, Chunks, "four chunks (the last sample asked again each time)");
	T.AssertEquals(2, Copy.Players.Num(), "the copy: both players");
	if (Copy.Players.Num() != 2)
	{
		return;
	}
	T.AssertEquals(5, Copy.Players[1].Samples.Num(), "the copy is the server's: 5 samples");
	for (int Index = 0; Index < Copy.Players[1].Samples.Num() && Index < 5; ++Index)
	{
		T.AssertEquals(Server.Players[1].Samples[Index].Workers, Copy.Players[1].Samples[Index].Workers, "the copy is the server's: workers of a sample");
		T.AssertEquals(Server.Players[1].Samples[Index].TimeSeconds, Copy.Players[1].Samples[Index].TimeSeconds, "the copy is the server's: time of a sample");
	}
	T.AssertEquals(3, Copy.Events.Num(), "the copy is the server's: events");
	T.AssertEquals(10, Copy.SampleIntervalSeconds, "the copy is the server's: interval");

	// The server replaces its last sample (a dump at the same time) and adds one: the copy follows.
	Server.Players[0].Samples[4].Workers = 99;
	FHOLMatchStatsSampleState Later;
	Later.TimeSeconds = 50;
	HOLMatchStatsRead::AddSample(Server, 0, Later);
	HOLMatchStatsRead::RecordUnitTrained(Server, 0);
	Chunk = HOLMatchStatsRead::MakeChunk(Server, HOLMatchStatsRead::GetNextFromSample(Copy), Copy.Events.Num(), 2);
	T.AssertTrue(Chunk.bComplete, "one chunk");
	T.AssertEquals(0, Chunk.Stats.Events.Num(), "no event again");
	HOLMatchStatsRead::MergeChunk(Copy, Chunk);
	T.AssertEquals(6, Copy.Players[0].Samples.Num(), "the added sample");
	T.AssertEquals(99, Copy.Players[0].Samples[4].Workers, "the replaced sample");
	T.AssertEquals(50, Copy.Players[0].Samples[5].TimeSeconds, "the new sample");
	T.AssertEquals(5, Copy.Players[1].Samples.Num(), "the other player's unchanged");
	T.AssertEquals(1, Copy.Players[0].UnitsTrained, "the tables refreshed");
	T.AssertEquals(3, Copy.Events.Num(), "events kept");
}

// One point a sample, in their order: X the match time, Y the value (HOLMatchStatsTests.cpp, Curves).
void Test_MatchStats_Curve(FUnitTest& T)
{
	FHOLPlayerMatchStatsState Player;
	FHOLMatchStatsSampleState First;
	First.Workers = 3;
	FHOLMatchStatsSampleState Second;
	Second.TimeSeconds = 20;
	Second.Workers = 7;
	Second.Military = 3;
	Player.Samples.Add(First);
	Player.Samples.Add(Second);
	TArray<FVector2D> Curve = HOLMatchStatsRead::MakeCurve(Player, 0);
	T.AssertEquals(2, Curve.Num(), "a point a sample");
	if (Curve.Num() == 2)
	{
		T.AssertEquals(0.0, Curve[0].X, "first point: time");
		T.AssertEquals(3.0, Curve[0].Y, "first point: workers");
		T.AssertEquals(20.0, Curve[1].X, "second point: time");
		T.AssertEquals(7.0, Curve[1].Y, "second point: workers");
	}
	TArray<FVector2D> Military = HOLMatchStatsRead::MakeCurve(Player, 1);
	if (Military.Num() == 2)
	{
		T.AssertEquals(3.0, Military[1].Y, "military series");
	}
	T.AssertEquals(0, HOLMatchStatsRead::MakeCurve(FHOLPlayerMatchStatsState(), 1).Num(), "no sample, no point");
}
