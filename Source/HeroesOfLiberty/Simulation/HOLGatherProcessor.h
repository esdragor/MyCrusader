// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityQuery.h"
#include "MassProcessor.h"
#include "Match/HOLResources.h"
#include "Player/HOLPlayerController.h"
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
 * where the owner's stock rises, and goes back to the same source. A used-up source sends it to the nearest source of
 * its massif with a free spot (D309), or leaves it idle (after unloading what it carries). Its walks are move orders it gives
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
		/** Gatherers that found no free spot at their source nor anywhere in its massif or its nearby groves: idle (D309). */
		int64 NoSpotStops = 0;
		/** Gatherers sent to a nearby grove, their massif full (D309 point 6). */
		int64 NearbyGroves = 0;
	};
	static const FCounters& GetCounters() { return Counters; }

	/**
	 * Distance (cm, centre to centre) between two gatherers' spots, at one source or two side by side (hol.Gather.SpotSpacing,
	 * D309): 100 cm, two gatherers' bodies (60 cm radius) at their spots overlap a little (the user's trial, after AoE IV). The
	 * separation keeps that distance between a gatherer at its source and a unit standing beside it.
	 */
	static double GetSpotSpacing();

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

	/**
	 * Walks to a free spot at its source, or at the nearest source of its massif with one, or of a nearby grove; otherwise
	 * stands idle in front (D309).
	 */
	void WalkToSource(const FStepContext& Step, FGatherer& Gatherer);
	/** Walks to its owner's nearest deposit that takes what it carries (a second try slides along the wall). */
	void WalkToDeposit(const FStepContext& Step, FGatherer& Gatherer);
	/**
	 * Its source (Gather.SourceIndex) is gone: the nearest source of its massif with a free spot (D309); all of them full,
	 * it stands idle in front of the nearest one standing; none standing, it unloads and stands.
	 */
	void OnSourceGone(const FStepContext& Step, FGatherer& Gatherer);
	/** Unloads at the deposit: the owner's stock rises (D296). */
	void Unload(const FStepContext& Step, FGatherer& Gatherer);
	void Stop(FGatherer& Gatherer);
	/**
	 * No room at its source nor anywhere in its massif or its nearby groves (D309): it stops gathering and walks in front of its
	 * source, where it stands idle on a point of its own (FindStandPoint).
	 */
	void StandBeforeSource(const FStepContext& Step, FGatherer& Gatherer, const FHOLSource& Source);
	/**
	 * A free point in front of a full source for Self to stand idle on, the side facing From first, claimed: rows beyond
	 * the gatherers' spots, never on a spot of this source or of one beside it, never closer to another claim than two
	 * gatherers stand (D309, the user's 3rd trial of J5b.2); row after row farther out while none is free (point 6: two
	 * idle gatherers never on one point). False if none is free nor on the navmesh, MaxStandRows out.
	 */
	bool FindStandPoint(const FStepContext& Step, const FHOLSource& Source, const FVector& From, float BodyRadius, FMassEntityHandle Self, FVector& OutPoint);
	/**
	 * Its activity from its place in the round trip (D307): gathering at its source, with the source's resource (J5c.1,
	 * sent with the activity), else on its round trip.
	 */
	void SyncActivity(const FStepContext& Step, FGatherer& Gatherer);
	/** Counts it in its owner's gatherers of its resource (J5b, D307). */
	void CountGatherer(const FStepContext& Step, const FGatherer& Gatherer);
	/** Each player's gatherers by resource, to its player state (the top bar of its HUD). */
	void PublishGathererCounts(const FStepContext& Step) const;
	/** Each player's unloads of the step, to its own controller alone: its « +10 » (J5c.3, D298). */
	void SendDepositPopups(const FStepContext& Step);
	/** It reached its source: it gathers there (its activity shows it, J5, D298). */
	void StartGathering(FGatherer& Gatherer);
	void IssueWalk(const FStepContext& Step, FGatherer& Gatherer, const FVector& Spot);

	/**
	 * A free spot touching the source, on the navmesh, the side facing Deposit first (with none, the side facing From),
	 * claimed for Self where it will stand: two gatherers' spots, at one source or two, are never closer than
	 * GetSpotSpacing (D309).
	 */
	bool FindSourceSpot(const FStepContext& Step, int32 SourceIndex, const FVector& From, float BodyRadius, FMassEntityHandle Self, FVector& OutSpot,
		const FHOLGatherDeposit* Deposit);
	/** Another gatherer's spot, or idle gatherer's point, within MinSpacing of Spot, whatever its source (Self's old one is left). */
	bool IsSpotTaken(const FStepContext& Step, const FVector2D& Spot, double MinSpacing, FMassEntityHandle Self);
	/**
	 * The nearest standing source of FromSource's massif with a free spot, from FromSource outward (FromSource itself left
	 * out), its spot claimed for the gatherer in OutSpot (D309, the user's rule: 175 Peasants sent to a wood surround it);
	 * the whole massif full, the first one with a free spot in its nearby groves (D309 point 6). INDEX_NONE if they are all
	 * full; OutNearestStanding, when given, then holds the first standing one met in FromSource's own massif.
	 */
	int32 FindMassifSpot(const FStepContext& Step, const FGatherer& Gatherer, int32 FromSource, FVector& OutSpot, int32* OutNearestStanding = nullptr);
	/** The massifs of the server's sources, linked again only when sources were added (D309). */
	const FHOLSourceMassifs& GetMassifs(const FStepContext& Step);
	/** Index in Deposits of the owner's nearest deposit taking Resource, INDEX_NONE if it has none. */
	int32 FindDeposit(uint8 OwnerPlayer, EHOLResource Resource, const FVector& From) const;
	/** The footprint of the owner's deposit nearest Source taking its resource, where its gatherers unload; null if none. */
	const FHOLGatherDeposit* FindSourceDeposit(uint8 OwnerPlayer, const FHOLSource& Source) const;

	TArray<FDeposit> Deposits;
	/** The spots held: their gatherers still walk to them or gather there; and the points of the idle ones in front of a full source. */
	FHOLSpotClaims SpotClaims;
	/** The sources' massifs: links built once for the sources registered (D309), and their nearby groves (point 6). */
	FHOLSourceMassifs Massifs;
	/** The hol.Gather.NearbyMassifRange the massifs were built with (cm). */
	double MassifsNearbyRange = -1.;
	/**
	 * Sources found with no free spot during this step, by a massif's search: not tried again before the next step (175
	 * Peasants ordered to a wood at once look through the trees filled before them once, not each). Another gatherer's
	 * own source is never skipped this way: its own claim may be there.
	 */
	TBitArray<> FullSources;
	/** This step's gatherers by owner (index) and resource. */
	TArray<FHOLResources> GathererCounts;
	/** This step's unloads by owner (index), MaxDepositPopupsPerStep each at most: their « +10 » (J5c.3). */
	TArray<TArray<FHOLDepositPopup>> DepositPopups;
	static FCounters Counters;

	FMassEntityQuery EntityQuery;
};
