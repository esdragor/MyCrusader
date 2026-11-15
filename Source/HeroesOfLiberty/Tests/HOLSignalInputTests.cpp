// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Player/HOLSignalInput.h"

#if WITH_DEV_AUTOMATION_TESTS

// SG3 (SignalsInProgress.md; GDD 15, D386, D784): the Alt + click's kind and the received icons' life, on the real functions the
// controller calls (HandleSelectCompleted -> KindOfAltClick -> RequestSignal; ClientReceiveSignal -> Expire -> the minimap's marks).
namespace UE::HeroesOfLiberty::Tests::SignalInput
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSignalAltClickTest, "HeroesOfLiberty.Signals.AltClick", UE::HeroesOfLiberty::Tests::SignalInput::Flags)

bool FHOLSignalAltClickTest::RunTest(const FString& Parameters)
{
	// Kinds as Script/Signals/HOLSignalRules.as (the GDD's order of the wheel).
	constexpr int32 Neutral = 0, Attack = 1, Defend = 2, NeedHelp = 3, Rally = 4, Scout = 5;
	const FVector2D Press(500.0, 400.0);
	// A brief Alt + click (a jitter of 3 px) is the neutral signal (GDD 15: "un simple Alt + clic").
	TestEqual(TEXT("A brief Alt + click places the neutral signal"), HOLSignalInput::KindOfAltClick(Press, Press + FVector2D(2.0, -2.0)), Neutral);
	// A drag of 60 px picks the wheel's kind by its direction, clockwise from the top, 72 degrees each.
	auto Drag = [&Press](const double Degrees)
	{
		const double Radians = FMath::DegreesToRadians(Degrees);
		return Press + FVector2D(FMath::Sin(Radians), -FMath::Cos(Radians)) * 60.0;
	};
	TestEqual(TEXT("Dragged up: attack here"), HOLSignalInput::KindOfAltClick(Press, Drag(0.0)), Attack);
	TestEqual(TEXT("Dragged up-right: defend here"), HOLSignalInput::KindOfAltClick(Press, Drag(72.0)), Defend);
	TestEqual(TEXT("Dragged down-right: need help"), HOLSignalInput::KindOfAltClick(Press, Drag(144.0)), NeedHelp);
	TestEqual(TEXT("Dragged down-left: rally"), HOLSignalInput::KindOfAltClick(Press, Drag(216.0)), Rally);
	TestEqual(TEXT("Dragged up-left: scout here"), HOLSignalInput::KindOfAltClick(Press, Drag(288.0)), Scout);
	TestEqual(TEXT("Dragged a little left of the top: still attack"), HOLSignalInput::KindOfAltClick(Press, Drag(350.0)), Attack);

	// The received icons: one at 10.0 s, one at 12.0 s; at 14.9 s both are lit, at 15.0 s the first is gone (5 s, D784), at 17.0 s none.
	TArray<FHOLReceivedSignal> Signals;
	FHOLReceivedSignal First;
	First.Kind = Attack;
	First.ReceivedAt = 10.0;
	FHOLReceivedSignal Second;
	Second.Kind = Rally;
	Second.ReceivedAt = 12.0;
	Signals.Add(First);
	Signals.Add(Second);
	HOLSignalInput::Expire(Signals, 14.9);
	TestEqual(TEXT("Both icons are lit before 5 s"), Signals.Num(), 2);
	HOLSignalInput::Expire(Signals, 15.0);
	if (TestEqual(TEXT("The first icon is gone at 5 s"), Signals.Num(), 1))
	{
		TestEqual(TEXT("The second one is left"), Signals[0].Kind, Rally);
	}
	HOLSignalInput::Expire(Signals, 17.0);
	TestEqual(TEXT("None at 17 s"), Signals.Num(), 0);

	// Each kind its own colour (default A of Q2): the attack's is not the neutral's.
	TestFalse(TEXT("The attack's icon is not coloured as the neutral one"), HOLSignalInput::ColorOf(Attack).Equals(HOLSignalInput::ColorOf(Neutral)));
	return true;
}

#endif
