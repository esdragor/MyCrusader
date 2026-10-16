// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class AHOLBuilding;

/**
 * One construction order of a player (J6a.3, D311): place a foundation of a type at a ground point, or cancel one of its
 * buildings not yet finished. Like the production orders, it waits in the order queue and the server applies it at its
 * next step, checking everything again (D288): the client's ghost is never trusted.
 */
struct FHOLConstructionCommand
{
	enum class EKind : uint8
	{
		Place,
		Cancel,
	};

	EKind Kind = EKind::Place;
	/** Slot of the player who gave the order (D244): it pays, it owns the foundation, only it cancels its own. */
	int32 PlayerIndex = INDEX_NONE;
	/** Place: the building type (a row of the building type table that builders build). */
	FName TypeName;
	/** Place: the ground point the footprint is snapped around (HOLConstruction::SnapToGrid). */
	FVector Location = FVector::ZeroVector;
	/** Cancel: the building. */
	TWeakObjectPtr<AHOLBuilding> Building;
};
