// What the AI researches (T7, GDD 11; TechnologiesInProgress.md): a pure choice over the technology table, what the slot has researched, its
// buildings, its stock and its hero's tier, tested alone in script (HOLAITech_Test.as). One research at a time per slot, in the order of
// the settings; the research is the production order the AI already has (a technology is an item of the building's queue, T-Q6, D583).
// Which difficulties research is NOT decided by the GDD: MinDifficulty is provisional (the brain sets it from hol.AI.TechMinLevel, 1 by
// default = Normal and Hard; the question is in Retours\Q-ia-technologies.md). Resources by index as everywhere in script.

struct FHOLAITechSettings
{
	// The lowest difficulty (0 easy, 1 normal, 2 hard) that researches; 3 = none. Provisional.
	int MinDifficulty = 1;

	// Not before this many Peasants are alive (the economy comes first).
	int MinPeasants = 20;

	// What a research must leave in the stock (food, wood, stone, gold): a Peasant, a house.
	TArray<int> Reserve;

	// The technologies it researches, most wanted first (the economy lines of the table: food, wood, carry, then the mines). A technology not
	// listed is never researched (the Forge's, the Chapel's: they come with the army and the Monk).
	TArray<FName> Order;
}

// What the AI wants to do.
struct FHOLAITechChoice
{
	bool bResearch = false;
	FName Technology;
	FVector BuildingAt;
	FString Reason;
}

namespace HOLAITech
{
	// The settings with their provisional values.
	FHOLAITechSettings DefaultSettings()
	{
		FHOLAITechSettings Rule;
		Rule.Reserve.Add(100);
		Rule.Reserve.Add(100);
		Rule.Reserve.Add(0);
		Rule.Reserve.Add(0);
		Rule.Order.Add(n"PloughI");
		Rule.Order.Add(n"DoubleAxesI");
		Rule.Order.Add(n"WheelbarrowI");
		Rule.Order.Add(n"MinerPicksI");
		Rule.Order.Add(n"PloughII");
		Rule.Order.Add(n"DoubleAxesII");
		Rule.Order.Add(n"WheelbarrowII");
		Rule.Order.Add(n"MinerPicksII");
		return Rule;
	}

	FHOLAITechChoice None(FString Reason)
	{
		FHOLAITechChoice Choice;
		Choice.Reason = Reason;
		return Choice;
	}

	// The one research the AI wants now, or none. Stock by resource; Peasants alive; HeroTier and HeroLevel the hero's; Difficulty 0 easy, 1 normal, 2 hard.
	// Held back by the difficulty, the economy (Rule.MinPeasants), a research already in a queue (one at a time). Then the technologies of
	// Rule.Order, most wanted first: one not open (researched, tier, line, no finished building, effect not coded) is passed over; the first
	// OPEN one it cannot pay with the reserve left is waited for (the AI saves, it does not spend on a lesser one at once).
	FHOLAITechChoice Choose(const FHOLAITechSettings& Rule, const TArray<FHOLAITechnology>& Table, const TArray<FName>& Done,
		const TArray<FHOLAIBuilding>& Buildings, const TArray<int>& Stock, int Peasants, int HeroTier, int HeroLevel, int Difficulty)
	{
		if (Difficulty < Rule.MinDifficulty)
		{
			return None("difficulty");
		}
		if (Peasants < Rule.MinPeasants)
		{
			return None("economy first");
		}
		// A research in any queue of the slot: the names of the table in the queues.
		for (int Each = 0; Each < Buildings.Num(); Each++)
		{
			for (int Item = 0; Item < Buildings[Each].Queue.Num(); Item++)
			{
				for (int Row = 0; Row < Table.Num(); Row++)
				{
					if (Table[Row].Name == Buildings[Each].Queue[Item])
					{
						return None("one at a time");
					}
				}
			}
		}
		for (int Wanted = 0; Wanted < Rule.Order.Num(); Wanted++)
		{
			int RowIndex = -1;
			for (int Row = 0; Row < Table.Num(); Row++)
			{
				if (Table[Row].Name == Rule.Order[Wanted])
				{
					RowIndex = Row;
					break;
				}
			}
			if (RowIndex < 0)
			{
				continue;
			}
			const FHOLAITechnology Technology = Table[RowIndex];

			// The finished building that researches it, with nothing in its queue (a research holds the building's units back, T-Q8).
			int BuildingIndex = -1;
			for (int Each = 0; Each < Buildings.Num(); Each++)
			{
				if (Buildings[Each].TypeName == Technology.Building && Buildings[Each].State == EHOLConstructionState::Finished && Buildings[Each].Queue.Num() == 0)
				{
					BuildingIndex = Each;
					break;
				}
			}
			bool bRequiresDone = Technology.Requires.IsNone() || Done.Contains(Technology.Requires);
			bool bCanPay = Stock[0] >= Technology.CostFood + Rule.Reserve[0] && Stock[1] >= Technology.CostWood + Rule.Reserve[1]
				&& Stock[2] >= Technology.CostStone + Rule.Reserve[2] && Stock[3] >= Technology.CostGold + Rule.Reserve[3];
			const int Block = HOLTechnologyRules::CheckImplemented(Technology.bImplemented, HeroTier, HeroLevel, Technology.MinTier, Technology.MinHeroLevel,
				bRequiresDone, BuildingIndex >= 0, bCanPay, Done.Contains(Technology.Name), false);
			if (Block == HOLTechnologyRules::CannotPay)
			{
				return None(f"saving for {Technology.Name}");
			}
			if (Block != HOLTechnologyRules::Available)
			{
				continue;
			}
			FHOLAITechChoice Choice;
			Choice.bResearch = true;
			Choice.Technology = Technology.Name;
			Choice.BuildingAt = Buildings[BuildingIndex].Location;
			Choice.Reason = f"the economy line {Technology.Name}";
			return Choice;
		}
		return None("nothing open");
	}
}
