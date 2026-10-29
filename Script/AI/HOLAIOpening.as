// The opening of the AI brain (IA1.5), written as data and pure rules over a summary (IA1.3), tested alone in script
// (HOLAIOpening_Test.as): how the idle Peasants spread (by the needs of HOLAINeeds.as, IA1.6), which building comes next and
// where, when a Peasant is queued and where it is sent. The brain (HOLAIBrain.as) reads the summary, asks these rules and
// sends the orders (IA1.4). IA1.7 (buildings) takes over from this opening.

// One building of the opening, in the order of the list.
struct FHOLAIOpeningStep
{
	// The building's type name (BuildingTypes.csv).
	FName Type;

	// Wood needed in stock to place it (its Cost in BuildingTypes.csv: the server refuses and logs the order otherwise).
	int WoodCost = 0;

	// Built beside the nearest source of this resource (0 food, 1 wood, 2 stone, 3 gold).
	int NearResource = 0;

}

// The numbers of the opening.
struct FHOLAIOpeningData
{
	// Share of the Peasants per resource, in the order food, wood, stone, gold: only when the needs (HOLAINeeds.as) ask for
	// nothing from any resource that has a source (provisional: IA4 tunes it on AI-only games).
	int WeightFood = 3;
	int WeightWood = 2;
	int WeightStone = 0;
	int WeightGold = 0;

	// Peasants waiting in the centre's queue (the one in production counts).
	int QueueLength = 2;

	// The Peasant and the House as the type tables give them (UnitTypes.csv, BuildingTypes.csv): DefaultData holds their
	// values as a fallback, the brain reads the tables (HOLAIBrain.as ReadFigures) so that the costs are not copied.
	FHOLAITypeFigures Peasant;
	FHOLAITypeFigures House;

	// Peasants (alive and queued) up to which the centre keeps producing (indicative: 80, GDD 5.3: 70 to 90).
	int PeasantTarget = 80;

	// The needs (HOLAINeeds.as): the spending wanted over this many seconds ...
	float WindowSeconds = 60.f;

	// ... of which this share of the stock in hand is already paid for ...
	float StockShare = 0.5f;

	// ... but the sources are asked at least this share of the spending ...
	float MinDemandShare = 0.25f;

	// ... and this much more over the window for what is not planned yet (the next camps, farms and army: IA1.7, IA2).
	float ReserveFood = 0.f;
	float ReserveWood = 30.f;
	float ReserveStone = 0.f;
	float ReserveGold = 0.f;

	// The house (IA1.7): placed when the places left (cap - used - waiting in a queue) plus the places of the houses under
	// construction fall to the margin, which is what the producing buildings fill during the house's building time plus this
	// walk of the builder (HouseMargin).
	float HouseWalkSeconds = 5.f;

	// The population the game allows (GDD): no house to go above it.
	int MaxPopulation = 175;

	// Sites (buildings not finished, the centre apart) under way at once: a house is not counted, the other buildings wait.
	int MaxSites = 3;

	// Peasants the slot must have for one to leave its work and build (a lone Peasant would stop the whole economy).
	int MinPeasantsToBuild = 2;

	// A camp comes again beside the nearest source of its resource when that source is this far (cm) from the nearest deposit
	// taking it: the first massif is cut, the next one is reached by its edge.
	float CampFarDistance = 1500.f;

	// The farm block (a mill and FarmCount farms, FindAIFarmBlock) is begun when the berries left would last less than this
	// many seconds at the pace of the Peasants on the food (about the time the block takes to build).
	float FarmLeadSeconds = 240.f;

	// Rings of tiles searched around the centre for the farm block.
	int FarmRings = 10;

	// Distance (cm) from the centre, on the side away from the sources, where the search for the house's spot starts.
	float AwayDistance = 1500.f;

	// Rings of tiles searched around the anchor (FindAIPlacement).
	int Rings = 8;

	// The buildings, in order: the next one is the first whose type the slot has none of (in any state).
	TArray<FHOLAIOpeningStep> Steps;

	// Farms in the block, and the Farm as the table gives it (the brain reads it; DefaultData holds the fallback).
	int FarmCount = 8;
	FHOLAITypeFigures Farm;
}

// The farm block the slot builds (IA1.7): the spots FindAIFarmBlock gave, kept by the brain from one thought to the next.
struct FHOLAIFarmPlan
{
	bool bActive = false;
	FVector Mill;
	TArray<FVector> Farms;
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
	// The exact spot (a farm block's, already searched): no search around Anchor.
	bool bAtSpot = false;
	FVector Spot;
	// Nothing to order yet: the farm block is to be planned (FindAIFarmBlock around the centre), then asked again.
	bool bPlanFarms = false;
	// Why nothing: "done", "not yet", "busy", "no wood", "no source", "no builder", "no centre"; empty with an order.
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
	// The opening as the game plays it: lumber camp near the wood, then mill near the food; the houses, the later camps and the farm block are rules of ChooseBuild (costs from
	// BuildingTypes.csv).
	FHOLAIOpeningData DefaultData()
	{
		FHOLAIOpeningData Data;
		// The tables' values today (fallback of the brain's read).
		Data.Peasant.CostFood = 50;
		Data.Peasant.BuildTime = 20.f;
		Data.Peasant.Population = 1;
		Data.Peasant.Speed = 800.f;
		Data.Peasant.CarryCapacity = 10;
		Data.Peasant.GatherFood = 0.7f;
		Data.Peasant.GatherWood = 0.7f;
		Data.Peasant.GatherStone = 0.7f;
		Data.Peasant.GatherGold = 0.7f;
		Data.House.CostWood = 50;
		Data.House.BuildTime = 20.f;
		Data.House.Population = 10;
		Data.Farm.CostWood = 75;
		Data.Farm.BuildTime = 15.f;
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

	// The buildings of a type the slot has (any state).
	int CountBuildings(const FHOLAISummary& Summary, FName Type)
	{
		int Count = 0;
		for (int Index = 0; Index < Summary.Buildings.Num(); Index++)
		{
			if (Summary.Buildings[Index].TypeName == Type)
			{
				Count++;
			}
		}
		return Count;
	}

	// The slot has a building of this type (any state) within 100 cm of the point, on the ground plane.
	bool HasBuildingAt(const FHOLAISummary& Summary, FName Type, FVector Point)
	{
		for (int Index = 0; Index < Summary.Buildings.Num(); Index++)
		{
			FVector Away = Summary.Buildings[Index].Location - Point;
			Away.Z = 0;
			if (Summary.Buildings[Index].TypeName == Type && Away.Size() < 100.0)
			{
				return true;
			}
		}
		return false;
	}

	// Sites under way: buildings not finished (the centre apart, it is not built by the AI).
	int SitesUnderWay(const FHOLAISummary& Summary)
	{
		int Sites = 0;
		for (int Index = 0; Index < Summary.Buildings.Num(); Index++)
		{
			if (Summary.Buildings[Index].State != EHOLConstructionState::Finished && Summary.Buildings[Index].TypeName != n"MainCentre")
			{
				Sites++;
			}
		}
		return Sites;
	}

	// The places the houses under construction will give.
	int PlacesComing(const FHOLAISummary& Summary, const FHOLAIOpeningData& Data)
	{
		int Coming = 0;
		for (int Index = 0; Index < Summary.Buildings.Num(); Index++)
		{
			if (Summary.Buildings[Index].TypeName == n"House" && Summary.Buildings[Index].State != EHOLConstructionState::Finished)
			{
				Coming += Data.House.Population;
			}
		}
		return Coming;
	}

	// The margin of places (IA1.7): what the buildings in production fill during a house's building time and its builder's
	// walk. A producer is a building with a unit in production; the centre counts as one while none is.
	int HouseMargin(const FHOLAISummary& Summary, const FHOLAIOpeningData& Data)
	{
		int Producers = 0;
		for (int Index = 0; Index < Summary.Buildings.Num(); Index++)
		{
			if (Summary.Buildings[Index].bProducing)
			{
				Producers++;
			}
		}
		if (Producers == 0 && Summary.bHasCentre)
		{
			Producers = 1;
		}
		if (Data.Peasant.BuildTime <= 0.f)
		{
			return Producers;
		}
		const int PerProducer = Math::CeilToInt((Data.House.BuildTime + Data.HouseWalkSeconds) / Data.Peasant.BuildTime);
		return Producers * Math::Max(1, PerProducer);
	}

	// A house is wanted when the places left, with the houses coming, are down to the margin and the game's population cap
	// is not reached by what is already there and coming.
	bool HouseWanted(const FHOLAISummary& Summary, const FHOLAIOpeningData& Data)
	{
		if (!Summary.bHasPopulation)
		{
			return false;
		}
		const int Coming = PlacesComing(Summary, Data);
		if (Summary.PopulationCap + Coming >= Data.MaxPopulation)
		{
			return false;
		}
		return PlacesLeft(Summary) + Coming <= HouseMargin(Summary, Data);
	}

	// Seconds the berries (the food sources of the summary) last at the pace of the Peasants on the food: 0 with no source,
	// a very long time with no pace to measure.
	float FoodSecondsLeft(const FHOLAISummary& Summary, const FHOLAIOpeningData& Data)
	{
		if (Summary.Food.Sources.Num() == 0)
		{
			return 0.f;
		}
		float Total = 0.f;
		for (int Index = 0; Index < Summary.Food.Sources.Num(); Index++)
		{
			Total += float(Summary.Food.Sources[Index].Remaining);
		}
		const float Pace = HOLAINeeds::NetRate(Data, Summary, 0) * float(Math::Max(1, Summary.Food.Peasants));
		if (Pace <= 0.f)
		{
			return 1000000.f;
		}
		return Total / Pace;
	}

	// The farms are wanted when the berries last less than the lead.
	bool FarmsWanted(const FHOLAISummary& Summary, const FHOLAIOpeningData& Data)
	{
		return Summary.bHasCentre && FoodSecondsLeft(Summary, Data) < Data.FarmLeadSeconds;
	}

	// The wood a type costs as the data holds it.
	int WoodCostOf(const FHOLAIOpeningData& Data, FName Type)
	{
		if (Type == n"House")
		{
			return Data.House.CostWood;
		}
		if (Type == n"Farm")
		{
			return Data.Farm.CostWood;
		}
		for (int Index = 0; Index < Data.Steps.Num(); Index++)
		{
			if (Data.Steps[Index].Type == Type)
			{
				return Data.Steps[Index].WoodCost;
			}
		}
		return 0;
	}

	// The Peasant sent to build: an idle one first, else the latest born, among those not sent to a site already.
	int PickBuilder(const FHOLAISummary& Summary, const TArray<int>& Busy)
	{
		for (int Index = 0; Index < Summary.IdlePeasants.Num(); Index++)
		{
			if (!Busy.Contains(Summary.IdlePeasants[Index]))
			{
				return Summary.IdlePeasants[Index];
			}
		}
		for (int Index = Summary.PeasantIds.Num() - 1; Index >= 0; Index--)
		{
			if (!Busy.Contains(Summary.PeasantIds[Index]))
			{
				return Summary.PeasantIds[Index];
			}
		}
		return -1;
	}

	// An order for a type at an anchor (or at an exact spot), wood and builder checked.
	FHOLAIBuildChoice OrderOf(const FHOLAISummary& Summary, const FHOLAIOpeningData& Data, FName Type, FVector Anchor, bool bAtSpot, const TArray<int>& Busy)
	{
		FHOLAIBuildChoice Choice;
		if (Summary.PeasantIds.Num() < Data.MinPeasantsToBuild)
		{
			Choice.Nothing = "no builder";
			return Choice;
		}
		if (Summary.Wood.Stock < WoodCostOf(Data, Type))
		{
			Choice.Nothing = "no wood";
			return Choice;
		}
		const int Builder = PickBuilder(Summary, Busy);
		if (Builder < 0)
		{
			Choice.Nothing = "no builder";
			return Choice;
		}
		Choice.bBuild = true;
		Choice.Type = Type;
		Choice.Anchor = Anchor;
		Choice.bAtSpot = bAtSpot;
		if (bAtSpot)
		{
			Choice.Spot = Anchor;
		}
		Choice.Builder = Builder;
		return Choice;
	}

	// The camp type of a resource: the lumber camp for wood, the mining camp for stone and gold (food has the mill).
	FName CampOf(int Resource)
	{
		if (Resource == 1)
		{
			return n"LumberCamp";
		}
		return n"MiningCamp";
	}

	// The next building (IA1.7), one decision, in this order: a house when the room is low (before anything, and not held
	// back by the sites); then, the sites under way being few (Data.MaxSites): the steps of the list (lumber camp, mill,
	// each beside its source), a camp again where a massif is cut (the nearest source far from every deposit, the resource
	// asked for), the farm block when the berries run low (its spots in Plan, asked for with bPlanFarms). Busy are the
	// builders already sent to a site.
	FHOLAIBuildChoice ChooseBuild(const FHOLAISummary& Summary, const FHOLAIOpeningData& Data, const FHOLAIFarmPlan& Plan, const TArray<int>& Busy)
	{
		FHOLAIBuildChoice Choice;
		if (!Summary.bHasCentre)
		{
			Choice.Nothing = "no centre";
			return Choice;
		}

		// 1. The house, on the side of the centre away from the nearest food and wood so it does not stand in the way.
		if (HouseWanted(Summary, Data))
		{
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
			return OrderOf(Summary, Data, n"House", Summary.CentreLocation + Away * Data.AwayDistance, false, Busy);
		}
		if (SitesUnderWay(Summary) >= Data.MaxSites)
		{
			Choice.Nothing = "busy";
			return Choice;
		}

		// 2. The steps: the first of the list the slot has none of. A step with no source waits for the next ones.
		FString Skipped;
		for (int Index = 0; Index < Data.Steps.Num(); Index++)
		{
			const FHOLAIOpeningStep Step = Data.Steps[Index];
			if (HasBuilding(Summary, Step.Type))
			{
				continue;
			}
			const FHOLAIResourceSummary Part = PartOf(Summary, Step.NearResource);
			if (Part.Sources.Num() == 0)
			{
				Skipped = "no source";
				continue;
			}
			return OrderOf(Summary, Data, Step.Type, Part.Sources[0].Location, false, Busy);
		}

		// 3. A camp again where the massif is cut: wood, stone and gold when the needs ask for them; one camp at a time.
		FHOLAINeeds Needs = HOLAINeeds::Compute(Summary, Data);
		for (int Resource = 1; Resource < 4; Resource++)
		{
			const FHOLAIResourceSummary Part = PartOf(Summary, Resource);
			if (Part.Sources.Num() == 0 || Needs.Share[Resource] <= 0.f || Part.Sources[0].DepositDistance <= Data.CampFarDistance)
			{
				continue;
			}
			bool bCampComing = false;
			for (int Each = 0; Each < Summary.Buildings.Num(); Each++)
			{
				bCampComing = bCampComing || (Summary.Buildings[Each].TypeName == CampOf(Resource) && Summary.Buildings[Each].State != EHOLConstructionState::Finished);
			}
			if (bCampComing)
			{
				continue;
			}
			return OrderOf(Summary, Data, CampOf(Resource), Part.Sources[0].Location, false, Busy);
		}

		// 4. The farm block: its mill and its farms in turn at the planned spots; planned first.
		if (Plan.bActive)
		{
			if (!HasBuildingAt(Summary, n"Mill", Plan.Mill))
			{
				return OrderOf(Summary, Data, n"Mill", Plan.Mill, true, Busy);
			}
			for (int Index = 0; Index < Plan.Farms.Num(); Index++)
			{
				if (!HasBuildingAt(Summary, n"Farm", Plan.Farms[Index]))
				{
					return OrderOf(Summary, Data, n"Farm", Plan.Farms[Index], true, Busy);
				}
			}
		}
		else if (FarmsWanted(Summary, Data) && CountBuildings(Summary, n"Farm") < Data.FarmCount
			&& Summary.Wood.Stock >= WoodCostOf(Data, n"Mill") && PickBuilder(Summary, Busy) >= 0
				&& Summary.PeasantIds.Num() >= Data.MinPeasantsToBuild)
		{
			Choice.bPlanFarms = true;
			Choice.Nothing = "plan farms";
			return Choice;
		}

		if (Skipped.Len() > 0)
		{
			Choice.Nothing = Skipped;
			return Choice;
		}
		Choice.Nothing = "done";
		return Choice;
	}

	// The same, without a farm block planned and no builder busy.
	FHOLAIBuildChoice ChooseBuild(const FHOLAISummary& Summary, const FHOLAIOpeningData& Data)
	{
		FHOLAIFarmPlan NoPlan;
		TArray<int> NoBusy;
		return ChooseBuild(Summary, Data, NoPlan, NoBusy);
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
		if (Summary.Food.Stock < Data.Peasant.CostFood)
		{
			Choice.Nothing = "no food";
			return Choice;
		}
		if (HOLAINeeds::PeasantsCounted(Summary) >= Data.PeasantTarget)
		{
			Choice.Nothing = "target reached";
			return Choice;
		}

		Choice.bProduce = true;
		Choice.Centre = Centre.Location;
		TArray<int> Counts = GatherersOf(Summary);
		const FHOLAINeeds Needs = HOLAINeeds::Compute(Summary, Data);
		const int Resource = HOLAINeeds::MostLacking(Needs, Counts, SumOf(Counts) + 1);
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

	// The Peasants Free spread by the needs (HOLAINeeds.as): each goes, in order, to the resource that lacks Peasants most
	// against its share (those already gathering and those sent before it counted), one group per resource with the nearest
	// source. A source used up has left the summary: its Peasants, idle, come here and go to what is left. Without a share on
	// any resource that has a source, all go to the lowest stock among those with one (the IA0.4 rule).
	TArray<FHOLAIGatherGroup> SplitGather(const FHOLAISummary& Summary, const FHOLAIOpeningData& Data, const TArray<int>& Free)
	{
		TArray<FHOLAIGatherGroup> Groups;
		if (Free.Num() == 0)
		{
			return Groups;
		}
		TArray<int> Counts = GatherersOf(Summary);
		const int Total = SumOf(Counts) + Free.Num();
		const FHOLAINeeds Needs = HOLAINeeds::Compute(Summary, Data);
		if (HOLAINeeds::MostLacking(Needs, Counts, Total) < 0)
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
			const int Resource = HOLAINeeds::MostLacking(Needs, Counts, Total);
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
