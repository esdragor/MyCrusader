// Copyright Epic Games, Inc. All Rights Reserved.

#include "HUD/HOLMinimapView.h"
#include "HAL/IConsoleManager.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// MM-Q1 / D624: the minimap alert is a circle that shrinks from its size (24 px) to 8 px, again and again. The diameter is a function
// of the alert's age on the GAME's clock (not the wall clock: a sequence of pictures taken on the simulation's step must show it
// changing, and a paused game must not animate it).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMinimapAlertRingTest, "HeroesOfLiberty.UI.Minimap.AlertRing",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLMinimapAlertRingTest::RunTest(const FString& Parameters)
{
	TestNearlyEqual(TEXT("age 0: the full size"), UHOLMinimapView::AlertDiameter(0.f, 1.f, 24.f), 24.f);
	TestNearlyEqual(TEXT("half a period: halfway"), UHOLMinimapView::AlertDiameter(0.5f, 1.f, 24.f), 16.f);
	TestNearlyEqual(TEXT("just before the period: nearly 8 px"), UHOLMinimapView::AlertDiameter(0.999f, 1.f, 24.f), 8.f, 0.05f);
	TestNearlyEqual(TEXT("a period later: again the full size"), UHOLMinimapView::AlertDiameter(3.f, 1.f, 24.f), 24.f);
	TestNearlyEqual(TEXT("a 6 s period, 3 s: halfway"), UHOLMinimapView::AlertDiameter(3.f, 6.f, 24.f), 16.f);
	// Sane against the console's values: a period of 0 would divide by zero; a size under 8 px never grows.
	TestTrue(TEXT("period 0 stays a number"), FMath::IsFinite(UHOLMinimapView::AlertDiameter(1.f, 0.f, 24.f)));
	TestTrue(TEXT("a size under 8 px does not grow past itself"), UHOLMinimapView::AlertDiameter(0.3f, 1.f, 6.f) <= 6.f);
	TestTrue(TEXT("a negative age is the start"), UHOLMinimapView::AlertDiameter(-1.f, 1.f, 24.f) <= 24.f);
	// The user's choice (Q-minimap-alerts-vitesse, 2026-11-02): 24 px, one pass in 0.75 s, an alert lives 6 s, no blink.
	const auto Setting = [](const TCHAR* Name) { const IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(Name); return Variable ? Variable->GetFloat() : -1.f; };
	TestNearlyEqual(TEXT("default pass of the shrinking"), Setting(TEXT("hol.Minimap.AlertShrinkSeconds")), 0.75f);
	TestNearlyEqual(TEXT("default size"), Setting(TEXT("hol.Minimap.AlertSize")), 24.f);
	TestNearlyEqual(TEXT("default life"), Setting(TEXT("hol.Minimap.AlertSeconds")), 6.f);
	// Shrinking, strictly, over a period (the picture of a ring that moves).
	float Previous = 1000.f;
	for (int32 Step = 0; Step < 4; ++Step)
	{
		const float Diameter = UHOLMinimapView::AlertDiameter(Step * 0.25f, 1.f, 24.f);
		TestTrue(FString::Printf(TEXT("t = %.2f s is smaller than the one before"), Step * 0.25f), Diameter < Previous);
		Previous = Diameter;
	}
	return true;
}

#endif
