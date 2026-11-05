// Sacred sites, victory count and gold by the minute, pure rules (SacredSitesInProgress.md S10 and S6, D431, D699, D703; HOLSiteVictory.as).
// Four slots, one site each unless a test says otherwise; the owner of every site is a slot, the camp of a slot its team. The count is
// the constant of the rule (300 s, D699), the steps per second the simulation's own (HOLHero::GetSimulationRateHz): read, never retyped.

TArray<int> VictoryTeams(int T0 = 0, int T1 = 1, int T2 = 2, int T3 = 3)
{
	TArray<int> Teams;
	Teams.Add(T0);
	Teams.Add(T1);
	Teams.Add(T2);
	Teams.Add(T3);
	return Teams;
}

// The owners of the sites, slot by slot (-1: nobody's), up to three sites.
TArray<int> VictoryOwners(int O0, int O1 = -2, int O2 = -2)
{
	TArray<int> Owners;
	Owners.Add(O0);
	if (O1 != -2)
		Owners.Add(O1);
	if (O2 != -2)
		Owners.Add(O2);
	return Owners;
}

// Which sites have an adverse unit in the zone (0 or 1 each), three sites.
TArray<int> VictoryContest(int C0 = 0, int C1 = 0, int C2 = 0)
{
	TArray<int> Contest;
	Contest.Add(C0);
	Contest.Add(C1);
	Contest.Add(C2);
	return Contest;
}

HOLSiteVictory::FHOLSiteVictoryState VictoryRun(const HOLSiteVictory::FHOLSiteVictoryState& From, const TArray<int>& Owners, const TArray<int>& Contest, const TArray<int>& Teams, int Steps)
{
	HOLSiteVictory::FHOLSiteVictoryState State = From;
	for (int Step = 0; Step < Steps; Step++)
	{
		State = HOLSiteVictory::Step(State, Owners, Contest, Teams);
	}
	return State;
}

// A camp holding every site for the whole count wins, not a step before; nobody wins before; the constant is 300 s of the simulation's steps.
void Test_VictoryHoldingAllSites(FUnitTest& T)
{
	T.AssertEquals(300, HOLSiteVictory::VictorySeconds, "the count of the victory is 300 s (D699)");
	T.AssertEquals(HOLSiteVictory::VictorySeconds * HOLHero::GetSimulationRateHz(), HOLSiteVictory::VictorySteps(), "in steps, at the simulation's rate");
	HOLSiteVictory::FHOLSiteVictoryState State;
	T.AssertEquals(HOLSiteVictory::None, State.Winner, "nobody wins at the start");
	State = VictoryRun(State, VictoryOwners(1, 1, 1), VictoryContest(), VictoryTeams(), HOLSiteVictory::VictorySteps() - 1);
	T.AssertEquals(HOLSiteVictory::None, State.Winner, "one step short: nobody");
	T.AssertEquals(HOLSiteVictory::VictorySteps() - 1, State.Held, "the count shows it");
	T.AssertEquals(1, State.Leader, "slot 1's camp leads the count");
	State = VictoryRun(State, VictoryOwners(1, 1, 1), VictoryContest(), VictoryTeams(), 1);
	T.AssertEquals(1, State.Winner, "the count full: the camp of slot 1 wins");
}

// A camp that does not hold every site has no count; two camps cannot count at the same time; no site at all is no victory.
void Test_VictoryNeedsEverySite(FUnitTest& T)
{
	HOLSiteVictory::FHOLSiteVictoryState State = VictoryRun(HOLSiteVictory::FHOLSiteVictoryState(), VictoryOwners(1, 1, -1), VictoryContest(), VictoryTeams(), 500);
	T.AssertEquals(0, State.Held, "a site nobody's: no count");
	T.AssertEquals(HOLSiteVictory::None, State.Leader, "nobody leads");
	State = VictoryRun(HOLSiteVictory::FHOLSiteVictoryState(), VictoryOwners(1, 2, 1), VictoryContest(), VictoryTeams(), 500);
	T.AssertEquals(0, State.Held, "two adverse camps each holding a site: no count");
	State = VictoryRun(HOLSiteVictory::FHOLSiteVictoryState(), TArray<int>(), TArray<int>(), VictoryTeams(), 500);
	T.AssertEquals(HOLSiteVictory::None, State.Winner, "no site on the map: no sacred victory");
	T.AssertEquals(0, State.Held, "no count");
}

// A contested site (an adverse unit in a zone) freezes the count, it does not recede; the count goes on when the zone is free.
void Test_VictoryFrozenByAContest(FUnitTest& T)
{
	HOLSiteVictory::FHOLSiteVictoryState State = VictoryRun(HOLSiteVictory::FHOLSiteVictoryState(), VictoryOwners(1, 1), VictoryContest(0, 1), VictoryTeams(), 100);
	T.AssertEquals(0, State.Held, "contested from the start: the count has not begun");
	State = VictoryRun(State, VictoryOwners(1, 1), VictoryContest(), VictoryTeams(), 200);
	T.AssertEquals(200, State.Held, "the zone free: it counts");
	State = VictoryRun(State, VictoryOwners(1, 1), VictoryContest(1, 0), VictoryTeams(), 5000);
	T.AssertEquals(200, State.Held, "a contest for a long time: frozen, never receding");
	State = VictoryRun(State, VictoryOwners(1, 1), VictoryContest(), VictoryTeams(), 10);
	T.AssertEquals(210, State.Held, "the zone free again: it goes on");
}

// A site changing hands resets the count, even to the profit of the same camp (another slot of it), and so does a lost site.
void Test_VictoryResetByAChangeOfHands(FUnitTest& T)
{
	HOLSiteVictory::FHOLSiteVictoryState State = VictoryRun(HOLSiteVictory::FHOLSiteVictoryState(), VictoryOwners(0, 0), VictoryContest(), VictoryTeams(0, 0, 2, 3), 300);
	T.AssertEquals(300, State.Held, "slots 0 and 1 are one camp: the count rises");
	State = VictoryRun(State, VictoryOwners(0, 1), VictoryContest(), VictoryTeams(0, 0, 2, 3), 1);
	T.AssertEquals(1, State.Held, "the second site went from slot 0 to its ally slot 1: the count starts again (one step in)");
	State = VictoryRun(State, VictoryOwners(0, 1), VictoryContest(), VictoryTeams(0, 0, 2, 3), 299);
	T.AssertEquals(300, State.Held, "teammates' sites count together");
	State = VictoryRun(State, VictoryOwners(0, 2), VictoryContest(), VictoryTeams(0, 0, 2, 3), 1);
	T.AssertEquals(0, State.Held, "a site lost to another camp: no count");
	T.AssertEquals(HOLSiteVictory::None, State.Leader, "nobody leads");
}

// An alliance breaking (the teams change, D703) resets the count: the camp that held every site is now two camps.
void Test_VictoryResetByABrokenAlliance(FUnitTest& T)
{
	HOLSiteVictory::FHOLSiteVictoryState State = VictoryRun(HOLSiteVictory::FHOLSiteVictoryState(), VictoryOwners(0, 1), VictoryContest(), VictoryTeams(0, 0, 2, 3), 400);
	T.AssertEquals(400, State.Held, "allies holding every site together: the count rises");
	State = VictoryRun(State, VictoryOwners(0, 1), VictoryContest(), VictoryTeams(0, 1, 2, 3), 1);
	T.AssertEquals(0, State.Held, "the alliance broke: each holds one site, no camp holds all, the count is gone");
	State = VictoryRun(State, VictoryOwners(0, 1), VictoryContest(), VictoryTeams(0, 1, 2, 3), 100);
	T.AssertEquals(0, State.Held, "and it stays gone");
	T.AssertEquals(HOLSiteVictory::None, State.Winner, "nobody wins");
}

// Gold by the minute (S6, D699): a minute of one site gives exactly the rate whatever the simulation rate, nothing is lost between steps,
// several sites add up, nothing held gives nothing (a negative count is no holding).
void Test_SiteGoldByTheMinute(FUnitTest& T)
{
	const int StepsPerMinute = 60 * HOLHero::GetSimulationRateHz();
	const int Rate = 12;
	HOLSiteVictory::FHOLGoldCarry Memory;
	int Total = 0;
	for (int Step = 0; Step < StepsPerMinute; Step++)
	{
		Total += HOLSiteVictory::GoldStep(Memory, 1, Rate);
	}
	T.AssertEquals(Rate, Total, "one site for a minute: the rate");
	HOLSiteVictory::FHOLGoldCarry Both;
	Total = 0;
	for (int Step = 0; Step < StepsPerMinute * 3; Step++)
	{
		Total += HOLSiteVictory::GoldStep(Both, 2, Rate);
	}
	T.AssertEquals(3 * 2 * Rate, Total, "two sites for three minutes");
	HOLSiteVictory::FHOLGoldCarry Nothing;
	Total = 0;
	for (int Step = 0; Step < StepsPerMinute; Step++)
	{
		Total += HOLSiteVictory::GoldStep(Nothing, 0, Rate);
		Total += HOLSiteVictory::GoldStep(Nothing, -2, Rate);
		Total += HOLSiteVictory::GoldStep(Nothing, 3, 0);
	}
	T.AssertEquals(0, Total, "no site, or no rate: no gold");
}
