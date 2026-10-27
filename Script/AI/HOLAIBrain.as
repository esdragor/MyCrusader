// The AI slots' brain (IA0.4): the C++ base (UHOLAIBrainSubsystem) calls Think for each AI slot every
// hol.AI.ThinkSteps steps, on the server only. First rule (IA0.4-Q1 A, B2; replaced by the needs of IA1.6): the idle
// Peasants gather the lowest stock among the resources that have a source near a deposit of the slot (a tie: food,
// wood, stone, gold), all of them in one order to its nearest source (Sources[0]). No idle Peasant or no source: nothing.
class UHOLAIBrain : UHOLAIBrainSubsystem
{
	// Thoughts this brain ran, all slots together.
	int NumScriptThoughts = 0;

	UFUNCTION(BlueprintOverride)
	bool Think(int PlayerIndex)
	{
		NumScriptThoughts++;
		FHOLAISummary Summary = HOLAISummary::ReadAISummary(PlayerIndex);
		const int NumIdle = Summary.IdlePeasants.Num();
		if (NumIdle == 0)
		{
			return true;
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

		// One line per decision (IA0.7 reads them): "AI slot 1 step 40: 3 idle -> gather Food #12".
		const int64 Step = GetStepIndex();
		if (Best < 0)
		{
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: {NumIdle} idle -> no source");
			return true;
		}
		const int SourceIndex = Parts[Best].Sources[0].SourceIndex;
		const FString ResourceName = Names[Best];
		const bool bQueued = HOLAIOrder::IssueAIGatherOrder(PlayerIndex, Summary.IdlePeasants, SourceIndex);
		if (bQueued)
		{
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: {NumIdle} idle -> gather {ResourceName} #{SourceIndex}");
		}
		else
		{
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: {NumIdle} idle -> gather {ResourceName} #{SourceIndex} refused");
		}
		return true;
	}
}
