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

/** Stress test behaviour: walk to random reachable points, forever (M1 trial). */
USTRUCT()
struct MYCRUSADER_API FMCWanderTag : public FMassTag
{
	GENERATED_BODY()
};
