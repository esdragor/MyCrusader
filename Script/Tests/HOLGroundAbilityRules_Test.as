// The ground-aimed unit ability, pure rule (Script/Units/HOLGroundAbilityRules.as). Fixtures only: the numbers below are test values,
// NOT game values (radius, duration, range and cool-down of the Dawn light seal A3 and the Knight's challenge A7 are undecided).

// Refusals come in a fixed order: caster dead (1) before cool-down running (2) before target out of range (3); 0 when accepted.
void Test_GroundAbilityTargetRefusalOrder(FUnitTest& T)
{
	const int Accepted = HOLGroundAbility::TargetRefusal(true, 500, 1000, 0);
	T.AssertEquals(0, Accepted, "alive, in range, cool-down over: accepted");
	const int AtRange = HOLGroundAbility::TargetRefusal(true, 1000, 1000, 0);
	T.AssertEquals(0, AtRange, "target exactly at the range: accepted");
	const int Dead = HOLGroundAbility::TargetRefusal(false, 500, 1000, 0);
	T.AssertEquals(1, Dead, "dead caster: refused");
	const int Cooling = HOLGroundAbility::TargetRefusal(true, 500, 1000, 1);
	T.AssertEquals(2, Cooling, "one ms of cool-down left: refused");
	const int TooFar = HOLGroundAbility::TargetRefusal(true, 1001, 1000, 0);
	T.AssertEquals(3, TooFar, "one cm too far: refused");
	const int DeadAndCooling = HOLGroundAbility::TargetRefusal(false, 500, 1000, 5000);
	T.AssertEquals(1, DeadAndCooling, "dead and cooling down: dead comes first");
	const int DeadAndFar = HOLGroundAbility::TargetRefusal(false, 5000, 1000, 0);
	T.AssertEquals(1, DeadAndFar, "dead and too far: dead comes first");
	const int CoolingAndFar = HOLGroundAbility::TargetRefusal(true, 5000, 1000, 5000);
	T.AssertEquals(2, CoolingAndFar, "cooling down and too far: cool-down comes first");
}

// The zone edge counts as inside; one centimetre further is outside (on a straight and on a 3-4-5 diagonal).
void Test_GroundAbilityZoneEdgeInclusive(FUnitTest& T)
{
	T.AssertTrue(HOLZoneEffect::IsInZone(100, 200, 300, 100, 200), "the centre is inside");
	T.AssertTrue(HOLZoneEffect::IsInZone(100, 200, 300, 400, 200), "on the edge, east: inside");
	T.AssertFalse(HOLZoneEffect::IsInZone(100, 200, 300, 401, 200), "one cm past the edge, east: outside");
	T.AssertTrue(HOLZoneEffect::IsInZone(100, 200, 300, 100, -100), "on the edge, south (negative side): inside");
	T.AssertFalse(HOLZoneEffect::IsInZone(100, 200, 300, 100, -101), "one cm past the edge, south: outside");
	T.AssertTrue(HOLZoneEffect::IsInZone(0, 0, 500, 300, 400), "on the edge, 3-4-5 diagonal: inside");
	T.AssertFalse(HOLZoneEffect::IsInZone(0, 0, 500, 301, 400), "past the edge on the diagonal: outside");
	T.AssertFalse(HOLZoneEffect::IsInZone(0, 0, 0, 1, 0), "a zero radius holds only its centre");
	T.AssertTrue(HOLZoneEffect::IsInZone(0, 0, 0, 0, 0), "a zero radius holds its centre");
}

// The zone is active through StartMs + DurationMs - 1 and over at StartMs + DurationMs (same convention as IsRageEffectEnded); the cool-down end counts from the cast.
void Test_GroundAbilityZoneExpires(FUnitTest& T)
{
	T.AssertTrue(HOLZoneEffect::ZoneActive(1000, 6000, 1000), "active the instant it is put down");
	T.AssertTrue(HOLZoneEffect::ZoneActive(1000, 6000, 6999), "active at start + duration - 1");
	T.AssertFalse(HOLZoneEffect::ZoneActive(1000, 6000, 7000), "over at start + duration");
	T.AssertFalse(HOLZoneEffect::ZoneActive(1000, 6000, 9000), "over long after");
	T.AssertFalse(HOLZoneEffect::ZoneActive(1000, 6000, 999), "not active before it is put down");
	T.AssertFalse(HOLZoneEffect::ZoneActive(1000, 0, 1000), "a zero duration is never active");
	const int CooldownEnd = HOLGroundAbility::CooldownEndMs(1000, 40000);
	T.AssertEquals(41000, CooldownEnd, "the cool-down ends at cast + cool-down");
}
