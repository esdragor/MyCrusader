// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityQuery.h"
#include "MassProcessor.h"
#include "MCAvoidanceHoldProcessors.generated.h"

/**
 * The avoidance solver is the whole cost of a step at 2 000 units (D220), so it only runs one step out of
 * several - and an avoidance impulse applied one step in six and forgotten the next is not avoidance at all:
 * two units on a collision course walk through each other (measured 1.3 to 4.7 cm apart at one run in two,
 * three or six against 66.5 cm with the solver running every step). Mass does not keep the force either:
 * UMassApplyForceProcessor resets the force fragment to zero after applying it. These two processors keep the
 * last avoidance impulse alive between the solver's runs: continuity without paying for the solver (D230).
 */
UCLASS()
class MYCRUSADER_API UMCAvoidanceSnapshotProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMCAvoidanceSnapshotProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};

/** Re-applies the held avoidance impulse on the steps the solver does not run. */
UCLASS()
class MYCRUSADER_API UMCAvoidanceHoldProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMCAvoidanceHoldProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
