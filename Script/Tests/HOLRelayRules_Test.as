// Neutral relays and territory XP (R1, HOLRelayRules.as; GDD 9.4, D388, D348, D380). Four slots; every slot its own team unless a test
// says otherwise. The steps per second are the simulation's own (HOLHero::GetSimulationRateHz); the capture time and the rates are the
// rules' provisional constants, read from them (a test does not retype a value).

TArray<int> RelayTeams(int T0 = 0, int T1 = 1, int T2 = 2, int T3 = 3)
{
	TArray<int> Teams;
	Teams.Add(T0);
	Teams.Add(T1);
	Teams.Add(T2);
	Teams.Add(T3);
	return Teams;
}

// Units in the zone, slot by slot.
TArray<int> RelayUnits(int U0 = 0, int U1 = 0, int U2 = 0, int U3 = 0)
{
	TArray<int> Units;
	Units.Add(U0);
	Units.Add(U1);
	Units.Add(U2);
	Units.Add(U3);
	return Units;
}

HOLRelay::FHOLRelayState RelayRun(const HOLRelay::FHOLRelayState& From, const TArray<int>& Units, const TArray<int>& Teams, int Steps)
{
	HOLRelay::FHOLRelayState State = From;
	for (int Step = 0; Step < Steps; Step++)
	{
		State = HOLRelay::Step(State, Units, Teams);
	}
	return State;
}

// The relay is neutral: one side alone in the zone fills the count and takes it when the count is full, not a step before.
void Test_RelayTakenByPresence(FUnitTest& T)
{
	HOLRelay::FHOLRelayState State;
	T.AssertEquals(HOLRelay::None, State.Owner, "a relay laid by the map belongs to nobody");
	State = RelayRun(State, RelayUnits(0, 3), RelayTeams(), HOLRelay::HoldSteps() - 1);
	T.AssertEquals(HOLRelay::None, State.Owner, "one step short: still nobody's");
	T.AssertEquals(HOLRelay::HoldSteps() - 1, State.Held, "the count shows it");
	T.AssertEquals(1, State.Taker, "slot 1 is filling it");
	State = RelayRun(State, RelayUnits(0, 3), RelayTeams(), 1);
	T.AssertEquals(1, State.Owner, "the count full: slot 1's");
	T.AssertEquals(0, State.Held, "the count starts again");
	T.AssertEquals(HOLRelay::None, State.Taker, "nobody is filling it");
	T.AssertEquals(HOLRelay::HoldSeconds * HOLHero::GetSimulationRateHz(), HOLRelay::HoldSteps(), "the capture time in steps follows the simulation rate");
}

// The owner keeps it with nobody there, however long: it stays its own until an enemy takes it back (D388).
void Test_RelayStaysWithItsOwner(FUnitTest& T)
{
	HOLRelay::FHOLRelayState State;
	State.Owner = 0;
	State = RelayRun(State, RelayUnits(), RelayTeams(), HOLRelay::HoldSteps() * 3);
	T.AssertEquals(0, State.Owner, "nobody there: still slot 0's");
	State = RelayRun(State, RelayUnits(5), RelayTeams(), HOLRelay::HoldSteps() * 2);
	T.AssertEquals(0, State.Owner, "its owner's units there: nothing to take");
	T.AssertEquals(0, State.Held, "no count");
}

// A unit of the owner freezes the count (the ruin's rule); an ally of the owner is no enemy; when the defenders leave, the count goes on.
void Test_RelayContestedAndAllied(FUnitTest& T)
{
	HOLRelay::FHOLRelayState State;
	State.Owner = 0;
	State = RelayRun(State, RelayUnits(0, 4), RelayTeams(), 100);
	T.AssertEquals(100, State.Held, "an enemy alone: the count rises");
	State = RelayRun(State, RelayUnits(1, 4), RelayTeams(), 200);
	T.AssertEquals(100, State.Held, "one unit of the owner in the zone: frozen");
	T.AssertEquals(0, State.Owner, "still the owner's");
	State = RelayRun(State, RelayUnits(0, 4), RelayTeams(), 10);
	T.AssertEquals(110, State.Held, "the defender gone: it goes on");
	HOLRelay::FHOLRelayState Allied;
	Allied.Owner = 0;
	Allied = RelayRun(Allied, RelayUnits(0, 4), RelayTeams(0, 0, 1, 2), HOLRelay::HoldSteps() * 2);
	T.AssertEquals(0, Allied.Owner, "slot 1 is slot 0's ally: no enemy, nothing taken");
	T.AssertEquals(0, Allied.Held, "no count");
	// Retaken: the enemy alone for the whole time takes it from its owner.
	HOLRelay::FHOLRelayState Retaken;
	Retaken.Owner = 0;
	Retaken = RelayRun(Retaken, RelayUnits(0, 0, 2), RelayTeams(), HOLRelay::HoldSteps());
	T.AssertEquals(2, Retaken.Owner, "retaken by slot 2");
}

// The count recedes with no enemy, and is lost at 0; two adverse sides at once freeze each other.
void Test_RelayRecedesAndSeveralSides(FUnitTest& T)
{
	HOLRelay::FHOLRelayState State;
	State = RelayRun(State, RelayUnits(0, 2), RelayTeams(), 100);
	State = RelayRun(State, RelayUnits(), RelayTeams(), 10);
	T.AssertEquals(100 - 10 * HOLRelay::DecaySteps, State.Held, "the count recedes");
	T.AssertEquals(1, State.Taker, "its taker is remembered while the count lasts");
	State = RelayRun(State, RelayUnits(), RelayTeams(), 100);
	T.AssertEquals(0, State.Held, "down to 0, never below");
	T.AssertEquals(HOLRelay::None, State.Taker, "nobody is filling it any more");
	HOLRelay::FHOLRelayState Two;
	Two = RelayRun(Two, RelayUnits(0, 2, 2), RelayTeams(), 500);
	T.AssertEquals(0, Two.Held, "two sides, neither alone: the neutral count does not rise");
	T.AssertEquals(HOLRelay::None, Two.Owner, "nobody takes it");
}

// The taker is the slot with the most units, the lowest slot on a tie; it takes the relay for its whole side's work.
void Test_RelayTakerIsTheMostNumerous(FUnitTest& T)
{
	HOLRelay::FHOLRelayState State;
	State = RelayRun(State, RelayUnits(0, 1, 3, 3), RelayTeams(0, 1, 1, 1), 1);
	T.AssertEquals(2, State.Taker, "three allies there: most units, lowest slot on the tie");
	State = RelayRun(State, RelayUnits(0, 1, 3, 4), RelayTeams(0, 1, 1, 1), HOLRelay::HoldSteps());
	T.AssertEquals(3, State.Owner, "the most numerous of the side owns it when the count is full");
}

// Territory XP: a minute of one relay gives exactly its rate whatever the simulation rate; nothing is lost between steps; none without a holding.
void Test_RelayTerritoryXP(FUnitTest& T)
{
	const int StepsPerMinute = 60 * HOLHero::GetSimulationRateHz();
	HOLRelay::FHOLTerritoryCarry Memory;
	int Total = 0;
	for (int Step = 0; Step < StepsPerMinute; Step++)
	{
		Total += HOLRelay::TerritoryXP(Memory, 1, 0);
	}
	T.AssertEquals(HOLRelay::RelayXPPerMinute, Total, "one relay for a minute: its rate");
	HOLRelay::FHOLTerritoryCarry Both;
	Total = 0;
	for (int Step = 0; Step < StepsPerMinute * 3; Step++)
	{
		Total += HOLRelay::TerritoryXP(Both, 2, 1);
	}
	T.AssertEquals(3 * (2 * HOLRelay::RelayXPPerMinute + HOLRelay::CentreXPPerMinute), Total, "two relays and a secondary centre for three minutes");
	HOLRelay::FHOLTerritoryCarry None;
	Total = 0;
	for (int Step = 0; Step < StepsPerMinute; Step++)
	{
		Total += HOLRelay::TerritoryXP(None, 0, 0);
		Total += HOLRelay::TerritoryXP(None, -3, -1);
	}
	T.AssertEquals(0, Total, "nothing held: no XP (a negative count is no holding)");
}
