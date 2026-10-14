// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * One move order (D226, D286): "go to this location", for a group of units. Orders always pass through a queue,
 * even in solo, applied at the next fixed step. The group is kept whole: its units share their paths by clusters
 * and are given their slots around the target together (D287).
 */
struct FHOLMoveOrder
{
	/** Stable identifiers of the units (D223). */
	TArray<uint32> UnitIds;
	/** On the navmesh (brought back to it when the order is issued, D228). */
	FVector Target = FVector::ZeroVector;
};
