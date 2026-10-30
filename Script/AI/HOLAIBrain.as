// The AI slots' brain (IA0.4, IA1.5, IA1.6, IA1.7): the C++ base (UHOLAIBrainSubsystem) calls Think for each AI slot every
// hol.AI.ThinkSteps steps, on the server only. Think reads the summary, asks the opening rules (HOLAIOpening.as, pure,
// tested in script) what to build, queue and gather, and sends the orders (UHOLAIOrderLibrary, HOLAIOrder::): the next
// building (the houses before the cap, lumber camp, mill, the camps where a massif is cut, the farm block when the berries
// run low) with one Peasant to build it, a Peasant queued at the centre with
// its rally on the resource that lacks Peasants, and the idle Peasants spread over the resources by their needs
// (HOLAINeeds.as: what the slot wants to spend over the minute, a source used up sending its Peasants on).
// The builders a slot has sent to sites not finished yet (IA1.7): not sent again, so that several sites go up at once.
struct FHOLAIBuilders
{
	TArray<int> Ids;
}

class UHOLAIBrain : UHOLAIBrainSubsystem
{
	// Thoughts this brain ran, all slots together.
	int NumScriptThoughts = 0;

	// The source each slot's centre rally was last set on (the order is sent again only when it changes).
	TMap<int, int> RallySources;

	// The farm block each slot is building (IA1.7: the spots of FindAIFarmBlock), and the builders it has sent to sites.
	TMap<int, FHOLAIFarmPlan> FarmPlans;
	TMap<int, FHOLAIBuilders> SentBuilders;

	// The step of the last status line per slot.
	TMap<int, int64> LastStatusStep;

	// The last reason a building could not be placed, per slot (written again only when it changes).
	TMap<int, FString> LastBuildReason;

	UFUNCTION(BlueprintOverride)
	bool Think(int PlayerIndex)
	{
		NumScriptThoughts++;
		FHOLAISummary Summary = HOLAISummary::ReadAISummary(PlayerIndex);
		FHOLAIOpeningData Data = HOLAIOpening::DefaultData();
		ReadFigures(Data);
		const int64 Step = GetStepIndex();
		LogStatus(PlayerIndex, Step, Summary);
		const int NumIdle = Summary.IdlePeasants.Num();
		TArray<int> Free = Summary.IdlePeasants;

		// 1. The next building (house, camp, mill, farm block: HOLAIOpening::ChooseBuild), placed by the rules of its type, one
		// Peasant sent to build it, not one already sent to another site.
		FHOLAIFarmPlan Plan;
		FarmPlans.Find(PlayerIndex, Plan);
		FHOLAIBuilders Sent;
		SentBuilders.Find(PlayerIndex, Sent);
		if (HOLAIOpening::SitesUnderWay(Summary) == 0)
		{
			Sent.Ids.Empty();
		}
		FHOLAIBuildChoice Build = HOLAIOpening::ChooseBuild(Summary, Data, Plan, Sent.Ids);
		if (Build.bPlanFarms)
		{
			Plan = PlanFarmBlock(PlayerIndex, Summary, Data);
			if (Plan.bActive)
			{
				FarmPlans.Add(PlayerIndex, Plan);
				Build = HOLAIOpening::ChooseBuild(Summary, Data, Plan, Sent.Ids);
			}
		}
		if (Build.bBuild)
		{
			FVector Spot = Build.Spot;
			bool bFound = true;
			if (!Build.bAtSpot)
			{
				bFound = HOLAIOrder::FindAIPlacement(PlayerIndex, Build.Type, Build.Anchor, Data.Rings, Spot);
			}
			if (!bFound)
			{
				LogBuildOnce(PlayerIndex, Step, f"no spot for {Build.Type}");
			}
			else
			{
				TArray<int> Builders;
				Builders.Add(Build.Builder);
				if (HOLAIOrder::IssueAIPlaceOrder(PlayerIndex, Build.Type, Spot, Builders))
				{
					Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: build {Build.Type} at ({Spot.X}, {Spot.Y}) by #{Build.Builder}");
					Free.Remove(Build.Builder);
					Sent.Ids.Add(Build.Builder);
				}
				else
				{
					LogBuildOnce(PlayerIndex, Step, f"build {Build.Type} refused");
					if (Build.bAtSpot)
					{
						// The planned spot is not free any more (another building came): the block is planned again.
						FarmPlans.Remove(PlayerIndex);
					}
				}
			}
		}
		SentBuilders.Add(PlayerIndex, Sent);

		// 2. A Peasant queued at the centre, new ones sent to the resource that lacks Peasants.
		FHOLAIProduceChoice Produce = HOLAIOpening::ChooseProduction(Summary, Data);
		if (Produce.bProduce)
		{
			int LastRally = -2;
			RallySources.Find(PlayerIndex, LastRally);
			if (Produce.RallyResource >= 0 && LastRally != Produce.RallySource)
			{
				if (HOLAIOrder::IssueAISetRallyOrder(PlayerIndex, Produce.Centre, Produce.RallyPoint, Produce.RallySource))
				{
					RallySources.Add(PlayerIndex, Produce.RallySource);
				}
			}
			if (HOLAIOrder::IssueAIProduceOrder(PlayerIndex, Produce.Centre, n"Peasant"))
			{
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: queue a Peasant, rally on source #{Produce.RallySource}");
			}
		}

		// 3. The idle Peasants (the builder left out) spread by the needs, one gather order per resource.
		if (NumIdle == 0)
		{
			return true;
		}
		TArray<FHOLAIGatherGroup> Groups = HOLAIOpening::SplitGather(Summary, Data, Free);
		if (Groups.Num() == 0)
		{
			if (Free.Num() > 0)
			{
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: {NumIdle} idle -> no source");
			}
			return true;
		}
		// One line per decision (IA0.7 and quick_check read them): "AI slot 1 step 40: 3 idle -> gather Food #12" for the
		// first group, then "AI slot 1 step 40: split Wood #20 x2" for each other.
		for (int Index = 0; Index < Groups.Num(); Index++)
		{
			const FHOLAIGatherGroup Group = Groups[Index];
			const bool bQueued = HOLAIOrder::IssueAIGatherOrder(PlayerIndex, Group.Peasants, Group.SourceIndex);
			const FString Refused = bQueued ? "" : " refused";
			if (Index == 0)
			{
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: {NumIdle} idle -> gather {Group.ResourceName} #{Group.SourceIndex}{Refused}");
			}
			else
			{
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: split {Group.ResourceName} #{Group.SourceIndex} x{Group.Peasants.Num()}{Refused}");
			}
		}
		return true;
	}

	// The farm block around the slot's centre: the mill and the farms where they fit (FindAIFarmBlock); not active when it
	// does not fit within the rings.
	FHOLAIFarmPlan PlanFarmBlock(int PlayerIndex, const FHOLAISummary& Summary, const FHOLAIOpeningData& Data)
	{
		FHOLAIFarmPlan Plan;
		FVector Mill;
		TArray<FVector> Farms;
		if (!HOLAIOrder::FindAIFarmBlock(PlayerIndex, Summary.CentreLocation, Data.FarmRings, Mill, Farms))
		{
			LogBuildOnce(PlayerIndex, GetStepIndex(), "no room for the farm block");
			return Plan;
		}
		Plan.bActive = true;
		Plan.Mill = Mill;
		Plan.Farms = Farms;
		Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {GetStepIndex()}: farm block planned, mill at ({Mill.X}, {Mill.Y}), {Farms.Num()} farms");
		return Plan;
	}

	// The costs and rates of the data from the type tables (UnitTypes.csv, BuildingTypes.csv), over the values DefaultData holds as
	// a fallback: nothing of the tables is copied in the script any more (IA1.6).
	void ReadFigures(FHOLAIOpeningData& Data)
	{
		FHOLAITypeFigures Figures;
		if (HOLAIOrder::ReadAIUnitFigures(n"Peasant", Figures))
		{
			Data.Peasant = Figures;
		}
		if (HOLAIOrder::ReadAIBuildingFigures(n"House", Figures))
		{
			Data.House = Figures;
		}
		if (HOLAIOrder::ReadAIBuildingFigures(n"Farm", Figures))
		{
			Data.Farm = Figures;
		}
		for (int Index = 0; Index < Data.Steps.Num(); Index++)
		{
			if (HOLAIOrder::ReadAIBuildingFigures(Data.Steps[Index].Type, Figures))
			{
				Data.Steps[Index].WoodCost = Figures.CostWood;
			}
		}
	}

	// One line every 100 steps (5 s) per slot for the tests of the AI games (quick_check aiduel reads it): population, the queue
	// of the centre, the four stocks, the Peasants and the buildings that matter.
	void LogStatus(int PlayerIndex, int64 Step, const FHOLAISummary& Summary)
	{
		int64 Last = -1000;
		LastStatusStep.Find(PlayerIndex, Last);
		if (Step - Last < 100)
		{
			return;
		}
		LastStatusStep.Add(PlayerIndex, Step);
		int Queued = 0;
		for (int Index = 0; Index < Summary.Buildings.Num(); Index++)
		{
			Queued += Summary.Buildings[Index].Queue.Num();
		}
		const int Houses = HOLAIOpening::CountBuildings(Summary, n"House");
		const int Camps = HOLAIOpening::CountBuildings(Summary, n"LumberCamp");
		const int Mills = HOLAIOpening::CountBuildings(Summary, n"Mill");
		const int Farms = HOLAIOpening::CountBuildings(Summary, n"Farm");
		Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: status pop {Summary.PopulationUsed}/{Summary.PopulationCap} queued {Queued} stock {Summary.Food.Stock} {Summary.Wood.Stock} {Summary.Stone.Stock} {Summary.Gold.Stock} peasants {Summary.PeasantIds.Num()} houses {Houses} camps {Camps} mills {Mills} farms {Farms}");
	}

	// Writes the reason unless it is the one just written for this slot (the thought comes every few steps).
	void LogBuildOnce(int PlayerIndex, int64 Step, FString Reason)
	{
		FString Last;
		if (LastBuildReason.Find(PlayerIndex, Last) && Last == Reason)
		{
			return;
		}
		LastBuildReason.Add(PlayerIndex, Reason);
		Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: {Reason}");
	}
}
