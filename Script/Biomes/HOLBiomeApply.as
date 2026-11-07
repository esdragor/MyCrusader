// What the client sets on the world for a biome, in the engine's own units (BiomeInProgress.md B1, GDD D651 to D656): the table's whole
// numbers (HOLBiomeRules.as: per mille, tenths of lux, 0.0001) turned into the floats of the ground instance's parameters (ColorA, ColorB,
// ColorC, the detail texture), the directional light, the sky light and the height haze. This is the whole of the DECISIONS of applying a
// biome; the setting itself (finding the instance and the lights of the loaded map, writing the properties) is the thin applier that
// reads this and has nothing to choose. Nothing here is a shadow, a reflection or a post process (D258, D265).

// One biome in the engine's units.
struct FHOLBiomeLook
{
	FString Key;

	// The ground (M_GroundVaried parameters, linear colours) and its CC0 detail texture (the source name, "" for none), the size of one
	// repeat (cm) and its strength (0 to 1).
	FLinearColor GroundA;
	FLinearColor GroundB;
	FLinearColor GroundC;
	FString DetailTexture;
	float DetailScaleCm = 900.0f;
	float DetailStrength = 0.7f;

	// The directional light: colour, strength (lux), pitch and yaw (degrees).
	FLinearColor SunColor;
	float SunIntensityLux = 10.0f;
	float SunPitch = -50.0f;
	float SunYaw = 30.0f;

	// The sky light: colour and strength (1 = the engine's default).
	FLinearColor SkyColor;
	float SkyIntensity = 1.0f;

	// The height haze: inscattering colour (linear), density (the engine's default is 0.02) and the distance where it starts (cm).
	FLinearColor FogColor;
	float FogDensity = 0.02f;
	float FogStartCm = 0.0f;
}

namespace HOLBiomeApply
{
	FLinearColor FromPermille(int R, int G, int B)
	{
		return FLinearColor(float(R) / 1000.0f, float(G) / 1000.0f, float(B) / 1000.0f, 1.0f);
	}

	// The look of a biome by index, the fresh grass's for an index out of the table (as HOLBiome::At reads it).
	FHOLBiomeLook LookOf(int Index)
	{
		FHOLBiome Biome = HOLBiome::At(Index);
		FHOLBiomeLook Look;
		Look.Key = Biome.Key;
		Look.GroundA = FromPermille(Biome.GroundAR, Biome.GroundAG, Biome.GroundAB);
		Look.GroundB = FromPermille(Biome.GroundBR, Biome.GroundBG, Biome.GroundBB);
		Look.GroundC = FromPermille(Biome.GroundCR, Biome.GroundCG, Biome.GroundCB);
		Look.DetailTexture = Biome.DetailTexture;
		Look.DetailScaleCm = float(Biome.DetailScaleCm);
		Look.DetailStrength = float(Biome.DetailStrengthPercent) / 100.0f;
		Look.SunColor = FromPermille(Biome.SunR, Biome.SunG, Biome.SunB);
		Look.SunIntensityLux = float(Biome.SunIntensityTenths) / 10.0f;
		Look.SunPitch = float(Biome.SunPitch);
		Look.SunYaw = float(Biome.SunYaw);
		Look.SkyColor = FromPermille(Biome.SkyR, Biome.SkyG, Biome.SkyB);
		Look.SkyIntensity = float(Biome.SkyIntensityPermille) / 1000.0f;
		Look.FogColor = FromPermille(Biome.FogR, Biome.FogG, Biome.FogB);
		Look.FogDensity = float(Biome.FogDensity) / 10000.0f;
		Look.FogStartCm = float(Biome.FogStartCm);
		return Look;
	}
	float SrgbChannelToLinear(float C)
	{
		return C <= 0.04045f ? C / 12.92f : Math::Pow((C + 0.055f) / 1.055f, 2.4f);
	}

	// The linear colour whose sRGB 8-bit form is the given one (the sky light's setter converts to sRGB itself).
	FLinearColor SrgbToLinear(const FLinearColor& Color)
	{
		return FLinearColor(SrgbChannelToLinear(Color.R), SrgbChannelToLinear(Color.G), SrgbChannelToLinear(Color.B), 1.0f);
	}

	// The biome to put on the world (B1 visual): the console's override first (hol.Biome.Index 0 to 4, the scenarios and the tour of the
	// five biomes), else the match's key (the lobby's choice, replicated with the configuration). -1 = the map's own: the look the map file
	// carries (Tools/Editor/maps/ambience.py) is left as it is, nothing is set. "default" and an unknown key are the map's own.
	int WantedBiome(const FString& MatchKey, int OverrideIndex)
	{
		if (HOLBiome::IsValid(OverrideIndex))
		{
			return OverrideIndex;
		}
		return HOLBiome::IndexOfKey(MatchKey);
	}

	// The line the applier writes when it sets a biome: what the WORLD must read back (hol.Biome.World in C++ reads the lights, the haze
	// and the ground of the loaded map and writes the same fields). Tools/Tests/biome_apply_check.ps1 compares the two.
	FString ExpectLine(const FHOLBiomeLook& Look)
	{
		return f"Biome expect: key={Look.Key} sun={Look.SunIntensityLux} sunPitch={Look.SunPitch} sky={Look.SkyIntensity} fog={Look.FogDensity} groundA={Look.GroundA.R},{Look.GroundA.G},{Look.GroundA.B}";
	}
}
