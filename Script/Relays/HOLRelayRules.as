// Neutral relays and territory XP, pure rules (R1; GDD 9.4, D388, D384, D348, D380): integers only, no world access, tested in
// HOLRelayRules_Test.as. A relay (provisional name) is a neutral place laid by the map. It is taken by standing in it without an enemy, with
// the rule of the ruin (D348) and of the altar (D380): the units of one adverse side alone in the zone fill a count; one unit of its owner (or
// of an ally) freezes it; nobody adverse makes it recede; the count full, the relay goes to the slot with the most units there, and stays
// its own until an enemy takes it back the same way. Territory gives XP by the minute and nothing else (no sight): each relay held and each
// secondary centre (D384) a rate. Values are indicative, to tune in test (D388: the territory share is about 15 % of a game's XP).
// Decided: the rate, 10 XP a minute (RQ1 = A, D587); two adverse sides in the zone freeze the count (RQ2 = A, D588). Still to tune in test: the
// capture time; whether allies share the XP.
namespace HOLRelay
{
	// Simulation steps per second (the bridge, as the hero's return).
	const int StepsPerSecond = HOLHero::GetSimulationRateHz();

	// No owner / no taker.
	const int None = -1;

	// Seconds an adverse side must hold the zone alone to take the relay (the ruin's, ForumHoldTime 60 s, D348; provisional).
	const int HoldSeconds = 60;
	// Steps of count given back each step with nobody adverse in the zone (the ruin's recession, ForumHoldDecay 2 s a second, V3-Q3; provisional).
	const int DecaySteps = 2;

	// XP a minute for each relay held and each secondary centre held, the weight of the source already in (RQ1 = A, D587: 10 XP a minute each).
	const int RelayXPPerMinute = 10;
	const int CentreXPPerMinute = 10;

	int HoldSteps()
	{
		return HoldSeconds * StepsPerSecond;
	}

	// One relay: its owner (a slot), the count of steps held toward its capture and who is filling it.
	struct FHOLRelayState
	{
		int Owner = -1;
		int Taker = -1;
		int Held = 0;
	}

	// The relay after one step. UnitsBySlot: living units of each slot in the zone; TeamBySlot: the team of each slot (equal teams are
	// allies). A slot with no units does not count; a slot out of the arrays is ignored.
	FHOLRelayState Step(const FHOLRelayState& State, const TArray<int>& UnitsBySlot, const TArray<int>& TeamBySlot)
	{
		return StepWith(State, UnitsBySlot, TeamBySlot, HoldSteps(), DecaySteps);
	}

	// The same step with the capture time and the recession the server holds (the bridge, UHOLRelayBoard: the server's hold time is the cvar hol.Relay.HoldSeconds
	// at the simulation rate, so it passes its steps; Step above is this with the constants of this file).
	FHOLRelayState StepWith(const FHOLRelayState& State, const TArray<int>& UnitsBySlot, const TArray<int>& TeamBySlot, int HoldCount, int DecayCount)
	{
		FHOLRelayState Next = State;
		const int Slots = Math::Min(UnitsBySlot.Num(), TeamBySlot.Num());
		const int OwnerTeam = (State.Owner >= 0 && State.Owner < TeamBySlot.Num()) ? TeamBySlot[State.Owner] : -1;
		bool bDefenders = false;
		int AdverseTeam = -1;
		int AdverseTeams = 0;
		int Taker = None;
		int TakerUnits = 0;
		for (int Slot = 0; Slot < Slots; Slot++)
		{
			if (UnitsBySlot[Slot] <= 0)
				continue;
			const int Team = TeamBySlot[Slot];
			if (State.Owner >= 0 && Team == OwnerTeam)
			{
				bDefenders = true;
				continue;
			}
			if (Team != AdverseTeam)
			{
				AdverseTeam = Team;
				AdverseTeams++;
			}
			// The taker: the most units, the lowest slot on a tie (strictly more to replace).
			if (UnitsBySlot[Slot] > TakerUnits)
			{
				Taker = Slot;
				TakerUnits = UnitsBySlot[Slot];
			}
		}
		if (AdverseTeams == 0)
		{
			// Nobody adverse: the count recedes.
			Next.Held = Math::Max(0, State.Held - DecayCount);
			if (Next.Held == 0)
				Next.Taker = None;
			return Next;
		}
		if (bDefenders || AdverseTeams > 1)
		{
			// Contested: the count is frozen.
			return Next;
		}
		// One adverse side alone: the count goes up, its slot with the most units shown as the taker.
		Next.Taker = Taker;
		Next.Held = State.Held + 1;
		if (Next.Held >= HoldCount)
		{
			Next.Owner = Taker;
			Next.Taker = None;
			Next.Held = 0;
		}
		return Next;
	}

	// R5: the share of the capture held, 0 to 99 (the ring is full only by the change of owner); 0 with no hold steps. The C++ twin is
	// HOLRelay::FillPercent (Source/HeroesOfLiberty/Simulation/HOLRelayRules.h), which fills the replicated FHOLRelayView the markers draw.
	int FillPercent(const FHOLRelayState& State, int Steps)
	{
		if (Steps <= 0)
			return 0;
		if (State.Held >= Steps)
			return 99;
		return Math::Clamp(Math::IntegerDivisionTrunc(State.Held * 100, Steps), 0, 99);
	}

	// Territory XP: the fractions carried from a step to the next (rate x holdings, in XP x steps), so that a minute of one relay gives
	// exactly its rate whatever the simulation rate.
	struct FHOLTerritoryCarry
	{
		int Carry = 0;
	}

	// The whole XP to give the owner of Relays relays and Centres secondary centres for this step (0 most steps).
	int TerritoryXP(FHOLTerritoryCarry& Memory, int Relays, int Centres)
	{
		return TerritoryXPAt(Memory, Relays, Centres, StepsPerSecond);
	}

	// The same at the simulation rate the server holds (the bridge, UHOLRelayBoard; at least 1 step a second, as the C++ rule).
	int TerritoryXPAt(FHOLTerritoryCarry& Memory, int Relays, int Centres, int Rate)
	{
		const int StepsPerMinute = 60 * Math::Max(1, Rate);
		Memory.Carry += Math::Max(0, Relays) * RelayXPPerMinute + Math::Max(0, Centres) * CentreXPPerMinute;
		const int Award = Math::IntegerDivisionTrunc(Memory.Carry, StepsPerMinute);
		Memory.Carry -= Award * StepsPerMinute;
		return Award;
	}
}
