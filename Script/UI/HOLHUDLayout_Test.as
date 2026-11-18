// HOLHUDLayoutTests.cpp ported to script (E3-08, socle step 1, D830, D836, D839): the rule stays in C++ (HOLHUDLayoutRules.h, read by
// HOLHUDLayoutReadLibrary, namespace HOLHUDLayoutRead), the vectors are those of the C++ tests. The C++ test stays until E3-FIN.
// FBox2D became two FVector2D (Min, Max); the two bool& of ShouldFoldCheats became the FHOLHUDFoldState given back each tick.

// Lesson 12b: the check that finds a panel hiding another. The AI level line (top left, 72 under the top) over the cheats' panel
// (16, 62 to 400) overlaps; a panel put under it, or beside it, does not; edges that only touch do not count.
// C++: HeroesOfLiberty.UI.Layout.Overlap.
void Test_HUDLayoutOverlap(FUnitTest& T)
{
	const FVector2D CheatsMin(16.0, 62.0);
	const FVector2D CheatsMax(220.0, 400.0);
	const float Tolerance = 1.0; // the rule's default
	T.AssertTrue(HOLHUDLayoutRead::Overlaps(FVector2D(16.0, 72.0), FVector2D(300.0, 100.0), CheatsMin, CheatsMax, Tolerance), "a line over the panel overlaps it");
	T.AssertTrue(HOLHUDLayoutRead::Overlaps(CheatsMin, CheatsMax, FVector2D(16.0, 72.0), FVector2D(300.0, 100.0), Tolerance), "whatever the order");
	T.AssertFalse(HOLHUDLayoutRead::Overlaps(FVector2D(16.0, 406.0), FVector2D(300.0, 434.0), CheatsMin, CheatsMax, Tolerance), "under it: no");
	T.AssertFalse(HOLHUDLayoutRead::Overlaps(FVector2D(230.0, 72.0), FVector2D(500.0, 100.0), CheatsMin, CheatsMax, Tolerance), "beside it: no");
	T.AssertFalse(HOLHUDLayoutRead::Overlaps(FVector2D(16.0, 400.0), FVector2D(300.0, 428.0), CheatsMin, CheatsMax, Tolerance), "touching edges: no");
	T.AssertFalse(HOLHUDLayoutRead::Overlaps(FVector2D(16.0, 399.5), FVector2D(300.0, 428.0), CheatsMin, CheatsMax, Tolerance), "a hairline under the tolerance: no");
	T.AssertTrue(HOLHUDLayoutRead::Overlaps(FVector2D(20.0, 70.0), FVector2D(40.0, 90.0), CheatsMin, CheatsMax, Tolerance), "a box inside another overlaps it");
}

// Q-heros-triches-600 = A: under 700 px of height the cheats' panel starts folded and folds when the hero's block opens (or the window
// becomes low with the panel open); a click that opens it afterwards stays; a tall window changes nothing.
// C++: HeroesOfLiberty.UI.Layout.CheatsLowWindow.
void Test_HUDLayoutCheatsLowWindow(FUnitTest& T)
{
	T.AssertTrue(HOLHUDLayoutRead::IsLowWindow(600.0f), "600 px is low");
	T.AssertFalse(HOLHUDLayoutRead::IsLowWindow(700.0f), "700 px is not");
	T.AssertFalse(HOLHUDLayoutRead::IsLowWindow(1080.0f), "1080 px is not");
	T.AssertFalse(HOLHUDLayoutRead::IsLowWindow(0.0f), "a size not known yet is not low");
	T.AssertFalse(HOLHUDLayoutRead::CheatsStartOpen(true, 600.0f), "low: starts folded whatever was kept");
	T.AssertTrue(HOLHUDLayoutRead::CheatsStartOpen(true, 1080.0f), "tall: starts as kept (open)");
	T.AssertFalse(HOLHUDLayoutRead::CheatsStartOpen(false, 1080.0f), "tall: starts as kept (folded)");

	FHOLHUDFoldState State;
	State = HOLHUDLayoutRead::ShouldFoldCheats(600.0f, false, true, State);
	T.AssertFalse(State.bFold, "low, hero block not there yet: nothing");
	State = HOLHUDLayoutRead::ShouldFoldCheats(600.0f, true, true, State);
	T.AssertTrue(State.bFold, "the hero block opens over an open panel: fold");
	State = HOLHUDLayoutRead::ShouldFoldCheats(600.0f, true, true, State);
	T.AssertFalse(State.bFold, "once: a click that opens it again stays");
	State = HOLHUDLayoutRead::ShouldFoldCheats(600.0f, true, false, State);
	T.AssertFalse(State.bFold, "already folded: nothing");

	FHOLHUDFoldState Tall;
	Tall = HOLHUDLayoutRead::ShouldFoldCheats(1080.0f, true, true, Tall);
	T.AssertFalse(Tall.bFold, "tall window, hero block shown: nothing");
	Tall = HOLHUDLayoutRead::ShouldFoldCheats(600.0f, true, true, Tall);
	T.AssertTrue(Tall.bFold, "the window becomes low with the panel open and the block there: fold");
}