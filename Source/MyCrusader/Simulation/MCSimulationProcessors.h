// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityQuery.h"
#include "MassProcessor.h"
#include "MCSimulationProcessors.generated.h"

/** First processor of each fixed step: keeps the transform the presentation interpolates from. */
UCLASS()
class MYCRUSADER_API UMCStorePreviousTransformProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMCStorePreviousTransformProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};

/**
 * Stress test behaviour (M1 trial): entities with FMCWanderTag walk to random reachable navmesh points.
 * Navmesh queries are synchronous and budgeted per step.
 */
UCLASS(config = Mass)
class MYCRUSADER_API UMCWanderProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMCWanderProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	/** New paths requested per step at most. */
	UPROPERTY(config, EditDefaultsOnly, Category = "Wander")
	int32 MaxPathRequestsPerStep = 100;

	/** Destinations are picked within this radius of the entity. */
	UPROPERTY(config, EditDefaultsOnly, Category = "Wander", meta = (ForceUnits = "cm"))
	float WanderRadius = 6000.f;

	FMassEntityQuery EntityQuery;
};
