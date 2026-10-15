// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/** A unit as drawn on the screen (pixels), for the selection (J4, D288): light units have no collision to trace. */
struct FHOLPickCandidate
{
	/** Feet and top of the head, projected. */
	FVector2D Foot = FVector2D::ZeroVector;
	FVector2D Head = FVector2D::ZeroVector;
	/** Half the body's width on the screen. */
	double HalfWidth = 0.;
	/** Distance from the camera: the nearest one is in front. */
	double Depth = 0.;
};

/**
 * Pure rules of the selection of units on the screen (J4, D288), tested on their own: a click takes the unit whose
 * drawn body holds the cursor, the one in front when bodies overlap; a box takes the units whose body's middle is in it.
 */
namespace HOLUnitPicking
{
	/** A press that moves farther than this (pixels) before its release draws a box instead of clicking. */
	inline constexpr double BoxThreshold = 6.;

	/** Index of the candidate under Cursor, or INDEX_NONE. */
	HEROESOFLIBERTY_API int32 PickAt(TConstArrayView<FHOLPickCandidate> Candidates, const FVector2D& Cursor);

	/** Indices of the candidates whose body's middle is inside the box between two corners (any order). */
	HEROESOFLIBERTY_API void PickInBox(TConstArrayView<FHOLPickCandidate> Candidates, const FVector2D& CornerA, const FVector2D& CornerB, TArray<int32>& OutIndices);

	/** True when a press at Start released at End is a box, not a click. */
	HEROESOFLIBERTY_API bool IsBox(const FVector2D& Start, const FVector2D& End);

	/**
	 * The "." key (D30, J5b): of the idle workers (identifiers in increasing order), the first one after Last, or the
	 * first of all after the last one or when Last is not one of them; 0 when there is none. Each press goes to the next.
	 */
	HEROESOFLIBERTY_API uint32 NextInCycle(TConstArrayView<uint32> SortedIds, uint32 Last);
}
