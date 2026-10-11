// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityQuery.h"
#include "MassProcessor.h"
#include "MCUnitOrderProcessor.generated.h"

/**
 * The RTS layer's step (D226), first of the Tasks group: applies the orders queued since the last step (the
 * unit walks straight to its destination, no navmesh path before M3, D228) and stands a unit down once it has
 * reached the goal of its order, which is the walk/stand state of D226.
 */
UCLASS()
class MYCRUSADER_API UMCUnitOrderProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMCUnitOrderProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	/** Applies the queued orders: move target, speed of the type, activity, action. */
	void ApplyQueuedOrders(FMassEntityManager& EntityManager);
	/** The units that reached the goal of their order stand down (activity Idle, action Stand). */
	void StopArrivedUnits(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

	FMassEntityQuery EntityQuery;
};
