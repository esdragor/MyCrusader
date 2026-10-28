// The AI slots' brain (IA0.4): the C++ base (UHOLAIBrainSubsystem) calls Think for each AI slot every
// hol.AI.ThinkSteps steps, on the server only. Think reads the summary, asks the economy rules (HOLAIEconomy.as, pure,
// tested in script) what to do and sends the order; first rule (IA0.4-Q1 A): the idle Peasants gather the lowest stock
// among the resources that have a source near a deposit of the slot, all of them to its nearest source.
class UHOLAIBrain : UHOLAIBrainSubsystem
{
	// Thoughts this brain ran, all slots together.
	int NumScriptThoughts = 0;

	UFUNCTION(BlueprintOverride)
	bool Think(int PlayerIndex)
	{
		NumScriptThoughts++;
		FHOLAISummary Summary = HOLAISummary::ReadAISummary(PlayerIndex);
		FHOLAIGatherChoice Choice = HOLAIEconomy::ChooseGather(Summary);
		const int NumIdle = Summary.IdlePeasants.Num();
		if (NumIdle == 0)
		{
			return true;
		}

		// One line per decision (IA0.7 reads them): "AI slot 1 step 40: 3 idle -> gather Food #12".
		const int64 Step = GetStepIndex();
		if (!Choice.bGather)
		{
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: {NumIdle} idle -> no source");
			return true;
		}
		const int SourceIndex = Choice.SourceIndex;
		const FString ResourceName = Choice.ResourceName;
		const bool bQueued = HOLAIOrder::IssueAIGatherOrder(PlayerIndex, Choice.Peasants, SourceIndex);
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
