// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLGameState.h"
#include "Match/HOLPlayerState.h"
#include "Match/HOLSpecializationSubsystem.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// SP2 (SpecializationInProgress.md; GDD 13, D07, D684, D785, answers B of Q-specialization-switch and Q-nomad-forum Q2): the server's
// specialization board is the script's (the real chain: C++ bridge -> Script/Specialization HOLSpecialization::Start, Cancel, Advance),
// and each seat's view lands on its player state (what the owner's screen reads):
// - slot 0 at tier 1 starts option 1: running, 30 s left; 29.999 s later not applied, 1 ms more applied (the event names tier 1, option 1);
// - the applied tier is final (AlreadyChosen); slot 1 has nothing (one state per seat);
// - tier 2 without a complete Forum: refused, the button off (NoForumComplete); with one: open, started; the other option replaces it at
//   the full 45 s (answer B); 10 s with the Forum down: paused (Q2 answer A); cancel: nothing runs, nothing chosen;
// - through the player state: started again, the world's 45 s of AdvanceAllNow apply it and the view says so.
namespace UE::HeroesOfLiberty::Tests::SpecializationBoard
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSpecializationBoardServerTest, "HeroesOfLiberty.Specialization.ServerBoard", UE::HeroesOfLiberty::Tests::SpecializationBoard::Flags)

bool FHOLSpecializationBoardServerTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
	World->SetGameState(GameState);

	UHOLSpecializationSubsystem* Board = UHOLSpecializationSubsystem::Find(World);
	if (TestNotNull(TEXT("the script board of specializations"), Board))
	{
		constexpr int32 Ok = 0;
		constexpr int32 AlreadyChosen = 4;
		constexpr int32 NoForumComplete = 8;
		int32 AppliedSlot = -1, AppliedTier = 0, AppliedOption = -1;
		Board->OnApplied.AddLambda([&](const int32 Slot, const int32 Tier, const int32 Option) { AppliedSlot = Slot; AppliedTier = Tier; AppliedOption = Option; });

		TestTrue(TEXT("Tier 1's choice is open at tier 1"), Board->IsOpenNow(0, 1, 1, true));
		TestEqual(TEXT("Slot 0 starts option 1 of tier 1"), Board->StartNow(0, 1, 1, 1, true), Ok);
		TestEqual(TEXT("It runs"), Board->ViewNow(0).PendingTier, 1);
		TestEqual(TEXT("Option 1"), Board->ViewNow(0).PendingOption, 1);
		TestEqual(TEXT("30 s left (D684)"), Board->ViewNow(0).RemainingMs, 30000);
		TestEqual(TEXT("29.999 s later: not applied"), Board->AdvanceNow(0, 29999, true), 0);
		TestEqual(TEXT("1 ms left"), Board->ViewNow(0).RemainingMs, 1);
		TestEqual(TEXT("1 ms more: tier 1 applied"), Board->AdvanceNow(0, 1, true), 1);
		TestEqual(TEXT("The event names slot 0"), AppliedSlot, 0);
		TestEqual(TEXT("The event names tier 1"), AppliedTier, 1);
		TestEqual(TEXT("The event names option 1"), AppliedOption, 1);
		TestEqual(TEXT("Option 1 is tier 1's"), Board->ViewNow(0).GetChosen(1), 1);
		TestEqual(TEXT("Nothing runs any more"), Board->ViewNow(0).PendingTier, 0);
		TestEqual(TEXT("Tier 1 is final"), Board->StartNow(0, 1, 0, 1, true), AlreadyChosen);
		TestEqual(TEXT("Slot 1 has chosen nothing"), Board->ViewNow(1).GetChosen(1), -1);

		TestFalse(TEXT("Without a complete Forum, tier 2's button is off (Q-nomad-forum Q2 = B)"), Board->IsOpenNow(0, 2, 2, false));
		TestEqual(TEXT("And the start is refused"), Board->StartNow(0, 2, 0, 2, false), NoForumComplete);
		TestTrue(TEXT("With one it is open"), Board->IsOpenNow(0, 2, 2, true));
		TestEqual(TEXT("Option 0 of tier 2"), Board->StartNow(0, 2, 0, 2, true), Ok);
		TestEqual(TEXT("10 s run"), Board->AdvanceNow(0, 10000, true), 0);
		TestEqual(TEXT("The other option replaces it (answer B)"), Board->StartNow(0, 2, 1, 2, true), Ok);
		TestEqual(TEXT("Option 1 runs"), Board->ViewNow(0).PendingOption, 1);
		TestEqual(TEXT("At the full 45 s"), Board->ViewNow(0).RemainingMs, 45000);
		TestEqual(TEXT("10 s with the Forum down"), Board->AdvanceNow(0, 10000, false), 0);
		TestEqual(TEXT("Paused: still 45 s (Q2 = A)"), Board->ViewNow(0).RemainingMs, 45000);
		TestEqual(TEXT("Cancel"), Board->CancelNow(0), Ok);
		TestEqual(TEXT("Nothing runs"), Board->ViewNow(0).PendingTier, 0);
		TestEqual(TEXT("Nothing chosen at tier 2"), Board->ViewNow(0).GetChosen(2), -1);

		AHOLPlayerState* PlayerState = World->SpawnActor<AHOLPlayerState>();
		if (TestNotNull(TEXT("slot 0's player state"), PlayerState))
		{
			PlayerState->SetMatchSlot(0, 0);
			TestEqual(TEXT("Option 0 of tier 2 again"), Board->StartNow(0, 2, 0, 2, true), Ok);
			Board->RefreshView(*PlayerState);
			TestEqual(TEXT("The player state shows it running"), PlayerState->GetSpecialization().PendingTier, 2);
			Board->AdvanceAllNow(44999);
			TestEqual(TEXT("1 ms left on the player state"), PlayerState->GetSpecialization().RemainingMs, 1);
			Board->AdvanceAllNow(1);
			TestEqual(TEXT("The player state shows option 0 applied at tier 2"), PlayerState->GetSpecialization().GetChosen(2), 0);
			TestEqual(TEXT("The event names tier 2"), AppliedTier, 2);
		}
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
