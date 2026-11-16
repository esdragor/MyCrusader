// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLSiteBoardSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// MIG-CPP-Site (D836 point 3): the sacred-site rules are the script's (Script/Sites/HOLSiteRules.as and HOLSiteVictory.as), reached by the server
// through the bridge UHOLSiteBoardSubsystem (Script/Sites/HOLSiteBoardScript.as), once per site and step with the counts aggregated. The rules
// are tested in script (HOLSiteRules_Test.as, HOLSiteVictory_Test.as); here the real chain C++ bridge -> script board, with the hold and decay
// steps GIVEN by the caller (the script must use them, not its own constants). Without a board there is no rule (the C++ copy is gone): the state is left unchanged.
namespace UE::HeroesOfLiberty::Tests::SiteBridge
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSiteServerBoardTest, "HeroesOfLiberty.Sites.ServerBoard", UE::HeroesOfLiberty::Tests::SiteBridge::Flags)

bool FHOLSiteServerBoardTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UHOLSiteBoardSubsystem* Board = UHOLSiteBoardSubsystem::Find(World);
	if (TestNotNull(TEXT("the script board of the sacred sites"), Board))
	{
		const TArray<int32> Teams = {0, 1, 2};
		constexpr int32 DecaySteps = 2;

		// The capture: slot 1 alone with a taker, one step short of the hold steps GIVEN by the caller (300): it takes the site.
		FHOLSiteState Held;
		Held.Held = 299;
		const FHOLSiteState Taken = Board->StepSiteNow(Held, {0, 1, 0}, {0, 1, 0}, Teams, 300, DecaySteps);
		TestEqual(TEXT("The taker takes the site at the hold steps the caller gives"), Taken.Owner, 1);
		TestEqual(TEXT("The count starts again after the capture"), Taken.Held, 0);
		TestEqual(TEXT("Nobody fills it after the capture"), Taken.Taker, static_cast<int32>(INDEX_NONE));
		const FHOLSiteState NotYet = Board->StepSiteNow(Held, {0, 1, 0}, {0, 1, 0}, Teams, 301, DecaySteps);
		TestEqual(TEXT("With one more hold step the site is not taken yet"), NotYet.Owner, static_cast<int32>(INDEX_NONE));
		TestEqual(TEXT("The count goes up by one"), NotYet.Held, 300);
		TestEqual(TEXT("The taker is the slot filling it"), NotYet.Taker, 1);

		// The recession at the decay steps GIVEN (5): nobody in the zone.
		FHOLSiteState Receding;
		Receding.Held = 10;
		Receding.Taker = 1;
		TestEqual(TEXT("Nobody in the zone: the count recedes by the decay steps the caller gives"), Board->StepSiteNow(Receding, {0, 0, 0}, {0, 0, 0}, Teams, 300, 5).Held, 5);

		// The victory: slots 0 and 1 are one camp (team 7) and own the two sites; two steps to hold.
		const TArray<int32> VictoryTeams = {7, 7, 9};
		FHOLSiteVictoryState Victory;
		Victory = Board->VictoryStepNow(Victory, {0, 1}, {0, 0}, VictoryTeams, 2);
		TestEqual(TEXT("The camp holding every site leads"), Victory.Leader, 7);
		TestEqual(TEXT("It has held them one step"), Victory.Held, 1);
		Victory = Board->VictoryStepNow(Victory, {0, 1}, {0, 0}, VictoryTeams, 2);
		TestEqual(TEXT("At the victory steps the caller gives it wins"), Victory.Winner, 7);
		FHOLSiteVictoryState Frozen = Board->VictoryStepNow(FHOLSiteVictoryState(), {0, 1}, {0, 1}, VictoryTeams, 2);
		Frozen = Board->VictoryStepNow(Frozen, {0, 1}, {0, 1}, VictoryTeams, 2);
		TestEqual(TEXT("A contested site freezes the count: no winner"), Frozen.Winner, static_cast<int32>(INDEX_NONE));

		// The server's call (the static one) is the board's answer.
		TestTrue(TEXT("The server's call with the board is the board's answer"), UHOLSiteBoardSubsystem::StepSite(Board, Held, {0, 1, 0}, {0, 1, 0}, Teams, 300, DecaySteps) == Taken);
		// Without a board there is no rule (the C++ copy is gone): the state is unchanged and an error is logged (once, maybe already earlier).
		AddExpectedError(TEXT("no script board"), EAutomationExpectedErrorFlags::Contains, 0);
		TestTrue(TEXT("Without a board the site is left unchanged"), UHOLSiteBoardSubsystem::StepSite(nullptr, Held, {0, 1, 0}, {0, 1, 0}, Teams, 300, DecaySteps) == Held);
		FHOLSiteVictoryState Counting;
		Counting.Leader = 7;
		Counting.Held = 3;
		const FHOLSiteVictoryState Idle = UHOLSiteBoardSubsystem::VictoryStep(nullptr, Counting, {0, 1}, {0, 0}, VictoryTeams, 2);
		TestTrue(TEXT("Without a board the victory count is left unchanged"), Idle.Leader == 7 && Idle.Held == 3 && Idle.Winner == INDEX_NONE);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
