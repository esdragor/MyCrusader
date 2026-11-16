// Lot L17-07 of the migration (D830, D836, D839, SAURON-1117b): the script port of HOLRelayRingTests.cpp (1 test), run against the read library of
// the rule that STAYS in C++ (UHOLRelayReadLibrary, S1f). Same vectors as the C++ test. The C++ test is kept until the registry says otherwise.
// Not ported: the FHOLRelayView of the C++ test (the replicated struct, filled by HOLRelay::MakeView); the library returns its read twin
// FHOLRelayReadView, so the checks below read the same fields (X, Y, Radius, Owner, Taker, FillPercent, Description) on the twin.

// HeroesOfLiberty.Relays.Ring (R5): the share of the capture held (FillPercent) and the view built from a relay's state (MakeView).
void Test_RelayRing(FUnitTest& T)
{
	T.AssertEquals(0, HOLRelayRead::FillPercent(-1, -1, 0, 600), "nothing held: 0");
	T.AssertEquals(25, HOLRelayRead::FillPercent(-1, -1, 150, 600), "a quarter: 25");
	T.AssertEquals(99, HOLRelayRead::FillPercent(-1, -1, 599, 600), "almost taken: 99, never 100 (the ring is full only by the change of owner)");
	T.AssertEquals(99, HOLRelayRead::FillPercent(-1, -1, 900, 600), "past the count: 99");
	T.AssertEquals(0, HOLRelayRead::FillPercent(-1, -1, 900, 0), "no hold steps: 0, no division by 0");

	FHOLRelayReadView View = HOLRelayRead::MakeView(FVector2D(1500.0, -2500.0), 1200.0f, 2, 1, 300, 600);
	T.AssertEquals(1500.0, View.X, "view: X");
	T.AssertEquals(-2500.0, View.Y, "view: Y");
	T.AssertEquals(1200.0, View.Radius, "view: radius");
	T.AssertEquals(2, View.Owner, "view: owner");
	T.AssertEquals(1, View.Taker, "view: taker");
	T.AssertEquals(50, View.FillPercent, "view: half filled");
	T.AssertTrue(View.Description == "(1500, -2500) r1200 owner 2 taker 1 fill 50", "view: the line the markers and the report read");
	FHOLRelayReadView Nobody = HOLRelayRead::MakeView(FVector2D(0.0, 0.0), 1200.0f, -1, -1, 0, 600);
	T.AssertTrue(Nobody.Description == "(0, 0) r1200 owner -1 taker -1 fill 0", "view of nobody");
}