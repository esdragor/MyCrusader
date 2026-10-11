// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MCAvoidanceHoldFragments.generated.h"

/**
 * Per-entity memory of the last avoidance impulse (D230). The avoidance solver only runs one step out of
 * several (D220) and Mass resets the force fragment after applying it, so an impulse computed on one step
 * would be forgotten on the next ones - and two units on a collision course then walk through each other.
 */
USTRUCT()
struct MYCRUSADER_API FMCAvoidanceHoldFragment : public FMassFragment
{
	GENERATED_BODY()

	/** Force the steering had produced, taken just before the avoidance solver ran. */
	FVector ForceBefore = FVector::ZeroVector;

	/** Avoidance impulse of the last run, re-applied while the solver waits for its next turn. */
	FVector HeldForce = FVector::ZeroVector;

	/** Steps the impulse can still be re-applied for (0 = nothing held). */
	int32 StepsLeft = 0;
};
