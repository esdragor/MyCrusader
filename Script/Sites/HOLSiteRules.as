// Sacred sites, pure rule (S1; GDD 14.3 Sacred sites, D429 to D431, D697 to D703, SacredSitesInProgress.md): integers only, no world access, tested
// in HOLSiteRules_Test.as. Derived from HOLRelay::Step (D348, D380, D388) with two counts a side instead of one. A site is a place laid by the
// map. It is TAKEN by the units able to take it (a faction's holy units and the hero: the takers) and FROZEN by any living unit that is no
// worker (the disturbers: the takers included, the Peasant counts for nothing, D698). The units of one adverse camp alone in the zone, one of
// them a taker, fill a count; a disturber of the owner's camp, or of a second adverse camp, freezes it; nobody makes it recede; the count full,
// the site goes to the taker slot with the most takers, and stays its own until another camp takes it back the same way (D698). A camp is a
// player alone or all his mutual allies (D703): the rule reads only the team of each slot, equal teams being one camp, a Neutral stance (D401)
// being a team of its own. Not here: the position, the radius, the unit types (the caller counts with AddUnit), the gold (S6), the victory
// count (S10), the cancelling of the count when an alliance breaks (S5, S10).
// Values are the relay's, provisional, to tune in test (D345: the user chooses them). Edge cases settled by the default of the code, not by the
// GDD (see Retours\Q-sites-rule-edges.md): an adverse soldier alone (no taker of his own camp) freezes a count another camp had begun and lets
// a count of his own camp recede; a count begun by one camp starts again from zero if another camp's taker takes over the zone alone (Q2 = B, answered; the relay's inheritance is not kept).
namespace HOLSite
{
	// Simulation steps per second (the bridge, as the relays' and the hero's return).
	const int StepsPerSecond = HOLHero::GetSimulationRateHz();

	// No owner / no taker.
	const int None = -1;

	// Seconds a camp must hold the zone alone to take the site (the relay's, 60 s; provisional, D699 only fixes the victory count at 300 s).
	const int HoldSeconds = 60;
	// Steps of count given back each step with nobody adverse in the zone (the relay's recession, 2 a step; provisional).
	const int DecaySteps = 2;

	int HoldSteps()
	{
		return HoldSeconds * StepsPerSecond;
	}

	// One site: its owner (a slot), the count of steps held toward its capture and who is filling it.
	struct FHOLSiteState
	{
		int Owner = -1;
		int Taker = -1;
		int Held = 0;
	}

	// Both count arrays set to Slots zeros.
	void ResetCounts(TArray<int>& Takers, TArray<int>& Disturbers, int Slots)
	{
		Takers.Reset();
		Disturbers.Reset();
		for (int Slot = 0; Slot < Slots; Slot++)
		{
			Takers.Add(0);
			Disturbers.Add(0);
		}
	}

	// One living unit of Slot in the zone. bCanTake: a holy unit of its faction, or the hero. bIsPeasant: the worker, who counts for nothing.
	// A slot out of the arrays is ignored.
	void AddUnit(TArray<int>& Takers, TArray<int>& Disturbers, int Slot, bool bCanTake, bool bIsPeasant)
	{
		if (bIsPeasant || Slot < 0 || Slot >= Takers.Num() || Slot >= Disturbers.Num())
			return;
		Disturbers[Slot] = Disturbers[Slot] + 1;
		if (bCanTake)
			Takers[Slot] = Takers[Slot] + 1;
	}

	// The site after one step. Takers / Disturbers: the units of each slot in the zone; TeamBySlot: the team of each slot (equal teams are one
	// camp). A slot with no unit does not count; a slot out of one of the arrays is ignored.
	FHOLSiteState Step(const FHOLSiteState& State, const TArray<int>& Takers, const TArray<int>& Disturbers, const TArray<int>& TeamBySlot)
	{
		FHOLSiteState Next = State;
		const int Slots = Math::Min(Math::Min(Takers.Num(), Disturbers.Num()), TeamBySlot.Num());
		const int OwnerTeam = (State.Owner >= 0 && State.Owner < TeamBySlot.Num()) ? TeamBySlot[State.Owner] : -1;
		const int TakerTeam = (State.Taker >= 0 && State.Taker < TeamBySlot.Num()) ? TeamBySlot[State.Taker] : -1;
		bool bDefenders = false;
		TArray<int> AdverseTeams;
		int Taker = None;
		int TakerUnits = 0;
		for (int Slot = 0; Slot < Slots; Slot++)
		{
			if (Takers[Slot] <= 0 && Disturbers[Slot] <= 0)
				continue;
			const int Team = TeamBySlot[Slot];
			if (State.Owner >= 0 && Team == OwnerTeam)
			{
				// A disturber of the owner's camp (the owner himself or an ally) defends; a taker of it takes nothing.
				bDefenders = true;
				continue;
			}
			if (!AdverseTeams.Contains(Team))
				AdverseTeams.Add(Team);
			// The taker: the most takers, the lowest slot on a tie (strictly more to replace).
			if (Takers[Slot] > TakerUnits)
			{
				Taker = Slot;
				TakerUnits = Takers[Slot];
			}
		}
		if (AdverseTeams.Num() == 0)
		{
			// Nobody adverse: the count recedes.
			Next.Held = Math::Max(0, State.Held - DecaySteps);
			if (Next.Held == 0)
				Next.Taker = None;
			return Next;
		}
		if (bDefenders || AdverseTeams.Num() > 1)
		{
			// Contested: the count is frozen.
			return Next;
		}
		if (Taker == None)
		{
			// One adverse camp alone, but no taker among it: it takes nothing. It freezes a count another camp had begun, and lets a count of
			// its own camp recede.
			if (State.Held > 0 && TakerTeam != AdverseTeams[0])
				return Next;
			Next.Held = Math::Max(0, State.Held - DecaySteps);
			if (Next.Held == 0)
				Next.Taker = None;
			return Next;
		}
		// One adverse camp alone with a taker: the count goes up, its slot with the most takers shown as the taker. A taker of another camp than
		// the one who began the count starts it again from zero, he inherits nothing (Q2 = B, answered 2026-11-05); an ally's slot goes on.
		const int StartedBy = (State.Held > 0 && State.Taker >= 0 && State.Taker < TeamBySlot.Num()) ? TeamBySlot[State.Taker] : -1;
		const int Held = (StartedBy >= 0 && StartedBy != TeamBySlot[Taker]) ? 0 : State.Held;
		Next.Taker = Taker;
		Next.Held = Held + 1;
		if (Next.Held >= HoldSteps())
		{
			Next.Owner = Taker;
			Next.Taker = None;
			Next.Held = 0;
		}
		return Next;
	}
}
