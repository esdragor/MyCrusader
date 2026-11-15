// Stealth zones, pure rule (B3, HOLStealthRules.as; D383). One zone 800 x 800 cm centred on the origin, the hidden unit at its centre unless a
// test says otherwise; N is the cvar value read by the rule (HOLStealth::RevealDistance), never a constant copied here.

TArray<FHOLStealthZone> StealthOneZone(float Half = 400.0)
{
	TArray<FHOLStealthZone> Zones;
	FHOLStealthZone Zone;
	Zone.Center = FVector2D(0.0, 0.0);
	Zone.HalfExtent = FVector2D(Half, Half);
	Zones.Add(Zone);
	return Zones;
}

TArray<FVector2D> StealthViewers(FVector2D A)
{
	TArray<FVector2D> Viewers;
	Viewers.Add(A);
	return Viewers;
}

// D383: an enemy outside the zone at N - 1 sees the unit; at N + 1 it does not (N = hol.Stealth.RevealDistance, read by the rule).
void Test_StealthSeenJustInsideRevealDistance(FUnitTest& T)
{
	float N = HOLStealth::RevealDistance();
	T.AssertTrue(N > 400.0, "N reaches beyond the zone edge, so the viewer stands outside the zone");
	T.AssertFalse(HOLStealth::IsHiddenFrom(StealthOneZone(), FVector2D(0.0, 0.0), StealthViewers(FVector2D(N - 1.0, 0.0)), N),
		"enemy outside the zone at N - 1: the unit is seen");
}

void Test_StealthHiddenJustBeyondRevealDistance(FUnitTest& T)
{
	float N = HOLStealth::RevealDistance();
	T.AssertTrue(HOLStealth::IsHiddenFrom(StealthOneZone(), FVector2D(0.0, 0.0), StealthViewers(FVector2D(N + 1.0, 0.0)), N),
		"enemy outside the zone at N + 1: the unit is hidden");
	T.AssertTrue(HOLStealth::IsHiddenFrom(StealthOneZone(), FVector2D(0.0, 0.0), TArray<FVector2D>(), N),
		"no enemy at all: the unit in the zone is hidden");
}

// D383: an enemy that entered the zone sees the unit, even farther than N.
void Test_StealthSeenByEnemyInsideTheZone(FUnitTest& T)
{
	float N = HOLStealth::RevealDistance();
	float Half = N * 2.0;
	FVector2D Unit(-Half + 1.0, 0.0);
	FVector2D Enemy(Half - 1.0, 0.0);
	T.AssertTrue(HOLStealth::IsHiddenFrom(StealthOneZone(Half), Unit, StealthViewers(FVector2D(Half + 1.0, 0.0)), N),
		"enemy just outside the far edge, beyond N: hidden");
	T.AssertFalse(HOLStealth::IsHiddenFrom(StealthOneZone(Half), Unit, StealthViewers(Enemy), N),
		"enemy inside the same zone, beyond N: seen");
}

// Outside every zone, stealth hides nothing (the normal fog vision applies).
void Test_StealthNothingHiddenOutsideZones(FUnitTest& T)
{
	float N = HOLStealth::RevealDistance();
	T.AssertFalse(HOLStealth::IsHiddenFrom(StealthOneZone(), FVector2D(5000.0, 0.0), TArray<FVector2D>(), N),
		"unit outside the zone: not hidden by stealth");
}
