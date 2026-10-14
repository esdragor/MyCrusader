// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityQuery.h"
#include "MassProcessor.h"
#include "HOLSimulationProcessors.generated.h"

/** First processor of each fixed step: keeps the transform the presentation interpolates from. */
UCLASS()
class HEROESOFLIBERTY_API UHOLStorePreviousTransformProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHOLStorePreviousTransformProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};

/**
 * A unit stands or walks at its full speed, nothing in between (the user's trial of J4, D290): Mass' steering eases
 * in, slows down over the last second of the way and overshoots its goal, then the standing steering brings it back
 * slowly. Between the force and the movement, a moving unit's velocity keeps the direction the steering, the
 * avoidance and the navmesh boundary gave it, at the full speed of its order; near its slot it walks straight to it
 * and its last step ends exactly there. A standing unit does not move at all.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLFullSpeedProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHOLFullSpeedProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
