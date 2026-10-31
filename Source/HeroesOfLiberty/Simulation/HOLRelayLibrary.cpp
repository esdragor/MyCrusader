// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLRelayLibrary.h"

#include "Simulation/HOLRelayRules.h"

void UHOLRelayRulesLibrary::StepRelay(const int32 Owner, const int32 Taker, const int32 Held, const TArray<int32>& UnitsBySlot, const TArray<int32>& TeamBySlot,
	const int32 HoldSteps, const int32 DecaySteps, int32& OutOwner, int32& OutTaker, int32& OutHeld)
{
	FHOLRelayState State;
	State.Owner = Owner;
	State.Taker = Taker;
	State.Held = Held;
	const FHOLRelayState Next = HOLRelay::Step(State, UnitsBySlot, TeamBySlot, HoldSteps, DecaySteps);
	OutOwner = Next.Owner;
	OutTaker = Next.Taker;
	OutHeld = Next.Held;
}
