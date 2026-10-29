// The needs of the AI brain (IA1.6), pure rules over a summary (IA1.3) and the opening's data, tested alone in script
// (HOLAINeeds_Test.as): what the slot wants to spend over the next minute, the Peasants that spending asks of each
// resource, and the share of the Peasants each resource should hold. The opening rules (HOLAIOpening.as) spread the idle
// Peasants and aim the centre's rally by these shares instead of fixed weights; the brain (HOLAIBrain.as) reads the costs
// from the type tables (UHOLAIOrderLibrary::ReadAIUnitFigures / ReadAIBuildingFigures) and puts them in the data.
// What is not planned yet comes with its tranche: the farms and the next houses (IA1.7), the army (IA2) through
// FHOLAIOpeningData::Reserve, tuned by AI-only games (IA4).

// What the slot wants over the window, by resource (0 food, 1 wood, 2 stone, 3 gold).
struct FHOLAINeeds
{
	// What it wants to spend over the window.
	TArray<float> Spend;

	// What the sources must give over the window: the spending, less a part of the stock already there (never under
	// FHOLAIOpeningData::MinDemandShare of the spending).
	TArray<float> Demand;

	// The Peasants that demand asks of the resource, its nearest source's round trip counted (0 without a source).
	TArray<float> Wanted;

	// The share of the Peasants each resource should hold, summing to 1 (all 0 when no resource has a source).
	TArray<float> Share;

	// Nothing to spend: the shares come from the weights of the data.
	bool bFromWeights = false;
}

namespace HOLAINeeds
{
	float RateOf(const FHOLAITypeFigures& Figures, int Resource)
	{
		if (Resource == 0)
		{
			return Figures.GatherFood;
		}
		if (Resource == 1)
		{
			return Figures.GatherWood;
		}
		if (Resource == 2)
		{
			return Figures.GatherStone;
		}
		return Figures.GatherGold;
	}

	// What a Peasant brings home per second from the resource's nearest source: its gathering rate, slowed by the round trip
	// to the deposit (the load over the time to gather it plus the walk there and back). 0 without a source or a rate.
	float NetRate(const FHOLAIOpeningData& Data, const FHOLAISummary& Summary, int Resource)
	{
		const FHOLAIResourceSummary Part = HOLAIOpening::PartOf(Summary, Resource);
		const float Rate = RateOf(Data.Peasant, Resource);
		if (Part.Sources.Num() == 0 || Rate <= 0.f)
		{
			return 0.f;
		}
		const float Load = Math::Max(1.f, float(Data.Peasant.CarryCapacity));
		float Trip = 0.f;
		if (Data.Peasant.Speed > 0.f)
		{
			Trip = 2.f * Part.Sources[0].DepositDistance / Data.Peasant.Speed;
		}
		return Load / (Load / Rate + Trip);
	}

	// The Peasants the slot has or is making: alive and in a queue.
	int PeasantsCounted(const FHOLAISummary& Summary)
	{
		int Queued = 0;
		for (int Index = 0; Index < Summary.Buildings.Num(); Index++)
		{
			for (int InQueue = 0; InQueue < Summary.Buildings[Index].Queue.Num(); InQueue++)
			{
				if (Summary.Buildings[Index].Queue[InQueue] == n"Peasant")
				{
					Queued++;
				}
			}
		}
		return Summary.PeasantIds.Num() + Queued;
	}

	// What it wants to spend over the window (the figures are per resource, 0 food to 3 gold): the centre's Peasants while
	// under the target, with the houses they need, the next building of the opening, and the reserve of the data.
	TArray<float> SpendOf(const FHOLAISummary& Summary, const FHOLAIOpeningData& Data)
	{
		TArray<float> Spend;
		Spend.Add(Data.ReserveFood);
		Spend.Add(Data.ReserveWood);
		Spend.Add(Data.ReserveStone);
		Spend.Add(Data.ReserveGold);

		// The centre keeps producing Peasants: one every BuildTime seconds, each paid and each needing its place.
		if (Summary.bHasCentre && Data.Peasant.BuildTime > 0.f && PeasantsCounted(Summary) < Data.PeasantTarget)
		{
			const float PerWindow = Data.WindowSeconds / Data.Peasant.BuildTime;
			Spend[0] = Spend[0] + PerWindow * float(Data.Peasant.CostFood);
			Spend[1] = Spend[1] + PerWindow * float(Data.Peasant.CostWood);
			Spend[2] = Spend[2] + PerWindow * float(Data.Peasant.CostStone);
			Spend[3] = Spend[3] + PerWindow * float(Data.Peasant.CostGold);
			if (Data.House.Population > 0)
			{
				const float Houses = PerWindow * float(Data.Peasant.Population) / float(Data.House.Population);
				Spend[1] = Spend[1] + Houses * float(Data.House.CostWood);
			}
		}

		// The next building of the list (the houses are the rate above).
		for (int Index = 0; Index < Data.Steps.Num(); Index++)
		{
			if (!HOLAIOpening::HasBuilding(Summary, Data.Steps[Index].Type))
			{
				Spend[1] = Spend[1] + float(Data.Steps[Index].WoodCost);
				break;
			}
		}

		// The farms of the block while the berries run low: the ones not built yet (IA1.7).
		if (HOLAIOpening::FarmsWanted(Summary, Data))
		{
			const int Missing = Math::Max(0, Data.FarmCount - HOLAIOpening::CountBuildings(Summary, n"Farm"));
			Spend[1] = Spend[1] + float(Missing * Data.Farm.CostWood);
		}
		AddArmySpend(Spend, Summary, Data);
		return Spend;
	}

	// The economy is established (IA2.4): enough Peasants, alive and queued, for the army to take a share of the spending.
	bool ArmyEstablished(const FHOLAISummary& Summary, const FHOLAIOpeningData& Data)
	{
		return Summary.bHasCentre && PeasantsCounted(Summary) >= Data.ArmyMinPeasants;
	}

	// The army's share of the spending (IA2.4): 0 until the economy is established, then the base rising linearly with the game's time to
	// the max, plus the bump while the enemy's army seen is stronger than ours (PowerRatio under 1); never over the cap.
	float ArmyShareOf(const FHOLAISummary& Summary, const FHOLAIOpeningData& Data)
	{
		if (!ArmyEstablished(Summary, Data))
		{
			return 0.f;
		}
		float Ramp = 1.f;
		if (Data.ArmyRampSeconds > 0.f)
		{
			Ramp = Math::Clamp(Data.GameSeconds / Data.ArmyRampSeconds, 0.f, 1.f);
		}
		float Share = Data.ArmyShareBase + (Data.ArmyShareMax - Data.ArmyShareBase) * Ramp;
		if (Summary.Enemies.Num() > 0 && HOLAIArmy::PowerRatio(Data.Army, Summary) < 1.f)
		{
			Share += Data.ArmyThreatBump;
		}
		return Math::Min(Share, Data.ArmyShareCap);
	}

	// What the slot's Peasants bring over the window: their number times the mean net rate of the resources that have a source.
	float IncomeOf(const FHOLAISummary& Summary, const FHOLAIOpeningData& Data)
	{
		float Sum = 0.f;
		int Counted = 0;
		for (int Resource = 0; Resource < 4; Resource++)
		{
			const float Net = NetRate(Data, Summary, Resource);
			if (Net > 0.f)
			{
				Sum += Net;
				Counted++;
			}
		}
		if (Counted == 0)
		{
			return 0.f;
		}
		return float(Summary.PeasantIds.Num()) * Sum / float(Counted) * Data.WindowSeconds;
	}

	// The army's part of the spending (IA2.4): its share of the income over the window, split between the military buildings; each
	// building spends it on the unit the composition would queue there (HOLAIArmy::ChooseComposition restricted to it), so the resources
	// asked of the Peasants follow the units (gold for the Man-at-arms and the Crossbowman), within what the buildings can make in the window.
	void AddArmySpend(TArray<float>& Spend, const FHOLAISummary& Summary, const FHOLAIOpeningData& Data)
	{
		const float Share = ArmyShareOf(Summary, Data);
		if (Share <= 0.f)
		{
			return;
		}
		const TArray<FName> Buildings = HOLAIArmy::BuildingsOf(Data.Army);
		if (Buildings.Num() == 0)
		{
			return;
		}
		const float Budget = Share * IncomeOf(Summary, Data) / float(Buildings.Num());
		for (int Index = 0; Index < Buildings.Num(); Index++)
		{
			const FHOLAIComposition Choice = HOLAIArmy::ChooseComposition(Summary, Data.Army, Buildings[Index]);
			if (!Choice.bValid)
			{
				continue;
			}
			const FHOLAITypeFigures Unit = HOLAIArmy::FiguresOf(Data.Army, Choice.Type);
			const float Cost = float(Unit.CostFood + Unit.CostWood + Unit.CostStone + Unit.CostGold);
			if (Cost <= 0.f)
			{
				continue;
			}
			float Units = Budget / Cost;
			if (Unit.BuildTime > 0.f)
			{
				Units = Math::Min(Units, Data.WindowSeconds / Unit.BuildTime * float(Math::Max(1, HOLAIOpening::CountBuildings(Summary, Buildings[Index]))));
			}
			Spend[0] = Spend[0] + Units * float(Unit.CostFood);
			Spend[1] = Spend[1] + Units * float(Unit.CostWood);
			Spend[2] = Spend[2] + Units * float(Unit.CostStone);
			Spend[3] = Spend[3] + Units * float(Unit.CostGold);
		}
	}

	// The needs of the slot: spending, demand, Peasants wanted and shares.
	FHOLAINeeds Compute(const FHOLAISummary& Summary, const FHOLAIOpeningData& Data)
	{
		FHOLAINeeds Needs;
		Needs.Spend = SpendOf(Summary, Data);
		float TotalWanted = 0.f;
		for (int Resource = 0; Resource < 4; Resource++)
		{
			const float Spend = Needs.Spend[Resource];
			const int Stock = HOLAIOpening::PartOf(Summary, Resource).Stock;
			float Demand = 0.f;
			if (Spend > 0.f)
			{
				Demand = Math::Max(Data.MinDemandShare * Spend, Spend - Data.StockShare * float(Stock));
			}
			Needs.Demand.Add(Demand);
			const float Net = NetRate(Data, Summary, Resource);
			float Wanted = 0.f;
			if (Net > 0.f)
			{
				Wanted = Demand / Data.WindowSeconds / Net;
			}
			Needs.Wanted.Add(Wanted);
			TotalWanted += Wanted;
		}

		// Nothing wanted from a resource with a source: the weights of the data, for the resources with a source.
		if (TotalWanted <= 0.f)
		{
			Needs.bFromWeights = true;
			float SumWeights = 0.f;
			for (int Resource = 0; Resource < 4; Resource++)
			{
				if (HOLAIOpening::PartOf(Summary, Resource).Sources.Num() > 0)
				{
					SumWeights += float(HOLAIOpening::WeightOf(Data, Resource));
				}
			}
			for (int Resource = 0; Resource < 4; Resource++)
			{
				float Share = 0.f;
				if (SumWeights > 0.f && HOLAIOpening::PartOf(Summary, Resource).Sources.Num() > 0)
				{
					Share = float(HOLAIOpening::WeightOf(Data, Resource)) / SumWeights;
				}
				Needs.Share.Add(Share);
			}
			return Needs;
		}
		for (int Resource = 0; Resource < 4; Resource++)
		{
			Needs.Share.Add(Needs.Wanted[Resource] / TotalWanted);
		}
		return Needs;
	}

	// The resource with the biggest deficit of Peasants against its share of Total Peasants, among those with a share (a tie:
	// the first in the order food, wood, stone, gold); -1 when none. Counts holds the Peasants per resource.
	int MostLacking(const FHOLAINeeds& Needs, const TArray<int>& Counts, int Total)
	{
		int Best = -1;
		float BestDeficit = 0.f;
		for (int Resource = 0; Resource < 4; Resource++)
		{
			if (Needs.Share[Resource] <= 0.f)
			{
				continue;
			}
			const float Deficit = float(Total) * Needs.Share[Resource] - float(Counts[Resource]);
			if (Best < 0 || Deficit > BestDeficit)
			{
				Best = Resource;
				BestDeficit = Deficit;
			}
		}
		return Best;
	}
}
