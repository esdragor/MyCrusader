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
		/** The building's rally point (J4, D288), or none when Location is on the building itself. */
		Rally,
	};

	EKind Kind = EKind::Queue;
	/** Slot of the player who gave the order (D244): only the building's owner is obeyed. */
	int32 PlayerIndex = INDEX_NONE;
	TWeakObjectPtr<AHOLBuilding> Building;
	/** Queue: the unit type to produce. */
	FName TypeName;
	/** Cancel: index of the item in the building's queue. */
	int32 Index = INDEX_NONE;
	/** Rally: the ground point. */
	FVector Location = FVector::ZeroVector;
	/** Rally: the resource source there (J5, D298), whose units born here gather it; INDEX_NONE: the ground. */
	int32 SourceIndex = INDEX_NONE;
	/**
	 * Rally: one of the player's sites not finished there (the user's trial of J6a.4), whose units born here build it;
	 * null: the ground or a source.
	 */
	TWeakObjectPtr<AHOLBuilding> RallySite;
	/** Rally with Shift (EtapeDefini N.1): a step added at the end of the rally point, not the rally point replaced. */
	bool bQueue = false;
};
