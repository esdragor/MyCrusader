// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLGameState.h"
#include "Match/HOLSignalSubsystem.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// SG2 (SignalsInProgress.md; GDD 15, D386, D784, answer A of Q-signals-spectators): the server's signal board is the script's (the real
// chain: C++ bridge -> Script/Signals HOLSignal::Post and Sees). Four seats in two teams (0 and 1, 2 and 3):
// - slot 0 places an Attack signal: it is on the board; its teammate 1 sees it, the enemies 2 and 3 do not, a spectator does;
// - the defeated teammate 1 places one: silenced, the board is unchanged (D784);
// - slot 0 places two more, then a fourth within 5 s: refused (3 every 5 s, D386), still three on the board;
// - 5 s later the faded ones are gone: slot 2's new signal is alone on the board, seen by 3, not by 0.
namespace UE::HeroesOfLiberty::Tests::SignalBoard
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSignalBoardServerTest, "HeroesOfLiberty.Signals.ServerBoard", UE::HeroesOfLiberty::Tests::SignalBoard::Flags)

bool FHOLSignalBoardServerTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
	World->SetGameState(GameState);

	UHOLSignalSubsystem* Board = UHOLSignalSubsystem::Find(World);
	if (TestNotNull(TEXT("the script board of signals"), Board))
	{
		FHOLSignalBookInput Book;
		Book.Slots = 4;
		Book.bLocked = true;
		const int32 Team[] = {1, 1, 2, 2};
		for (int32 From = 0; From < 4; ++From)
		{
			Book.IsAI.Add(false);
			for (int32 To = 0; To < 4; ++To)
			{
				Book.Stances.Add(Team[From] == Team[To] ? 2 : 0);
			}
		}
		const FVector Place(1200.0, -300.0, 0.0);
		constexpr int32 Attack = 1;
		constexpr int32 Ok = 0;
		constexpr int32 Silenced = 3;
		constexpr int32 TooMany = 4;

		TestEqual(TEXT("Slot 0's signal is taken"), Board->PostNow(Book, 0, false, Attack, Place, 1000), Ok);
		TestEqual(TEXT("It is on the board"), Board->GetNumSignalsNow(), 1);
		TestTrue(TEXT("Its teammate 1 sees it"), Board->SeesNow(Book, 0, 1, false));
		TestFalse(TEXT("The enemy 2 does not"), Board->SeesNow(Book, 0, 2, false));
		TestFalse(TEXT("The enemy 3 does not"), Board->SeesNow(Book, 0, 3, false));
		TestTrue(TEXT("A spectator (no seat) sees it"), Board->SeesNow(Book, 0, -1, true));

		TestEqual(TEXT("The defeated teammate places none (D784)"), Board->PostNow(Book, 1, true, Attack, Place, 1050), Silenced);
		TestEqual(TEXT("The board is unchanged"), Board->GetNumSignalsNow(), 1);

		TestEqual(TEXT("Second signal"), Board->PostNow(Book, 0, false, Attack, Place, 1100), Ok);
		TestEqual(TEXT("Third signal"), Board->PostNow(Book, 0, false, Attack, Place, 1200), Ok);
		TestEqual(TEXT("A fourth within 5 s is refused (D386)"), Board->PostNow(Book, 0, false, Attack, Place, 1300), TooMany);
		TestEqual(TEXT("Three on the board"), Board->GetNumSignalsNow(), 3);

		TestEqual(TEXT("Slot 2's signal after the others faded"), Board->PostNow(Book, 2, false, Attack, Place, 6300), Ok);
		TestEqual(TEXT("It is alone on the board"), Board->GetNumSignalsNow(), 1);
		TestTrue(TEXT("Its teammate 3 sees it"), Board->SeesNow(Book, 2, 3, false));
		TestFalse(TEXT("The enemy 0 does not"), Board->SeesNow(Book, 2, 0, false));
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
