// GLORFINDEL-1120f: the edge cases of HOLFactionPowerCast::Decide listed in GLORFINDEL-1120e (negative index, refusal priority, a
// refusal writes nothing, a cool-down ended long ago, distance rounding, negative deltas, huge coordinates). Every expected value is
// computed by the rule chain (Decide over HOLDawnPowers, HOLFactionResource, HOLGroundAbility), never copied. D838: a script test.
// Only cases whose result does not depend on what the Seal of Light costs (Retours/Q-sceau-cout.md) are here: a funded gauge is a
// full gauge, and "gauge short" is always the LAST refusal checked, so it never decides a case below.

namespace FactionPowerCastEdgeTest
{
	FHOLFactionPowerDecision Cast(int Faction, int PowerIndex, const FHOLFactionResource& Gauge, int CooldownEndMs, int NowMs,
		int OriginX, int OriginY, int PointX, int PointY)
	{
		return HOLFactionPowerCast::Decide(Faction, PowerIndex, Gauge, CooldownEndMs, NowMs, OriginX, OriginY, PointX, PointY);
	}

	FHOLFactionResource EmptyGauge()
	{
		return HOLFactionResource::Make(0, 0, HOLDawnValues::HonorCap());
	}

	// A decision for the Dawn, power 0, ready, funded, from (0,0) to (PointX, PointY).
	FHOLFactionPowerDecision CastTo(int PointX, int PointY)
	{
		FHOLFactionResource Full = FactionPowerCastTest::FullGauge();
		return Cast(FactionPowerCastTest::Dawn(), 0, Full, 0, 0, 0, 0, PointX, PointY);
	}
}

void Test_FactionPowerCastNegativeIndex(FUnitTest& T)
{
	FHOLFactionResource Full = FactionPowerCastTest::FullGauge();
	FHOLFactionPowerDecision Result = FactionPowerCastEdgeTest::Cast(FactionPowerCastTest::Dawn(), -1, Full, 7000, 1000, 0, 0, 0, 0);
	T.AssertFalse(Result.bAccepted, "a negative power index is refused");
	T.AssertEquals(HOLFactionPowerCast::RefusalNoSuchPower(), Result.Refusal, "the reason is that the power does not exist");
	T.AssertEquals(Full.Value, Result.NewGaugeValue, "a negative index leaves the gauge untouched");
	T.AssertEquals(7000, Result.NewCooldownEndMs, "a negative index leaves the cool-down end untouched");
}

void Test_FactionPowerCastRefusalPriority(FUnitTest& T)
{
	FHOLFactionResource Full = FactionPowerCastTest::FullGauge();
	FHOLFactionResource Empty = FactionPowerCastEdgeTest::EmptyGauge();
	const int Dawn = FactionPowerCastTest::Dawn();
	const int FarX = int(HOLFactionPowerCast::SealRangeCm()) + 50;
	const int NoPower = HOLDawnPowers::Rows().Num();

	// (a) not the Dawn wins over everything else: invalid index, cool-down running, out of range, empty gauge.
	FHOLFactionPowerDecision NotDawn = FactionPowerCastEdgeTest::Cast(Dawn + 1, -1, Empty, 9000, 1000, 0, 0, FarX, 0);
	T.AssertEquals(HOLFactionPowerCast::RefusalNotDawn(), NotDawn.Refusal, "not the Dawn wins over an invalid index, a cool-down, a range and an empty gauge");

	// (b) a power that does not exist wins over a running cool-down, a range and an empty gauge.
	FHOLFactionPowerDecision NoSuch = FactionPowerCastEdgeTest::Cast(Dawn, NoPower, Empty, 9000, 1000, 0, 0, FarX, 0);
	T.AssertEquals(HOLFactionPowerCast::RefusalNoSuchPower(), NoSuch.Refusal, "no such power wins over a cool-down, a range and an empty gauge");

	// (c) cool-down + out of range -> the cool-down.
	FHOLFactionPowerDecision CooldownAndRange = FactionPowerCastEdgeTest::Cast(Dawn, 0, Full, 9000, 1000, 0, 0, FarX, 0);
	T.AssertEquals(HOLFactionPowerCast::RefusalCooldown(), CooldownAndRange.Refusal, "the cool-down is reported before the range");

	// (d) out of range + empty gauge -> the range.
	FHOLFactionPowerDecision RangeAndGauge = FactionPowerCastEdgeTest::Cast(Dawn, 0, Empty, 0, 0, 0, 0, FarX, 0);
	T.AssertEquals(HOLFactionPowerCast::RefusalOutOfRange(), RangeAndGauge.Refusal, "the range is reported before an empty gauge");

	// (e) cool-down + empty gauge -> the cool-down.
	FHOLFactionPowerDecision CooldownAndGauge = FactionPowerCastEdgeTest::Cast(Dawn, 0, Empty, 9000, 1000, 0, 0, 0, 0);
	T.AssertEquals(HOLFactionPowerCast::RefusalCooldown(), CooldownAndGauge.Refusal, "the cool-down is reported before an empty gauge");

	// (f) all three at once (cool-down, range, empty gauge) -> the cool-down.
	FHOLFactionPowerDecision AllThree = FactionPowerCastEdgeTest::Cast(Dawn, 0, Empty, 9000, 1000, 0, 0, FarX, 0);
	T.AssertEquals(HOLFactionPowerCast::RefusalCooldown(), AllThree.Refusal, "with a cool-down, a range and an empty gauge the cool-down comes first");
}

void Test_FactionPowerCastRefusalWritesNothing(FUnitTest& T)
{
	FHOLFactionResource Full = FactionPowerCastTest::FullGauge();
	const int Dawn = FactionPowerCastTest::Dawn();
	const int FarX = int(HOLFactionPowerCast::SealRangeCm()) + 50;
	const int Cooldown = 12345;

	// A refusal hands back the cool-down end it was given (and the gauge it was given), whatever the reason.
	FHOLFactionPowerDecision NotDawn = FactionPowerCastEdgeTest::Cast(Dawn + 1, 0, Full, Cooldown, 1, 0, 0, 0, 0);
	T.AssertFalse(NotDawn.bAccepted, "not the Dawn is refused");
	T.AssertEquals(Cooldown, NotDawn.NewCooldownEndMs, "not the Dawn: cool-down end unchanged");
	T.AssertEquals(Full.Value, NotDawn.NewGaugeValue, "not the Dawn: gauge unchanged");

	FHOLFactionPowerDecision NoSuch = FactionPowerCastEdgeTest::Cast(Dawn, HOLDawnPowers::Rows().Num(), Full, Cooldown, 1, 0, 0, 0, 0);
	T.AssertFalse(NoSuch.bAccepted, "a power beyond the book is refused");
	T.AssertEquals(Cooldown, NoSuch.NewCooldownEndMs, "no such power: cool-down end unchanged");
	T.AssertEquals(Full.Value, NoSuch.NewGaugeValue, "no such power: gauge unchanged");

	FHOLFactionPowerDecision Running = FactionPowerCastEdgeTest::Cast(Dawn, 0, Full, Cooldown, 1, 0, 0, 0, 0);
	T.AssertFalse(Running.bAccepted, "a running cool-down refuses");
	T.AssertEquals(Cooldown, Running.NewCooldownEndMs, "cool-down refusal: cool-down end unchanged, not pushed back");
	T.AssertEquals(Full.Value, Running.NewGaugeValue, "cool-down refusal: gauge unchanged");

	// Ready (the clock is past the stored end), but out of range.
	FHOLFactionPowerDecision Far = FactionPowerCastEdgeTest::Cast(Dawn, 0, Full, Cooldown, Cooldown + 1, 0, 0, FarX, 0);
	T.AssertFalse(Far.bAccepted, "out of range is refused even when the cool-down has ended");
	T.AssertEquals(Cooldown, Far.NewCooldownEndMs, "range refusal: cool-down end unchanged");
	T.AssertEquals(Full.Value, Far.NewGaugeValue, "range refusal: gauge unchanged");
}

void Test_FactionPowerCastCooldownLongEnded(FUnitTest& T)
{
	FHOLFactionResource Full = FactionPowerCastTest::FullGauge();
	const int Now = 1000000;
	// The stored end is far in the past: the remaining time is negative, which must read as "ready", and the new end counts from now.
	FHOLFactionPowerDecision Result = FactionPowerCastEdgeTest::Cast(FactionPowerCastTest::Dawn(), 0, Full, 500, Now, 0, 0, 0, 0);
	T.AssertTrue(Result.bAccepted, "a cool-down that ended long ago does not refuse");
	int ExpectedEnd = HOLGroundAbility::CooldownEndMs(Now, HOLFactionPowerCast::SealCooldownMs());
	T.AssertEquals(ExpectedEnd, Result.NewCooldownEndMs, "the new cool-down end counts from the cast, not from the old end");
}

void Test_FactionPowerCastDistanceRounding(FUnitTest& T)
{
	const int Range = int(HOLFactionPowerCast::SealRangeCm());

	// The exact diagonal 3-4-5 scaled to the range (480, 640) is 800.000 cm: accepted.
	FHOLFactionPowerDecision Diagonal = FactionPowerCastEdgeTest::CastTo(Range * 3 / 5, Range * 4 / 5);
	T.AssertTrue(Diagonal.bAccepted, "the exact diagonal at the range is accepted");

	// (800, 1) is 800.0006 cm: the distance is rounded up to 801, so it is refused (this is what proves the ceiling).
	FHOLFactionPowerDecision JustOver = FactionPowerCastEdgeTest::CastTo(Range, 1);
	T.AssertFalse(JustOver.bAccepted, "800.0006 cm is beyond the range (rounded up)");
	T.AssertEquals(HOLFactionPowerCast::RefusalOutOfRange(), JustOver.Refusal, "the reason is the range");

	// (799, 1) is 799.0006 cm, rounded up to 800: still in range.
	FHOLFactionPowerDecision JustUnder = FactionPowerCastEdgeTest::CastTo(Range - 1, 1);
	T.AssertTrue(JustUnder.bAccepted, "799.0006 cm rounds up to the range and is accepted");
}

void Test_FactionPowerCastNegativeDeltas(FUnitTest& T)
{
	FHOLFactionResource Full = FactionPowerCastTest::FullGauge();
	const int Dawn = FactionPowerCastTest::Dawn();
	const int Range = int(HOLFactionPowerCast::SealRangeCm());

	FHOLFactionPowerDecision Left = FactionPowerCastEdgeTest::CastTo(-Range, 0);
	T.AssertTrue(Left.bAccepted, "a point exactly at the range on the left is accepted");
	FHOLFactionPowerDecision LeftFar = FactionPowerCastEdgeTest::CastTo(-Range - 1, 0);
	T.AssertFalse(LeftFar.bAccepted, "one centimetre beyond the range on the left is refused");
	T.AssertEquals(HOLFactionPowerCast::RefusalOutOfRange(), LeftFar.Refusal, "the reason is the range");

	FHOLFactionPowerDecision Below = FactionPowerCastEdgeTest::CastTo(0, -Range);
	T.AssertTrue(Below.bAccepted, "a point exactly at the range below is accepted");
	FHOLFactionPowerDecision BelowFar = FactionPowerCastEdgeTest::CastTo(0, -Range - 1);
	T.AssertFalse(BelowFar.bAccepted, "one centimetre beyond the range below is refused");

	FHOLFactionPowerDecision DiagonalNeg = FactionPowerCastEdgeTest::CastTo(-Range * 3 / 5, -Range * 4 / 5);
	T.AssertTrue(DiagonalNeg.bAccepted, "the exact negative diagonal at the range is accepted");

	// A non-zero origin, the point on the negative side of its circle.
	FHOLFactionPowerDecision OnCircle = FactionPowerCastEdgeTest::Cast(Dawn, 0, Full, 0, 0, 1000, -500, 1000 - Range, -500);
	T.AssertTrue(OnCircle.bAccepted, "from a non-zero origin, a point at the range on the negative side is accepted");
	FHOLFactionPowerDecision OffCircle = FactionPowerCastEdgeTest::Cast(Dawn, 0, Full, 0, 0, 1000, -500, 1000 - Range - 1, -500);
	T.AssertFalse(OffCircle.bAccepted, "from a non-zero origin, one centimetre further is refused");
}

void Test_FactionPowerCastHugeCoordinates(FUnitTest& T)
{
	const int IntMax = 2147483647;
	const int IntMin = -2147483647 - 1;

	// The delta is 2^32 - 1 on one axis, then on both axes: its square (about 1.8e19) is beyond an int64 (9.2e18) and wraps negative,
	// so the distance is computed from a negative number. The point is nowhere near the range and must be refused, never accepted
	// by a wrapped distance (the client chooses the point).
	FHOLFactionResource Full = FactionPowerCastTest::FullGauge();
	const int Dawn = FactionPowerCastTest::Dawn();
	FHOLFactionPowerDecision OneAxis = FactionPowerCastEdgeTest::Cast(Dawn, 0, Full, 0, 0, IntMin, 0, IntMax, 0);
	T.AssertFalse(OneAxis.bAccepted, "a delta of 2^32 - 1 cm on one axis is refused");
	T.AssertEquals(HOLFactionPowerCast::RefusalOutOfRange(), OneAxis.Refusal, "one axis: the reason is the range");

	FHOLFactionPowerDecision BothAxes = FactionPowerCastEdgeTest::Cast(Dawn, 0, Full, 0, 0, IntMin, IntMin, IntMax, IntMax);
	T.AssertFalse(BothAxes.bAccepted, "a delta of 2^32 - 1 cm on both axes is refused (no overflow acceptance)");
	T.AssertEquals(HOLFactionPowerCast::RefusalOutOfRange(), BothAxes.Refusal, "both axes: the reason is the range");
	T.AssertEquals(Full.Value, BothAxes.NewGaugeValue, "and the gauge is untouched");
}
