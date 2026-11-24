// What the AI does with the Watch Tower (point 7 of Priorite_Socle_Factions, FARAMIR-1124sg): when it builds one. Until now the AI built no tower
// at all (no `TowerWood` in the AI scripts), so its garrison call (HOLAIGarrisonCall.as) had no tower to read in a match. Pure choice, tested alone
// in script (HOLAITower_Test.as); the brain reads the summary and calls it (HOLAIBrain::ThinkTowerBuild), as for the Stable (HOLAICavalry.as, whose
// shape this follows). The numbers are PROVISIONAL, not a design decision: a setting each (Retours\Q-ai-towers.md), so that the answer changes a value.
//
// The levels are those the slot PLAYS now (0 Easy, 1 Normal, 2 Hard); any other number counts as Hard.

struct FHOLAITowerSettings
{
	// How many Watch Towers (finished or under way) a level wants: Easy none, Normal one, Hard two.
	int EasyMax = 0;
	int NormalMax = 1;
	int HardMax = 2;

	// The tier from which it builds one (the Watch Tower is a tier 0 building, BuildingTypes.csv).
	int BuildTier = 0;

	// It builds one only with this many soldiers already: the army, then the tower that its soldiers can enter.
	int MinSoldiers = 4;
}

namespace HOLAITower
{
	// How many towers a level wants at most.
	int MaxFor(const FHOLAITowerSettings& Rule, int Level)
	{
		if (Level <= 0)
		{
			return Rule.EasyMax;
		}
		if (Level == 1)
		{
			return Rule.NormalMax;
		}
		return Rule.HardMax;
	}

	// Whether the slot builds a tower now: the level wants more than it has (towers finished or under way), the tier open, an enemy centre to
	// guard against, a first army, the wood to pay for it.
	bool WantsTower(const FHOLAITowerSettings& Rule, int Level, int Tier, int Towers, bool bHasEnemyCentre, int Soldiers, int WoodStock, int WoodCost)
	{
		return Towers < MaxFor(Rule, Level) && Tier >= Rule.BuildTier && bHasEnemyCentre && Soldiers >= Rule.MinSoldiers && WoodStock >= WoodCost;
	}
}
