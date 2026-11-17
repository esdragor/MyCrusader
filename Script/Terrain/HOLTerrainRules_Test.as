// The terrain rules of the script (forest F4, ford F5, charge stopped in a marsh; GDD 5.4, 8.1, D33), read through UHOLTerrainReadLibrary
// (D836, 2026-11-17: no script copy of the rule, one source in C++): settings built from the struct's defaults, HOLTerrain::ForestSpeedPercent /
// FordDamageTaken / StopsCharge / WalkerSpeedPercentAt asked; the last three tests state the D810 numbers themselves (80, 70, 120, kinds 3 and 4).
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

// D836 (2026-11-17): the script holds no copy of the rule, it reads the C++'s (UHOLTerrainReadLibrary): the kinds, the categories and the
// defaults are the C++'s numbers (D810: Forest 3, Ford 4; 2 was the removed cover zone), the settings start at them.
void Test_TerrainReadsTheCppRule(FUnitTest& T)
{
	T.AssertEquals(0, HOLTerrain::Marsh, "marsh kind");
	T.AssertEquals(1, HOLTerrain::Road, "road kind");
	T.AssertEquals(3, HOLTerrain::Forest, "forest kind (2 was the removed cover zone)");
	T.AssertEquals(4, HOLTerrain::Ford, "ford kind");
	T.AssertEquals(0, HOLTerrain::OnFoot, "on foot");
	T.AssertEquals(1, HOLTerrain::Cavalry, "cavalry");
	T.AssertEquals(2, HOLTerrain::Siege, "siege");
	FHOLTerrainSettings S;
	T.AssertEquals(80, S.ForestCavalrySpeedPercent, "forest cavalry 80 (Q-forest-ford Q2 = A)");
	T.AssertEquals(70, S.ForestSiegeSpeedPercent, "forest siege 70");
	T.AssertEquals(120, S.FordDamageTakenPercent, "ford 120");
	T.AssertEquals(60, HOLTerrainRead::GetDefaultMarshSpeedPercent(), "marsh 60");
	T.AssertEquals(130, HOLTerrainRead::GetDefaultRoadSpeedPercent(), "road 130");
}

// D810 F4 at a point: a painted forest zone slows cavalry and siege, not a unit on foot; outside the zone (edge counts in) nobody; the
// marsh and the forest together: the slowest wins; a marsh slows the one on foot too.
void Test_TerrainWalkerSpeedAtPoint(FUnitTest& T)
{
	TArray<FHOLTerrainZone> Zones;
	FHOLTerrainZone Forest;
	Forest.Kind = HOLTerrain::Forest;
	Forest.X = 1000;
	Forest.Y = 2000;
	Forest.Radius = 500;
	Zones.Add(Forest);
	T.AssertEquals(80, HOLTerrain::WalkerSpeedPercentAt(Zones, HOLTerrain::Cavalry, 1000, 2000), "cavalry in a forest");
	T.AssertEquals(70, HOLTerrain::WalkerSpeedPercentAt(Zones, HOLTerrain::Siege, 1000, 2000), "siege in a forest");
	T.AssertEquals(100, HOLTerrain::WalkerSpeedPercentAt(Zones, HOLTerrain::OnFoot, 1000, 2000), "on foot in a forest");
	T.AssertEquals(80, HOLTerrain::WalkerSpeedPercentAt(Zones, HOLTerrain::Cavalry, 1500, 2000), "the edge counts in");
	T.AssertEquals(100, HOLTerrain::WalkerSpeedPercentAt(Zones, HOLTerrain::Cavalry, 1501, 2000), "one centimetre outside");
	T.AssertEquals(100, HOLTerrain::WalkerSpeedPercentAt(TArray<FHOLTerrainZone>(), HOLTerrain::Cavalry, 0, 0), "no zone, full speed");
	Zones.Add(MarshZone(1000, 2000, 300));
	T.AssertEquals(60, HOLTerrain::WalkerSpeedPercentAt(Zones, HOLTerrain::Cavalry, 1000, 2000), "marsh and forest: the slowest wins");
	T.AssertEquals(60, HOLTerrain::WalkerSpeedPercentAt(Zones, HOLTerrain::OnFoot, 1000, 2000), "a marsh slows the one on foot too");
}

// D810 F5: whether a point is on a ford is the zone's (the C++ rule); the damage taken composes with it.
void Test_TerrainFordZone(FUnitTest& T)
{
	TArray<FHOLTerrainZone> Zones;
	FHOLTerrainZone Ford;
	Ford.Kind = HOLTerrain::Ford;
	Ford.X = 0;
	Ford.Y = 0;
	Ford.Radius = 400;
	Zones.Add(Ford);
	T.AssertTrue(HOLTerrain::IsInZoneOfKind(Zones, HOLTerrain::Ford, 400, 0), "on the ford (edge in)");
	T.AssertFalse(HOLTerrain::IsInZoneOfKind(Zones, HOLTerrain::Ford, 401, 0), "off the ford");
	T.AssertFalse(HOLTerrain::IsInZoneOfKind(Zones, HOLTerrain::Forest, 0, 0), "a ford is not a forest");
	FHOLTerrainSettings S;
	T.AssertEquals(30, HOLTerrain::FordDamageTaken(S, 25, HOLTerrain::IsInZoneOfKind(Zones, HOLTerrain::Ford, 0, 0)), "25 on the ford is 30");
	T.AssertEquals(25, HOLTerrain::FordDamageTaken(S, 25, HOLTerrain::IsInZoneOfKind(Zones, HOLTerrain::Ford, 900, 0)), "25 off the ford is 25");
}
