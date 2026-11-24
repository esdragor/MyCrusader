// GLORFINDEL-1124b (D839, two bindings exposed to the script): UHOLHeroLibrary::GetHeroUnitId (D900: the cast's walk order names the hero by his unit id)
// and UHOLSpecializationReadLibrary::GetChosenOptionForSlot (D893: the seat's applied option, read server side). Both go through the real chains:
// the unit table's hero (SpawnTestHero) and the seat's AHOLPlayerState in the game state's PlayerArray (SetSpecialization).
#include "Engine/World.h"
#include "Match/HOLGameState.h"
#include "Match/HOLPlayerState.h"
#include "Match/HOLSpecializationReadLibrary.h"
#include "Match/HOLSpecializationView.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLHeroLibrary.h"
#include "Tests/HOLTestWorldLibrary.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroUnitIdTest, "HeroesOfLiberty.Hero.GetHeroUnitId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLHeroUnitIdTest::RunTest(const FString& Parameters)
{
	UHOLTestWorldHandle* Handle = UHOLTestWorldLibrary::CreateTestWorld();
	if (TestNotNull(TEXT("the test world"), Handle))
	{
		UWorld* World = Handle->GetTestWorld();
		TestEqual(TEXT("no hero born yet: -1"), UHOLHeroLibrary::GetHeroUnitId(World, 0), -1);
		const int32 HeroUnit = UHOLTestWorldLibrary::SpawnTestHero(Handle, 0, FVector(1000.0, 2000.0, 0.0));
		if (TestTrue(TEXT("a hero unit was born"), HeroUnit > 0))
		{
			TestEqual(TEXT("the slot's hero is named by the id the unit table gave him"), UHOLHeroLibrary::GetHeroUnitId(World, 0), HeroUnit);
			TestEqual(TEXT("another slot has no hero: -1"), UHOLHeroLibrary::GetHeroUnitId(World, 1), -1);
		}
		TestEqual(TEXT("a slot out of range: -1"), UHOLHeroLibrary::GetHeroUnitId(World, -1), -1);
		TestEqual(TEXT("no world: -1"), UHOLHeroLibrary::GetHeroUnitId(nullptr, 0), -1);
		UHOLTestWorldLibrary::DestroyTestWorld(Handle);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSeatChosenOptionTest, "HeroesOfLiberty.Specialization.GetChosenOptionForSlot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLSeatChosenOptionTest::RunTest(const FString& Parameters)
{
	UHOLTestWorldHandle* Handle = UHOLTestWorldLibrary::CreateTestWorld();
	if (TestNotNull(TEXT("the test world"), Handle))
	{
		UWorld* World = Handle->GetTestWorld();
		AHOLGameState* State = UHOLTestWorldLibrary::SpawnGameState(Handle, 0, 0);
		AHOLPlayerState* Seat = UHOLTestWorldLibrary::SpawnPlayerState(Handle, 2, 0, TEXT("Seat2"));
		if (TestNotNull(TEXT("the game state"), State) && TestNotNull(TEXT("the seat's player state"), Seat))
		{
			FHOLSpecializationView View;
			View.Chosen = { -1, 2, -1, 0 };
			TestTrue(TEXT("the server side writes the seat's view"), Seat->SetSpecialization(View));
			TestEqual(TEXT("tier 1: the applied option, read by slot"), UHOLSpecializationReadLibrary::GetChosenOptionForSlot(World, 2, 1), 2);
			TestEqual(TEXT("tier 2: no option yet"), UHOLSpecializationReadLibrary::GetChosenOptionForSlot(World, 2, 2), -1);
			TestEqual(TEXT("tier 3: option 0 is an option"), UHOLSpecializationReadLibrary::GetChosenOptionForSlot(World, 2, 3), 0);
			TestEqual(TEXT("a tier out of range: -1"), UHOLSpecializationReadLibrary::GetChosenOptionForSlot(World, 2, 9), -1);
			TestEqual(TEXT("a slot without a player state: -1"), UHOLSpecializationReadLibrary::GetChosenOptionForSlot(World, 5, 1), -1);
		}
		TestEqual(TEXT("no world: -1"), UHOLSpecializationReadLibrary::GetChosenOptionForSlot(nullptr, 2, 1), -1);
		UHOLTestWorldLibrary::DestroyTestWorld(Handle);
	}
	return true;
}

#endif
