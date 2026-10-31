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

int32 UHOLRelayRulesLibrary::GetHoldSeconds()
{
	return FMath::RoundToInt(HOLRelay::DefaultHoldSeconds);
}

int32 UHOLRelayRulesLibrary::GetDecaySteps()
{
	return HOLRelay::DefaultDecaySteps;
}

int32 UHOLRelayRulesLibrary::GetRelayXPPerMinute()
{
	return HOLRelay::RelayXPPerMinute;
}

int32 UHOLRelayRulesLibrary::GetCentreXPPerMinute()
{
	return HOLRelay::CentreXPPerMinute;
}

int32 UHOLRelayRulesLibrary::TerritoryXPStep(const int32 Carry, const int32 Relays, const int32 Centres, const int32 StepsPerSecond, int32& OutCarry)
{
	OutCarry = Carry;
	return HOLRelay::TerritoryXP(OutCarry, Relays, Centres, StepsPerSecond);
}
