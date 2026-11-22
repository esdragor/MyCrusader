// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Presentation/HOLUnitOpacity.h"

#if WITH_DEV_AUTOMATION_TESTS

// ARWEN-1122g (REVUE-1122 defect 1): the opacity table is a process static; a value set in an old game world must not be read in a new one (unit ids
// restart at 1 per world), and a second world starting while the first lives (two PIE worlds) must not wipe the first one's values. The values read
// are computed by the real chain: real game worlds (UWorld::CreateWorld creates their subsystems) and UHOLUnitOpacityLibrary::GetStealthOpacity.

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLUnitOpacityNewWorldForgetsOldTest, "HeroesOfLiberty.Presentation.UnitDrawOpacity.NewWorldForgetsOld",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHOLUnitOpacityNewWorldForgetsOldTest::RunTest(const FString& Parameters)
{
	if (UHOLStealthOpacityResetSubsystem::GetLiveGameWorlds() != 0)
	{
		AddWarning(TEXT("a game world is already alive (PIE running): the test needs none, skipped"));
		return true;
	}
	UHOLUnitOpacityLibrary::ClearStealthOpacities();

	UWorld* WorldA = UWorld::CreateWorld(EWorldType::Game, false, TEXT("HOLOpacityResetWorldA"));
	if (!TestNotNull(TEXT("world A created"), WorldA))
	{
		return false;
	}
	TestEqual(TEXT("world A is counted"), UHOLStealthOpacityResetSubsystem::GetLiveGameWorlds(), 1);
	UHOLUnitOpacityLibrary::SetStealthOpacity(7, 0.5f);
	TestEqual(TEXT("the opacity set in world A is read"), UHOLUnitOpacityLibrary::GetStealthOpacity(7), 0.5f);

	// A second world starts while A lives (two PIE worlds): A's value must survive.
	UWorld* WorldB = UWorld::CreateWorld(EWorldType::Game, false, TEXT("HOLOpacityResetWorldB"));
	if (!TestNotNull(TEXT("world B created"), WorldB))
	{
		WorldA->DestroyWorld(false);
		return false;
	}
	TestEqual(TEXT("a second live world does not wipe the first one's opacity"), UHOLUnitOpacityLibrary::GetStealthOpacity(7), 0.5f);

	// A ends while B lives: still kept. B ends too: the match is over, nothing is kept.
	WorldA->DestroyWorld(false);
	TestEqual(TEXT("one world ending while another lives keeps the opacity"), UHOLUnitOpacityLibrary::GetStealthOpacity(7), 0.5f);
	WorldB->DestroyWorld(false);

	// The next match: a new world sees no opacity left by the old ones.
	UWorld* WorldC = UWorld::CreateWorld(EWorldType::Game, false, TEXT("HOLOpacityResetWorldC"));
	if (!TestNotNull(TEXT("world C created"), WorldC))
	{
		return false;
	}
	TestEqual(TEXT("a new world does not read an opacity set in an old one"), UHOLUnitOpacityLibrary::GetStealthOpacity(7), 1.f);
	WorldC->DestroyWorld(false);
	TestEqual(TEXT("no world left"), UHOLStealthOpacityResetSubsystem::GetLiveGameWorlds(), 0);

	UHOLUnitOpacityLibrary::ClearStealthOpacities();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
