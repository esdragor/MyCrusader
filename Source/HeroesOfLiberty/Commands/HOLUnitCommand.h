// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * One order for one unit (D226): "go to this location". Orders always pass through a queue, even in solo, so
 * that M2.5's replication (D216) plugs in here without touching the movement code.
 */
struct FHOLUnitMoveCommand
{
	/** Stable identifier of the unit (D223). */
	uint32 UnitId = 0;
	FVector Target = FVector::ZeroVector;
};
