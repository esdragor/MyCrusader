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
 *
 * The bodies a displaying machine keeps (J7c, bodies kept 30 s, HOLCorpse) are drawn in a second pass, the same way: their
 * fall held at its last frame, then sunk; an expired body is destroyed here (deferred).
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
	/** The bodies drawn (J7c, bodies kept 30 s): their held fall, sunk at the end (HOLCorpse). */
	FMassEntityQuery CorpseQuery;
	/** Every body, seen or not (no visibility filter), so an expired one goes even off screen. */
	FMassEntityQuery CorpseExpiryQuery;

	/** Each slot's colour index, gathered from the actors at most twice a second (FSlotColors), and when. */
	TArray<uint8> CachedColorIndices;
	double ColorsRefreshedAt = -1.0;
};
