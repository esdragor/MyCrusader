// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MCSimulationFragments.generated.h"

/** Transform at the start of the last fixed step: the presentation interpolates from it to FTransformFragment. */
USTRUCT()
struct MYCRUSADER_API FMCPreviousTransformFragment : public FMassFragment
{
	GENERATED_BODY()

	FTransform Transform = FTransform::Identity;
};

/** Every light unit (D29): filters the presentation processors. */
USTRUCT()
struct MYCRUSADER_API FMCLightUnitTag : public FMassTag
{
	GENERATED_BODY()
};

/** What the RTS layer sees a unit doing (D226): standing, or walking to its current order. */
UENUM()
enum class EMCUnitActivity : uint8
{
	Idle,
	Moving,
};

/** Per-entity state of a unit (D222), the values that change: identifier, health, activity. */
USTRUCT()
struct MYCRUSADER_API FMCUnitStateFragment : public FMassFragment
{
	GENERATED_BODY()

	/** Stable identifier (D223), attributed by the server and never recycled during a match. */
	UPROPERTY()
	uint32 UnitId = 0;

	UPROPERTY()
	int32 CurrentHealth = 0;

	UPROPERTY()
	EMCUnitActivity Activity = EMCUnitActivity::Idle;
};
