// The opening of the AI brain (IA1.5), written as data and pure rules over a summary (IA1.3), tested alone in script
// (HOLAIOpening_Test.as): how the starting Peasants spread, which building comes next and where, when a Peasant is queued
// and where it is sent. The brain (HOLAIBrain.as) reads the summary, asks these rules and sends the orders (IA1.4).
// IA1.6 (needs) and IA1.7 (buildings) take over from this opening.

// One building of the opening, in the order of the list.
struct FHOLAIOpeningStep
{
	// The building's type name (BuildingTypes.csv).
	FName Type;

	// Wood needed in stock to place it (its Cost in BuildingTypes.csv: the server refuses and logs the order otherwise).
	int WoodCost = 0;

	// Built beside the nearest source of this resource (0 food, 1 wood, 2 stone, 3 gold); -1: near the centre, on the side
	// away from the nearest food and wood.
	int NearResource = -1;

	// Waits until the places left are few (HOLAIOpeningData::HouseMargin): the house.
	bool bWhenRoomLow = false;
}

// The numbers of the opening.
struct FHOLAIOpeningData
{
	// Share of the Peasants per resource, in the order food, wood, stone, gold (provisional: IA4 tunes it on AI-only games).
	int WeightFood = 3;
	int WeightWood = 2;
	int WeightStone = 0;
	int WeightGold = 0;

	// Peasants waiting in the centre's queue (the one in production counts).
	int QueueLength = 2;

	// Food of one Peasant (UnitTypes.csv).
	int PeasantFood = 50;

	// Peasants (alive and queued) up to which the centre keeps producing (indicative: 80, GDD 5.3; IA1.6 decides).
	int PeasantTarget = 80;

	// The house is placed when the places left (cap - used - waiting in a queue) fall to this.
	int HouseMargin = 3;

	// Distance (cm) from the centre, on the side away from the sources, where the search for the house's spot starts.
	float AwayDistance = 1500.f;

	// Rings of tiles searched around the anchor (FindAIPlacement).
	int Rings = 8;

	// The buildings, in order: the next one is the first whose type the slot has none of (in any state).
	TArray<FHOLAIOpeningStep> Steps;
}

// The building the opening wants now: where to look for its spot and who builds it.
struct FHOLAIBuildChoice
{
	bool bBuild = false;
	FName Type;
	// Where the search for the spot starts.
	FVector Anchor;
	// The Peasant sent to build it (an idle one first), -1 without an order.
	int Builder = -1;
	// Why nothing: "done", "not yet", "no wood", "no source", "no builder", "no centre"; empty with an order.
	FString Nothing;
}

// A Peasant to queue at the centre and where the new Peasants go.
struct FHOLAIProduceChoice
{
	bool bProduce = false;
	// The centre's ground point (names the building to the order library).
	FVector Centre;
	// Why nothing: "no centre", "queue full", "no room", "no food", "target reached"; empty with an order.
	FString Nothing;
	// The rally: the resource (-1: none, no source) and its nearest source.
	int RallyResource = -1;
	int RallySource = -1;
	FVector RallyPoint;
}

// Some idle Peasants sent to one source.
struct FHOLAIGatherGroup
{
	int Resource = -1;
	FString ResourceName;
	int SourceIndex = -1;
	TArray<int> Peasants;
}

namespace HOLAIOpening
{
	// The opening as the game plays it: house, then lumber camp near the wood, then mill near the food (costs from
	// BuildingTypes.csv).
	FHOLAIOpeningData DefaultData()
	{
		FHOLAIOpeningData Data;
		FHOLAIOpeningStep House;
		House.Type = n"House";
		House.WoodCost = 50;
		House.bWhenRoomLow = true;
		Data.Steps.Add(House);
		FHOLAIOpeningStep Camp;
		Camp.Type = n"LumberCamp";
		Camp.WoodCost = 100;
		Camp.NearResource = 1;
		Data.Steps.Add(Camp);
		FHOLAIOpeningStep Mill;
		Mill.Type = n"Mill";
		Mill.WoodCost = 100;
		Mill.NearResource = 0;
		Data.Steps.Add(Mill);
		return Data;
	}

	// The part of the summary for a resource (0 food, 1 wood, 2 stone, 3 gold).
	FHOLAIResourceSummary PartOf(const FHOLAISummary& Summary, int Resource)
	{
		if (Resource == 0)
		{
			return Summary.Food;
		}
		if (Resource == 1)
		{
			return Summary.Wood;
		}
		if (Resource == 2)
		{
			return Summary.Stone;
		}
		return Summary.Gold;
	}

	FString NameOf(int Resource)
	{
		if (Resource == 0)
		{
			return "Food";
		}
		if (Resource == 1)
		{
			return "Wood";
		}
		if (Resource == 2)
		{
			return "Stone";
		}
		return "Gold";
	}

	int WeightOf(const FHOLAIOpeningData& Data, int Resource)
	{
		if (Resource == 0)
		{
			return Data.WeightFood;
		}
		if (Resource == 1)
		{
			return Data.WeightWood;
		}
		if (Resource == 2)
		{
			return Data.WeightStone;
		}
		return Data.WeightGold;
	}

	// The slot has a building of this type, in any state (a foundation counts).
	bool HasBuilding(const FHOLAISummary& Summary, FName Type)
	{
		for (int Index = 0; Index < Summary.Buildings.Num(); Index++)
		{
			if (Summary.Buildings[Index].TypeName == Type)
			{
				return true;
			}
		}
		return false;
	}

	// Units waiting in a queue for their turn (the head in production already has its place in PopulationUsed).
	int WaitingInQueues(const FHOLAISummary& Summary)
	{
		int Waiting = 0;
		for (int Index = 0; Index < Summary.Buildings.Num(); Index++)
		{
			Waiting += Summary.Buildings[Index].Queue.Num();
			if (Summary.Buildings[Index].bProducing)
			{
				Waiting -= 1;
			}
		}
		return Waiting;
	}

	// The places left: cap - used - waiting in a queue.
	int PlacesLeft(const FHOLAISummary& Summary)
	{
		return Summary.PopulationCap - Summary.PopulationUsed - WaitingInQueues(Summary);
	}

	// The resource with the biggest deficit of Peasants against its weight, over Total Peasants, among those with a weight and
	// a source (a tie: the first in the order food, wood, stone, gold); -1 when none. Counts holds the Peasants per resource.
	int MostLacking(const FHOLAISummary& Summary, const FHOLAIOpeningData& Data, const TArray<int>& Counts, int Total)
	{
		int SumWeights = 0;
		for (int Resource = 0; Resource < 4; Resource++)
		{
			SumWeights += WeightOf(Data, Resource);
		}
		int Best = -1;
		float BestDeficit = 0.f;
		for (int Resource = 0; Resource < 4; Resource++)
		{
			if (WeightOf(Data, Resource) <= 0 || PartOf(Summary, Resource).Sources.Num() == 0)
			{
				continue;
			}
			const float Deficit = float(Total) * float(WeightOf(Data, Resource)) / float(SumWeights) - float(Counts[Resource]);
			if (Best < 0 || Deficit > BestDeficit)
			{
				Best = Resource;
				BestDeficit = Deficit;
			}
		}
		return Best;
	}

	TArray<int> GatherersOf(const FHOLAISummary& Summary)
	{
		TArray<int> Counts;
		Counts.Add(Summary.Food.Peasants);
		Counts.Add(Summary.Wood.Peasants);
		Counts.Add(Summary.Stone.Peasants);
		Counts.Add(Summary.Gold.Peasants);
		return Counts;
	}

	int SumOf(const TArray<int>& Counts)
	{
		return Counts[0] + Counts[1] + Counts[2] + Counts[3];
	}

	// The next building: the first of the list the slot has none of, when its turn has come, its wood is there, its anchor
	// exists and a Peasant can build it. One decision, the builder an idle Peasant first, else the latest born.
	FHOLAIBuildChoice ChooseBuild(const FHOLAISummary& Summary, const FHOLAIOpeningData& Data)
	{
		FHOLAIBuildChoice Choice;
		if (!Summary.bHasCentre)
		{
			Choice.Nothing = "no centre";
			return Choice;
		}
		int StepIndex = -1;
		for (int Index = 0; Index < Data.Steps.Num(); Index++)
		{
			if (!HasBuilding(Summary, Data.Steps[Index].Type))
			{
				StepIndex = Index;
				break;
			}
		}
		if (StepIndex < 0)
		{
			Choice.Nothing = "done";
			return Choice;
		}
		const FHOLAIOpeningStep Step = Data.Steps[StepIndex];
		if (Step.bWhenRoomLow && (!Summary.bHasPopulation || PlacesLeft(Summary) > Data.HouseMargin))
		{
			Choice.Nothing = "not yet";
			return Choice;
		}
		if (Summary.Wood.Stock < Step.WoodCost)
		{
			Choice.Nothing = "no wood";
			return Choice;
		}

		FVector Anchor;
		if (Step.NearResource >= 0)
		{
			const FHOLAIResourceSummary Part = PartOf(Summary, Step.NearResource);
			if (Part.Sources.Num() == 0)
			{
				Choice.Nothing = "no source";
				return Choice;
			}
			Anchor = Part.Sources[0].Location;
		}
		else
		{
			// Away from the nearest food and wood, so the house does not stand in the Peasants' way.
			FVector Toward = FVector(0, 0, 0);
			if (Summary.Food.Sources.Num() > 0)
			{
				Toward += Summary.Food.Sources[0].Location - Summary.CentreLocation;
			}
			if (Summary.Wood.Sources.Num() > 0)
			{
				Toward += Summary.Wood.Sources[0].Location - Summary.CentreLocation;
			}
			Toward.Z = 0;
			FVector Away = FVector(-1, 0, 0);
			if (Toward.Size() > 1.0)
			{
				Away = Toward.GetSafeNormal() * -1.0;
			}
			Anchor = Summary.CentreLocation + Away * Data.AwayDistance;
		}

		int Builder = -1;
		if (Summary.IdlePeasants.Num() > 0)
		{
			Builder = Summary.IdlePeasants[0];
		}
		else if (Summary.PeasantIds.Num() > 0)
		{
			Builder = Summary.PeasantIds[Summary.PeasantIds.Num() - 1];
		}
		if (Builder < 0)
		{
			Choice.Nothing = "no builder";
			return Choice;
		}

		Choice.bBuild = true;
		Choice.Type = Step.Type;
		Choice.Anchor = Anchor;
		Choice.Builder = Builder;
		return Choice;
	}

	// A Peasant at the centre while its queue, the population and the food allow, and the Peasants are under the target; the
	// rally of the new ones on the nearest source of the resource that lacks Peasants.
	FHOLAIProduceChoice ChooseProduction(const FHOLAISummary& Summary, const FHOLAIOpeningData& Data)
	{
		FHOLAIProduceChoice Choice;
		int CentreIndex = -1;
		for (int Index = 0; Index < Summary.Buildings.Num(); Index++)
		{
			if (Summary.Buildings[Index].TypeName == n"MainCentre")
			{
				CentreIndex = Index;
				break;
			}
		}
		if (CentreIndex < 0 || Summary.Buildings[CentreIndex].State != EHOLConstructionState::Finished)
		{
			Choice.Nothing = "no centre";
			return Choice;
		}
		const FHOLAIBuilding Centre = Summary.Buildings[CentreIndex];
		if (Centre.Queue.Num() >= Data.QueueLength)
		{
			Choice.Nothing = "queue full";
			return Choice;
		}
		if (!Summary.bHasPopulation || PlacesLeft(Summary) <= 0)
		{
			Choice.Nothing = "no room";
			return Choice;
		}
		if (Summary.Food.Stock < Data.PeasantFood)
		{
			Choice.Nothing = "no food";
			return Choice;
		}
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
		if (Summary.PeasantIds.Num() + Queued >= Data.PeasantTarget)
		{
			Choice.Nothing = "target reached";
			return Choice;
		}

		Choice.bProduce = true;
		Choice.Centre = Centre.Location;
		TArray<int> Counts = GatherersOf(Summary);
		const int Resource = MostLacking(Summary, Data, Counts, SumOf(Counts) + 1);
		if (Resource >= 0)
		{
			const FHOLAISourceNear Source = PartOf(Summary, Resource).Sources[0];
			Choice.RallyResource = Resource;
			Choice.RallySource = Source.SourceIndex;
			Choice.RallyPoint = Source.Location;
		}
		return Choice;
	}

	// Adds the group of a resource when it has Peasants: the resource's nearest source.
	void AddGroup(TArray<FHOLAIGatherGroup>& Groups, const FHOLAISummary& Summary, int Resource, const FHOLAIGatherGroup& Per)
	{
		if (Per.Peasants.Num() == 0)
		{
			return;
		}
		FHOLAIGatherGroup Group;
		Group.Resource = Resource;
		Group.ResourceName = NameOf(Resource);
		Group.SourceIndex = PartOf(Summary, Resource).Sources[0].SourceIndex;
		Group.Peasants = Per.Peasants;
		Groups.Add(Group);
	}

	// The Peasants Free spread by the weights: each goes, in order, to the resource that lacks Peasants most (those already
	// gathering and those sent before it counted), one group per resource with the nearest source. Without a weighted
	// resource that has a source, all go to the lowest stock among those with one (the IA0.4 rule).
	TArray<FHOLAIGatherGroup> SplitGather(const FHOLAISummary& Summary, const FHOLAIOpeningData& Data, const TArray<int>& Free)
	{
		TArray<FHOLAIGatherGroup> Groups;
		if (Free.Num() == 0)
		{
			return Groups;
		}
		TArray<int> Counts = GatherersOf(Summary);
		const int Total = SumOf(Counts) + Free.Num();
		if (MostLacking(Summary, Data, Counts, Total) < 0)
		{
			FHOLAISummary Alone = Summary;
			Alone.IdlePeasants = Free;
			FHOLAIGatherChoice Choice = HOLAIEconomy::ChooseGather(Alone);
			if (Choice.bGather)
			{
				FHOLAIGatherGroup Group;
				Group.Resource = Choice.Resource;
				Group.ResourceName = Choice.ResourceName;
				Group.SourceIndex = Choice.SourceIndex;
				Group.Peasants = Choice.Peasants;
				Groups.Add(Group);
			}
			return Groups;
		}

		// Script has no nested containers: a struct holds the four lists.
		FHOLAIGatherGroup Per0;
		FHOLAIGatherGroup Per1;
		FHOLAIGatherGroup Per2;
		FHOLAIGatherGroup Per3;
		for (int Index = 0; Index < Free.Num(); Index++)
		{
			const int Resource = MostLacking(Summary, Data, Counts, Total);
			Counts[Resource] = Counts[Resource] + 1;
			if (Resource == 0)
			{
				Per0.Peasants.Add(Free[Index]);
			}
			else if (Resource == 1)
			{
				Per1.Peasants.Add(Free[Index]);
			}
			else if (Resource == 2)
			{
				Per2.Peasants.Add(Free[Index]);
			}
			else
			{
				Per3.Peasants.Add(Free[Index]);
			}
		}
		AddGroup(Groups, Summary, 0, Per0);
		AddGroup(Groups, Summary, 1, Per1);
		AddGroup(Groups, Summary, 2, Per2);
		AddGroup(Groups, Summary, 3, Per3);
		return Groups;
	}
}
