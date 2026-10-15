// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityQuery.h"
#include "MassProcessor.h"
#include "Simulation/HOLGatherRules.h"
#include "HOLGatherProcessor.generated.h"

class ANavigationData;
class UHOLCommandSubsystem;
class UHOLProductionSubsystem;
class UHOLSourceSubsystem;
class UHOLUnitSubsystem;
class UNavigationSystemV1;
struct FHOLGatherFragment;
struct FHOLMoveOrderFragment;
struct FHOLSource;
struct FHOLUnitStateFragment;
struct FHOLUnitTypeConstants;

/**
 * The gathering (J5, D296), on the server (and in solo), right after the orders of the step: a gatherer walks to a free
 * spot touching its source, takes from it at its rate until it carries its load, walks to its owner's nearest deposit,
 * where the owner's stock rises, and goes back to the same source. A used-up source sends it to the nearest one of the
 * same resource within 15 m, or leaves it idle (after unloading what it carries). Its walks are move orders it gives
 * itself through the order queue (the paths, slots and arrivals of J4); a player's move order ends the gathering, the
 * load kept.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLGatherProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHOLGatherProcessor();

	/** What every gathering did since the program started (stress test S7): walks asked for, arrivals that missed. */
	struct FCounters
	{
		int64 WalksToSource = 0;
		int64 WalksToDeposit = 0;
		int64 MissedSpots = 0;
		int64 MissedDeposits = 0;
		int64 Unloads = 0;
		int64 NoSpotWaits = 0;
	};
	static const FCounters& GetCounters() { return Counters; }

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	/** What one step works with. */
	struct FStepContext
	{
		FMassEntityManager* EntityManager = nullptr;
		UHOLSourceSubsystem* Sources = nullptr;
		UHOLCommandSubsystem* Commands = nullptr;
		UHOLUnitSubsystem* Units = nullptr;
		UHOLProductionSubsystem* Production = nullptr;
		UNavigationSystemV1* NavSystem = nullptr;
		const ANavigationData* NavData = nullptr;
		float StepSeconds = 0.05f;
	};

	/** A deposit of the match: its footprint, its owner, what it takes. */
	struct FDeposit
	{
		FHOLGatherDeposit Footprint;
		float GroundZ = 0.f;
		uint8 OwnerPlayer = 0;
		TArray<EHOLResource> Accepts;
	};

	/** A gatherer's spot at a source, kept while it walks there and gathers. */
	struct FSpotClaim
	{
		FMassEntityHandle Entity;
		FVector2D Spot = FVector2D::ZeroVector;
	};

	/** The gatherer met by the step, and what it needs. */
	struct FGatherer
	{
		FMassEntityHandle Entity;
		FHOLGatherFragment& Gather;
		const FHOLMoveOrderFragment& Move;
		FHOLUnitStateFragment& State;
		FVector Location;
		uint8 OwnerPlayer = 0;
		const FHOLUnitTypeConstants& Constants;
	};

	void RefreshDeposits(UWorld& World);
	void ApplyGatherOrders(const FStepContext& Step);
	void Advance(const FStepContext& Step, FGatherer& Gatherer);

	/** Walks to a free spot at its source, or at a neighbour of the same resource; otherwise waits, or stops. */
	void WalkToSource(const FStepContext& Step, FGatherer& Gatherer);
	/** Walks to its owner's nearest deposit that takes what it carries (a second try slides along the wall). */
	void WalkToDeposit(const FStepContext& Step, FGatherer& Gatherer);
	/** Its source is gone: the nearest standing one of the same resource within 15 m, or unload and stand. */
	void OnSourceGone(const FStepContext& Step, FGatherer& Gatherer, const FVector& GoneLocation);
	/** Unloads at the deposit: the owner's stock rises (D296). */
	void Unload(const FStepContext& Step, FGatherer& Gatherer);
	void Stop(FGatherer& Gatherer);
	/** It reached its source: it gathers there (its activity shows it, J5, D298). */
	void StartGathering(FGatherer& Gatherer);
	void IssueWalk(const FStepContext& Step, FGatherer& Gatherer, const FVector& Spot);

	/** A free spot touching the source, on the navmesh, the side facing From first. */
	bool FindSourceSpot(const FStepContext& Step, int32 SourceIndex, const FVector& From, float BodyRadius, FMassEntityHandle Self, FVector& OutSpot);
	/** Standing sources of Resource within NeighbourSourceDistance of Location, nearest first (Exclude left out). */
	void FindNeighbours(const FStepContext& Step, EHOLResource Resource, const FVector& Location, int32 Exclude, TArray<int32>& OutSources) const;
	/** Index in Deposits of the owner's nearest deposit taking Resource, INDEX_NONE if it has none. */
	int32 FindDeposit(uint8 OwnerPlayer, EHOLResource Resource, const FVector& From) const;

	TArray<FDeposit> Deposits;
	TMap<int32, TArray<FSpotClaim>> SpotClaims;
	static FCounters Counters;

	FMassEntityQuery EntityQuery;
};
