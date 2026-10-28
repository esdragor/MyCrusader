// The economy rules of the AI brain (IA0.4 B3): pure functions of a summary, tested alone in script
// (HOLAIEconomy_Test.as, IA0.4-Q2 A); the brain (HOLAIBrain.as) reads the summary, calls them and sends the orders.

// What one thought decides for the idle Peasants: one gather order, or nothing and why.
struct FHOLAIGatherChoice
{
	// True when an order is to be sent.
	bool bGather = false;

	// The resource chosen, in the tie's order: 0 food, 1 wood, 2 stone, 3 gold; -1 without an order.
	int Resource = -1;

	// Its name, for the log: "Food", "Wood", "Stone", "Gold"; empty without an order.
	FString ResourceName;

	// The source to gather (SourceIndex of the resource's Sources[0], its nearest); -1 without an order.
	int SourceIndex = -1;

	// The Peasants to send: all the summary's idle ones; empty without an order.
	TArray<int> Peasants;

	// Why nothing: "no idle" or "no source"; empty with an order.
	FString Nothing;
}

namespace HOLAIEconomy
{
	// First rule (IA0.4-Q1 A; replaced by the needs of IA1.6): with idle Peasants, the resource is the lowest stock among
	// those with at least one source (a tie: food, wood, stone, gold), and all the idle Peasants go to its Sources[0].
	// No idle Peasant or no source: nothing.
	FHOLAIGatherChoice ChooseGather(const FHOLAISummary& Summary)
	{
		FHOLAIGatherChoice Choice;
		if (Summary.IdlePeasants.Num() == 0)
		{
			Choice.Nothing = "no idle";
			return Choice;
		}

		// The four parts in the tie's order: the first lowest stock wins.
		TArray<FHOLAIResourceSummary> Parts;
		Parts.Add(Summary.Food);
		Parts.Add(Summary.Wood);
		Parts.Add(Summary.Stone);
		Parts.Add(Summary.Gold);
		TArray<FString> Names;
		Names.Add("Food");
		Names.Add("Wood");
		Names.Add("Stone");
		Names.Add("Gold");

		int Best = -1;
		for (int Index = 0; Index < Parts.Num(); Index++)
		{
			if (Parts[Index].Sources.Num() == 0)
			{
				continue;
			}
			if (Best < 0 || Parts[Index].Stock < Parts[Best].Stock)
			{
				Best = Index;
			}
		}
		if (Best < 0)
		{
			Choice.Nothing = "no source";
			return Choice;
		}

		Choice.bGather = true;
		Choice.Resource = Best;
		Choice.ResourceName = Names[Best];
		Choice.SourceIndex = Parts[Best].Sources[0].SourceIndex;
		Choice.Peasants = Summary.IdlePeasants;
		return Choice;
	}
}
