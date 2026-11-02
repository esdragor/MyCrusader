#include "HUD/HOLHUDLayoutRules.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// Lesson 12b: the check that finds a panel hiding another. The AI level line (top left, 72 under the top) over the cheats' panel
// (16, 62 to 400) overlaps; a panel put under it, or beside it, does not; edges that only touch do not count.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHUDLayoutOverlapTest, "HeroesOfLiberty.UI.Layout.Overlap",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLHUDLayoutOverlapTest::RunTest(const FString& Parameters)
{
	const FBox2D Cheats(FVector2D(16.0, 62.0), FVector2D(220.0, 400.0));
	const FBox2D LineOver(FVector2D(16.0, 72.0), FVector2D(300.0, 100.0));
	const FBox2D LineUnder(FVector2D(16.0, 406.0), FVector2D(300.0, 434.0));
	const FBox2D LineBeside(FVector2D(230.0, 72.0), FVector2D(500.0, 100.0));
	const FBox2D LineTouching(FVector2D(16.0, 400.0), FVector2D(300.0, 428.0));
	const FBox2D LineHairline(FVector2D(16.0, 399.5), FVector2D(300.0, 428.0));
	TestTrue(TEXT("a line over the panel overlaps it"), HOLHUDLayout::Overlaps(LineOver, Cheats));
	TestTrue(TEXT("whatever the order"), HOLHUDLayout::Overlaps(Cheats, LineOver));
	TestFalse(TEXT("under it: no"), HOLHUDLayout::Overlaps(LineUnder, Cheats));
	TestFalse(TEXT("beside it: no"), HOLHUDLayout::Overlaps(LineBeside, Cheats));
	TestFalse(TEXT("touching edges: no"), HOLHUDLayout::Overlaps(LineTouching, Cheats));
	TestFalse(TEXT("a hairline under the tolerance: no"), HOLHUDLayout::Overlaps(LineHairline, Cheats));
	TestTrue(TEXT("a box inside another overlaps it"), HOLHUDLayout::Overlaps(FBox2D(FVector2D(20.0, 70.0), FVector2D(40.0, 90.0)), Cheats));
	return true;
}

// Q-heros-triches-600 = A: under 700 px of height the cheats' panel starts folded and folds when the hero's block opens (or the window
// becomes low with the panel open); a click that opens it afterwards stays; a tall window changes nothing.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHUDLayoutLowWindowTest, "HeroesOfLiberty.UI.Layout.CheatsLowWindow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLHUDLayoutLowWindowTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("600 px is low"), HOLHUDLayout::IsLowWindow(600.f));
	TestFalse(TEXT("700 px is not"), HOLHUDLayout::IsLowWindow(700.f));
	TestFalse(TEXT("1080 px is not"), HOLHUDLayout::IsLowWindow(1080.f));
	TestFalse(TEXT("a size not known yet is not low"), HOLHUDLayout::IsLowWindow(0.f));
	TestFalse(TEXT("low: starts folded whatever was kept"), HOLHUDLayout::CheatsStartOpen(true, 600.f));
	TestTrue(TEXT("tall: starts as kept (open)"), HOLHUDLayout::CheatsStartOpen(true, 1080.f));
	TestFalse(TEXT("tall: starts as kept (folded)"), HOLHUDLayout::CheatsStartOpen(false, 1080.f));

	bool bWasLow = false;
	bool bWasShown = false;
	TestFalse(TEXT("low, hero block not there yet: nothing"), HOLHUDLayout::ShouldFoldCheats(600.f, false, true, bWasLow, bWasShown));
	TestTrue(TEXT("the hero block opens over an open panel: fold"), HOLHUDLayout::ShouldFoldCheats(600.f, true, true, bWasLow, bWasShown));
	TestFalse(TEXT("once: a click that opens it again stays"), HOLHUDLayout::ShouldFoldCheats(600.f, true, true, bWasLow, bWasShown));
	TestFalse(TEXT("already folded: nothing"), HOLHUDLayout::ShouldFoldCheats(600.f, true, false, bWasLow, bWasShown));

	bool bTallLow = false;
	bool bTallShown = false;
	TestFalse(TEXT("tall window, hero block shown: nothing"), HOLHUDLayout::ShouldFoldCheats(1080.f, true, true, bTallLow, bTallShown));
	TestTrue(TEXT("the window becomes low with the panel open and the block there: fold"), HOLHUDLayout::ShouldFoldCheats(600.f, true, true, bTallLow, bTallShown));
	return true;
}

#endif
