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
	void Tick(float DeltaTime)
	{
		SinceLook += DeltaTime;
		if (SinceLook < 0.25f)
		{
			return;
		}
		SinceLook = 0.0f;

		int Wanted = HOLBiomeApply::WantedBiome(HOLBiomeRead::GetMatchBiomeKey(), CVar_BiomeIndex.GetInt());
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
	// The colours of the table are the ones ambience.py hands the editor as 8-bit values (no gamma step), hence bSRGB false.
	void ApplyToWorld(const FHOLBiomeLook& Look)
	{
		TArray<ADirectionalLight> Suns;
		GetAllActorsOfClass(Suns);
		for (ADirectionalLight Sun : Suns)
		{
			Sun.SetActorRotation(FRotator(Look.SunPitch, Look.SunYaw, 0.0f));
			Sun.LightComponent.SetIntensity(Look.SunIntensityLux);
			Sun.LightComponent.SetLightColor(Look.SunColor, false);
		}

		TArray<ASkyLight> Skies;
		GetAllActorsOfClass(Skies);
		for (ASkyLight Sky : Skies)
		{
			Sky.LightComponent.SetIntensity(Look.SkyIntensity);
			// The sky light's setter takes a linear colour and goes through sRGB itself: hand it the linear value of the 8-bit one.
			Sky.LightComponent.SetLightColor(HOLBiomeApply::SrgbToLinear(Look.SkyColor));
			Sky.LightComponent.RecaptureSky();
		}

		TArray<AExponentialHeightFog> Hazes;
		GetAllActorsOfClass(Hazes);
		for (AExponentialHeightFog Haze : Hazes)
		{
			Haze.Component.SetFogDensity(Look.FogDensity);
			Haze.Component.SetFogInscatteringColor(Look.FogColor);
			Haze.Component.SetStartDistance(Look.FogStartCm);
		}

		// The ground: the instance of M_GroundVaried the map points to (MI_Ground_*), made dynamic once, then reused for the next biome.
		TArray<AStaticMeshActor> Meshes;
		GetAllActorsOfClass(Meshes);
		for (AStaticMeshActor Mesh : Meshes)
		{
			UStaticMeshComponent Component = Mesh.StaticMeshComponent;
			UMaterialInterface Material = Component.GetMaterial(0);
			if (Material == nullptr)
			{
				continue;
			}
			UMaterialInstanceDynamic Ground = Cast<UMaterialInstanceDynamic>(Material);
			if (Ground == nullptr)
			{
				if (!Material.GetName().ToString().StartsWith("MI_Ground_"))
				{
					continue;
				}
				Ground = Component.CreateDynamicMaterialInstance(0);
			}
			Ground.SetVectorParameterValue(n"ColorA", Look.GroundA);
			Ground.SetVectorParameterValue(n"ColorB", Look.GroundB);
			Ground.SetVectorParameterValue(n"ColorC", Look.GroundC);
			Ground.SetScalarParameterValue(n"TextureScale", Look.DetailScaleCm);
			Ground.SetScalarParameterValue(n"DetailStrength", Look.DetailStrength);
		}
	}
}
