// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityQuery.h"
#include "MassProcessor.h"
#include "HOLInterpolatedInstancesProcessor.generated.h"

/**
 * Per-frame replacement of Mass' instance updates (UMassUpdateISMProcessor, UMassUpdateInstancedSkinnedMeshProcessor
 * and UMassConsumeInstancedSkinnedMeshAnimationProcessor, disabled in DefaultMass.ini): instances are drawn between
 * the two last fixed-step transforms, so 20 Hz movement looks smooth at any frame rate (D217).
 *
 * Skinned instances (D232) also get, in the same pass, the animation chosen from the drawn speed (D234), the activity
 * and the load (J5c.1, HOLUnitAnimation::Choose; a gatherer drawn facing its source, a builder its site, J6a.4), the prop
 * shown (J5c.2) and the colour of their owner (D233): Mass pairs a skinned instance's transform, animation and custom
 * data by the order they are added in the frame, so they must come from one loop.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLInterpolatedInstancesProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHOLInterpolatedInstancesProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
