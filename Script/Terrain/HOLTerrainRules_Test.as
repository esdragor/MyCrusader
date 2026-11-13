// The terrain zones' speed rule, alone (MapsInProgress.md C3, GDD 5.4 D33, M-Q2 = A): zones and settings built from the preset (never the
// preset's numbers retyped in the expectations), HOLTerrain::SpeedPercentAt / ScaledSpeed / StopsCharge asked.

FHOLTerrainZone MarshZone(int X, int Y, int Radius)
{
	FHOLTerrainZone Zone;
	Zone.Kind = HOLTerrain::Marsh;
	Zone.X = X;
	Zone.Y = Y;
	Zone.Radius = Radius;
	return Zone;
}

// Outside every zone a walker keeps its speed; inside a marsh it walks at the preset's percent; the edge counts in.
void Test_TerrainMarshSlows(FUnitTest& T)
{
	FHOLTerrainSettings S = HOLTerrain::Preset(0);
	TArray<FHOLTerrainZone> Zones;
	Zones.Add(MarshZone(1000, 2000, 500));
	T.AssertTrue(S.MarshSpeedPercent < 100, "the preset's marsh slows");
	T.AssertTrue(S.MarshSpeedPercent > 0, "the preset's marsh does not stop");
	T.AssertEquals(S.MarshSpeedPercent, HOLTerrain::SpeedPercentAt(S, Zones, 1000, 2000), "in the middle of the marsh");
	T.AssertEquals(S.MarshSpeedPercent, HOLTerrain::SpeedPercentAt(S, Zones, 1500, 2000), "on the edge of the marsh");
	T.AssertEquals(100, HOLTerrain::SpeedPercentAt(S, Zones, 1501, 2000), "one centimetre outside");
	T.AssertEquals(100, HOLTerrain::SpeedPercentAt(S, Zones, -4000, 7000), "far away");
}

// No zone at all: full speed everywhere. Two overlapping zones: the slowest rule, not a product.
void Test_TerrainNoZoneAndOverlap(FUnitTest& T)
{
	FHOLTerrainSettings S = HOLTerrain::Preset(0);
	TArray<FHOLTerrainZone> None;
	T.AssertEquals(100, HOLTerrain::SpeedPercentAt(S, None, 0, 0), "no zone, full speed");
	TArray<FHOLTerrainZone> Two;
	Two.Add(MarshZone(0, 0, 800));
	Two.Add(MarshZone(300, 0, 800));
	T.AssertEquals(S.MarshSpeedPercent, HOLTerrain::SpeedPercentAt(S, Two, 200, 0), "in both marshes: slowed once");
}

// The two presets differ (the user picks one), both slow without stopping; a speed scaled never reaches 0 for a walker.
void Test_TerrainPresetsAndScaledSpeed(FUnitTest& T)
{
	FHOLTerrainSettings Strong = HOLTerrain::Preset(0);
	FHOLTerrainSettings Mild = HOLTerrain::Preset(1);
	T.AssertTrue(Strong.MarshSpeedPercent < Mild.MarshSpeedPercent, "preset 0 slows more than preset 1");
	T.AssertTrue(Mild.MarshSpeedPercent < 100, "preset 1 still slows");
	T.AssertEquals(0, HOLTerrain::ScaledSpeed(0, Strong.MarshSpeedPercent), "a unit that does not walk stays still");
	T.AssertEquals(1, HOLTerrain::ScaledSpeed(1, 10), "a walker never reaches 0 cm/s");
	T.AssertEquals(100 * Strong.MarshSpeedPercent / 100, HOLTerrain::ScaledSpeed(100, Strong.MarshSpeedPercent), "the percent of the speed");
	T.AssertEquals(697, HOLTerrain::ScaledSpeed(697, 100), "full speed outside");
}

// A marsh stops a charge (GDD 5.4) unless the settings say otherwise; outside it does not.
void Test_TerrainMarshStopsCharge(FUnitTest& T)
{
	FHOLTerrainSettings S = HOLTerrain::Preset(0);
	TArray<FHOLTerrainZone> Zones;
	Zones.Add(MarshZone(0, 0, 400));
	T.AssertTrue(HOLTerrain::StopsCharge(S, Zones, 100, 100), "in the marsh");
	T.AssertFalse(HOLTerrain::StopsCharge(S, Zones, 900, 0), "outside the marsh");
	S.bMarshStopsCharge = false;
	T.AssertFalse(HOLTerrain::StopsCharge(S, Zones, 100, 100), "the setting turned off");
}

// C5 (La Foret profonde, CQ6a = A): a road speeds up the walker, the edge counts in, a marsh over a road wins, the script and the server agree.
void Test_TerrainRoadSpeedsUp(FUnitTest& T)
{
	FHOLTerrainSettings S = HOLTerrain::Preset(0);
	T.AssertTrue(S.RoadSpeedPercent > 100, "the road speeds up");
	T.AssertEquals(HOLTerrain::GetServerDefaultRoadPercent(), S.RoadSpeedPercent, "the server and the script start with the same road speed");
	TArray<FHOLTerrainZone> Zones;
	FHOLTerrainZone Road;
	Road.Kind = HOLTerrain::Road;
	Road.X = 5000;
	Road.Y = 0;
	Road.Radius = 400;
	Zones.Add(Road);
	T.AssertEquals(S.RoadSpeedPercent, HOLTerrain::SpeedPercentAt(S, Zones, 5000, 0), "on the road");
	T.AssertEquals(S.RoadSpeedPercent, HOLTerrain::SpeedPercentAt(S, Zones, 5400, 0), "on the road's edge");
	T.AssertEquals(100, HOLTerrain::SpeedPercentAt(S, Zones, 5401, 0), "one centimetre off the road");
	T.AssertEquals(HOLTerrain::ServerRoadSpeedPercent(S.RoadSpeedPercent, 5000, 0, 400, 5400, 0), HOLTerrain::SpeedPercentAt(S, Zones, 5400, 0), "server and script on the edge");
	T.AssertEquals(HOLTerrain::ServerRoadSpeedPercent(S.RoadSpeedPercent, 5000, 0, 400, 5401, 0), HOLTerrain::SpeedPercentAt(S, Zones, 5401, 0), "server and script off the edge");
	Zones.Add(MarshZone(5000, 0, 200));
	T.AssertEquals(S.MarshSpeedPercent, HOLTerrain::SpeedPercentAt(S, Zones, 5000, 0), "a marsh over a road slows");
	T.AssertEquals(S.RoadSpeedPercent, HOLTerrain::SpeedPercentAt(S, Zones, 5300, 0), "beside the marsh, still the road");
	T.AssertEquals(130 * 100 / 100, HOLTerrain::ScaledSpeed(100, 130), "a scaled speed above 100 percent");
}

// The server (C++, FHOLTerrainRules) and this script's rule must agree: the same preset 0, the same percent on a grid of points of a
// zone (inside, on the edge, one centimetre out, far), the same scaled speeds.
void Test_TerrainServerAndScriptRulesAgree(FUnitTest& T)
{
	FHOLTerrainSettings S = HOLTerrain::Preset(0);
	T.AssertEquals(HOLTerrain::GetServerDefaultMarshPercent(), S.MarshSpeedPercent, "the server and the script start with the same marsh strength");
	TArray<FHOLTerrainZone> Zones;
	Zones.Add(MarshZone(1000, -2000, 500));
	for (int X = -1000; X <= 3000; X += 250)
	{
		for (int Y = -3000; Y <= 0; Y += 250)
		{
			T.AssertEquals(HOLTerrain::ServerSpeedPercent(S.MarshSpeedPercent, 1000, -2000, 500, X, Y), HOLTerrain::SpeedPercentAt(S, Zones, X, Y),
				"the server and the script agree at (" + X + ", " + Y + ")");
		}
	}
	T.AssertEquals(HOLTerrain::ServerSpeedPercent(S.MarshSpeedPercent, 1000, -2000, 500, 1500, -2000), HOLTerrain::SpeedPercentAt(S, Zones, 1500, -2000), "on the edge");
	T.AssertEquals(HOLTerrain::ServerSpeedPercent(S.MarshSpeedPercent, 1000, -2000, 500, 1501, -2000), HOLTerrain::SpeedPercentAt(S, Zones, 1501, -2000), "one centimetre out");
	for (int Speed = 0; Speed <= 700; Speed += 97)
	{
		T.AssertEquals(HOLTerrain::ServerScaledSpeed(Speed, S.MarshSpeedPercent), HOLTerrain::ScaledSpeed(Speed, S.MarshSpeedPercent), "the scaled speed " + Speed);
	}
	T.AssertEquals(HOLTerrain::ServerScaledSpeed(1, 10), HOLTerrain::ScaledSpeed(1, 10), "a walker never reaches 0");
}

// B4 (GDD 8.1, D33 "cover in a forest", Q-forest-cover): the damage a hit really deals to a target. Q1 = A (default): a painted forest
// zone; a ranged hit on a target inside deals less, a melee hit the same, the same arrow outside the full damage; the forest does not slow.
void Test_TerrainForestZoneCoversFromRanged(FUnitTest& T)
{
	FHOLTerrainSettings S = HOLTerrain::Preset(0);
	T.AssertTrue(S.ForestRangedDamagePercent < 100, "the preset's forest covers");
	T.AssertTrue(S.ForestRangedDamagePercent > 0, "the preset's forest does not make a unit untouchable");
	TArray<FHOLTerrainZone> Zones;
	FHOLTerrainZone Forest;
	Forest.Kind = HOLTerrain::Forest;
	Forest.X = 0;
	Forest.Y = 0;
	Forest.Radius = 1000;
	Zones.Add(Forest);
	TArray<FHOLTerrainTree> NoTrees;
	int Arrow = 20;
	int InForestRanged = HOLTerrain::ScaledDamage(Arrow, HOLTerrain::DamagePercentAt(S, Zones, NoTrees, true, 300, 0));
	int InForestMelee = HOLTerrain::ScaledDamage(Arrow, HOLTerrain::DamagePercentAt(S, Zones, NoTrees, false, 300, 0));
	int OutsideRanged = HOLTerrain::ScaledDamage(Arrow, HOLTerrain::DamagePercentAt(S, Zones, NoTrees, true, 1001, 0));
	T.AssertTrue(InForestRanged < Arrow, "an arrow on a target in the forest deals less damage");
	T.AssertEquals(Arrow * S.ForestRangedDamagePercent / 100, InForestRanged, "by the forest's percent");
	T.AssertEquals(Arrow, InForestMelee, "a melee hit in the forest deals its full damage");
	T.AssertEquals(Arrow, OutsideRanged, "the same arrow one centimetre outside deals its full damage");
	T.AssertTrue(HOLTerrain::InForest(S, Zones, NoTrees, 1000, 0), "the forest's edge counts in");
	T.AssertEquals(100, HOLTerrain::SpeedPercentAt(S, Zones, 300, 0), "the forest does not slow (not this rule)");
	T.AssertEquals(1, HOLTerrain::ScaledDamage(1, S.ForestRangedDamagePercent), "a covered hit still hurts");
}

// Q1 = B behind hol.Terrain.ForestByTrees: in a forest = enough trees near the unit; a painted zone alone no longer counts, a felled
// grove (fewer trees) no longer covers.
void Test_TerrainForestByTreesCovers(FUnitTest& T)
{
	FHOLTerrainSettings S = HOLTerrain::Preset(0);
	S.bForestByTrees = true;
	TArray<FHOLTerrainZone> NoZones;
	TArray<FHOLTerrainTree> Trees;
	for (int Index = 0; Index < S.ForestTreeCount; Index++)
	{
		FHOLTerrainTree Tree;
		Tree.X = 5000 + Index * 100;
		Tree.Y = 0;
		Trees.Add(Tree);
	}
	int Arrow = 20;
	T.AssertTrue(HOLTerrain::ScaledDamage(Arrow, HOLTerrain::DamagePercentAt(S, NoZones, Trees, true, 5000, 0)) < Arrow, "among the trees: covered");
	T.AssertEquals(Arrow, HOLTerrain::ScaledDamage(Arrow, HOLTerrain::DamagePercentAt(S, NoZones, Trees, false, 5000, 0)), "melee among the trees: full");
	T.AssertEquals(Arrow, HOLTerrain::ScaledDamage(Arrow, HOLTerrain::DamagePercentAt(S, NoZones, Trees, true, 5000 + 200 + S.ForestTreeRadius + 1, 0)),
		"just too far from the last tree: full");
	Trees.RemoveAt(0);
	T.AssertEquals(Arrow, HOLTerrain::ScaledDamage(Arrow, HOLTerrain::DamagePercentAt(S, NoZones, Trees, true, 5100, 0)), "one tree felled: no cover");
	FHOLTerrainZone Forest;
	Forest.Kind = HOLTerrain::Forest;
	Forest.Radius = 1000;
	TArray<FHOLTerrainZone> Zones;
	Zones.Add(Forest);
	T.AssertFalse(HOLTerrain::InForest(S, Zones, Trees, 0, 0), "by trees, a painted zone without trees does not count");
}

// The console variables give the question's defaults (Q-forest-cover Q1 = A zone, Q2 = A 75 %).
void Test_TerrainForestCVarsDefaults(FUnitTest& T)
{
	FHOLTerrainSettings S = HOLTerrain::SettingsFromCVars();
	T.AssertEquals(HOLTerrain::Preset(0).ForestRangedDamagePercent, S.ForestRangedDamagePercent, "hol.Terrain.ForestRangedPercent starts at the preset's");
	T.AssertFalse(S.bForestByTrees, "hol.Terrain.ForestByTrees starts off (painted zone)");
}
