// Secondary centres, pure rules (SC1; GDD 5.4, D384, with D22, D347, D351, D388, D397, D403 and the tier grid of 11.1): integers and
// booleans only, no world access, tested in Script/Tests/HOLSecondaryCentreRules_Test.as. A secondary centre is a real forum more, as
// AoE4's town centre: it trains Peasants, takes every resource, holds a garrison and fires, gives a population base and a place for the
// hero's return; unlocked at tier 1 (Essor), costly (mostly stone), no cap (the boom is a bet the rush punishes). It has no lives, never
// protects from defeat (the main forum alone counts, D347), is destroyed as any building (no ruin, D351), and has no anti-build zone (D22)
// nor anti-duel zone (D403). Held standing, it counts for the territory XP as a relay (D388, HOLRelay::TerritoryXP, 10 XP a minute, D587).
// Provisional (Retours/Q-secondary-centers.md): the cost and the population base, one line each below.
namespace HOLSecondaryCentre
{
	// The tier that unlocks it (GDD 11.1: tier 1, Essor).
	const int UnlockTier = 1;

	// D783 (Q-secondary-centers Q1 = B, 2026-11-12): its cost, almost all stone (D384: no miners' camp, no centre).
	const int CostFood = 0;
	const int CostWood = 150;
	const int CostStone = 400;
	const int CostGold = 0;

	// D783 (Q-secondary-centers Q2 = A): its population base, the main forum's (BuildingTypes.csv MainCentre, 10).
	const int PopulationBase = 10;

	struct FHOLCentreCost
	{
		int Food = 0;
		int Wood = 0;
		int Stone = 0;
		int Gold = 0;
	}

	// What a centre does in its state: the main forum (standing or down, D351) or a secondary centre (standing or a foundation).
	struct FHOLCentreRole
	{
		bool bProducesPeasants = false;
		bool bTakesAllResources = false;
		bool bGarrison = false;
		bool bFires = false;
		int Population = 0;
		bool bRespawnPlace = false;
		bool bTerritory = false;
		int Lives = 0;
		bool bCountsForDefeat = false;
		bool bFallsIntoRuin = false;
		bool bAntiBuildZone = false;
		bool bAntiDuelZone = false;
	}

	FHOLCentreCost Cost()
	{
		FHOLCentreCost Result;
		Result.Food = CostFood;
		Result.Wood = CostWood;
		Result.Stone = CostStone;
		Result.Gold = CostGold;
		return Result;
	}

	// A player at Tier who already has Owned secondary centres (finished or not) may order one more: from tier 1, with no cap (D384).
	bool CanOrder(int Tier, int Owned)
	{
		return Tier >= UnlockTier && Owned >= 0;
	}

	// The role of a centre. bPrimary: the main forum of its owner (the start's, the only one with lives). bFinished: built (a foundation
	// does nothing). bDown: the main forum in ruin or rebuilding (D351; never true for a secondary centre, destroyed instead).
	// MainLives: the main forum's lives, given by the match (D349), unused for a secondary centre.
	FHOLCentreRole RoleOf(bool bPrimary, bool bFinished, bool bDown, int MainLives = 0)
	{
		FHOLCentreRole Role;
		if (bPrimary)
		{
			const bool bWorks = !bDown;
			Role.bProducesPeasants = bWorks;
			Role.bTakesAllResources = bWorks;
			Role.bGarrison = bWorks;
			Role.bFires = bWorks;
			// D351: down, the population base, the hero's return (at the ruin) and the zones stay.
			Role.Population = PopulationBase;
			Role.bRespawnPlace = true;
			Role.bTerritory = false;
			Role.Lives = Math::Max(0, MainLives);
			Role.bCountsForDefeat = true;
			Role.bFallsIntoRuin = true;
			Role.bAntiBuildZone = true;
			Role.bAntiDuelZone = true;
			return Role;
		}
		if (!bFinished)
			return Role;
		Role.bProducesPeasants = true;
		Role.bTakesAllResources = true;
		Role.bGarrison = true;
		Role.bFires = true;
		Role.Population = PopulationBase;
		Role.bRespawnPlace = true;
		Role.bTerritory = true;
		return Role;
	}

	// The hero of a player may come back at this centre (D397, 9.8: the main forum, even in ruin, D351, or a secondary centre it holds,
	// standing). bOwn: the centre is the hero's owner's. bFinished: built. bDown: a main forum in ruin or rebuilding.
	bool IsRespawnPlace(bool bPrimary, bool bOwn, bool bFinished, bool bDown)
	{
		if (!bOwn)
			return false;
		return RoleOf(bPrimary, bFinished, bDown).bRespawnPlace;
	}

	// The secondary centres that count for the territory XP among a player's centres (HOLRelay::TerritoryXP's Centres).
	int CountTerritory(const TArray<FHOLCentreRole>& Centres)
	{
		int Count = 0;
		for (const FHOLCentreRole& Role : Centres)
		{
			if (Role.bTerritory)
				Count++;
		}
		return Count;
	}

	// The population base the centres give together (the houses are added elsewhere, up to the cap of 175, D03, D83).
	int PopulationOf(const TArray<FHOLCentreRole>& Centres)
	{
		int Total = 0;
		for (const FHOLCentreRole& Role : Centres)
			Total += Role.Population;
		return Total;
	}
}
