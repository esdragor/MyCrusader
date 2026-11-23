// The Dawn's power book rows (D859, step 1b; AubeInProgress.md): the table the power book rule (HOLPowerBook::StateOf) and the
// gauge rule (HOLFactionResource::TrySpend) read. ONE source (D839). bEffectCoded stays false: the Sacred Light has no coded
// effect yet, the book shows it "Soon" (D601) until the effect exists. MinTier: not decided, the default 0 is kept.
// Tested in Script/Tests/HOLDawnPowers_Test.as.
namespace HOLDawnPowers
{
	// D890 (2026-11-23): the Sacred Light's cast range from the caster's origin is its OWN 1200 cm (12 m), a line apart from the Seal's
	// 800 cm (the LightSeal row of the unit ability table). Read by HOLFactionPowerCast::Decide. Tested: Test_SacredLightRangeIs1200.
	int SacredLightRangeCm()
	{
		return 1200;
	}

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
}
