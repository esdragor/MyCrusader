// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Presentation/HOLUnitOpacity.h"

#if WITH_DEV_AUTOMATION_TESTS

// D824 Q3: the custom data of a unit instance (what UHOLInterpolatedInstancesProcessor hands to Mass) carries the opacity the stealth rule set for
// that unit: 0.5 in a zone, 1 outside. The values asserted are computed by the real filling function (HOLUnitDraw::MakeUnitInstanceCustomData).

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLUnitDrawOpacityInZoneTest, "HeroesOfLiberty.Presentation.UnitDrawOpacity.InZone",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHOLUnitDrawOpacityInZoneTest::RunTest(const FString& Parameters)
{
	UHOLUnitOpacityLibrary::ClearStealthOpacities();
	UHOLUnitOpacityLibrary::SetStealthOpacity(7, 0.5f);

	const HOLUnitDraw::FInstanceCustomData InZone = HOLUnitDraw::MakeUnitInstanceCustomData(FVector3f(0.2f, 0.4f, 0.6f), 2, 7);
	TestEqual(TEXT("a unit in a zone is drawn at half opacity"), InZone.Opacity, 0.5f);
	// The player colour and the prop are untouched by the opacity (a shifted index would break the colours).
	TestEqual(TEXT("colour R kept"), InZone.Color.X, 0.2f);
	TestEqual(TEXT("colour G kept"), InZone.Color.Y, 0.4f);
	TestEqual(TEXT("colour B kept"), InZone.Color.Z, 0.6f);
	TestEqual(TEXT("prop kept"), InZone.Prop, 2.f);

	UHOLUnitOpacityLibrary::ClearStealthOpacities();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLUnitDrawOpacityOutsideTest, "HeroesOfLiberty.Presentation.UnitDrawOpacity.Outside",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHOLUnitDrawOpacityOutsideTest::RunTest(const FString& Parameters)
{
	UHOLUnitOpacityLibrary::ClearStealthOpacities();
	UHOLUnitOpacityLibrary::SetStealthOpacity(7, 0.5f);

	// Another unit, never in a zone: opaque. A unit that left the zone (set back to 1) is opaque again.
	const HOLUnitDraw::FInstanceCustomData Other = HOLUnitDraw::MakeUnitInstanceCustomData(FVector3f(1.f, 0.f, 0.f), 0, 8);
	TestEqual(TEXT("a unit outside any zone is opaque"), Other.Opacity, 1.f);
	UHOLUnitOpacityLibrary::SetStealthOpacity(7, 1.f);
	const HOLUnitDraw::FInstanceCustomData Left = HOLUnitDraw::MakeUnitInstanceCustomData(FVector3f(1.f, 0.f, 0.f), 0, 7);
	TestEqual(TEXT("a unit that left the zone is opaque again"), Left.Opacity, 1.f);

	UHOLUnitOpacityLibrary::ClearStealthOpacities();
	return true;
}

// The float layout Mass copies into its array: 0-2 colour, 3 prop, 4 opacity (the material reads custom data index 4 for the opacity).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLUnitDrawLayoutTest, "HeroesOfLiberty.Presentation.UnitDrawOpacity.Layout",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHOLUnitDrawLayoutTest::RunTest(const FString& Parameters)
{
	UHOLUnitOpacityLibrary::ClearStealthOpacities();
	UHOLUnitOpacityLibrary::SetStealthOpacity(21, 0.25f);

	const HOLUnitDraw::FInstanceCustomData Data = HOLUnitDraw::MakeUnitInstanceCustomData(FVector3f(0.1f, 0.2f, 0.3f), 3, 21);
	TestEqual(TEXT("five floats per instance"), static_cast<int32>(sizeof(HOLUnitDraw::FInstanceCustomData) / sizeof(float)), HOLUnitDraw::NumInstanceCustomFloats);
	float Floats[HOLUnitDraw::NumInstanceCustomFloats] = {};
	FMemory::Memcpy(Floats, &Data, sizeof(Data));
	TestEqual(TEXT("float 0 is the colour R"), Floats[0], 0.1f);
	TestEqual(TEXT("float 1 is the colour G"), Floats[1], 0.2f);
	TestEqual(TEXT("float 2 is the colour B"), Floats[2], 0.3f);
	TestEqual(TEXT("float 3 is the prop"), Floats[3], 3.f);
	TestEqual(TEXT("float 4 is the opacity"), Floats[4], 0.25f);

	UHOLUnitOpacityLibrary::ClearStealthOpacities();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
