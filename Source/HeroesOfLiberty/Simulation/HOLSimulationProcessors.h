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
 *
 * Every step, a walker looks ahead for the bodies it would meet on its way within a short time and turns aside from
 * them, the smallest turn first (D293): Mass' avoidance runs one step out of 6 (D220) and turned too late, the walker
 * pushed through. When no way within the largest turn is clear (a crowd, its slot behind other units), the detour is
 * too long: it goes on and pushes its way (UHOLUnitSeparationProcessor, D292).
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

	/** Takes the units' positions, velocities and radii into a hashed grid; false when no unit walks (nothing to do). */
	bool BuildLookAhead(FMassExecutionContext& Context);
	static int32 LookAheadBucketOf(const FIntPoint& Cell);
	/** The way a walker takes: Direction if nothing is met on it within Horizon (s), else the smallest clear turn, else Direction. */
	FVector AvoidBodies(FMassEntityHandle Entity, const FVector& Location, float Radius, const FVector& Direction, float Speed, double Horizon) const;

	FMassEntityQuery EntityQuery;

	/** The step's units before they move (kept between steps to spare the allocations). */
	TArray<FMassEntityHandle> Entities;
	TArray<FVector2D> Positions;
	TArray<FVector2D> Velocities;
	TArray<float> Radii;
	float MaxRadius = 1.f;
	float MaxSpeed = 0.f;
	/** Cells of the positions, hashed into buckets: first unit of each bucket, the next unit of the same bucket, each unit's cell. */
	TArray<int32> CellHeads;
	TArray<int32> NextInCell;
	TArray<FIntPoint> Cells;
};

/**
 * No body walks through another (the user's trial of J4, D292): at full speed the avoidance turns too late, and a
 * standing unit no longer steps aside (D290). Once the units have moved, every overlap a walker takes part in is
 * undone, each unit of the pair taking half: a walker pushes a standing unit out of its way (as in AoE) and the pushed
 * unit moves no further, two walkers part. Standing units among themselves are left as they are. Pairs found in a
 * hashed grid of the step's positions, one pass a step.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLUnitSeparationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHOLUnitSeparationProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;

	/** The step's units, in the order the query visits them (kept between steps to spare the allocations). */
	TArray<FVector2D> Positions;
	TArray<FVector2D> Corrections;
	TArray<float> Radii;
	TArray<bool> Walking;
	/** Units that looked for their neighbours this step, and those to look (the walkers, then the units they pushed). */
	TArray<bool> Looked;
	TArray<bool> Queued;
	TArray<int32> ToLook;
	/** Cells of the positions, hashed into buckets: first unit of each bucket, the next unit of the same bucket, each unit's cell. */
	TArray<int32> CellHeads;
	TArray<int32> NextInCell;
	TArray<FIntPoint> Cells;
};
