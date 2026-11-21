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

// Per-camp view (D824 Q1 A, line 61): the enemy camp sees a unit in a zone only at N cm or less; allies see it at any distance.
// N is read from hol.Stealth.RevealDistance by the rule. Camp 0 owns the hidden unit, camp 1 is the enemy.
void Test_StealthCampViewEnemyBeyondRevealDistanceDoesNotSee(FUnitTest& T)
{
	float N = HOLStealth::RevealDistance();
	bool bSeen = HOLStealth::IsVisibleToCamp(StealthOneZone(), 0, FVector2D(0.0, 0.0), 1, StealthViewers(FVector2D(N + 1.0, 0.0)), N);
	T.AssertFalse(bSeen, "enemy camp at N + 1 cm: the unit in the zone is not seen");
	bool bSeenByNobody = HOLStealth::IsVisibleToCamp(StealthOneZone(), 0, FVector2D(0.0, 0.0), 1, TArray<FVector2D>(), N);
	T.AssertFalse(bSeenByNobody, "enemy camp without any unit: not seen");
}

void Test_StealthCampViewEnemyAtRevealDistanceSees(FUnitTest& T)
{
	float N = HOLStealth::RevealDistance();
	bool bSeen = HOLStealth::IsVisibleToCamp(StealthOneZone(), 0, FVector2D(0.0, 0.0), 1, StealthViewers(FVector2D(N, 0.0)), N);
	T.AssertTrue(bSeen, "enemy camp at exactly N cm: the unit in the zone is seen");
}

void Test_StealthCampViewAllyAlwaysSees(FUnitTest& T)
{
	float N = HOLStealth::RevealDistance();
	bool bSeenFar = HOLStealth::IsVisibleToCamp(StealthOneZone(), 0, FVector2D(0.0, 0.0), 0, StealthViewers(FVector2D(N * 50.0, 0.0)), N);
	T.AssertTrue(bSeenFar, "ally at 50 N: the unit in the zone is seen");
	bool bSeenNoViewer = HOLStealth::IsVisibleToCamp(StealthOneZone(), 0, FVector2D(0.0, 0.0), 0, TArray<FVector2D>(), N);
	T.AssertTrue(bSeenNoViewer, "own camp: seen even with no listed viewer");
}

// Look (D824 Q3 B): the zone outline shows on hover or when an own unit stands in the zone; never otherwise.
void Test_StealthOutlineShownOnHover(FUnitTest& T)
{
	FHOLStealthZone Zone = StealthOneZone()[0];
	bool bShown = HOLStealth::ShowsOutline(Zone, true, TArray<FVector2D>());
	T.AssertTrue(bShown, "hovered zone, no own unit: the outline shows");
}

void Test_StealthOutlineShownWhenOwnUnitEnters(FUnitTest& T)
{
	FHOLStealthZone Zone = StealthOneZone()[0];
	bool bShown = HOLStealth::ShowsOutline(Zone, false, StealthViewers(FVector2D(100.0, -50.0)));
	T.AssertTrue(bShown, "own unit inside the zone, not hovered: the outline shows");
}

void Test_StealthOutlineHiddenOtherwise(FUnitTest& T)
{
	FHOLStealthZone Zone = StealthOneZone()[0];
	bool bNone = HOLStealth::ShowsOutline(Zone, false, TArray<FVector2D>());
	T.AssertFalse(bNone, "not hovered, no own unit: no outline");
	float N = HOLStealth::RevealDistance();
	bool bOutside = HOLStealth::ShowsOutline(Zone, false, StealthViewers(FVector2D(N * 10.0, 0.0)));
	T.AssertFalse(bOutside, "own unit far outside the zone, not hovered: no outline");
}
