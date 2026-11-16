// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLGarrisonRuleLibrary.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLGarrisonBoard.h"

#if WITH_DEV_AUTOMATION_TESTS

// F7, bascule Garrison (D830/D836 point 4, AngelscriptInProgress.md): the server asks the script's rule through UHOLGarrisonBoard
// (Script/Garrison/HOLGarrisonBoardScript.as -> HOLGarrison::Admitted of HOLGarrisonRules.as). The C++ rule stays until the proof, so the
// test also checks that the board and the C++ rule give the same list.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGarrisonServerBoardTest, "HeroesOfLiberty.Garrison.ServerBoard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHOLGarrisonServerBoardTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UHOLGarrisonBoard* Board = UHOLGarrisonBoard::Find(World);
	if (TestNotNull(TEXT("the script board of the garrison"), Board))
	{
		// Five candidates of slot 0, 0, 1, 0, 0: melee, siege (3), melee of an ally's slot, ranged (1), hero (4).
		const TArray<int32> UnitSlots = {0, 0, 1, 0, 0};
		const TArray<int32> UnitKinds = {0, 3, 0, 1, 4};
		const TArray<int32> TwoPlaces = Board->AdmittedNow(UnitSlots, UnitKinds, 0, 2, 0);
		TestTrue(TEXT("Two places: the melee and the shooter; siege and a foreign slot are refused, a refused one takes no place"), TwoPlaces == TArray<int32>({0, 3}));
		TestTrue(TEXT("One free place left (2 of 3 inside): only the first admitted"), Board->AdmittedNow(UnitSlots, UnitKinds, 0, 3, 2) == TArray<int32>({0}));
		TestEqual(TEXT("A building of nobody (slot -1) admits nobody"), Board->AdmittedNow(UnitSlots, UnitKinds, INDEX_NONE, 5, 0).Num(), 0);
		TestEqual(TEXT("A kind outside 0..4 never enters"), Board->AdmittedNow({0}, {9}, 0, 5, 0).Num(), 0);
		TestTrue(TEXT("The board and the C++ rule give the same list"), TwoPlaces == UHOLGarrisonRuleLibrary::AdmittedBySettings(UnitSlots, UnitKinds, 0, 2, 0));

		// The path of the server's order (UHOLGarrisonSubsystem): kinds as HOLGarrison::EKind.
		const TArray<HOLGarrison::EKind> Kinds = {HOLGarrison::EKind::MeleeInfantry, HOLGarrison::EKind::Siege, HOLGarrison::EKind::MeleeInfantry,
			HOLGarrison::EKind::RangedInfantry, HOLGarrison::EKind::Hero};
		TestTrue(TEXT("AdmittedForOrder goes through the board"), UHOLGarrisonBoard::AdmittedForOrder(World, UnitSlots, Kinds, 0, 2, 0) == TArray<int32>({0, 3}));
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
