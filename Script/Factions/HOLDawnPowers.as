// The Dawn's power book rows (D859, step 1b; AubeInProgress.md): the table the power book rule (HOLPowerBook::StateOf) and the
// gauge rule (HOLFactionResource::TrySpend) read. ONE source (D839). bEffectCoded stays false: the Sacred Light has no coded
// effect yet, the book shows it "Soon" (D601) until the effect exists. MinTier: not decided, the default 0 is kept.
// Tested in Script/Tests/HOLDawnPowers_Test.as.
namespace HOLDawnPowers
{
	TArray<FHOLPowerRow> Rows()
	{
		TArray<FHOLPowerRow> Result;
		FHOLPowerRow SacredLight;
		SacredLight.Key = "SacredLight";
		SacredLight.Cost = 25;
		SacredLight.CooldownMs = 20000;
		// S1-b: the Sacred Light now has a coded effect (HOLZoneEffect::StartSacredLight, cast by ServerCastFactionPower).
		SacredLight.bEffectCoded = true;
		Result.Add(SacredLight);
		return Result;
	}

	// D890: the Sacred Light's OWN cast range (a value of its own, no longer the Seal row's 800 cm), given by the user: 1200 cm. Measured from the cast origin to the aimed point (D897).
	int64 SacredLightRangeCm()
	{
		return 1200;
	}
}
