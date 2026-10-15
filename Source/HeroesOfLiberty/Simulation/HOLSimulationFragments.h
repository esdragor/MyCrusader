// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Simulation/HOLGatherRules.h"
#include "HOLSimulationFragments.generated.h"

/** Transform at the start of the last fixed step: the presentation interpolates from it to FTransformFragment. */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLPreviousTransformFragment : public FMassFragment
{
	GENERATED_BODY()

	FTransform Transform = FTransform::Identity;
};

/** Every light unit (D29): filters the presentation processors. */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLLightUnitTag : public FMassTag
{
	GENERATED_BODY()
};

/**
 * What the RTS layer sees a unit doing (D226): standing, walking to its current order, gathering at its source, or on
 * its gathering's round trip. Sent to every client in 3 bits (FHOLNetUnitState::ActivityBits): room for 8 values.
 */
UENUM()
enum class EHOLUnitActivity : uint8
{
	/** Standing with nothing to do: an idle worker for the HUD and the "." key (J5b, D307). */
	Idle,
	Moving,
	/** Standing at its source, taking from it (J5, D298): the client shows it, the presentation will animate it (J5c). */
	Gathering,
	/**
	 * On its gathering's round trip (J5b, D307): walking to its source or to a deposit, or waiting for a free spot. It
	 * walks as Moving does, but is never idle: a gatherer between two walks no longer shows Idle for a step.
	 */
	GatherTrip,
};

/** True while the unit walks: to a player's order, or on its gathering's round trip. */
inline bool IsWalkingActivity(const EHOLUnitActivity Activity)
{
	return Activity == EHOLUnitActivity::Moving || Activity == EHOLUnitActivity::GatherTrip;
}

/** Where a unit is in its move order (D286, D287). */
UENUM()
enum class EHOLMovePhase : uint8
{
	/** No order, or the order is over. */
	None,
	/** Ordered: its cluster's path is not computed yet (a budget of path requests per step). */
	WaitingPath,
	/** Following its cluster's path (Mass' short path along the shared corridor). */
	Path,
	/** From the end of the path straight to its own slot, then it stands. */
	FinalLeg,
};

/** A unit's part of its move order (J4, D286, D287), on the server. */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLMoveOrderFragment : public FMassFragment
{
	GENERATED_BODY()

	/** Its slot around the target, on the navmesh. */
	FVector Slot = FVector::ZeroVector;

	/** Distance to the slot at the last step of the final leg, to tell a unit that no longer gets closer. */
	float LastSlotDistance = 0.f;

	/** Sideways place (cm, to the right of the march) along its cluster's corridor, so the units do not walk in a file. */
	float Lateral = 0.f;

	/** Order the unit follows: a path computed for an older order is not applied to it. */
	uint32 OrderSerial = 0;

	/** Steps of the final leg without getting closer to the slot (pushed by others, or the slot taken). */
	uint16 StuckSteps = 0;

	EHOLMovePhase Phase = EHOLMovePhase::None;
};

/** Where a gatherer is in its round trip (J5, D296). */
UENUM()
enum class EHOLGatherPhase : uint8
{
	/** Not gathering: idle, or walking where a player sent it (its load kept). */
	None,
	/** Walking to its spot at its source. */
	ToSource,
	/** At its source, taking from it. */
	Gathering,
	/** Carrying its load to the nearest deposit. */
	ToDeposit,
};

/** A unit's gathering (J5, D296), on the server: its source, its load, its place in the round trip. */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLGatherFragment : public FMassFragment
{
	GENERATED_BODY()

	/**
	 * The source it gathers (an index of UHOLSourceSubsystem), INDEX_NONE: none. Kept while it carries its load to the
	 * deposit, so it walks back to the same source; INDEX_NONE on the way to the deposit: it stands there once unloaded.
	 */
	int32 SourceIndex = INDEX_NONE;

	/** Where it walks: its spot at the source, or against the deposit. */
	FVector Spot = FVector::ZeroVector;

	/** Where its current walk began, and how far it has walked since (hol.Gather.LogWalks: detours, D308). */
	FVector WalkStart = FVector::ZeroVector;
	FVector LastLocation = FVector::ZeroVector;
	float Walked = 0.f;
	/** Farthest it went from the straight line to Spot, and the steps its walk took (hol.Gather.LogWalks). */
	float MaxLateral = 0.f;
	uint16 WalkSteps = 0;

	/** Its move order's serial when it gave itself the walk to Spot: a new serial, then no order, is the arrival. */
	uint32 OrderSerialBefore = 0;

	/** What it carries; kept by a player's move order, lost when it gathers another resource (D296). */
	FHOLGatherLoad Load;

	/** Arrivals in a row that missed their spot (taken, or pushed away). */
	uint8 Misses = 0;

	/** The walk to Spot is asked for (it starts at the next step, through the order queue). */
	bool bWalkIssued = false;

	/**
	 * It started gathering this step (D309): having walked through every unit to its spot, it pushes a standing unit it
	 * landed on apart, down to the spots' spacing (UHOLUnitSeparationProcessor: a gatherer beside it on its own spot is
	 * left); the next step clears it.
	 */
	bool bSettling = false;

	EHOLGatherPhase Phase = EHOLGatherPhase::None;

	bool IsGathering() const { return Phase != EHOLGatherPhase::None; }
};

/** Per-entity state of a unit (D222), the values that change: identifier, health, activity. */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLUnitStateFragment : public FMassFragment
{
	GENERATED_BODY()

	/** Stable identifier (D223), attributed by the server and never recycled during a match. */
	UPROPERTY()
	uint32 UnitId = 0;

	UPROPERTY()
	int32 CurrentHealth = 0;

	UPROPERTY()
	EHOLUnitActivity Activity = EHOLUnitActivity::Idle;

	/** Player the unit belongs to (D233): a conversion (D37) rewrites it. The client turns it into a colour. */
	UPROPERTY()
	uint8 OwnerPlayer = 0;
};
