// What the client sets on the world for a biome, in the engine's own units (BiomeInProgress.md B1, GDD D651 to D656): the table's whole
// numbers (per mille, tenths of lux, 0.0001) turned into the floats of the ground instance's parameters, the lights and the haze. The tests
// hold the conversion to the UNITS (a factor of ten wrong would put the sun at 100 lux or the haze at 0.2) by checking every biome's values
// against the engine ranges the baked ambiences already live in (Tools/Editor/maps/ambience.py BOUNDS, checked by Tools/Tests/biome_bounds_check.py
// against the table), and to the table by reading it back, never by retyped numbers.

bool ApplyNear(float A, float B)
{
	return Math::Abs(A - B) < 0.0005f;
}

// The lights and the haze of every biome are inside the engine ranges (units right) and are the table's numbers divided by their unit.
void Test_ApplyUnitsMatchTheEngine(FUnitTest& T)
{
	for (int Index = 0; Index < HOLBiome::Count; Index++)
	{
		FHOLBiome Biome = HOLBiome::At(Index);
		FHOLBiomeLook Look = HOLBiomeApply::LookOf(Index);
		T.AssertTrue(Look.SunIntensityLux >= 7.0f && Look.SunIntensityLux <= 12.0f, f"biome {Index}: the sun's strength in lux ({Look.SunIntensityLux}) is in the engine's range");
		T.AssertTrue(ApplyNear(Look.SunIntensityLux * 10.0f, float(Biome.SunIntensityTenths)), f"biome {Index}: lux are tenths over ten");
		T.AssertTrue(Look.SkyIntensity >= 0.85f && Look.SkyIntensity <= 1.3f, f"biome {Index}: the sky light's strength is in range");
		T.AssertTrue(ApplyNear(Look.SkyIntensity * 1000.0f, float(Biome.SkyIntensityPermille)), f"biome {Index}: the sky's strength is per mille over a thousand");
		T.AssertTrue(Look.FogDensity >= 0.004f && Look.FogDensity <= 0.025f, f"biome {Index}: the haze's density ({Look.FogDensity}) is in the engine's range");
		T.AssertTrue(ApplyNear(Look.FogDensity * 10000.0f, float(Biome.FogDensity)), f"biome {Index}: the density is 0.0001 units");
		T.AssertTrue(ApplyNear(Look.FogStartCm, float(Biome.FogStartCm)), f"biome {Index}: the haze starts where the table says");
		T.AssertTrue(ApplyNear(Look.SunPitch, float(Biome.SunPitch)) && ApplyNear(Look.SunYaw, float(Biome.SunYaw)), f"biome {Index}: the sun's angles are degrees as they are");
		T.AssertTrue(ApplyNear(Look.SunColor.R * 1000.0f, float(Biome.SunR)) && ApplyNear(Look.SunColor.G * 1000.0f, float(Biome.SunG)) && ApplyNear(Look.SunColor.B * 1000.0f, float(Biome.SunB)), f"biome {Index}: the sun's colour is per mille over a thousand");
		T.AssertTrue(ApplyNear(Look.FogColor.B * 1000.0f, float(Biome.FogB)) && ApplyNear(Look.FogColor.R * 1000.0f, float(Biome.FogR)), f"biome {Index}: the haze's colour is per mille over a thousand");
		T.AssertTrue(ApplyNear(Look.SkyColor.G * 1000.0f, float(Biome.SkyG)), f"biome {Index}: the sky's colour is per mille over a thousand");
	}
}

// The ground: three colours that are the table's, a detail texture name and repeat size, a strength between 0 and 1 (the snow has none yet).
void Test_ApplyGround(FUnitTest& T)
{
	for (int Index = 0; Index < HOLBiome::Count; Index++)
	{
		FHOLBiome Biome = HOLBiome::At(Index);
		FHOLBiomeLook Look = HOLBiomeApply::LookOf(Index);
		T.AssertTrue(ApplyNear(Look.GroundA.G * 1000.0f, float(Biome.GroundAG)) && ApplyNear(Look.GroundB.R * 1000.0f, float(Biome.GroundBR)) && ApplyNear(Look.GroundC.B * 1000.0f, float(Biome.GroundCB)), f"biome {Index}: the ground's three colours are the table's");
		T.AssertEquals(Biome.DetailTexture, Look.DetailTexture, f"biome {Index}: the detail texture's source name");
		T.AssertTrue(Look.DetailStrength >= 0.0f && Look.DetailStrength <= 1.0f, f"biome {Index}: the detail strength is a fraction");
		T.AssertTrue(ApplyNear(Look.DetailStrength * 100.0f, float(Biome.DetailStrengthPercent)), f"biome {Index}: the strength is percent over a hundred");
		T.AssertTrue(ApplyNear(Look.DetailScaleCm, float(Biome.DetailScaleCm)), f"biome {Index}: the repeat size in cm");
		// A ground is never black nor blown out (the team colours must stay readable on it): at least one channel is lit, none is above 1.
		T.AssertTrue(Look.GroundA.R <= 1.0f && Look.GroundA.G <= 1.0f && Look.GroundA.B <= 1.0f && Look.GroundA.R + Look.GroundA.G + Look.GroundA.B > 0.1f, f"biome {Index}: the main ground colour is neither black nor blown out");
	}
	T.AssertEquals(0.0f, HOLBiomeApply::LookOf(HOLBiome::Snow).DetailStrength, "the snow has no detail texture yet (BiomeInProgress.md)");
}

// Same rule as the table: an index out of it applies the fresh grass; the key rides along so the log says what was applied.
void Test_ApplyUnknownIsFreshGrassAndKeyRidesAlong(FUnitTest& T)
{
	FHOLBiomeLook Wrong = HOLBiomeApply::LookOf(99);
	FHOLBiomeLook Grass = HOLBiomeApply::LookOf(HOLBiome::FreshGrass);
	T.AssertEquals(Grass.Key, Wrong.Key, "an unknown biome applies the fresh grass");
	T.AssertTrue(ApplyNear(Wrong.FogDensity, Grass.FogDensity) && ApplyNear(Wrong.GroundA.R, Grass.GroundA.R), "its numbers too");
	for (int Index = 0; Index < HOLBiome::Count; Index++)
	{
		T.AssertEquals(HOLBiome::KeyOf(Index), HOLBiomeApply::LookOf(Index).Key, f"biome {Index}: its key rides along");
	}
}
