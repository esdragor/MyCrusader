// The Dawn's power book rows (D859, step 1b; AubeInProgress.md): the table the power book rule (HOLPowerBook::StateOf) and the
// gauge rule (HOLFactionResource::TrySpend) read. ONE source (D839). bEffectCoded stays false: the Sacred Light has no coded
// effect yet, the book shows it "Soon" (D601) until the effect exists. MinTier: not decided, the default 0 is kept.
// Tested in Script/Tests/HOLDawnPowers_Test.as.
// STUB of the first commit of step 1b: cost and cool-down are 0 (the test is red); the values come with the next commit.
namespace HOLDawnPowers
{
	TArray<FHOLPowerRow> Rows()
	{
		TArray<FHOLPowerRow> Result;
		FHOLPowerRow SacredLight;
		SacredLight.Key = "SacredLight";
		SacredLight.Cost = 0;
		SacredLight.CooldownMs = 0;
		Result.Add(SacredLight);
		return Result;
	}
}
