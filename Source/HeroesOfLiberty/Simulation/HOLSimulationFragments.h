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

/** What the RTS layer sees a unit doing (D226): standing, or walking to its current order. */
UENUM()
enum class EHOLUnitActivity : uint8
{
	Idle,
	Moving,
};

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

	/** Its move order's serial when it gave itself the walk to Spot: a new serial, then no order, is the arrival. */
	uint32 OrderSerialBefore = 0;

	/** What it carries; kept by a player's move order, lost when it gathers another resource (D296). */
	FHOLGatherLoad Load;

	/** Steps to wait before looking for a spot again (none was free). */
	uint16 WaitSteps = 0;

	/** Arrivals in a row that missed their spot (taken, or pushed away). */
	uint8 Misses = 0;

	/** The walk to Spot is asked for (it starts at the next step, through the order queue). */
	bool bWalkIssued = false;

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
