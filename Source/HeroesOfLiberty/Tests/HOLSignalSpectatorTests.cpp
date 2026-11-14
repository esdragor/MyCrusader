// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLSignalSubsystem.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// SG (SignalsInProgress.md; D376, D784, answer A of Q-signals-spectators): who watches the signals, read on the real game state
// (UHOLSignalSubsystem::IsSignalSpectator, the stances by ReadBook through UHOLDiplomacyReadLibrary), what the two-client net test
// does not reach. Four human seats of a Standard match: 0 and 1 in team 1, 2 in team 2, 3 in team 3.
// - ReadBook reads the teams: 4 seats, 0 and 1 allied, 0 and 2 not, locked (no Diplomacy mode);
// - slot 2 places a signal, slot 0 another;
// - slot 1 defeated, its ally 0 alive: not a spectator, sees 0's signal, not the enemy 2's (D784);
// - slot 3 defeated, no ally: a spectator, sees the enemy 2's signal (answer A);
// - a pure spectator (no seat, viewer -1): sees it;
// - slot 0 alive, no link with 2: not a spectator, does not see 2's signal.
namespace UE::HeroesOfLiberty::Tests::SignalSpectator
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSignalSpectatorTest, "HeroesOfLiberty.Signals.DefeatedSpectator", UE::HeroesOfLiberty::Tests::SignalSpectator::Flags)

bool FHOLSignalSpectatorTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
	World->SetGameState(GameState);

	UHOLSignalSubsystem* Board = UHOLSignalSubsystem::Find(World);
	if (TestNotNull(TEXT("game state"), GameState) && TestNotNull(TEXT("the script board of signals"), Board))
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		const int32 Team[] = {1, 1, 2, 3};
		for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
		{
			Config.Slots[Index].State = Index < 4 ? EHOLSlotState::Human : EHOLSlotState::Closed;
			Config.Slots[Index].Team = Index < 4 ? Team[Index] : 0;
		}
		GameState->InitVictory(Config);

		const FHOLSignalBookInput Book = UHOLSignalSubsystem::ReadBook(World);
		TestEqual(TEXT("ReadBook: four seats in the match"), Book.Slots, 4);
		TestTrue(TEXT("ReadBook: locked outside the Diplomacy mode"), Book.bLocked);
		if (TestEqual(TEXT("ReadBook: a stance per pair"), Book.Stances.Num(), 16))
		{
			TestEqual(TEXT("ReadBook: 0 and 1 allied (team 1)"), Book.Stances[0 * 4 + 1], 2);
			TestNotEqual(TEXT("ReadBook: 0 and 2 not allied"), Book.Stances[0 * 4 + 2], 2);
			TestNotEqual(TEXT("ReadBook: 3 and 2 not allied"), Book.Stances[3 * 4 + 2], 2);
		}

		const FVector Place(800.0, 400.0, 0.0);
		constexpr int32 Attack = 1;
		constexpr int32 Ok = 0;
		TestEqual(TEXT("Slot 2's signal is taken"), Board->PostNow(Book, 2, false, Attack, Place, 1000), Ok);
		TestEqual(TEXT("Slot 0's signal is taken"), Board->PostNow(Book, 0, false, Attack, Place, 1100), Ok);

		TestFalse(TEXT("Slot 3 alive is not a spectator"), UHOLSignalSubsystem::IsSignalSpectator(World, 3, false));
		TestTrue(TEXT("Slot 1 defeated"), GameState->DefeatSlot(1, EHOLDefeatReason::Surrender));
		TestTrue(TEXT("Slot 3 defeated"), GameState->DefeatSlot(3, EHOLDefeatReason::Surrender));
		TestFalse(TEXT("The match goes on (teams 1 and 2 alive)"), GameState->IsMatchOver());

		const bool bSpectator1 = UHOLSignalSubsystem::IsSignalSpectator(World, 1, false);
		TestFalse(TEXT("Defeated 1 with its ally 0 alive is no spectator (D784)"), bSpectator1);
		TestTrue(TEXT("Defeated 1 sees its ally 0's signal"), Board->SeesNow(Book, 0, 1, bSpectator1));
		TestFalse(TEXT("Defeated 1 does not see the enemy 2's signal"), Board->SeesNow(Book, 2, 1, bSpectator1));

		const bool bSpectator3 = UHOLSignalSubsystem::IsSignalSpectator(World, 3, false);
		TestTrue(TEXT("Defeated 3 without ally is a spectator (answer A)"), bSpectator3);
		TestTrue(TEXT("Defeated 3 sees the enemy 2's signal"), Board->SeesNow(Book, 2, 3, bSpectator3));

		const bool bPure = UHOLSignalSubsystem::IsSignalSpectator(World, -1, true);
		TestTrue(TEXT("A spectator without seat is one"), bPure);
		TestTrue(TEXT("A spectator without seat sees 2's signal"), Board->SeesNow(Book, 2, -1, bPure));

		const bool bSpectator0 = UHOLSignalSubsystem::IsSignalSpectator(World, 0, false);
		TestFalse(TEXT("Slot 0 alive is no spectator"), bSpectator0);
		TestFalse(TEXT("Slot 0 alive does not see the enemy 2's signal"), Board->SeesNow(Book, 2, 0, bSpectator0));
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
