// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Avoidance/MassAvoidanceProcessors.h"
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

/*
 * Steering towards the move target. Mass' per-unit code draws each standing unit's target selection cooldown with
 * FMath::RandRange, the C runtime's global random state, shared by the whole game and called in an order that depends
 * on the threads: run in parallel chunks, the same battle gave different results from one run to the next (J7c.4, S4).
 * The per-unit code is copied from UMassSteerToMoveTargetProcessor::Execute (Engine/Plugins/AI/MassAI/Source/
 * MassNavigation/Private/MassSteeringProcessors.cpp, UE 5.8): re-copy it if the engine changes. The two draws there
 * become a value in the same range from a hash of the unit's UnitId, the fixed step's index and the call site.
 * hol.Simulation.DeterministicSteering 0 runs Mass' own Execute, for A / B measures.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLParallelSteerToMoveTargetProcessor : public UMassSteerToMoveTargetProcessor
{
	GENERATED_BODY()

public:
	UHOLParallelSteerToMoveTargetProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	/** Mass' query (EntityQuery), the same requirements, plus the unit's state (read only) for its UnitId. */
	FMassEntityQuery SteerQuery;
};

/*
 * Standing avoidance (J7c.4, S3): 9-10 ms a run at 2 000 units in melee on one core. Unlike the two above, each standing
 * unit reads its neighbours' ghosts, written in the same pass (Gauss-Seidel): a plain parallel chunk run would change
 * the results. The same results, bit for bit, in three phases: A lists the standing units in Mass' visiting order;
 * B (parallel, read-only) finds and sorts each one's close neighbours, reading nothing this pass writes; C, one after
 * the other in the order of A, computes the forces reading the neighbours' ghosts live and writes each unit's ghost,
 * as Mass' single loop does. The per-unit code is copied from UMassStandingAvoidanceProcessor::Execute
 * (Engine/Plugins/AI/MassAI/Source/MassNavigation/Private/MassAvoidanceProcessors.cpp, UE 5.8): re-copy it if the
 * engine changes. hol.Simulation.ParallelStandingAvoidance 0 runs Mass' own Execute, for A / B measures.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLParallelStandingAvoidanceProcessor : public UMassStandingAvoidanceProcessor
{
	GENERATED_BODY()

public:
	UHOLParallelStandingAvoidanceProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	/** A copy of Mass' query (UMassStandingAvoidanceProcessor::EntityQuery is private), the same requirements. */
	FMassEntityQuery StandingQuery;
};
