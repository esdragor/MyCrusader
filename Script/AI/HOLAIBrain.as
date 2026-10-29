// The AI slots' brain (IA0.4, IA1.5): the C++ base (UHOLAIBrainSubsystem) calls Think for each AI slot every
// hol.AI.ThinkSteps steps, on the server only. Think reads the summary, asks the opening rules (HOLAIOpening.as, pure,
// tested in script) what to build, queue and gather, and sends the orders (UHOLAIOrderLibrary, HOLAIOrder::): the next
// building of the opening (house, lumber camp, mill) with one Peasant to build it, a Peasant queued at the centre with
// its rally on the resource that lacks Peasants, and the idle Peasants spread over the resources by their weights.
class UHOLAIBrain : UHOLAIBrainSubsystem
{
	// Thoughts this brain ran, all slots together.
	int NumScriptThoughts = 0;

	// The source each slot's centre rally was last set on (the order is sent again only when it changes).
	TMap<int, int> RallySources;

	// The last reason a building could not be placed, per slot (written again only when it changes).
	TMap<int, FString> LastBuildReason;

	UFUNCTION(BlueprintOverride)
	bool Think(int PlayerIndex)
	{
		NumScriptThoughts++;
		FHOLAISummary Summary = HOLAISummary::ReadAISummary(PlayerIndex);
		FHOLAIOpeningData Data = HOLAIOpening::DefaultData();
		const int64 Step = GetStepIndex();
		const int NumIdle = Summary.IdlePeasants.Num();
		TArray<int> Free = Summary.IdlePeasants;

		// 1. The next building of the opening, placed beside its anchor, one Peasant sent to build it.
		FHOLAIBuildChoice Build = HOLAIOpening::ChooseBuild(Summary, Data);
		if (Build.bBuild)
		{
			FVector Spot;
			if (!HOLAIOrder::FindAIPlacement(PlayerIndex, Build.Type, Build.Anchor, Data.Rings, Spot))
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
				}
				else
				{
					LogBuildOnce(PlayerIndex, Step, f"build {Build.Type} refused");
				}
			}
		}

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

		// 3. The idle Peasants (the builder left out) spread by the weights, one gather order per resource.
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
