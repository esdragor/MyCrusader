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
