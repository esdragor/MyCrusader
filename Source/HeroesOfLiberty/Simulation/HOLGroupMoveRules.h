// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Pure rules of a group's move order (J4, D286, D287), on the ground plane (cm). The server cuts the ordered units
 * into clusters of neighbours, one path each (D286); at the end of its path, each unit walks to its own slot of a
 * tight honeycomb around the target, oriented along the march, the slots given so that the units keep their relative
 * places and their paths do not cross (D230: two units whose paths cross go through each other). No formation: each
 * unit at its own speed. No engine state: tested on their own.
 */
namespace HOLGroupMove
{
	/** Units closer than this to another unit of a cluster belong to it (D286). */
	inline constexpr double ClusterLinkDistance = 1500.;

	/** Gap between the centres of two slots, in body diameters (D287: a diameter and ~20 %). */
	inline constexpr double SlotSpacingInDiameters = 1.2;

	/**
	 * Cuts units into clusters: two units closer than LinkDistance are in the same cluster, and so are their own
	 * neighbours (a chain of units is one cluster). OutClusterOfUnit[i] is the cluster of unit i, numbered from 0 in
	 * the order of their first unit; returns the number of clusters.
	 */
	HEROESOFLIBERTY_API int32 MakeClusters(TConstArrayView<FVector2D> Positions, double LinkDistance, TArray<int32>& OutClusterOfUnit);

	/**
	 * The Count slots nearest the target on a honeycomb of Spacing (cm), as offsets in the march's frame (X forward,
	 * Y to the right), nearest first; the first one is the target itself. Asking for more slots than units lets the
	 * caller skip those it cannot use (in an obstacle) and keep the disc tight.
	 */
	HEROESOFLIBERTY_API void MakeSlots(int32 Count, double Spacing, TArray<FVector2D>& OutOffsets);

	/** A slot offset of the march's frame turned into the world: Forward is the march's direction (normalised here). */
	HEROESOFLIBERTY_API FVector2D ToWorld(const FVector2D& Offset, const FVector2D& Forward);

	/**
	 * Gives each unit a slot, keeping the units' places relative to each other: the units' offsets from their centre
	 * and the slots' offsets from theirs are brought to the same spread, then the closest unit and slot are paired
	 * first, and so on. A group already in the slots' shape keeps it; the unit on the left arrives on the left.
	 * Positions are world points, Slots world points too (as many slots as units, or more: the extra ones stay free).
	 * OutSlotOfUnit[i] is the index in Slots given to unit i.
	 */
	HEROESOFLIBERTY_API void AssignSlots(TConstArrayView<FVector2D> Positions, TConstArrayView<FVector2D> Slots, TArray<int32>& OutSlotOfUnit);
}
