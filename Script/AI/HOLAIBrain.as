// The AI slots' brain (IA0.4 B1): the C++ base (UHOLAIBrainSubsystem) calls Think for each AI slot every
// hol.AI.ThinkSteps steps, on the server only. No rule yet (B2): a thought only counts.
class UHOLAIBrain : UHOLAIBrainSubsystem
{
	// Thoughts this brain ran, all slots together.
	int NumScriptThoughts = 0;

	UFUNCTION(BlueprintOverride)
	bool Think(int PlayerIndex)
	{
		NumScriptThoughts++;
		return true;
	}
}
