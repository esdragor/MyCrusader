// Copyright Epic Games, Inc. All Rights Reserved.

#include "HUD/HOLMinimapView.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// Performance review (PerformanceInProgress.md, P2): the minimap's terrain layer (480 x 480 texels, the fog blurred then read texel
// by texel) is made again only when what it shows changed. The sources it draws are the standing ones: the signature that tells
// must change when one is used up or added, and not when one is only gathered from (an amount going down is no change of picture).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMinimapTerrainSourcesTest, "HeroesOfLiberty.UI.Minimap.TerrainSources",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLMinimapTerrainSourcesTest::RunTest(const FString& Parameters)
{
	const auto Make = [](const int32 Remaining, const FVector& Location)
	{
		FHOLSource Source;
		Source.TypeIndex = 0;
		Source.Resource = EHOLResource::Wood;
		Source.Location = Location;
		Source.Remaining = Remaining;
		return Source;
	};
	TArray<FHOLSource> Sources{ Make(100, FVector(0, 0, 0)), Make(100, FVector(500, 0, 0)), Make(100, FVector(0, 500, 0)) };
	const uint32 Whole = HOLMinimapTerrain::GetSourcesSignature(Sources);
	TestEqual(TEXT("read twice: the same"), HOLMinimapTerrain::GetSourcesSignature(Sources), Whole);

	Sources[1].Remaining = 40;
	TestEqual(TEXT("gathered from, still standing: no change"), HOLMinimapTerrain::GetSourcesSignature(Sources), Whole);

	Sources[1].Remaining = 0;
	const uint32 UsedUp = HOLMinimapTerrain::GetSourcesSignature(Sources);
	TestNotEqual(TEXT("used up: changed"), UsedUp, Whole);

	Sources[2].Remaining = 0;
	TestNotEqual(TEXT("another used up: changed again"), HOLMinimapTerrain::GetSourcesSignature(Sources), UsedUp);

	Sources[1].Remaining = 100;
	Sources[2].Remaining = 100;
	TestEqual(TEXT("standing again (a farm field): back to the whole"), HOLMinimapTerrain::GetSourcesSignature(Sources), Whole);

	Sources.Add(Make(100, FVector(900, 900, 0)));
	TestNotEqual(TEXT("a source added: changed"), HOLMinimapTerrain::GetSourcesSignature(Sources), Whole);

	TArray<FHOLSource> Moved{ Make(100, FVector(0, 0, 0)), Make(100, FVector(500, 0, 0)), Make(100, FVector(0, 800, 0)) };
	TestNotEqual(TEXT("a source elsewhere: changed"), HOLMinimapTerrain::GetSourcesSignature(Moved), Whole);

	TestEqual(TEXT("no source: the empty signature, stable"), HOLMinimapTerrain::GetSourcesSignature(TConstArrayView<FHOLSource>()),
		HOLMinimapTerrain::GetSourcesSignature(TConstArrayView<FHOLSource>()));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
