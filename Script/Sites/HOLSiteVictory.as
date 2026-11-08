// Sacred sites, victory count and gold by the minute, pure rules (SacredSitesInProgress.md S10 and S6; GDD 14.3 Sacred sites, D429 to D431,
// D699, D703): integers only, no world access, tested in HOLSiteVictory_Test.as. The victory: the CAMP (a player alone or all his mutual
// allies, the team of the site owners' slots) that holds EVERY site for 600 s (D747, a constant of the game) wins; a site with an adverse unit in
// its zone freezes the count (it never recedes); a site changing hands, even to another slot of the same camp, resets it, and so does an alliance
// breaking (the teams change: the camp that held all is two camps); two camps cannot count at once (a camp must hold every site). The winner
// once set stays. Not here: the defeat of the adversaries (the new reason of EHOLDefeatReason, S10's C++), the announcements, the display.
// Gold by the minute (D699: to the taker alone, with the carry of fractions so that a minute of one site gives exactly its rate whatever the
// simulation rate, the pattern of HOLRelay::TerritoryXP): the RATE is an argument, never a constant here, because the user has not chosen it (D345).
namespace HOLSiteVictory
{
	// No leader / no winner.
	const int None = -1;

	// Seconds a camp must hold every site (600 s, chosen by the user, D747, 2026-11-08).
	const int VictorySeconds = 600;

	int VictorySteps()
	{
		return VictorySeconds * HOLHero::GetSimulationRateHz();
	}

	// The count: the camp (a team) that holds every site now, the steps held, the winning camp once the count is full, and the owners of the sites
	// at the last step (to see a site change hands).
	struct FHOLSiteVictoryState
	{
		int Leader = -1;
		int Held = 0;
		int Winner = -1;
		TArray<int> LastOwners;
	}

	// The camp holding every site: the team of the owners' slots when every site has an owner and the owners are all of one team, else None.
	int HoldingCamp(const TArray<int>& OwnerBySite, const TArray<int>& TeamBySlot)
	{
		if (OwnerBySite.Num() == 0)
			return None;
		int Camp = None;
		for (int Site = 0; Site < OwnerBySite.Num(); Site++)
		{
			const int Owner = OwnerBySite[Site];
			if (Owner < 0 || Owner >= TeamBySlot.Num())
				return None;
			const int Team = TeamBySlot[Owner];
			if (Site == 0)
				Camp = Team;
			else if (Team != Camp)
				return None;
		}
		return Camp;
	}

	// True when the two lists of owners differ (a site changed hands, a site appeared or went).
	bool OwnersDiffer(const TArray<int>& A, const TArray<int>& B)
	{
		if (A.Num() != B.Num())
			return true;
		for (int Index = 0; Index < A.Num(); Index++)
		{
			if (A[Index] != B[Index])
				return true;
		}
		return false;
	}

	// The count after one step. OwnerBySite: the owner slot of each site (-1: nobody's); ContestedBySite: nonzero when an adverse unit stands in
	// the site's zone; TeamBySlot: the team of each slot.
	FHOLSiteVictoryState Step(const FHOLSiteVictoryState& State, const TArray<int>& OwnerBySite, const TArray<int>& ContestedBySite, const TArray<int>& TeamBySlot)
	{
		if (State.Winner >= 0)
			return State;
		FHOLSiteVictoryState Next = State;
		Next.LastOwners = OwnerBySite;
		const int Camp = HoldingCamp(OwnerBySite, TeamBySlot);
		if (Camp == None)
		{
			Next.Leader = None;
			Next.Held = 0;
			return Next;
		}
		if (Camp != State.Leader || OwnersDiffer(State.LastOwners, OwnerBySite))
		{
			Next.Leader = Camp;
			Next.Held = 0;
		}
		for (int Site = 0; Site < ContestedBySite.Num(); Site++)
		{
			if (ContestedBySite[Site] != 0)
				return Next;
		}
		Next.Held = Next.Held + 1;
		if (Next.Held >= VictorySteps())
			Next.Winner = Next.Leader;
		return Next;
	}

	// Gold: the fractions carried from a step to the next (rate x sites, in gold x steps).
	struct FHOLGoldCarry
	{
		int Carry = 0;
	}

	// The whole gold to give the taker of Sites sites for this step (0 most steps); a negative count or rate is nothing.
	int GoldStep(FHOLGoldCarry& Memory, int Sites, int RatePerMinute)
	{
		const int StepsPerMinute = 60 * HOLHero::GetSimulationRateHz();
		Memory.Carry += Math::Max(0, Sites) * Math::Max(0, RatePerMinute);
		const int Award = Math::IntegerDivisionTrunc(Memory.Carry, StepsPerMinute);
		Memory.Carry -= Award * StepsPerMinute;
		return Award;
	}
}
