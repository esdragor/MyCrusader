// The thin applier of a biome (BiomeInProgress.md B1 visual, GDD D651 to D656): at the loading of the map, and again when the match's
// biome arrives or changes, it reads the biome of the match (the lobby's key, or hol.Biome.Index for the scenarios), asks HOLBiomeApply
// what the world must look like (LookOf: units of the engine) and sets it on the lights, the haze and the ground of the loaded map.
// It decides nothing: the decisions are in HOLBiomeApply.as and HOLBiomeRules.as. It touches no unit, no gameplay and no shadow (D258).
// The map's own biome ("default") leaves the look the map file carries as it is; going back to it after a biome gives that look back.

const FConsoleVariable CVar_BiomeIndex("hol.Biome.Index", -1);

// The layout seed the generic decor is laid with (1 until the maps carry their own seed, BiomeInProgress.md B3): a measuring knob, read
// at the moment the decor is laid (set it, go back to -1, then pick the biome again).
const FConsoleVariable CVar_BiomeLayoutSeed("hol.Biome.LayoutSeed", 1);

class UHOLBiomeApplier : UScriptWorldSubsystem
{
	// The biome set on the world: -2 = not looked at yet, -1 = the map's own (nothing set), 0 to 4 = a biome of the table.
	int Applied = -2;

	// Seconds since the biome was last looked at (the configuration of a client arrives after the map's begin play).
	float SinceLook = 10.0f;

	// What the map file carried, taken once before the first biome is set (given back when the match goes back to the map's own look).
	bool bRemembered = false;
	FRotator MapSunRotation;
	float MapSunIntensity = 0.0f;
	FLinearColor MapSunColor;
	float MapSkyIntensity = 0.0f;
	FLinearColor MapSkyColor;
	float MapFogDensity = 0.0f;
	FLinearColor MapFogColor;
	float MapFogStart = 0.0f;

	// The ground components this applier took over (the only ones it ever touches) and the material each one had.
	TArray<UStaticMeshComponent> GroundComponents;
	TArray<UMaterialInterface> GroundOriginals;

	// The generic decor laid for the biome (B3): one actor with an instanced mesh per kind. Null when none is laid.
	AHOLBiomeDecor DecorLayer;

	// The densities (per hectare) the map asks of each slot of the biome's kind list: PROVISIONAL, the middle of what the baked DECOR of the
	// maps asks (Tools/Editor/maps), until each map names its own (Reference of HOLBiomeDecor::Plan).
	TArray<double> ReferenceDensities()
	{
		TArray<double> Reference;
		Reference.Add(60.0);
		Reference.Add(30.0);
		Reference.Add(20.0);
		Reference.Add(12.0);
		Reference.Add(8.0);
		Reference.Add(6.0);
		Reference.Add(4.0);
		Reference.Add(3.0);
		return Reference;
	}

	void RemoveDecor()
	{
		if (DecorLayer != nullptr)
		{
			DecorLayer.DestroyActor();
			DecorLayer = nullptr;
		}
	}

	UStaticMesh ShapeMesh(int Shape)
	{
		FString Path = "/Engine/BasicShapes/Cone.Cone";
		if (Shape == HOLBiomeDecor::ShapeSphere)
		{
			Path = "/Engine/BasicShapes/Sphere.Sphere";
		}
		else if (Shape == HOLBiomeDecor::ShapeCylinder)
		{
			Path = "/Engine/BasicShapes/Cylinder.Cylinder";
		}
		return Cast<UStaticMesh>(LoadObject(nullptr, Path));
	}

	// Lays the plan of the biome on the loaded map: from the bounds of the map, the points to keep clear (HOLBiomeDecorSites) and the layout
	// seed 1. Purely visual (D656): no collision, no navigation, no shadow (D258), cut at 12 000 cm like the baked decor.
	void LayDecor(int Biome)
	{
		TArray<AHOLMapBounds> Bounds;
		GetAllActorsOfClass(Bounds);
		if (Bounds.Num() == 0)
		{
			Log(n"LogHOLMatch", "Biome decor expect: pieces=0 kinds=0 (the map has no bounds)");
			return;
		}
		FVector Origin;
		FVector Extent;
		Bounds[0].GetActorBounds(false, Origin, Extent);
		double Half = Math::Max(Extent.X, Extent.Y);
		TArray<FVector2D> Avoid;
		for (FVector2D Site : HOLBiomeDecorSites::Gather())
		{
			Avoid.Add(FVector2D(Site.X - Origin.X, Site.Y - Origin.Y));
		}
		TArray<FHOLGenericDecor> Pieces = HOLBiomeDecor::Plan(Biome, CVar_BiomeLayoutSeed.GetInt(), Half, Avoid, ReferenceDensities());
		DecorLayer = Cast<AHOLBiomeDecor>(SpawnActor(AHOLBiomeDecor::StaticClass(), FVector::ZeroVector));
		UMaterialInterface Matte = Cast<UMaterialInterface>(LoadObject(nullptr, "/Game/Rendering/M_MatteColor.M_MatteColor"));
		TMap<int, UHierarchicalInstancedStaticMeshComponent> Parts;
		for (FHOLGenericDecor Piece : Pieces)
		{
			if (!Parts.Contains(Piece.Kind))
			{
				UHierarchicalInstancedStaticMeshComponent Part = UHierarchicalInstancedStaticMeshComponent::Create(DecorLayer, FName(f"BiomeDecor_{Piece.Kind}"));
				Part.SetStaticMesh(ShapeMesh(Piece.Shape));
				Part.SetCollisionEnabled(ECollisionEnabled::NoCollision);
				Part.SetCollisionProfileName(n"NoCollision");
				Part.SetGenerateOverlapEvents(false);
				Part.SetCastShadow(false);
				Part.SetCullDistances(8000, 12000);
				UMaterialInstanceDynamic Paint = Part.CreateDynamicMaterialInstance(0, Matte);
				if (Paint != nullptr)
				{
					Paint.SetVectorParameterValue(n"Color", FLinearColor(float(Piece.R) / 1000.0f, float(Piece.G) / 1000.0f, float(Piece.B) / 1000.0f, 1.0f));
				}
				Parts.Add(Piece.Kind, Part);
			}
			FTransform At(FRotator(0.0f, float(Piece.YawDegrees), 0.0f),
				FVector(Piece.X + Origin.X, Piece.Y + Origin.Y, Piece.ZCm),
				FVector(float(Piece.ScaleXPercent) / 100.0f, float(Piece.ScaleYPercent) / 100.0f, float(Piece.ScaleZPercent) / 100.0f));
			Parts[Piece.Kind].AddInstance(At, true);
		}
		Log(n"LogHOLMatch", f"Biome decor expect: pieces={Pieces.Num()} kinds={Parts.Num()} half={int(Half)} seed={CVar_BiomeLayoutSeed.GetInt()}");
	}

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
		int Before = Applied;
		Applied = Wanted;
		RemoveDecor();
		if (Wanted < 0)
		{
			if (Before >= 0)
			{
				RestoreMapLook();
			}
			return;
		}
		if (!bRemembered)
		{
			RememberMapLook();
		}
		FHOLBiomeLook Look = HOLBiomeApply::LookOf(Wanted);
		Log(n"LogHOLMatch", HOLBiomeApply::ExpectLine(Look));
		ApplyToWorld(Look);
		LayDecor(Wanted);
	}

	void RememberMapLook()
	{
		bRemembered = true;
		TArray<ADirectionalLight> Suns;
		GetAllActorsOfClass(Suns);
		if (Suns.Num() > 0)
		{
			MapSunRotation = Suns[0].GetActorRotation();
			MapSunIntensity = Suns[0].LightComponent.Intensity;
			MapSunColor = Suns[0].LightComponent.LightColor;
		}
		TArray<ASkyLight> Skies;
		GetAllActorsOfClass(Skies);
		if (Skies.Num() > 0)
		{
			MapSkyIntensity = Skies[0].LightComponent.Intensity;
			MapSkyColor = Skies[0].LightComponent.LightColor;
		}
		TArray<AExponentialHeightFog> Hazes;
		GetAllActorsOfClass(Hazes);
		if (Hazes.Num() > 0)
		{
			MapFogDensity = Hazes[0].Component.FogDensity;
			MapFogColor = Hazes[0].Component.FogInscatteringLuminance;
			MapFogStart = Hazes[0].Component.StartDistance;
		}
	}

	void RestoreMapLook()
	{
		if (!bRemembered)
		{
			return;
		}
		TArray<ADirectionalLight> Suns;
		GetAllActorsOfClass(Suns);
		for (ADirectionalLight Sun : Suns)
		{
			Sun.SetActorRotation(MapSunRotation);
			Sun.LightComponent.SetIntensity(MapSunIntensity);
			// The getter gave the linear value of the map's 8-bit colour (sRGB step): the setter takes it back the same way.
			Sun.LightComponent.SetLightColor(MapSunColor, true);
		}
		TArray<ASkyLight> Skies;
		GetAllActorsOfClass(Skies);
		for (ASkyLight Sky : Skies)
		{
			Sky.LightComponent.SetIntensity(MapSkyIntensity);
			Sky.LightComponent.SetLightColor(MapSkyColor);
			Sky.LightComponent.RecaptureSky();
		}
		TArray<AExponentialHeightFog> Hazes;
		GetAllActorsOfClass(Hazes);
		for (AExponentialHeightFog Haze : Hazes)
		{
			Haze.Component.SetFogDensity(MapFogDensity);
			Haze.Component.SetFogInscatteringColor(MapFogColor);
			Haze.Component.SetStartDistance(MapFogStart);
		}
		for (int I = 0; I < GroundComponents.Num(); I++)
		{
			GroundComponents[I].SetMaterial(0, GroundOriginals[I]);
		}
		GroundComponents.Empty();
		GroundOriginals.Empty();
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
		// Only the map's own MI_Ground_* meshes are taken over (a mesh with some other dynamic material is never recoloured).
		if (GroundComponents.Num() == 0)
		{
			TArray<AStaticMeshActor> Meshes;
			GetAllActorsOfClass(Meshes);
			for (AStaticMeshActor Mesh : Meshes)
			{
				UStaticMeshComponent Component = Mesh.StaticMeshComponent;
				UMaterialInterface Material = Component.GetMaterial(0);
				if (Material == nullptr || !Material.GetName().ToString().StartsWith("MI_Ground_"))
				{
					continue;
				}
				GroundComponents.Add(Component);
				GroundOriginals.Add(Material);
				Component.CreateDynamicMaterialInstance(0);
			}
		}
		for (UStaticMeshComponent Component : GroundComponents)
		{
			UMaterialInstanceDynamic Ground = Cast<UMaterialInstanceDynamic>(Component.GetMaterial(0));
			if (Ground == nullptr)
			{
				continue;
			}
			Ground.SetVectorParameterValue(n"ColorA", Look.GroundA);
			Ground.SetVectorParameterValue(n"ColorB", Look.GroundB);
			Ground.SetVectorParameterValue(n"ColorC", Look.GroundC);
			Ground.SetScalarParameterValue(n"TextureScale", Look.DetailScaleCm);
			Ground.SetScalarParameterValue(n"DetailStrength", Look.DetailStrength);
		}
	}
}
