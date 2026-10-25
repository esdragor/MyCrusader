// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLMatchStats.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// C2: the pure rules of the match statistics (aggregation, sampling), apart from any world.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMatchStatsRulesTest, "HeroesOfLiberty.Match.Stats.Rules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHOLMatchStatsRulesTest::RunTest(const FString& Parameters)
{
	// Sampling: 10 s at 0.05 s a step is every 200 steps, step 0 included; never less than one step.
	TestEqual(TEXT("10 s at 20 steps a second"), HOLMatchStats::GetSampleSteps(0.05f, 10), 200);
	TestEqual(TEXT("no step length: every step"), HOLMatchStats::GetSampleSteps(0.f, 10), 1);
	TestEqual(TEXT("an interval under a step: every step"), HOLMatchStats::GetSampleSteps(5.f, 1), 1);
	TestTrue(TEXT("step 0 sampled"), HOLMatchStats::IsSampleStep(0, 200));
	TestFalse(TEXT("step 199 not"), HOLMatchStats::IsSampleStep(199, 200));
	TestTrue(TEXT("step 400 sampled"), HOLMatchStats::IsSampleStep(400, 200));

	// Deaths: a loss for the victim, a kill for another player only.
	FHOLMatchStats Stats;
	HOLMatchStats::RecordUnitDeath(Stats, 1, true, 0);
	HOLMatchStats::RecordUnitDeath(Stats, 1, false, 0);
	HOLMatchStats::RecordUnitDeath(Stats, 0, false, INDEX_NONE);
	HOLMatchStats::RecordUnitDeath(Stats, 0, false, 0);
	TestEqual(TEXT("two slots"), Stats.Players.Num(), 2);
	TestEqual(TEXT("slot 0 killed 2 (not its own unit, not nobody's)"), Stats.Players[0].UnitsKilled, 2);
	TestEqual(TEXT("slot 0 lost 2"), Stats.Players[0].UnitsLost, 2);
	TestEqual(TEXT("slot 1 lost 2"), Stats.Players[1].UnitsLost, 2);
	TestEqual(TEXT("slot 1 lost 1 worker"), Stats.Players[1].WorkersLost, 1);
	TestEqual(TEXT("slot 1 killed none"), Stats.Players[1].UnitsKilled, 0);

	HOLMatchStats::RecordUnitTrained(Stats, 3);
	HOLMatchStats::RecordBuildingLost(Stats, 3);
	TestEqual(TEXT("slot 3 added"), Stats.Players.Num(), 4);
	TestEqual(TEXT("slot 2 unknown"), Stats.Players[2].PlayerIndex, static_cast<int32>(INDEX_NONE));
	TestEqual(TEXT("slot 3 trained 1"), Stats.Players[3].UnitsTrained, 1);
	TestEqual(TEXT("slot 3 lost a building"), Stats.Players[3].BuildingsLost, 1);

	// Samples: the curves grow, the maxima keep the highest, the gathered totals are the last sample's.
	FHOLMatchStatsSample First;
	First.TimeSeconds = 0;
	First.Workers = 7;
	First.Military = 1;
	First.Population = 8;
	First.Gathered = FHOLResources(10, 0, 0, 0);
	FHOLMatchStatsSample Second = First;
	Second.TimeSeconds = 10;
	Second.Workers = 5;
	Second.Military = 4;
	Second.Population = 9;
	Second.Gathered = FHOLResources(30, 20, 0, 0);
	HOLMatchStats::AddSample(Stats, 0, First);
	HOLMatchStats::AddSample(Stats, 0, Second);
	TestEqual(TEXT("two samples"), Stats.Players[0].Samples.Num(), 2);
	TestEqual(TEXT("max workers"), Stats.Players[0].MaxWorkers, 7);
	TestEqual(TEXT("max military"), Stats.Players[0].MaxMilitary, 4);
	TestEqual(TEXT("max population"), Stats.Players[0].MaxPopulation, 9);
	TestTrue(TEXT("gathered from the last sample"), Stats.Players[0].Gathered == FHOLResources(30, 20, 0, 0));

	HOLMatchStats::RecordEvent(Stats, 1, EHOLMatchStatsEvent::Defeated, 125);
	TestEqual(TEXT("one event"), Stats.Events.Num(), 1);
	TestEqual(TEXT("its time"), Stats.Events[0].TimeSeconds, 125);
	return true;
}

// C3: who gets the statistics (M6-Q11), and the chunks a client asks for and merges.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMatchStatsSendTest, "HeroesOfLiberty.Match.Stats.Send",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHOLMatchStatsSendTest::RunTest(const FString& Parameters)
{
	TestFalse(TEXT("a player during the match: refused"), HOLMatchStats::CanReceiveStats(false, false));
	TestTrue(TEXT("a spectator during the match"), HOLMatchStats::CanReceiveStats(true, false));
	TestTrue(TEXT("a player after the end"), HOLMatchStats::CanReceiveStats(false, true));
	TestTrue(TEXT("a spectator after the end"), HOLMatchStats::CanReceiveStats(true, true));

	// The server: two players, 5 samples each, 3 events.
	FHOLMatchStats Server;
	Server.SampleIntervalSeconds = 10;
	for (int32 Index = 0; Index < 5; ++Index)
	{
		for (int32 Player = 0; Player < 2; ++Player)
		{
			FHOLMatchStatsSample Sample;
			Sample.TimeSeconds = Index * 10;
			Sample.Workers = Index + Player;
			HOLMatchStats::AddSample(Server, Player, Sample);
		}
	}
	HOLMatchStats::RecordUnitDeath(Server, 1, false, 0);
	HOLMatchStats::RecordEvent(Server, 1, EHOLMatchStatsEvent::ForumFell, 20);
	HOLMatchStats::RecordEvent(Server, 1, EHOLMatchStatsEvent::ForumFell, 30);
	HOLMatchStats::RecordEvent(Server, 1, EHOLMatchStatsEvent::Defeated, 40);

	// A client with nothing asks from 0, 2 samples a chunk: three chunks.
	FHOLMatchStats Copy;
	TestEqual(TEXT("an empty copy asks from 0"), HOLMatchStats::GetNextFromSample(Copy), 0);
	FHOLMatchStatsChunk Chunk = HOLMatchStats::MakeChunk(Server, 0, 0, 2);
	TestFalse(TEXT("first chunk incomplete"), Chunk.bComplete);
	if (!TestEqual(TEXT("first chunk: both players"), Chunk.Stats.Players.Num(), 2))
	{
		return false;
	}
	TestEqual(TEXT("first chunk: 2 samples"), Chunk.Stats.Players[1].Samples.Num(), 2);
	TestEqual(TEXT("first chunk: the tables"), Chunk.Stats.Players[0].UnitsKilled, 1);
	TestEqual(TEXT("first chunk: every event"), Chunk.Stats.Events.Num(), 3);
	HOLMatchStats::MergeChunk(Copy, Chunk);
	TestEqual(TEXT("then asks again from its last sample"), HOLMatchStats::GetNextFromSample(Copy), 1);
	int32 Chunks = 1;
	while (!Chunk.bComplete && Chunks < 10)
	{
		Chunk = HOLMatchStats::MakeChunk(Server, HOLMatchStats::GetNextFromSample(Copy), Copy.Events.Num(), 2);
		HOLMatchStats::MergeChunk(Copy, Chunk);
		++Chunks;
	}
	TestEqual(TEXT("four chunks (the last sample asked again each time)"), Chunks, 4);
	if (!TestEqual(TEXT("the copy: both players"), Copy.Players.Num(), 2))
	{
		return false;
	}
	TestTrue(TEXT("the copy is the server's: samples"), Copy.Players[1].Samples == Server.Players[1].Samples);
	TestEqual(TEXT("the copy is the server's: events"), Copy.Events.Num(), 3);
	TestEqual(TEXT("the copy is the server's: interval"), Copy.SampleIntervalSeconds, 10);

	// The server replaces its last sample (a dump at the same time) and adds one: the copy follows.
	Server.Players[0].Samples.Last().Workers = 99;
	FHOLMatchStatsSample Later;
	Later.TimeSeconds = 50;
	HOLMatchStats::AddSample(Server, 0, Later);
	HOLMatchStats::RecordUnitTrained(Server, 0);
	Chunk = HOLMatchStats::MakeChunk(Server, HOLMatchStats::GetNextFromSample(Copy), Copy.Events.Num(), 2);
	TestTrue(TEXT("one chunk"), Chunk.bComplete);
	TestEqual(TEXT("no event again"), Chunk.Stats.Events.Num(), 0);
	HOLMatchStats::MergeChunk(Copy, Chunk);
	TestTrue(TEXT("the replaced sample"), Copy.Players[0].Samples == Server.Players[0].Samples);
	TestTrue(TEXT("the other player's unchanged"), Copy.Players[1].Samples == Server.Players[1].Samples);
	TestEqual(TEXT("the tables refreshed"), Copy.Players[0].UnitsTrained, 1);
	TestEqual(TEXT("events kept"), Copy.Events.Num(), 3);
	return true;
}

// C4: the scores of the statistics screen.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMatchStatsScoresTest, "HeroesOfLiberty.Match.Stats.Scores",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHOLMatchStatsScoresTest::RunTest(const FString& Parameters)
{
	FHOLPlayerMatchStats Player;
	TestEqual(TEXT("nothing done: no military score"), HOLMatchStats::GetMilitaryScore(Player), 0);
	TestEqual(TEXT("nothing done: no economy score"), HOLMatchStats::GetEconomyScore(Player), 0);

	Player.UnitsKilled = 7;
	Player.UnitsLost = 4;
	Player.BuildingsLost = 2;
	TestEqual(TEXT("10 a kill, losses not counted"), HOLMatchStats::GetMilitaryScore(Player), 70);

	Player.Gathered = FHOLResources(120, 85, 30, 9);
	Player.UnitsTrained = 3;
	TestEqual(TEXT("244 gathered is 24, 3 trained 15"), HOLMatchStats::GetEconomyScore(Player), 39);
	TestEqual(TEXT("military unchanged by the economy"), HOLMatchStats::GetMilitaryScore(Player), 70);
	return true;
}

// C5: the curves of the « Chronologie »: a sample's values, the points, the round graduations of the axes.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMatchStatsCurvesTest, "HeroesOfLiberty.Match.Stats.Curves",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHOLMatchStatsCurvesTest::RunTest(const FString& Parameters)
{
	FHOLMatchStatsSample Sample;
	Sample.TimeSeconds = 20;
	Sample.Workers = 7;
	Sample.Military = 3;
	Sample.Population = 11;
	Sample.Buildings = 4;
	Sample.Gathered = FHOLResources(120, 85, 30, 9);
	Sample.Stock = FHOLResources(50, 10, 0, 5);
	TestEqual(TEXT("workers"), HOLMatchStats::GetSeriesValue(Sample, EHOLMatchStatsSeries::Workers), 7);
	TestEqual(TEXT("military"), HOLMatchStats::GetSeriesValue(Sample, EHOLMatchStatsSeries::Military), 3);
	TestEqual(TEXT("population"), HOLMatchStats::GetSeriesValue(Sample, EHOLMatchStatsSeries::Population), 11);
	TestEqual(TEXT("buildings"), HOLMatchStats::GetSeriesValue(Sample, EHOLMatchStatsSeries::Buildings), 4);
	TestEqual(TEXT("gathered: the four together"), HOLMatchStats::GetSeriesValue(Sample, EHOLMatchStatsSeries::Gathered), 244);
	TestEqual(TEXT("stock: the four together"), HOLMatchStats::GetSeriesValue(Sample, EHOLMatchStatsSeries::Stock), 65);

	// One point a sample, in their order: X the match time, Y the value.
	FHOLPlayerMatchStats Player;
	FHOLMatchStatsSample First;
	First.Workers = 3;
	Player.Samples = { First, Sample };
	const TArray<FVector2D> Curve = HOLMatchStats::MakeCurve(Player, EHOLMatchStatsSeries::Workers);
	if (TestEqual(TEXT("a point a sample"), Curve.Num(), 2))
	{
		TestEqual(TEXT("first point"), Curve[0], FVector2D(0.0, 3.0));
		TestEqual(TEXT("second point"), Curve[1], FVector2D(20.0, 7.0));
	}
	TestEqual(TEXT("no sample, no point"), HOLMatchStats::MakeCurve(FHOLPlayerMatchStats(), EHOLMatchStatsSeries::Military).Num(), 0);

	// Round steps: 1, 2, 5 x 10^n, MaxTicks at most over the range.
	TestEqual(TEXT("0..7 in 5 ticks: 2"), HOLMatchStats::GetNiceStep(7, 5), 2);
	TestEqual(TEXT("0..10 in 5 ticks: 2"), HOLMatchStats::GetNiceStep(10, 5), 2);
	TestEqual(TEXT("0..3 in 5 ticks: 1"), HOLMatchStats::GetNiceStep(3, 5), 1);
	TestEqual(TEXT("nothing: 1"), HOLMatchStats::GetNiceStep(0, 5), 1);
	TestEqual(TEXT("0..244 in 5 ticks: 50"), HOLMatchStats::GetNiceStep(244, 5), 50);
	TestEqual(TEXT("0..1300 in 5 ticks: 500"), HOLMatchStats::GetNiceStep(1300, 5), 500);
	TestEqual(TEXT("0..90000 in 4 ticks: 50000"), HOLMatchStats::GetNiceStep(90000, 4), 50000);
	// Time: steps a clock reads (10, 15, 30 s, 1, 2, 5, 10, 15, 30 min, ...).
	TestEqual(TEXT("35 s in 6 ticks: 10 s"), HOLMatchStats::GetNiceStep(35, 6, true), 10);
	TestEqual(TEXT("80 s in 6 ticks: 15 s"), HOLMatchStats::GetNiceStep(80, 6, true), 15);
	TestEqual(TEXT("5 min in 6 ticks: 1 min"), HOLMatchStats::GetNiceStep(300, 6, true), 60);
	TestEqual(TEXT("40 min in 6 ticks: 10 min"), HOLMatchStats::GetNiceStep(2400, 6, true), 600);
	TestEqual(TEXT("3 h in 6 ticks: 30 min"), HOLMatchStats::GetNiceStep(10800, 6, true), 1800);
	TestEqual(TEXT("5 s in 6 ticks: 1 s"), HOLMatchStats::GetNiceStep(5, 6, true), 1);

	TestEqual(TEXT("7 by 2: 8"), HOLMatchStats::GetAxisEnd(7, 2), 8);
	TestEqual(TEXT("10 by 2: 10"), HOLMatchStats::GetAxisEnd(10, 2), 10);
	TestEqual(TEXT("nothing: one step"), HOLMatchStats::GetAxisEnd(0, 50), 50);
	// A value axis keeps a tenth of its highest value at least above it: a curve at its highest stays off the top edge.
	TestEqual(TEXT("10 by 2, headroom: 12"), HOLMatchStats::GetAxisEnd(10, 2, /*bHeadroom*/ true), 12);
	TestEqual(TEXT("7 by 2, headroom: 8"), HOLMatchStats::GetAxisEnd(7, 2, /*bHeadroom*/ true), 8);
	TestEqual(TEXT("250 by 50, headroom: 300"), HOLMatchStats::GetAxisEnd(250, 50, /*bHeadroom*/ true), 300);
	TestEqual(TEXT("1 by 1, headroom: 2"), HOLMatchStats::GetAxisEnd(1, 1, /*bHeadroom*/ true), 2);
	TestEqual(TEXT("nothing, headroom: one step"), HOLMatchStats::GetAxisEnd(0, 50, /*bHeadroom*/ true), 50);

	TestEqual(TEXT("0:00"), HOLMatchStats::FormatMatchTime(0), FString(TEXT("0:00")));
	TestEqual(TEXT("1:05"), HOLMatchStats::FormatMatchTime(65), FString(TEXT("1:05")));
	TestEqual(TEXT("12:30"), HOLMatchStats::FormatMatchTime(750), FString(TEXT("12:30")));
	return true;
}

#endif
