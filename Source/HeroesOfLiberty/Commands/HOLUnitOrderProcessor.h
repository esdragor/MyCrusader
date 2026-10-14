// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityQuery.h"
#include "MassProcessor.h"
#include "HOLUnitOrderProcessor.generated.h"

class ANavigationData;
class UNavigationSystemV1;

/**
 * The RTS layer's step (D226), first of the Tasks group, on the server (and in solo). A move order (D286, D287):
 * its units are given their slots around the target and cut into clusters of neighbours; each cluster gets one
 * navmesh path, within a budget of path requests per step, turned into a corridor its units share, each one walking
 * at its own sideways place along it (Mass' short path follow); at the end of the path, each unit walks straight to
 * its slot and stands there.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLUnitOrderProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHOLUnitOrderProcessor();

	/** Path requests served since the start, and waiting now (for reports). */
	int64 GetNumPathsServed() const { return NumPathsServed; }
	int32 GetNumPathsWaiting() const { return PendingPaths.Num(); }

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	/** The orders queued since the last step: slots given, units waiting for their cluster's path. */
	void ApplyQueuedOrders(FMassEntityManager& EntityManager, const UNavigationSystemV1* NavSystem, const ANavigationData* NavData);
	/** Path requests of the waiting clusters, oldest first, up to the step's budget. */
	void ServePathRequests(FMassEntityManager& EntityManager, UNavigationSystemV1* NavSystem, const ANavigationData* NavData);
	/** Next part of a path, then the final leg to the slot, then standing (activity Idle, action Stand). */
	void AdvanceUnits(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

	/** One cluster of an order waiting for its path. */
	struct FPathRequest
	{
		uint32 OrderSerial = 0;
		FVector Start = FVector::ZeroVector;
		FVector Target = FVector::ZeroVector;
		TArray<FMassEntityHandle> Entities;
	};
	TArray<FPathRequest> PendingPaths;

	uint32 NextOrderSerial = 1;
	int64 NumPathsServed = 0;

	FMassEntityQuery EntityQuery;
};
