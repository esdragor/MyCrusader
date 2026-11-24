// FARAMIR-1124a (D900, answer C of Q-lumiere-sacree-visee): an aimed Sacred Light beyond the range is not refused, the hero walks up to the range, then casts.
// First step: the pure rule HOLFactionPowerServer::ApproachPointOf (Script/Factions/HOLFactionPowerServer.as). The stop is 50 cm short of the range, measured
// from the aimed point toward the hero (range 1200 cm, hero at 0, point at 2000: the stop is at 2000 - 1150 = 850). The value asserted is COMPUTED by the rule.

// Beyond the range: the hero must walk, and he stops on the line to the point, 50 cm inside the range.
void Test_FactionApproachBeyondRangeWalksToRangeMinusMargin(FUnitTest& T)
{
	const FHOLFactionApproach Walk = HOLFactionPowerServer::ApproachPointOf(FVector(0.0, 0.0, 0.0), FVector(2000.0, 0.0, 0.0), 1200.0);
	T.AssertTrue(Walk.bNeedsWalk, "a point at 2000 cm with a range of 1200 cm needs a walk");
	T.AssertTrue(Math::Abs(Walk.StopPoint.X - 850.0) < 1.0, "the hero stops at 850 cm: 1150 cm (range - 50) short of the point");
	T.AssertTrue(Math::Abs(Walk.StopPoint.Y) < 1.0, "the stop stays on the line to the point");
}

// Inside the range: no walk, the cast is immediate (the stop is the hero's own place).
void Test_FactionApproachInRangeNeedsNoWalk(FUnitTest& T)
{
	const FHOLFactionApproach Stay = HOLFactionPowerServer::ApproachPointOf(FVector(0.0, 0.0, 0.0), FVector(900.0, 0.0, 0.0), 1200.0);
	T.AssertTrue(!Stay.bNeedsWalk, "a point at 900 cm is inside the range of 1200 cm");
	T.AssertTrue(Math::Abs(Stay.StopPoint.X) < 1.0 && Math::Abs(Stay.StopPoint.Y) < 1.0, "no walk: the stop is where the hero stands");
}

// The rule is a distance, not an axis: a diagonal point is reached on the same line.
void Test_FactionApproachDiagonalStopsOnTheLine(FUnitTest& T)
{
	const FVector Hero = FVector(100.0, 100.0, 0.0);
	const FVector Point = FVector(100.0 + 1200.0, 100.0 + 1600.0, 0.0);
	const FHOLFactionApproach Walk = HOLFactionPowerServer::ApproachPointOf(Hero, Point, 1200.0);
	T.AssertTrue(Walk.bNeedsWalk, "2000 cm away on a diagonal needs a walk");
	const double LeftToPoint = Math::Sqrt(Math::Square(Point.X - Walk.StopPoint.X) + Math::Square(Point.Y - Walk.StopPoint.Y));
	T.AssertTrue(Math::Abs(LeftToPoint - 1150.0) < 1.0, "the stop is 1150 cm from the point");
	T.AssertTrue(Math::Abs((Walk.StopPoint.X - Hero.X) * 4.0 - (Walk.StopPoint.Y - Hero.Y) * 3.0) < 1.0, "the stop is on the hero-to-point line (3:4)");
}
