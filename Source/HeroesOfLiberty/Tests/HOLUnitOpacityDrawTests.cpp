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

// The drawing pass reads the table ONCE per run (a snapshot, no lock per unit). The values are computed by the real chain: SetStealthOpacity ->
// TakeOpacitySnapshot -> MakeUnitInstanceCustomData(..., Snapshot).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLUnitDrawSnapshotInZoneTest, "HeroesOfLiberty.Presentation.UnitDrawOpacity.SnapshotInZone",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHOLUnitDrawSnapshotInZoneTest::RunTest(const FString& Parameters)
{
	UHOLUnitOpacityLibrary::ClearStealthOpacities();
	UHOLUnitOpacityLibrary::SetStealthOpacity(7, 0.5f);

	const HOLUnitDraw::FOpacitySnapshot Snapshot = HOLUnitDraw::TakeOpacitySnapshot();
	const HOLUnitDraw::FInstanceCustomData InZone = HOLUnitDraw::MakeUnitInstanceCustomData(FVector3f(0.2f, 0.4f, 0.6f), 2, 7, Snapshot);
	const HOLUnitDraw::FInstanceCustomData Outside = HOLUnitDraw::MakeUnitInstanceCustomData(FVector3f(0.2f, 0.4f, 0.6f), 2, 8, Snapshot);
	TestEqual(TEXT("a unit in a zone is drawn at half opacity through the snapshot"), InZone.Opacity, 0.5f);
	TestEqual(TEXT("a unit outside any zone is opaque through the snapshot"), Outside.Opacity, 1.f);
	TestEqual(TEXT("colour kept"), InZone.Color.Y, 0.4f);
	TestEqual(TEXT("prop kept"), InZone.Prop, 2.f);

	UHOLUnitOpacityLibrary::ClearStealthOpacities();
	return true;
}

// The normal case (no zone): the snapshot is empty and every unit is opaque.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLUnitDrawSnapshotEmptyTest, "HeroesOfLiberty.Presentation.UnitDrawOpacity.SnapshotEmpty",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHOLUnitDrawSnapshotEmptyTest::RunTest(const FString& Parameters)
{
	UHOLUnitOpacityLibrary::ClearStealthOpacities();

	const HOLUnitDraw::FOpacitySnapshot Snapshot = HOLUnitDraw::TakeOpacitySnapshot();
	TestTrue(TEXT("no zone: the snapshot is empty"), Snapshot.IsEmpty());
	const HOLUnitDraw::FInstanceCustomData Data = HOLUnitDraw::MakeUnitInstanceCustomData(FVector3f(1.f, 0.f, 0.f), 0, 7, Snapshot);
	TestEqual(TEXT("an empty table draws every unit opaque"), Data.Opacity, 1.f);
	return true;
}

// A change made during the pass (script writes between two units) never alters a snapshot already taken; the next snapshot sees it.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLUnitDrawSnapshotStableTest, "HeroesOfLiberty.Presentation.UnitDrawOpacity.SnapshotStable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHOLUnitDrawSnapshotStableTest::RunTest(const FString& Parameters)
{
	UHOLUnitOpacityLibrary::ClearStealthOpacities();
	UHOLUnitOpacityLibrary::SetStealthOpacity(7, 0.5f);

	const HOLUnitDraw::FOpacitySnapshot Taken = HOLUnitDraw::TakeOpacitySnapshot();
	UHOLUnitOpacityLibrary::SetStealthOpacity(7, 1.f);
	UHOLUnitOpacityLibrary::SetStealthOpacity(8, 0.25f);

	TestEqual(TEXT("unit 7 keeps 0.5 in the snapshot already taken"), HOLUnitDraw::MakeUnitInstanceCustomData(FVector3f::ZeroVector, 0, 7, Taken).Opacity, 0.5f);
	TestEqual(TEXT("unit 8 (set after) stays opaque in the snapshot already taken"), HOLUnitDraw::MakeUnitInstanceCustomData(FVector3f::ZeroVector, 0, 8, Taken).Opacity, 1.f);

	const HOLUnitDraw::FOpacitySnapshot Next = HOLUnitDraw::TakeOpacitySnapshot();
	TestEqual(TEXT("the next snapshot sees unit 7 opaque again"), HOLUnitDraw::MakeUnitInstanceCustomData(FVector3f::ZeroVector, 0, 7, Next).Opacity, 1.f);
	TestEqual(TEXT("the next snapshot sees unit 8 at 0.25"), HOLUnitDraw::MakeUnitInstanceCustomData(FVector3f::ZeroVector, 0, 8, Next).Opacity, 0.25f);

	UHOLUnitOpacityLibrary::ClearStealthOpacities();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
