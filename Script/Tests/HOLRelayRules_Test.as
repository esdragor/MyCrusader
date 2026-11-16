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

// The arrangements of the four slots' teams the agreement test goes through.
TArray<int> RelayArrangement(int Index)
{
	if (Index == 1)
		return RelayTeams(0, 0, 1, 1);
	if (Index == 2)
		return RelayTeams(0, 1, 0, 1);
	if (Index == 3)
		return RelayTeams(0, 1, 1, 1);
	if (Index == 4)
		return RelayTeams(0, 0, 0, 0);
	return RelayTeams();
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

// The simulation's copy of the rules (C++, HOLRelay::Step) gives the same relay as the script's on every case (MIG-J-Relay, extended): each
// owner (nobody, each of the four slots, a slot out of the arrays), each count (0, 1, 2, 100, one step short, full), each presence of the four
// slots (0 to 3 units each), five arrangements of the teams (each alone, two pairs, crossed pairs, one against three, all allied) and a sixth
// where the zone counts three slots only (arrays of unequal lengths). Both relays are computed by their own copy, never written by hand.
void Test_RelayScriptAndSimulationRulesAgree(FUnitTest& T)
{
	TArray<int> Owners;
	Owners.Add(-1);
	Owners.Add(0);
	Owners.Add(1);
	Owners.Add(2);
	Owners.Add(3);
	Owners.Add(7);
	TArray<int> Helds;
	Helds.Add(0);
	Helds.Add(1);
	Helds.Add(2);
	Helds.Add(100);
	Helds.Add(HOLRelay::HoldSteps() - 1);
	Helds.Add(HOLRelay::HoldSteps());
	int Cases = 0;
	int Differences = 0;
	int Captures = 0;
	int Frozen = 0;
	for (int O = 0; O < Owners.Num(); O++)
	{
		for (int H = 0; H < Helds.Num(); H++)
		{
			for (int A = 0; A < 6; A++)
			{
				const TArray<int> Teams = RelayArrangement(A == 5 ? 0 : A);
				for (int Presence = 0; Presence < 256; Presence++)
				{
					TArray<int> Units;
					int Rest = Presence;
					for (int Slot = 0; Slot < 4; Slot++)
					{
						Units.Add(Rest % 4);
						Rest = Rest / 4;
					}
					if (A == 5)
						Units.RemoveAt(3);
					HOLRelay::FHOLRelayState State;
					State.Owner = Owners[O];
					State.Taker = Helds[H] > 0 ? (O + H) % 4 : -1;
					State.Held = Helds[H];
					const HOLRelay::FHOLRelayState Script = HOLRelay::Step(State, Units, Teams);
					int OwnerOut = -9;
					int TakerOut = -9;
					int HeldOut = -9;
					HOLRelayRules::StepRelay(State.Owner, State.Taker, State.Held, Units, Teams, HOLRelay::HoldSteps(), HOLRelay::DecaySteps, OwnerOut, TakerOut, HeldOut);
					Cases++;
					if (Script.Owner != OwnerOut || Script.Taker != TakerOut || Script.Held != HeldOut)
						Differences++;
					if (Script.Owner != State.Owner)
						Captures++;
					if (Script.Held == State.Held && State.Held > 0)
						Frozen++;
				}
			}
		}
	}
	T.AssertEquals(Owners.Num() * Helds.Num() * 6 * 256, Cases, "every case compared");
	T.AssertEquals(0, Differences, "the script and the simulation agree on every case");
	T.AssertTrue(Captures > 0, "the table holds captures (" + Captures + ")");
	T.AssertTrue(Frozen > 0, "the table holds frozen counts (" + Frozen + ")");
}

// R5, the C++ HeroesOfLiberty.Relays.Ring ported (its FillPercent part; MakeView and FHOLRelayView::Describe stay in C++: the view is the
// replicated struct, not exposed to the script): what a relay's ring shows, the share of the capture held.
void Test_RelayRing(FUnitTest& T)
{
	HOLRelay::FHOLRelayState State;
	T.AssertEquals(0, HOLRelay::FillPercent(State, 600), "nothing held: 0");
	State.Held = 150;
	T.AssertEquals(25, HOLRelay::FillPercent(State, 600), "a quarter: 25");
	State.Held = 599;
	T.AssertEquals(99, HOLRelay::FillPercent(State, 600), "almost taken: 99, never 100 (the ring is full only by the change of owner)");
	State.Held = 900;
	T.AssertEquals(99, HOLRelay::FillPercent(State, 600), "past the count: 99");
	T.AssertEquals(0, HOLRelay::FillPercent(State, 0), "no hold steps: 0, no division by 0");
	State.Owner = 2;
	State.Taker = 1;
	State.Held = 300;
	T.AssertEquals(50, HOLRelay::FillPercent(State, 600), "the C++ view's case (owner 2, taker 1, 300 of 600): half filled");
	State.Held = -5;
	T.AssertEquals(0, HOLRelay::FillPercent(State, 600), "a negative count shows nothing");
	// Through the real chain: an enemy alone for half the capture time fills half the ring, one step short of the capture 99.
	HOLRelay::FHOLRelayState Half = RelayRun(HOLRelay::FHOLRelayState(), RelayUnits(0, 2), RelayTeams(), HOLRelay::HoldSteps() / 2);
	T.AssertEquals(50, HOLRelay::FillPercent(Half, HOLRelay::HoldSteps()), "an enemy alone for half the capture time: half the ring");
	HOLRelay::FHOLRelayState Short = RelayRun(Half, RelayUnits(0, 2), RelayTeams(), HOLRelay::HoldSteps() / 2 - 1);
	T.AssertEquals(99, HOLRelay::FillPercent(Short, HOLRelay::HoldSteps()), "one step short of the capture: 99");
}

// The provisional values and the territory XP of the simulation (C++) are the script's.
void Test_RelayConstantsAndXPAgreeWithTheSimulation(FUnitTest& T)
{
	T.AssertEquals(HOLRelay::HoldSeconds, HOLRelayRules::GetHoldSeconds(), "the capture time");
	T.AssertEquals(HOLRelay::DecaySteps, HOLRelayRules::GetDecaySteps(), "the recession");
	T.AssertEquals(HOLRelay::RelayXPPerMinute, HOLRelayRules::GetRelayXPPerMinute(), "the relay's rate");
	T.AssertEquals(HOLRelay::CentreXPPerMinute, HOLRelayRules::GetCentreXPPerMinute(), "the centre's rate");
	HOLRelay::FHOLTerritoryCarry Script;
	int SimCarry = 0;
	int Differences = 0;
	int ScriptTotal = 0;
	int SimTotal = 0;
	for (int Step = 0; Step < 3000; Step++)
	{
		// Negative counts too (no holding, the rule's Max(0, ...)).
		const int Relays = (Step / 700) % 5 - 1;
		const int Centres = (Step / 900) % 4 - 1;
		const int ScriptXP = HOLRelay::TerritoryXP(Script, Relays, Centres);
		int NewCarry = 0;
		const int SimXP = HOLRelayRules::TerritoryXPStep(SimCarry, Relays, Centres, HOLRelay::StepsPerSecond, NewCarry);
		SimCarry = NewCarry;
		ScriptTotal += ScriptXP;
		SimTotal += SimXP;
		if (ScriptXP != SimXP)
			Differences++;
	}
	T.AssertEquals(0, Differences, "the same XP at every step");
	T.AssertEquals(ScriptTotal, SimTotal, "the same total");
	T.AssertTrue(ScriptTotal > 0, "the case does give XP");
}
