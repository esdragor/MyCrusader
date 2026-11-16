// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLRelayBoard.h"

#if WITH_DEV_AUTOMATION_TESTS

// Bascule Relay (D830/D836 point 3, AngelscriptInProgress.md): the server asks the script's rule through UHOLRelayBoard
// (Script/Relays/HOLRelayBoardScript.as -> HOLRelay::StepWith / TerritoryXPAt of HOLRelayRules.as). The C++ rules stay until the proof, so the
// test also checks that the board and the C++ rules give the same answers, through the server's own entry points (StepForRelay, TerritoryXPFor).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLRelayServerBoardTest, "HeroesOfLiberty.Relay.ServerBoard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHOLRelayServerBoardTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UHOLRelayBoard* Board = UHOLRelayBoard::Find(World);
	if (TestNotNull(TEXT("the script board of the relays"), Board))
	{
		const TArray<int32> Teams = {0, 1};

		// Slot 0 alone in a neutral relay: the count goes up, and at HoldSteps (3) the relay is its own.
		FHOLRelayBoardState State;
		State = Board->StepNow(State, {5, 0}, Teams, 3, 2);
		TestEqual(TEXT("First step: slot 0 fills the count"), State.Taker, 0);
		TestEqual(TEXT("First step: one step held"), State.Held, 1);
		State = Board->StepNow(State, {5, 0}, Teams, 3, 2);
		State = Board->StepNow(State, {5, 0}, Teams, 3, 2);
		TestEqual(TEXT("Third step: the relay goes to slot 0"), State.Owner, 0);
		TestEqual(TEXT("Third step: the count starts again"), State.Held, 0);

		// A unit of the owner freezes the count; nobody adverse makes it recede by DecaySteps, to 0.
		FHOLRelayBoardState Held;
		Held.Owner = 0;
		Held.Taker = 1;
		Held.Held = 2;
		TestEqual(TEXT("A defender freezes the count"), Board->StepNow(Held, {1, 3}, Teams, 10, 2).Held, 2);
		TestEqual(TEXT("Nobody adverse: the count recedes by two"), Board->StepNow(Held, {0, 0}, Teams, 10, 2).Held, 0);

		// The server's entry point and the C++ rule give the same relay (and the same answer as the board on the case above).
		const FHOLRelayState Neutral;
		TestTrue(TEXT("StepForRelay goes through the board and agrees with the C++ rule"),
			UHOLRelayBoard::StepForRelay(World, Neutral, {5, 0}, Teams, 3, 2) == HOLRelay::Step(Neutral, {5, 0}, Teams, 3, 2));

		// Territory XP: one relay and one secondary centre at 10 steps a second give 20 x steps per 600: one XP in 30 steps, nothing carried left.
		int32 Carry = 0;
		int32 CarryCpp = 0;
		int32 Total = 0;
		bool bAgree = true;
		for (int32 Step = 0; Step < 30; ++Step)
		{
			const int32 Award = UHOLRelayBoard::TerritoryXPFor(World, Carry, 1, 1, 10);
			bAgree = bAgree && Award == HOLRelay::TerritoryXP(CarryCpp, 1, 1, 10) && Carry == CarryCpp;
			Total += Award;
		}
		TestEqual(TEXT("Thirty steps of one relay and one centre at 10 Hz give 1 XP"), Total, 1);
		TestEqual(TEXT("and nothing is carried"), Carry, 0);
		TestTrue(TEXT("TerritoryXPFor agrees with the C++ rule step by step"), bAgree);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
