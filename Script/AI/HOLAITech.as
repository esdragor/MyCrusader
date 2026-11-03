// What the AI researches (T7, GDD 11; TechnologiesInProgress.md): a pure choice over the technology table, what the slot has researched, its
// buildings, its stock and its hero's tier, tested alone in script (HOLAITech_Test.as). One research at a time per slot, in the order of
// the settings; the research is the production order the AI already has (a technology is an item of the building's queue, T-Q6, D583).
// Every level researches (D648); what differs is the DELAY, in game seconds before its first research: Hard first (0: the economy test below is its only wait),
// Normal later, Easy much later. The delays are the console variables hol.AI.TechDelayEasy / Normal / Hard (Hard 0, Normal 6 min, Easy 14 min: the user's answer A, D657). An Evolving AI is resolved by the
// caller to the level it plays now (HOLAILives::Played), so it follows its stage. Resources by index as everywhere in script.

struct FHOLAITechSettings
{
	// By level played (0 easy, 1 normal, 2 hard): the game seconds before the AI researches its first technology (D648).
	TArray<float> DelaySeconds;

	// Not before this many Peasants are alive (the economy comes first).
	int MinPeasants = 20;

	// What a research must leave in the stock (food, wood, stone, gold) on top of its cost; the default keeps none.
	TArray<int> Reserve;

	// The technologies it researches, most wanted first (the economy lines of the table: food, wood, carry, then the mines). A technology not
	// listed is never researched (the Forge's, the Chapel's: they come with the army and the Monk).
	TArray<FName> Order;

	// It saves for the technology it cannot pay yet: for at most SaveSeconds the brain holds back from the other purchases what the stock already has of its cost
	// (the AI spends all it has at each thought, its stock never reaches a cost otherwise); then it gives up for GiveUpSeconds so that houses and units are not
	// starved for long.
	float SaveSeconds = 45.f;
	float GiveUpSeconds = 150.f;
}

// What the brain remembers per slot: since when it saves (-1 = not saving) and until when it has given up.
struct FHOLAITechMemory
{
	float SavingSince = -1.f;
	float GivenUpUntil = -1.f;
}

// What the AI wants to do.
struct FHOLAITechChoice
{
	bool bResearch = false;
	FName Technology;
	FVector BuildingAt;
	FString Reason;

	// Saving for Technology (only the purse is missing): what the other purchases must leave in the stock, by resource (zeros when not saving).
	bool bSaving = false;
	TArray<int> Earmark;
}

namespace HOLAITech
{
	// The settings with their provisional values.
	FHOLAITechSettings DefaultSettings()
	{
		FHOLAITechSettings Rule;
		for (int Level = 0; Level < 3; Level++)
		{
			Rule.DelaySeconds.Add(HOLAITechnology::GetAITechDelay(Level));
		}
		// None kept (measured: the AI spends all it has, its stock stays under ~150 food and ~70 wood, a reserve on top of the cost never came true).
		Rule.Reserve.Add(0);
		Rule.Reserve.Add(0);
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

	// The one research the AI wants now, or none. Stock by resource; Peasants alive; HeroTier and HeroLevel the hero's; Difficulty 0 easy, 1 normal, 2 hard
	// (not read here: the level's delay is Think's).
	// Held back by the economy (Rule.MinPeasants), a research already in a queue (one at a time). Then the technologies of
	// Rule.Order, most wanted first: one not open (researched, tier, line, no finished building, effect not coded) is passed over; the first
	// OPEN one it cannot pay with the reserve left is waited for (the AI saves, it does not spend on a lesser one at once).
	FHOLAITechChoice Choose(const FHOLAITechSettings& Rule, const TArray<FHOLAITechnology>& Table, const TArray<FName>& Done,
		const TArray<FHOLAIBuilding>& Buildings, const TArray<int>& Stock, int Peasants, int HeroTier, int HeroLevel, int Difficulty)
	{
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
				FHOLAITechChoice Saving = None(f"saving for {Technology.Name}");
				Saving.bSaving = true;
				Saving.Technology = Technology.Name;
				Saving.Earmark.Add(Math::Min(Stock[0], Technology.CostFood));
				Saving.Earmark.Add(Math::Min(Stock[1], Technology.CostWood));
				Saving.Earmark.Add(Math::Min(Stock[2], Technology.CostStone));
				Saving.Earmark.Add(Math::Min(Stock[3], Technology.CostGold));
				return Saving;
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
			// The cost, for the brain to take out of the stock its other purchases of the thought see (the production pays at the next step).
			Choice.Earmark.Add(Technology.CostFood);
			Choice.Earmark.Add(Technology.CostWood);
			Choice.Earmark.Add(Technology.CostStone);
			Choice.Earmark.Add(Technology.CostGold);
			return Choice;
		}
		return None("nothing open");
	}

	// Choose with the delay of the level played (nothing before it) and the memory of saving: a save longer than Rule.SaveSeconds is given up (nothing earmarked)
	// for Rule.GiveUpSeconds. Now in game seconds.
	FHOLAITechChoice Think(const FHOLAITechSettings& Rule, FHOLAITechMemory& Memory, float Now, const TArray<FHOLAITechnology>& Table, const TArray<FName>& Done,
		const TArray<FHOLAIBuilding>& Buildings, const TArray<int>& Stock, int Peasants, int HeroTier, int HeroLevel, int Difficulty)
	{
		const int Level = Math::Clamp(Difficulty, 0, Rule.DelaySeconds.Num() - 1);
		if (Level >= 0 && Now < Rule.DelaySeconds[Level])
		{
			Memory.SavingSince = -1.f;
			return None("too early for this level");
		}
		FHOLAITechChoice Choice = Choose(Rule, Table, Done, Buildings, Stock, Peasants, HeroTier, HeroLevel, Difficulty);
		if (!Choice.bSaving)
		{
			Memory.SavingSince = -1.f;
			return Choice;
		}
		if (Now < Memory.GivenUpUntil)
		{
			return None("gave up for now");
		}
		if (Memory.SavingSince < 0.f)
		{
			Memory.SavingSince = Now;
		}
		if (Now - Memory.SavingSince > Rule.SaveSeconds)
		{
			Memory.SavingSince = -1.f;
			Memory.GivenUpUntil = Now + Rule.GiveUpSeconds;
			return None("gave up saving");
		}
		return Choice;
	}
}
