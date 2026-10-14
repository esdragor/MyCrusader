// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class AHOLBuilding;

/**
 * One production order of a player (J3, D282): queue one unit of a type at a building, or cancel an item of its
 * queue. Like the unit orders, it waits in the order queue and the server applies it at its next step.
 */
struct FHOLProductionCommand
{
	enum class EKind : uint8
	{
		Queue,
		Cancel,
	};

	EKind Kind = EKind::Queue;
	/** Slot of the player who gave the order (D244): only the building's owner is obeyed. */
	int32 PlayerIndex = INDEX_NONE;
	TWeakObjectPtr<AHOLBuilding> Building;
	/** Queue: the unit type to produce. */
	FName TypeName;
	/** Cancel: index of the item in the building's queue. */
	int32 Index = INDEX_NONE;
};
