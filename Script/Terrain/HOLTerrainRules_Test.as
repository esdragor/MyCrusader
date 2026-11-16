// The script-only terrain rules, alone (forest F4, ford F5, charge stopped in a marsh; GDD 5.4, 8.1, D33): settings built from the struct's
// defaults (never their numbers retyped in the expectations), HOLTerrain::ForestSpeedPercent / FordDamageTaken / StopsCharge asked.
// D836 (2026-11-16): the speed rule of the marsh and the road is the C++'s alone; its script copy and its tests (Test_TerrainMarshSlows,
// Test_TerrainNoZoneAndOverlap, Test_TerrainPresetsAndScaledSpeed, Test_TerrainRoadSpeedsUp, Test_TerrainServerAndScriptRulesAgree,
// Test_TerrainServerAndScriptAgreeOnEveryZone) are removed; the C++ tests HeroesOfLiberty.Terrain.Rules and .WalkSpeed cover it.

FHOLTerrainZone MarshZone(int X, int Y, int Radius)
{
	FHOLTerrainZone Zone;
	Zone.Kind = HOLTerrain::Marsh;
	Zone.X = X;
	Zone.Y = Y;
	Zone.Radius = Radius;
	return Zone;
}

// A marsh stops a charge (GDD 5.4) unless the settings say otherwise; outside it does not.
void Test_TerrainMarshStopsCharge(FUnitTest& T)
{
	FHOLTerrainSettings S;
	TArray<FHOLTerrainZone> Zones;
	Zones.Add(MarshZone(0, 0, 400));
	T.AssertTrue(HOLTerrain::StopsCharge(S, Zones, 100, 100), "in the marsh");
	T.AssertFalse(HOLTerrain::StopsCharge(S, Zones, 900, 0), "outside the marsh");
	S.bMarshStopsCharge = false;
	T.AssertFalse(HOLTerrain::StopsCharge(S, Zones, 100, 100), "the setting turned off");
}

// D796 (2026-11-14): the forest's cover (B4: Test_TerrainForestZoneCoversFromRanged, Test_TerrainForestByTreesCovers,
// Test_TerrainForestCVarsDefaults) is removed with its rule; a zone of an unknown kind (2, the old forest) does not stop a charge (its
// speed is the C++'s, D836).
void Test_TerrainUnknownZoneKindNeutral(FUnitTest& T)
{
	FHOLTerrainSettings S;
	TArray<FHOLTerrainZone> Zones;
	FHOLTerrainZone Old;
	Old.Kind = 2;
	Old.Radius = 1000;
	Zones.Add(Old);
	T.AssertFalse(HOLTerrain::StopsCharge(S, Zones, 300, 0), "an old forest zone does not stop a charge");
	Zones.Add(MarshZone(0, 0, 1000));
	T.AssertTrue(HOLTerrain::StopsCharge(S, Zones, 300, 0), "a marsh over it still does");
}

// F4 (GDD 8.1, D33): a cavalry walker and a siege engine go slower in a forest, a unit on foot does not; out of a forest nobody is slowed.
// The speeds are walked through the rule (ForestSpeedPercent, then the server's scaling, the one the movement step uses: D836), never the
// settings' numbers retyped.
void Test_TerrainForestSlowsCavalryAndSiege(FUnitTest& T)
{
	FHOLTerrainSettings S;
	int Speed = 600;
	int CavalryInForest = HOLTerrain::ServerScaledSpeed(Speed, HOLTerrain::ForestSpeedPercent(S, HOLTerrain::Cavalry, true));
	int CavalryOutside = HOLTerrain::ServerScaledSpeed(Speed, HOLTerrain::ForestSpeedPercent(S, HOLTerrain::Cavalry, false));
	int SiegeInForest = HOLTerrain::ServerScaledSpeed(Speed, HOLTerrain::ForestSpeedPercent(S, HOLTerrain::Siege, true));
	int FootInForest = HOLTerrain::ServerScaledSpeed(Speed, HOLTerrain::ForestSpeedPercent(S, HOLTerrain::OnFoot, true));
	T.AssertTrue(CavalryInForest < CavalryOutside, "the cavalry goes slower in a forest (" + CavalryInForest + " < " + CavalryOutside + ")");
	T.AssertTrue(CavalryInForest > 0, "the cavalry is slowed, not stopped");
	T.AssertTrue(SiegeInForest < Speed, "a siege engine goes slower in a forest (" + SiegeInForest + ")");
	T.AssertEquals(Speed, FootInForest, "a unit on foot keeps its speed in a forest");
	T.AssertEquals(Speed, CavalryOutside, "out of a forest the cavalry keeps its speed");
	T.AssertTrue(CavalryInForest < Speed && CavalryInForest >= Speed / 2, "a moderate slowing (between half and full speed)");
	S.ForestCavalrySpeedPercent = 50;
	T.AssertEquals(Speed / 2, HOLTerrain::ServerScaledSpeed(Speed, HOLTerrain::ForestSpeedPercent(S, HOLTerrain::Cavalry, true)), "the one setting line changes the cavalry");
	T.AssertEquals(SiegeInForest, HOLTerrain::ServerScaledSpeed(Speed, HOLTerrain::ForestSpeedPercent(S, HOLTerrain::Siege, true)), "and not the siege");
}

// F5 (GDD 8.1, D33): a unit on a ford defends less well (takes more of the same hit); off the ford, the hit is unchanged; 0 stays 0.
void Test_TerrainFordWeakensDefence(FUnitTest& T)
{
	FHOLTerrainSettings S;
	int Hit = 25;
	int OnFord = HOLTerrain::FordDamageTaken(S, Hit, true);
	int OffFord = HOLTerrain::FordDamageTaken(S, Hit, false);
	T.AssertTrue(OnFord > OffFord, "a unit on a ford takes more of the same hit (" + OnFord + " > " + OffFord + ")");
	T.AssertEquals(Hit, OffFord, "off the ford the hit is unchanged");
	T.AssertTrue(OnFord < Hit * 2, "a moderate weakening (less than double)");
	T.AssertEquals(0, HOLTerrain::FordDamageTaken(S, 0, true), "no hit, no damage, even on a ford");
	S.FordDamageTakenPercent = 200;
	T.AssertEquals(Hit * 2, HOLTerrain::FordDamageTaken(S, Hit, true), "the one setting line changes the ford");
}
