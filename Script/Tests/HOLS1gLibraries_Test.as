// Socle step 1, lot S1g (D830, D836, D839, ELROND-1117b): one smoke test per read library of the rules that STAY in C++, each on ONE vector read in
// its C++ test. The full port of each C++ test is the next lot (L17-09, L17-11, L17-12 of Auto_Journey_File.md), after the build.
// This file compiles only once the plugin is rebuilt with the three libraries (HOLOrderChainReadLibrary, HOLMatchStatsReadLibrary,
// HOLProductionReadLibrary): until then the script compile fails, as expected.

// HOLOrderChainTests.cpp (HeroesOfLiberty.Orders.Chains, "Peasants"): different queues, one chain each, a unit without steps left out; BuildingQueue: MaySeek.
void Test_OrderChainReadLibrarySmoke(FUnitTest& T)
{
	FHOLOrderChainStep BuildSeven = HOLOrderChainRead::MakeBuild(7);
	TArray<FHOLOrderChainUnitSteps> Units;
	int Ids[4] = {10, 11, 12, 13};
	int Sources[4] = {3, 0, 4, 3};
	for (int Index = 0; Index < 4; ++Index)
	{
		FHOLOrderChainUnitSteps Unit;
		Unit.UnitId = Ids[Index];
		if (Index != 1)
		{
			Unit.Steps.Add(BuildSeven);
			Unit.Steps.Add(HOLOrderChainRead::MakeGather(Sources[Index]));
		}
		Units.Add(Unit);
	}
	TArray<FHOLOrderChain> Chains = HOLOrderChainRead::MakeChains(Units);
	T.AssertEquals(2, Chains.Num(), "Peasants: two chains");
	if (Chains.Num() == 2)
	{
		T.AssertEquals(2, Chains[0].UnitIds.Num(), "Peasants: first chain has two units");
		if (Chains[0].UnitIds.Num() == 2)
		{
			T.AssertEquals(10, Chains[0].UnitIds[0], "Peasants: first chain #10");
			T.AssertEquals(13, Chains[0].UnitIds[1], "Peasants: first chain #13");
		}
		T.AssertEquals(1, Chains[1].UnitIds.Num(), "Peasants: second chain #12");
	}
	T.AssertTrue(HOLOrderChainRead::KindName(EHOLOrderChainStepKind::Rally) == "Rally", "Rally: its name");
	T.AssertEquals(16, HOLOrderChainRead::GetMaxSteps(), "MaxSteps");
	T.AssertTrue(HOLOrderChainRead::IsCurrentOrderOver(false, false, false, false, false), "nothing held: the current order is over");
	T.AssertFalse(HOLOrderChainRead::IsCurrentOrderOver(false, false, true, false, false), "an ordered building: not over");
	T.AssertTrue(HOLOrderChainRead::MaySeek(false, false, false, false, false), "at rest: it takes on enemy units");
	T.AssertFalse(HOLOrderChainRead::MaySeek(false, false, false, true, false), "ordered building: never dropped for an enemy unit");
	T.AssertTrue(HOLOrderChainRead::MaySeek(true, true, false, false, false), "attack-move: it takes on enemy units");
}

// HOLMatchStatsTests.cpp (HeroesOfLiberty.Match.Stats.Scores, Curves): the scores and a few of the axis rules.
void Test_MatchStatsReadLibrarySmoke(FUnitTest& T)
{
	T.AssertEquals(70, HOLMatchStatsRead::GetMilitaryScore(7), "10 a kill: 7 kills are 70");
	FHOLResources Gathered;
	Gathered.Food = 120;
	Gathered.Wood = 85;
	Gathered.Stone = 30;
	Gathered.Gold = 9;
	T.AssertEquals(39, HOLMatchStatsRead::GetEconomyScore(Gathered, 3), "244 gathered is 24, 3 trained 15");
	T.AssertEquals(244, HOLMatchStatsRead::GetSeriesValue(7, 3, 11, 4, Gathered, FHOLResources(), 4), "gathered: the four together");
	T.AssertEquals(50, HOLMatchStatsRead::GetNiceStep(244, 5, false), "0..244 in 5 ticks: 50");
	T.AssertEquals(300, HOLMatchStatsRead::GetAxisEnd(250, 50, true), "250 by 50, headroom: 300");
	T.AssertTrue(HOLMatchStatsRead::FormatMatchTime(750) == "12:30", "12:30");
	T.AssertEquals(200, HOLMatchStatsRead::GetSampleSteps(0.05f, 10), "10 s at 20 steps a second");
}

// HOLProductionTests.cpp (HeroesOfLiberty.Buildings.Production): the population cap and the progress of an item that started at step 10, 4 steps to build.
void Test_ProductionReadLibrarySmoke(FUnitTest& T)
{
	T.AssertEquals(175, HOLProductionRead::GetPopulationCap(400), "population cap: 175 at most (D03)");
	T.AssertEquals(10, HOLProductionRead::GetPopulationCap(10), "population cap: what the buildings give");
	T.AssertEquals(0.5f, HOLProductionRead::GetProgress(10, 12.0, 4), "half done two steps later");
	T.AssertFalse(HOLProductionRead::IsWaitingForRoom(10, 13.0, 4), "not waiting for room before 100 %");
	T.AssertTrue(HOLProductionRead::IsWaitingForRoom(10, 14.0, 4), "it waits at 100 % for a place");
}