// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Commands/HOLCommandSubsystem.h"
#include "CoreMinimal.h"
#include "MassEntityQuery.h"
#include "MassProcessor.h"
#include "HOLUnitOrderProcessor.generated.h"

class ANavigationData;
class UNavigationSystemV1;
struct FHOLMoveOrderFragment;
struct FMassNavMeshCachedPathFragment;
struct FNavCorridor;
struct FNavigationPath;

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
	/** Path requests served by every order processor since the program started (stress test S7: paths per second). */
	static int64 GetTotalPathsServed() { return TotalPathsServed; }
	/** Task walks given a kept path instead of a new request since the program started (D305, stress test S7). */
	static int64 GetTotalPathsReused() { return TotalPathsReused; }
	/** New paths of walkers a building kept putting back since the program started (D310, stress test S7). */
	static int64 GetTotalBuildingRepaths() { return TotalBuildingRepaths; }
	/** Walkers given a new path because a new footprint cut their way, since the program started (J6b.2, D314, stress test S7). */
	static int64 GetTotalFootprintRepaths() { return TotalFootprintRepaths; }
	/** Kept paths of the task walks dropped because a new footprint cut them, since the program started (J6b.2, D314). */
	static int64 GetTotalFootprintDroppedPaths() { return TotalFootprintDroppedPaths; }
	int32 GetNumPathsWaiting() const { return PendingPaths.Num(); }

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	/** The orders queued since the last step: slots given, units waiting for their cluster's path. */
	void ApplyQueuedOrders(FMassEntityManager& EntityManager, const UNavigationSystemV1* NavSystem, const ANavigationData* NavData);
	/** Path requests of the waiting clusters, oldest first, up to the step's budget. */
	void ServePathRequests(FMassEntityManager& EntityManager, UNavigationSystemV1* NavSystem, const ANavigationData* NavData);
	/** Next part of a path, then the final leg to the slot, then standing (activity Idle, action Stand). */
	void AdvanceUnits(FMassEntityManager& EntityManager, FMassExecutionContext& Context, const ANavigationData* NavData);

	/** One cluster of an order waiting for its path. */
	struct FPathRequest
	{
		uint32 OrderSerial = 0;
		FVector Start = FVector::ZeroVector;
		FVector Target = FVector::ZeroVector;
		TArray<FMassEntityHandle> Entities;
		/** A task's walk of one unit (J5: to its source, to the deposit): its path may be a kept one (D305). */
		bool bReusable = false;
		/**
		 * A new path because a building is in the way (D310: put back at its wall; J6b.2: a new footprint across it): never
		 * straight to the slot out of a pocket of the navmesh (D308), which would walk into that building again.
		 */
		bool bForBuilding = false;
	};
	TArray<FPathRequest> PendingPaths;

	uint32 NextOrderSerial = 1;
	int64 NumPathsServed = 0;
	static int64 TotalPathsServed;
	static int64 TotalPathsReused;
	static int64 TotalBuildingRepaths;
	static int64 TotalFootprintRepaths;
	static int64 TotalFootprintDroppedPaths;

	/**
	 * Paths of the task walks (D305), kept for the next walk between the same points (a gatherer goes back and forth
	 * between the same spots): by the cells of ReuseCellSize of their start and their end. No path: the walk was
	 * straight. Forgotten when the navmesh changes across them, or when no walk took them for a while.
	 */
	struct FReusedPath
	{
		TSharedPtr<FNavigationPath, ESPMode::ThreadSafe> Path;
		TSharedPtr<FNavCorridor> Corridor;
		FBox2D Bounds;
		/** The walk that found it, from its start to its target: a straight one has no other point (J6b.2). */
		FVector2D From = FVector2D::ZeroVector;
		FVector2D To = FVector2D::ZeroVector;
		uint32 LastUsedStep = 0;
	};
	TMap<FIntVector4, FReusedPath> ReusedPaths;
	/** Areas where the navmesh changed lately: its tiles are rebuilt a little later, a path found meanwhile is not kept. */
	struct FRecentNavChange
	{
		FBox2D Area;
		uint32 UntilStep = 0;
	};
	TArray<FRecentNavChange> RecentNavChanges;
	uint32 StepCount = 0;
	static FIntVector4 GetReuseKey(const FVector& Start, const FVector& Target);
	/** The navmesh changes of the last step: the kept paths across them are forgotten. */
	void ForgetChangedPaths(UHOLCommandSubsystem& Commands);

	/**
	 * A footprint that started blocking the navmesh (J6b.2, D314: a site's first work, a building placed finished by the
	 * code), waiting until its tiles are seen rebuilt: no path found across it meanwhile is kept, then the walkers whose way
	 * crosses it get a new path (AdvanceUnits).
	 */
	struct FNewFootprint
	{
		FHOLBlockedFootprint Building;
		/** Its bounds on the ground, and the points under it that leave the navmesh once its tiles are rebuilt. */
		FBox2D Bounds = FBox2D(ForceInit);
		TArray<FVector2D> Probes;
		uint32 SinceStep = 0;
		/** Kept paths dropped for it (at once), walkers given a new path and destinations moved out of it (once rebuilt). */
		int32 DroppedPaths = 0;
		int32 Walkers = 0;
		int32 MovedSlots = 0;
		bool bRebuilt = false;
	};
	TArray<FNewFootprint> NewFootprints;
	/** Those whose tiles are rebuilt this step: AdvanceUnits checks the walkers against them, then they are logged. */
	TArray<FNewFootprint> ReadyFootprints;
	/** The new footprints of the last step taken in; those whose tiles are rebuilt moved to ReadyFootprints. */
	void UpdateNewFootprints(UHOLCommandSubsystem& Commands, const UNavigationSystemV1* NavSystem, const ANavigationData* NavData);
	/** A walker whose way crosses a ready footprint, given a new path after the units' pass (grouped by order, J4.3). */
	struct FCutWalker
	{
		FMassEntityHandle Entity;
		uint32 OrderSerial = 0;
		FVector Location = FVector::ZeroVector;
		FVector Slot = FVector::ZeroVector;
		/** The footprint that cuts its way (an index of ReadyFootprints), and its body's radius. */
		int32 Footprint = INDEX_NONE;
		float Radius = 0.f;
	};
	TArray<FCutWalker> CutWalkers;
	/** The way of the walker being checked (kept between walkers: no allocation per unit). */
	TArray<FVector2D> CutWay;
	/**
	 * A walker at Location (on its path or its final leg) whose way crosses a ready footprint: added to CutWalkers and to
	 * the footprint's count. A body already against the footprint is left to the footprints' guard (D310).
	 */
	bool IsWayCut(const FHOLMoveOrderFragment& Order, const FVector& Location, const FMassNavMeshCachedPathFragment& CachedPath, float Radius, FMassEntityHandle Entity);
	/** The new paths of the cut walkers, one per cluster of each order, as an order's (D286); a slot under the footprint moved out. */
	void RequestCutPaths(FMassEntityManager& EntityManager, const UNavigationSystemV1* NavSystem, const ANavigationData* NavData);
	/** The straight way from Start to Target crosses a building that blocks the navmesh (with the margin of D310). */
	static bool DoesCrossBuilding(const UWorld& World, const FVector& Start, const FVector& Target, const ANavigationData* NavData);

	/**
	 * Slots given to units still on their way, by cell of ClaimCellSize: a later order does not give them again (units
	 * sent one by one to a rally point stand side by side). A claim is checked when read (the unit still walks to
	 * that slot) and dropped otherwise; the whole register is swept from time to time.
	 */
	TMap<FIntPoint, TArray<FMassEntityHandle>> SlotClaims;
	int32 StepsSinceClaimSweep = 0;
	static constexpr double ClaimCellSize = 500.;
	static FIntPoint GetClaimCell(const FVector2D& Point)
	{
		return FIntPoint(FMath::FloorToInt32(Point.X / ClaimCellSize), FMath::FloorToInt32(Point.Y / ClaimCellSize));
	}
	/** True when the unit still walks to a slot of that cell. */
	static bool IsClaimLive(const FMassEntityManager& EntityManager, FMassEntityHandle Entity, const FIntPoint& Cell);

	FMassEntityQuery EntityQuery;
};
