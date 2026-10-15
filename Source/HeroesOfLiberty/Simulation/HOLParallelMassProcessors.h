// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Avoidance/MassAvoidanceProcessors.h"
#include "SmoothOrientation/MassSmoothOrientationProcessors.h"
#include "Steering/MassSteeringProcessors.h"
#include "HOLParallelMassProcessors.generated.h"

/*
 * Mass processors whose chunks run in parallel (D302): Mass runs them one after the other on one core. Each unit only
 * writes its own fragments and reads nothing these processors write on another unit: the results are the same in any
 * order. Mass' own Execute runs first on a copy of the context whose chunk queue keeps each query's per-chunk
 * function and runs nothing; each query then runs that function over its chunks in parallel, in the same order.
 * hol.Simulation.ParallelMass 0 runs them as Mass does, for A / B measures.
 */

/** Moving avoidance: 7.6 ms a run at 2 000 walkers in S2 on one core (one step out of 6, D220). */
UCLASS()
class HEROESOFLIBERTY_API UHOLParallelMovingAvoidanceProcessor : public UMassMovingAvoidanceProcessor
{
	GENERATED_BODY()

public:
	UHOLParallelMovingAvoidanceProcessor();

protected:
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};

/** Steering towards the move target. */
UCLASS()
class HEROESOFLIBERTY_API UHOLParallelSteerToMoveTargetProcessor : public UMassSteerToMoveTargetProcessor
{
	GENERATED_BODY()

public:
	UHOLParallelSteerToMoveTargetProcessor();

protected:
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};

/** Orientation smoothed after the movement. */
UCLASS()
class HEROESOFLIBERTY_API UHOLParallelSmoothOrientationProcessor : public UMassSmoothOrientationProcessor
{
	GENERATED_BODY()

public:
	UHOLParallelSmoothOrientationProcessor();

protected:
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};
