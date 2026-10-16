// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "HOLBuildFragments.generated.h"

/** Where a builder is at its site (J6a.4, D312). */
UENUM()
enum class EHOLBuildPhase : uint8
{
	/** Not building: idle, gathering, or walking where a player sent it. */
	None,
	/** Walking to its spot around the footprint of the head of its queue. */
	ToSite,
	/** At its spot, at contact: it works on the site. */
	Building,
	/**
	 * The head of its queue is a source (Shift + right click on a resource, the user's trial of J6a.4): the gathering
	 * carries it out (FHOLGatherFragment) until its massif is used up, then the next item of its queue comes.
	 */
	Gathering,
};

/**
 * A unit's building (J6a.4, D312), on the server: its spot around the footprint of the site it builds, its place in the
 * work. Its queue (Shift, D311: sites, and sources to gather after them) is kept by UHOLBuildProcessor (a fragment holds
 * no array). A player's move or gather order ends it (Phase None: the queue is forgotten); a walk it gives itself to its
 * spot (through the order queue, as the gathering does) does not.
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLBuildFragment : public FMassFragment
{
	GENERATED_BODY()

	/** Where it walks and stands, around the head's footprint. */
	FVector Spot = FVector::ZeroVector;

	/**
	 * The footprint of the site it works on, written when it starts working there (Phase Building): its hammering is drawn
	 * turned towards the nearest point of it (J6a.4, display only).
	 */
	FBox2D SiteFootprint = FBox2D(ForceInit);

	/** Its move order's serial when it gave itself the walk to Spot: a new serial, then no order, is the arrival. */
	uint32 OrderSerialBefore = 0;

	/** Arrivals in a row that missed the site (pushed away, its spot taken). */
	uint8 Misses = 0;

	/** The walk to Spot is asked for (it starts at the next step, through the order queue). */
	bool bWalkIssued = false;

	/**
	 * Its queue holds more after its head (written by UHOLBuildProcessor): a gathering at the head whose massif is full
	 * goes on to the next item instead of standing idle in front of it (D309).
	 */
	bool bQueueNext = false;

	EHOLBuildPhase Phase = EHOLBuildPhase::None;

	/** It has a queue: building, walking to a site, or gathering at the head of its queue. */
	bool IsBuilding() const { return Phase != EHOLBuildPhase::None; }

	/** Gathering the source at the head of its queue, with more queued after it. */
	bool HasQueuedNext() const { return Phase == EHOLBuildPhase::Gathering && bQueueNext; }

	/** Stops building, its queue forgotten (a player's order elsewhere, D312). */
	void Clear()
	{
		Phase = EHOLBuildPhase::None;
		bWalkIssued = false;
		bQueueNext = false;
		Misses = 0;
	}
};
