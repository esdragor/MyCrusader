// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLRelayView.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLRelayRules.h"

#if WITH_DEV_AUTOMATION_TESTS

// R5 (RelaysInProgress.md): what a relay's ring shows, from the state the server replicates (FHOLRelayView): the share of the capture held
// (FillPercent) and the view built from a relay's state (HOLRelay::MakeView).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLRelayRingTest, "HeroesOfLiberty.Relays.Ring", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLRelayRingTest::RunTest(const FString& Parameters)
{
	FHOLRelayState State;
	TestEqual(TEXT("nothing held: 0"), HOLRelay::FillPercent(State, 600), 0);
	State.Held = 150;
	TestEqual(TEXT("a quarter: 25"), HOLRelay::FillPercent(State, 600), 25);
	State.Held = 599;
	TestEqual(TEXT("almost taken: 99, never 100 (the ring is full only by the change of owner)"), HOLRelay::FillPercent(State, 600), 99);
	State.Held = 900;
	TestEqual(TEXT("past the count: 99"), HOLRelay::FillPercent(State, 600), 99);
	TestEqual(TEXT("no hold steps: 0, no division by 0"), HOLRelay::FillPercent(State, 0), 0);

	State.Owner = 2;
	State.Taker = 1;
	State.Held = 300;
	const FHOLRelayView View = HOLRelay::MakeView(FVector2D(1500., -2500.), 1200.f, State, 600);
	TestEqual(TEXT("view: X"), View.X, 1500.f);
	TestEqual(TEXT("view: Y"), View.Y, -2500.f);
	TestEqual(TEXT("view: radius"), View.Radius, 1200.f);
	TestEqual(TEXT("view: owner"), static_cast<int32>(View.Owner), 2);
	TestEqual(TEXT("view: taker"), static_cast<int32>(View.Taker), 1);
	TestEqual(TEXT("view: half filled"), static_cast<int32>(View.FillPercent), 50);
	TestEqual(TEXT("view: the line the markers and the report read"), View.Describe(), FString(TEXT("(1500, -2500) r1200 owner 2 taker 1 fill 50")));
	TestEqual(TEXT("view of nobody"), HOLRelay::MakeView(FVector2D::ZeroVector, 1200.f, FHOLRelayState(), 600).Describe(), FString(TEXT("(0, 0) r1200 owner -1 taker -1 fill 0")));
	return true;
}

#endif
