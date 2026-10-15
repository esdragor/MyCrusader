// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLResources.h"

/**
 * Pure rules of gathering (J5, D296), applied by the server at each simulation step, on the ground plane (cm): a
 * gatherer in contact with a source takes from it at its rate until it carries its load, walks to the nearest deposit,
 * where the player's stock rises, and goes back to the same source; a used-up source sends it to the nearest one of the
 * same resource within ~15 m, or leaves it idle; gathering another resource loses the load (as in AoE). No engine
 * state: tested on their own.
 */

/** What a gatherer carries: one resource at a time. */
struct FHOLGatherLoad
{
	EHOLResource Resource = EHOLResource::Food;

	/** Whole units carried. */
	int32 Amount = 0;

	/** Share of the next unit already gathered (0 to 1): a rate is a fraction of a unit per step. */
	float Progress = 0.f;
};

/** A body on the ground plane: a deposit, or a source. */
struct FHOLGatherSite
{
	FVector2D Position = FVector2D::ZeroVector;

	/** Body radius (cm). */
	float Radius = 0.f;
};

/** A source as the rules see it. */
struct FHOLGatherSource
{
	FHOLGatherSite Site;
	EHOLResource Resource = EHOLResource::Wood;

	/** What it still holds; 0 = used up. */
	int32 Remaining = 0;
};

namespace HOLGather
{
	/** Distance (cm) between the centres of a used-up source and the one its gatherers move on to, at most (D296). */
	inline constexpr double NeighbourSourceDistance = 1500.;

	/** Gap (cm) between the bodies of a gatherer and its source within which it gathers, or of a carrier and its deposit. */
	inline constexpr double ContactGap = 30.;

	/**
	 * Readies Load for gathering Resource: a load of another resource is lost, one of the same resource is kept (D296).
	 * Returns the amount lost.
	 */
	HEROESOFLIBERTY_API int32 StartGathering(FHOLGatherLoad& Load, EHOLResource Resource);

	/**
	 * Gathers for one step of StepSeconds at Rate (units per second): whole units go from SourceRemaining to Load, up to
	 * Capacity. The progress toward the next unit is dropped once the load is full or the source used up. Returns the
	 * units taken this step.
	 */
	HEROESOFLIBERTY_API int32 Gather(FHOLGatherLoad& Load, float Rate, float StepSeconds, int32 Capacity, int32& SourceRemaining);

	/** The gatherer carries all it can: time to walk to a deposit. */
	HEROESOFLIBERTY_API bool IsFull(const FHOLGatherLoad& Load, int32 Capacity);

	/** Empties Load into Stock (the stock rises at the deposit only, D296). Returns the amount deposited. */
	HEROESOFLIBERTY_API int32 Deposit(FHOLGatherLoad& Load, FHOLResources& Stock);

	/** Distance (cm) from Position to the edge of Site's body; negative inside it. */
	HEROESOFLIBERTY_API double GetEdgeDistance(const FVector2D& Position, const FHOLGatherSite& Site);

	/** A body of BodyRadius at Position touches Site, give or take ContactGap. */
	HEROESOFLIBERTY_API bool IsInContact(const FVector2D& Position, float BodyRadius, const FHOLGatherSite& Site);

	/** Index of the deposit whose edge is nearest Position (the first one of a tie), INDEX_NONE if there is none. */
	HEROESOFLIBERTY_API int32 FindNearestDeposit(const FVector2D& Position, TConstArrayView<FHOLGatherSite> Deposits);

	/**
	 * Where the gatherers of a used-up source at From go: the index of the nearest source of Resource that still holds
	 * something, its centre within MaxDistance of From (the first one of a tie); INDEX_NONE = none, they become idle.
	 */
	HEROESOFLIBERTY_API int32 FindNeighbourSource(const FVector2D& From, EHOLResource Resource, TConstArrayView<FHOLGatherSource> Sources,
		double MaxDistance = NeighbourSourceDistance);
}
