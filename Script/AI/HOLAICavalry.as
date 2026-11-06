// What the AI does with the cavalry (C1, D672, CV-Q4: every level makes cavalry, the quantity depends on the level): when it builds the Stable,
// and the Light Cavalry it queues there. Pure choices, tested alone in script (HOLAICavalry_Test.as); the brain reads the summary and calls them
// (HOLAIBrain::ThinkCavalryBuild, ThinkCavalryProduce), as it does for the siege (HOLAISiege.as, whose shape this follows).
// NOT COMPILED, NOT RUN (written 2026-11-04 without a build, the machine was forbidden to build). The numbers are PROVISIONAL, not a design
// decision: a setting each, so that the answer to a design question changes a value, not the rule.
//
// The levels are those the slot PLAYS now (0 Easy, 1 Normal, 2 Hard): an Evolving slot is resolved before the call (HOLAILives::Played), so it
// makes the cavalry of the stage it is in. Any other number counts as Hard.

// When the Stable is built.
struct FHOLAICavalrySettings
{
	// The lowest level that builds a Stable: 0, every level (D672); 3 = none.
	int MinDifficulty = 0;

	// The tier from which it builds one (the Stable is an age 1 building, GDD 11.1: the second page of the Peasants' grid).
	int BuildTier = 1;

	// It builds one only with this many soldiers already (cavalry alone is torn apart by a Spearman, +17 against it): the army comes first.
	int MinSoldiers = 6;
}

// The cavalry it wants at once, by level (provisional): the quantity is the only thing that changes with the level.
struct FHOLAICavalryTrain
{
	int EasyMax = 2;
	int NormalMax = 4;
	int HardMax = 6;
}

// The cavalry the Stable queues now: Light Cavalry, or nothing (bQueue false).
struct FHOLAICavalryChoice
{
	bool bQueue = false;
	FName Type = NAME_None;
}

namespace HOLAICavalry
{
	// How many Light Cavalry (alive plus queued) a level wants at most.
	int MaxFor(const FHOLAICavalryTrain& Train, int Level)
	{
		if (Level <= 0)
		{
			return Train.EasyMax;
		}
		if (Level == 1)
		{
			return Train.NormalMax;
		}
		return Train.HardMax;
	}

	// Whether the slot builds its Stable now: the level from Rule.MinDifficulty up, the tier open, no Stable yet (finished or under way), an
	// enemy centre to go and strike, a first army, the wood to pay for it.
	bool WantsStable(const FHOLAICavalrySettings& Rule, int Level, int Tier, bool bHasStable, bool bHasEnemyCentre, int Soldiers, int WoodStock,
		int WoodCost)
	{
		return Level >= Rule.MinDifficulty && Tier >= Rule.BuildTier && !bHasStable && bHasEnemyCentre && Soldiers >= Rule.MinSoldiers
			&& WoodStock >= WoodCost;
	}

	// The unit to queue at a finished Stable whose queue is empty: a Light Cavalry while the train of the level is short, the purse pays it
	// and a place is left. Cavalry is the alive plus the queued (HOLAIArmy::CountOfType).
	FHOLAICavalryChoice ChooseCavalry(const FHOLAICavalrySettings& Rule, const FHOLAICavalryTrain& Train, int Level, bool bStableFinished, int QueueLength,
		int Cavalry, bool bCanPay, bool bRoom)
	{
		FHOLAICavalryChoice None;
		if (Level < Rule.MinDifficulty || !bStableFinished || QueueLength > 0 || !bCanPay || !bRoom)
		{
			return None;
		}
		if (Cavalry >= MaxFor(Train, Level))
		{
			return None;
		}
		FHOLAICavalryChoice Choice;
		Choice.bQueue = true;
		Choice.Type = n"LightCavalry";
		return Choice;
	}
}
