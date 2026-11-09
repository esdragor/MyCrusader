// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "HOLAvoidanceHoldFragments.generated.h"

/**
 * Per-entity memory of the last avoidance impulse (D230). The avoidance solver only runs one step out of
 * several (D220) and Mass resets the force fragment after applying it, so an impulse computed on one step
 * would be forgotten on the next ones - and two units on a collision course then walk through each other.
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLAvoidanceHoldFragment : public FMassFragment
{
	GENERATED_BODY()

	/** Force the steering had produced, taken just before the avoidance solver ran. */
	FVector ForceBefore = FVector::ZeroVector;

	/** Avoidance impulse of the last run, re-applied while the solver waits for its next turn. */
	FVector HeldForce = FVector::ZeroVector;

	/** Steps the impulse can still be re-applied for (0 = nothing held). */
	int32 StepsLeft = 0;
};

/**
 * Per-entity memory of the look-ahead's detour (D293, MV.2): the body turned around, the hand and the turn, kept while
 * that body is ahead, forgotten on a new order or leg. Without it each step started again from the straight course and
 * the heading flipped every 2 steps.
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLDetourFragment : public FMassFragment
{
	GENERATED_BODY()

	/** The body turned around (unset: no detour). */
	FMassEntityHandle Blocker;

	/** The order and the leg the detour was taken on. */
	uint32 OrderSerial = 0;

	uint8 Phase = 0;

	/** The hand (+1 right, -1 left) and the turn, in look-ahead angle steps (0: none). */
	int8 Side = 0;

	uint8 Turn = 0;

	/** Steps in a row the course one turn narrower was clear (MV.2: eased back after HOLAvoid::EaseBackSteps). */
	uint8 ClearSteps = 0;

	/** Steps this detour has been held while the walker was near its goal (hol.Units.SlotDetourMaxSteps). */
	uint8 DetourSteps = 0;

	/** Set for the rest of the leg once that limit is passed: the walker goes straight to its goal, pushing (neutrals trial, melee_spin: a walker circling its own slot). */
	uint8 bSlotCommit = 0;

	/** Set by the look-ahead at the step when a still ENEMY stands within hol.Units.SlotBlockedRadius of the walker's goal (Celeborn-12: a place inside an
	 * enemy's body is never reached; D293 does not push an enemy): read by the order processor, which ends the final leg at once near it. */
	uint8 bSlotBlocked = 0;

	/** The walker met at a crossing and this one's part against it (HOLAvoid::ECrossingRole), kept while it is near (MV.2:
	 * predicted from last step's velocities, the part would flip once one turned). */
	FMassEntityHandle Crosser;

	uint8 CrossRole = 0;

	int8 CrossSide = 0;
};
