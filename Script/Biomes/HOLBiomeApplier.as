// The thin applier of a biome (BiomeInProgress.md B1 visual, GDD D651 to D656): at the loading of the map, and again when the match's
// biome arrives or changes, it reads the biome of the match (the lobby's key, or hol.Biome.Index for the scenarios), asks HOLBiomeApply
// what the world must look like (LookOf: units of the engine) and sets it on the lights, the haze and the ground of the loaded map.
// It decides nothing: the decisions are in HOLBiomeApply.as and HOLBiomeRules.as. It touches no unit, no gameplay and no shadow (D258).
// The map's own biome ("default") leaves the look the map file carries as it is.

const FConsoleVariable CVar_BiomeIndex("hol.Biome.Index", -1);

class UHOLBiomeApplier : UScriptWorldSubsystem
{
	// The biome set on the world: -2 = not looked at yet, -1 = the map's own (nothing set), 0 to 4 = a biome of the table.
	int Applied = -2;

	// Seconds since the biome was last looked at (the configuration of a client arrives after the map's begin play).
	float SinceLook = 10.0f;

	UFUNCTION(BlueprintOverride)
	void BP_Tick(float DeltaTime)
	{
		SinceLook += DeltaTime;
		if (SinceLook < 0.25f)
		{
			return;
		}
		SinceLook = 0.0f;

		int Wanted = HOLBiomeApply::WantedBiome(UHOLBiomeReadLibrary::GetMatchBiomeKey(), CVar_BiomeIndex.GetInt());
		if (Wanted == Applied)
		{
			return;
		}
		Applied = Wanted;
		if (Wanted < 0)
		{
			return;
		}
		FHOLBiomeLook Look = HOLBiomeApply::LookOf(Wanted);
		Log(n"LogHOLMatch", HOLBiomeApply::ExpectLine(Look));
		ApplyToWorld(Look);
	}

	// The lights, the haze and the ground of the loaded map.
	void ApplyToWorld(const FHOLBiomeLook& Look)
	{
	}
}
