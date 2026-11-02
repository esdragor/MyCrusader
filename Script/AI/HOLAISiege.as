// What the AI does with the siege (SiegeInProgress.md S6, GDD 6.1): when it builds the Siege Workshop. A pure choice, tested alone in
// script (HOLAISiege_Test.as); the brain reads the summary and calls it (HOLAIBrain::ThinkSiegeBuild). The numbers of the rule are in
// one struct so that the answer to a design question (which difficulties, how big an army first) changes a value, not the rule.
// PROVISIONAL, not a design decision: Hard only (the user: "Difficile oui, Facile non"; Normal is not decided, like the Market's MK-Q3 C),
// from the tier where the Workshop opens (an age 1 building, like the Market), once the army has a first body.

struct FHOLAISiegeSettings
{
	// The lowest difficulty (0 easy, 1 normal, 2 hard) that builds a Siege Workshop; 3 = none.
	int MinDifficulty = 2;

	// The tier from which it builds one (the Workshop is an age 1 building: the second page of the Peasants' grid).
	int BuildTier = 1;

	// It builds one only with this many soldiers already (engines escorted by nobody die): the army comes first.
	int MinSoldiers = 8;
}

// The engines it wants at once (provisional, as the rest): a small train that the army can escort.
struct FHOLAIEngineSettings
{
	int MaxRams = 2;
	int MaxMangonels = 1;
}

// The engine the Workshop queues now: Ram or Mangonel, or none (bQueue false).
struct FHOLAIEngineChoice
{
	bool bQueue = false;
	EHOLSiegeEngine Engine = EHOLSiegeEngine::Ram;
	FName Type = NAME_None;
}

namespace HOLAISiege
{
	// The engine to queue at a finished Workshop whose queue is empty: Hard only (same switch as the Workshop), the Mangonel first once the
	// tier opens it and its train is short, else the Ram while the train of Rams is short. HOLSiege::Check says whether the tier, the Workshop and the
	// stock allow it. Rams and Mangonels are the engines alive plus queued.
	FHOLAIEngineChoice ChooseEngine(const FHOLAISiegeSettings& Rule, const FHOLAIEngineSettings& Train, int Difficulty, int Tier, bool bWorkshopFinished,
		int QueueLength, int Rams, int Mangonels, bool bCanPayRam, bool bCanPayMangonel)
	{
		FHOLAIEngineChoice None;
		if (Difficulty < Rule.MinDifficulty || !bWorkshopFinished || QueueLength > 0)
		{
			return None;
		}
		if (Mangonels < Train.MaxMangonels && HOLSiege::Check(EHOLSiegeEngine::Mangonel, Tier, bWorkshopFinished, bCanPayMangonel) == HOLSiege::Available)
		{
			FHOLAIEngineChoice Choice;
			Choice.bQueue = true;
			Choice.Engine = EHOLSiegeEngine::Mangonel;
			Choice.Type = n"Mangonel";
			return Choice;
		}
		if (Rams < Train.MaxRams && HOLSiege::Check(EHOLSiegeEngine::Ram, Tier, bWorkshopFinished, bCanPayRam) == HOLSiege::Available)
		{
			FHOLAIEngineChoice Choice;
			Choice.bQueue = true;
			Choice.Engine = EHOLSiegeEngine::Ram;
			Choice.Type = n"Ram";
			return Choice;
		}
		return None;
	}

	// Whether the slot builds its Siege Workshop now: the difficulty from Rule.MinDifficulty up, the tier open, no Workshop yet (finished or
	// under way), an enemy centre to go and break, a first army, the wood to pay for it.
	bool WantsWorkshop(const FHOLAISiegeSettings& Rule, int Difficulty, int Tier, bool bHasWorkshop, bool bHasEnemyCentre, int Soldiers,
		int WoodStock, int WoodCost)
	{
		return Difficulty >= Rule.MinDifficulty && Tier >= Rule.BuildTier && !bHasWorkshop && bHasEnemyCentre && Soldiers >= Rule.MinSoldiers
			&& WoodStock >= WoodCost;
	}
}
