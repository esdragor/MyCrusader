// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * One neutral relay (R3; GDD 9.4, D388, D348, D380): its owner (a slot), who is filling its count and the count of steps held toward its
 * capture. The simulation's copy of the rules written and tested in Script/Relays/HOLRelayRules.as (the simulation cannot call the script):
 * the test Relays.ScriptAgreement compares the two on every case.
 */
struct FHOLRelayState
{
	/** The owner's slot, INDEX_NONE for nobody. */
	int32 Owner = INDEX_NONE;
	/** The slot with the most units among the adverse side filling the count, INDEX_NONE while nobody fills it. */
	int32 Taker = INDEX_NONE;
	/** Steps held toward the capture. */
	int32 Held = 0;

	bool operator==(const FHOLRelayState& Other) const { return Owner == Other.Owner && Taker == Other.Taker && Held == Other.Held; }
};

namespace HOLRelay
{
	/**
	 * The relay after one step. UnitsBySlot: the living units of each slot in the zone; TeamBySlot: each slot's team (equal teams are allies).
	 * One adverse side alone (no unit of the owner's team): the count goes up and, full (HoldSteps), the relay goes to the taker. A unit of the
	 * owner's team, or two adverse sides: the count is frozen. Nobody adverse: it recedes by DecaySteps, to 0.
	 */
	HEROESOFLIBERTY_API FHOLRelayState Step(const FHOLRelayState& State, const TArray<int32>& UnitsBySlot, const TArray<int32>& TeamBySlot, int32 HoldSteps,
		int32 DecaySteps);
}
