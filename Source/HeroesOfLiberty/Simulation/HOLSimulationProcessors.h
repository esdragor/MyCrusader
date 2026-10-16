// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include <atomic>
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
 * avoidance and the navmesh boundary gave it, at the full speed of its order; on the final leg it walks straight to
 * its slot (D294) and its last step ends exactly there. A standing unit does not move at all.
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
 * No unit's body in a building (D310, J6a.1): once the units have moved and parted, a body in a footprint is put back
 * against its wall, out by the nearest way. The navmesh keeps the walkers' paths the clearance away
 * (UHOLBuildingSettings::BodyClearance); what pushed bodies in was the separation of a crowd standing against a wall
 * (D292: 19 Peasants sent onto the main centre stood inside it, up to 4.6 m deep), the look-ahead's turns (D293), and a
 * final leg to a slot by the wall. The same rule for every building, each unit's own radius.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLFootprintKeepOutProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHOLFootprintKeepOutProcessor();

	/** Bodies put back out of a footprint since the program started (hol.Nav.Footprints report). */
	static int64 GetNumPushedOut() { return NumPushedOut.load(); }

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;

	/** A footprint of the step, in its own frame. */
	struct FFootprint
	{
		FVector2D Centre = FVector2D::ZeroVector;
		FVector2D HalfExtent = FVector2D::ZeroVector;
		/** Its yaw's cosine and sine. */
		double Cos = 1.;
		double Sin = 0.;
		/** Where a body can touch it: its bounds widened by the largest body. */
		FBox2D Reach;
	};
	TArray<FFootprint> Footprints;
	/**
	 * The footprints each cell of a grid over their reaches may concern (S7: 32 buildings), a unit tried against these
	 * alone: cell C's are CellFootprints[CellStarts[C]] to CellFootprints[CellStarts[C + 1]].
	 */
	FBox2D GridBounds;
	FIntPoint GridSize = FIntPoint::ZeroValue;
	TArray<int32> CellStarts;
	TArray<int32> CellFootprints;

	static std::atomic<int64> NumPushedOut;
};

/**
 * Orientation, instant (user's choice at the J5a trial, as D290 made the speed): a walking unit faces the way it walks,
 * from the step it starts or turns; a unit at rest keeps its facing (pushed aside, it does not turn). Replaces Mass'
 * smoothed orientation, which blended the walk with the path's direction and turned in ~0.3 s.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLInstantOrientationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHOLInstantOrientationProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};

/**
 * No body walks through another (the user's trial of J4, D292): at full speed the avoidance turns too late, and a
 * standing unit no longer steps aside (D290). Once the units have moved, every overlap a walker takes part in is
 * undone, each unit of the pair taking half: a walker pushes a standing unit out of its way (as in AoE) and the pushed
 * unit moves no further, two walkers part. Standing units among themselves are left as they are, but for a gatherer
 * that walked through every unit to its spot (D309): landed, it parts from a unit standing there. A gatherer at its
 * source and a unit standing beside it part down to the spots' spacing only (100 cm: gatherers pressed against a wood's
 * edge overlap a little, the user's trial of D309). Pairs found in a hashed grid of the step's positions, one
 * pass a step.
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
	/** The walkers, and the gatherers that landed at their spot this step (D309): the units that look for overlaps. */
	TArray<bool> Walking;
	/** Gatherers on their round trip (D309): they pass through every unit and none pushes them. */
	TArray<bool> Ghosts;
	/** Units walking an order (not a gatherer settling at its spot). */
	TArray<bool> Moving;
	/**
	 * Gatherers at their source (D309): with a unit standing beside them, they keep the spots' spacing only (100 cm,
	 * hol.Gather.SpotSpacing), not two bodies; a walker still pushes them out of its way.
	 */
	TArray<bool> AtSource;
	/** Units that looked for their neighbours this step, and those to look (the walkers, then the units they pushed). */
	TArray<bool> Looked;
	TArray<bool> Queued;
	TArray<int32> ToLook;
	/** Cells of the positions, hashed into buckets: first unit of each bucket, the next unit of the same bucket, each unit's cell. */
	TArray<int32> CellHeads;
	TArray<int32> NextInCell;
	TArray<FIntPoint> Cells;
};
