// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Commands/HOLCommandSubsystem.h"
#include "CoreMinimal.h"
#include "MassEntityQuery.h"
#include "MassProcessor.h"
#include "Simulation/HOLOrderChainRules.h"
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
	/** New paths of walkers held on theirs since the program started (J6b.3, stress test report). */
	static int64 GetTotalStuckRepaths() { return TotalStuckRepaths; }
	/** Walks back to a formation place after a push since the program started (J7a.4). */
	static int64 GetTotalPlaceReturns() { return TotalPlaceReturns; }
	/** The last block march (J7a.4, for the scripted checks): its form-up steps, slowest pace, and the march's worst slot gap and speed. */
	struct FBlockReport
	{
		int32 Units = 0;
		/** Steps of a stop before the march: 0, the block forms its rows on the move (playtest note 1 of J7b); -1 no block. */
		int32 FormUpSteps = -1;
		/** Steps of the march until every unit of the body stood in its file (within BlockLagDistance across; files on the move; -1 not yet). */
		int32 JoinSteps = -1;
		/** Distance the block marched (cm) until then (-1 not yet); its point's start behind its path's (cm); units still sliding to their files as it began re-forming at its end. */
		float JoinDistance = -1.f;
		float SetBack = 0.f;
		int32 FormingAtEnd = 0;
		int32 MarchSteps = 0;
		/** Steps the block's point was held: only while it re-forms at its end (rows on the move: never on the march). */
		int32 HeldSteps = 0;
		/**
		 * Members that left the block as stragglers: none since files on the move (the user's 5th trial of playtest note 1
		 * of J7b: a member's depth is its own, it never falls behind a slot); kept for the reports.
		 */
		int32 LeftUnits = 0;
		/**
		 * The closest two of its members stood (cm, centre to centre) from its first half second (BlockSpacingFromStep)
		 * until its arrival's blend, re-forming left out (the units taking their slots keep apart, the user's trial of
		 * playtest note 1 of J7c); -1 not measured.
		 */
		float MinSpacing = -1.f;
		/**
		 * Units left behind at the start, far from the group's body (HOLFormation::FindStragglers), plus the members that fell
		 * behind on the way (LeftUnits, counted in both), and those of them that joined it on the way.
		 */
		int32 Stragglers = 0;
		int32 JoinedStragglers = 0;
		/** Stragglers whose kind had no slot in the block's last row (a mixed group): they follow it, their places at the arrival. */
		int32 QueuedStragglers = 0;
		/** Units of smaller blocks of its order merged into it on the march (HOLFormation::BlockMergeDistance). */
		int32 MergedUnits = 0;
		float Pace = 0.f;
		/** The farthest a member stood across from its file on the march (cm), its first steps left out. */
		float MaxSlotGap = 0.f;
		float MaxSpeed = 0.f;
		bool bArrived = false;
		/**
		 * After the block's end (the user's 2nd trial of J7a.4): steps until every unit stood on its place (within 30 cm,
		 * under 20 cm/s; -1 still going), pairs whose straight ways to their places crossed, separation pushes (a step that
		 * took a unit more than 2 cm further from its place) and the farthest a unit stood from its place at the end.
		 */
		int32 SettleSteps = -1;
		int32 Crossings = 0;
		int32 Pushes = 0;
		float ArrivalOff = 0.f;
		/**
		 * Steps of the march (its arrival's re-forming left out) on which a unit of the block walked against the march
		 * (its velocity along the block's direction below -1 cm/s): the user's 4th trial of playtest note 1, never.
		 */
		int32 BackSteps = 0;
		/**
		 * Steps of the march (its first half second and its arrival's re-forming left out, stragglers too) on which a member
		 * walked along the march below BlockSlowedShare of the pace (the user's 5th trial of playtest note 1: nobody slows
		 * down on the march; only the slowest kind sliding to its file loses a little, cos FileSlideMaxAngle of it).
		 */
		int32 SlowedSteps = 0;
		/**
		 * Steps of the arrival's re-forming (its blend, before the final legs) on which a unit of the block walked against
		 * the march (below -1 cm/s along it), and the ground so walked back by all its units (cm): the rows closing up
		 * should not send the head back (the user's 5th trial of playtest note 1).
		 */
		int32 ArrivalBackSteps = 0;
		float ArrivalBackDistance = 0.f;
	};
	static const FBlockReport& GetLastBlockReport() { return LastBlockReport; }

	/**
	 * The navmesh's path Found from Start to Target, or a shorter one through the way between two neighbouring buildings
	 * near its line (D324: the navmesh's search often goes around that strip one body wide). What the orders walk;
	 * hol.Nav.Path shows it too.
	 */
	static TSharedPtr<FNavigationPath, ESPMode::ThreadSafe> FindPathThroughWays(UWorld& World, UNavigationSystemV1& NavSystem, const ANavigationData& NavData,
		const FVector& Start, const FVector& Target, const TSharedPtr<FNavigationPath, ESPMode::ThreadSafe>& Found);
	/** Walkers given a new path because a new footprint cut their way, since the program started (J6b.2, D314, stress test S7). */
	static int64 GetTotalFootprintRepaths() { return TotalFootprintRepaths; }
	/** Kept paths of the task walks dropped because a new footprint cut them, since the program started (J6b.2, D314). */
	static int64 GetTotalFootprintDroppedPaths() { return TotalFootprintDroppedPaths; }
	int32 GetNumPathsWaiting() const { return PendingPaths.Num(); }

	/**
	 * The chains of queued orders (D328, J7b.5b-1a): the units with steps from moves and attacks, those with queued moves
	 * (Shift) or still carrying out the last one started. Empty, at no cost, when nobody has a queue.
	 */
	void GetChainUnitIds(TArray<uint32>& OutUnitIds) const;
	/** A unit's steps: its current action (when known), then its queued moves and attacks, head first. */
	void GetChainSteps(uint32 UnitId, TArray<HOLOrderChain::FStep>& OutSteps) const;
	/** The world it last ran for (hol.Orders.Chains finds it so). */
	const UWorld* GetLastWorld() const { return LastWorld.Get(); }
	/**
	 * A player's chains now (D328; server or solo; Slot INDEX_NONE: everyone's), from these order, build and gather
	 * processors (a builder's chain first, then a gatherer's, then its moves).
	 */
	static void CollectChains(const UWorld& World, const UHOLUnitOrderProcessor* Orders, const class UHOLBuildProcessor* Builds,
		const class UHOLGatherProcessor* Gathers, int32 Slot, TArray<HOLOrderChain::FChain>& OutChains);
	/** Where a step is now (server or solo); its target for the logs. False: its target is gone. Sites: filled at the first site. */
	static bool LocateChainStep(const UWorld& World, const HOLOrderChain::FStep& Step, TMap<uint32, const class AHOLBuilding*>& Sites,
		FVector& OutLocation, FString* OutTarget = nullptr);

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	/** The orders queued since the last step: slots given, units waiting for their cluster's path. */
	void ApplyQueuedOrders(FMassEntityManager& EntityManager, UNavigationSystemV1* NavSystem, const ANavigationData* NavData);
	/**
	 * Shift's moves (the user's 1st trial of J7a.4): each queued move whose units still in it have all ended their walks
	 * before it (standing, or walking back to a formation place) starts, handed to the next step's ApplyQueuedOrders as an
	 * order of those units (its formation made then); a unit gone or put to other work leaves it.
	 */
	void StartQueuedMoves(FMassEntityManager& EntityManager);
	/** Path requests of the waiting clusters, oldest first, up to the step's budget. */
	void ServePathRequests(FMassEntityManager& EntityManager, UNavigationSystemV1* NavSystem, const ANavigationData* NavData);
	/** Next part of a path, then the final leg to the slot, then standing (activity Idle, action Stand). */
	void AdvanceUnits(FMassEntityManager& EntityManager, FMassExecutionContext& Context, const ANavigationData* NavData);

	/**
	 * A group's block march (J7a.4, as in AoE4): its units, their offsets in the block (X ahead, Y to the right) and their
	 * arrival places, the group's one path, the block's point along it and its pace (the slowest's).
	 */
	struct FBlockMarch
	{
		TArray<FMassEntityHandle> Entities;
		TArray<FVector2D> Offsets;
		TArray<FVector> Places;
		/** Steps in a row each member was far from its slot while re-forming (beyond BlockSoftHoldDistance). */
		TArray<int32> HeldSteps;
		/**
		 * Steps in a row the re-forming block crawled (its soft pace under BlockCrawlShare, held by a member just short of
		 * BlockSoftHoldDistance that could not close in: ridge_army, 17 s): at BlockMemberStuckSteps, its slow members leave it.
		 */
		int32 CrawlSteps = 0;
		/**
		 * Each member's own depth in the block (cm along the march from the block's point, the user's 5th trial of playtest
		 * note 1: files on the move, rows at the arrival): read from where it stands every step of the march, kept from the
		 * arrival's blend on (its slot then that depth in its file, sliding to its place).
		 */
		TArray<double> Depths;
		TArray<FVector2D> Path;
		double Length = 0.;
		double Progress = 0.;
		/**
		 * Its corridor along the path (MV.5, HOLFormation::MakeCorridor): per sample (CorridorSampleStep apart), the
		 * files' shift across and squeeze, so they clear an obstacle ahead of time; built for its files' span across
		 * (CorridorMinY, CorridorMaxY), again when units joining it widen that span.
		 */
		TArray<double> CorridorShift;
		TArray<double> CorridorSqueeze;
		double CorridorMinY = 0.;
		double CorridorMaxY = 0.;
		bool bCorridor = false;
		/** Pace (cm/s, the table's, before hol.Units.SpeedScale). */
		float Pace = 0.f;
		int32 Steps = 0;
		/** Each unit's kind (row priority index), and the final facing: its re-forming at the end (the user's 3rd trial). */
		TArray<int32> Kinds;
		FVector2D FinalForward = FVector2D(1., 0.);
		/**
		 * The arrival's blend started, over the path's last BlendDistance. (Its units still sliding to their files on the
		 * move, playtest note 1 of J7b, are those in the FormUp phase; Progress starts at the units' mean depth.)
		 */
		bool bBlending = false;
		double BlendDistance = 0.;
		/**
		 * Its stragglers (a playtest note of J7c): out of the block from the start, each with a slot kept in the block's
		 * last row (HOLFormation::ReserveRearSlots), on its own path at its own speed, then straight into its slot, never
		 * holding the block; one within BlockLagDistance of its slot is one of the block (the user's trial of playtest
		 * note 1); a queued one, and those that never catch up, stand on their places at the end.
		 */
		struct FStraggler
		{
			FMassEntityHandle Entity;
			FVector2D Offset = FVector2D::ZeroVector;
			FVector Place = FVector::ZeroVector;
			int32 Kind = 0;
			/** Its type's speed (cm/s, the table's): the block slows down to it once it is one of the block. */
			float Speed = 0.f;
			/** Its slot is not in the block's last row (HOLFormation::ReserveRearSlots): it follows behind, its place at the arrival. */
			bool bQueued = false;
			/** Steered straight to its slot (or its queue's point), through the units of its order, instead of its own path. */
			bool bSteering = false;
		};
		TArray<FStraggler> Stragglers;
		/** Where a queued straggler walks: this far along the path from the block's point (behind it: a row behind its last row). */
		double QueueOffset = 0.;
		/** Its own report (the last block's is LastBlockReport). */
		FBlockReport Report;
		/**
		 * Its order, its number in the order's blocks and how many there are (playtest note 1 of J7b, the user's 2nd trial:
		 * small blocks): block 0 is the body (with the stragglers), each other one a separate packet of two units or more
		 * (HOLFormation::FindPackets) marching in its own formation, on its own path from where its units stand, at its own
		 * pace. All the blocks of an order share one formation (the same Offsets frame, the same arrival places), each with
		 * the offsets of its own units: at the arrival every unit already stands in the common formation. When two meet (their
		 * points within HOLFormation::BlockMergeDistance on the march), the smaller merges into the larger (AdvanceBlocks):
		 * its units slide to their files at the larger one's point, keeping their depths.
		 */
		uint32 Serial = 0;
		int32 Index = 0;
		int32 NumBlocks = 1;
		/** Its tag in the logs: none for the body (the scripted checks read its lines), "block 2 of 3: " for the others. */
		FString GetTag() const { return Index > 0 ? FString::Printf(TEXT("block %d of %d: "), Index + 1, NumBlocks) : FString(); }
	};
	/** The blocks on the march, by MakeBlockKey(order serial, block number). */
	TMap<uint64, FBlockMarch> Blocks;
	static uint64 MakeBlockKey(const uint32 Serial, const int32 Index) { return (static_cast<uint64>(Serial) << 32) | static_cast<uint32>(Index); }
	/** A block's units after its end, watched until they all stand on their places (FBlockReport::SettleSteps). */
	struct FArrivalWatch
	{
		TArray<FMassEntityHandle> Entities;
		TArray<FVector> Places;
		TArray<double> LastOff;
		int32 Steps = 0;
		FBlockReport Report;
		/** Its block's order and number (FBlockMarch::Serial, Index). */
		uint32 Serial = 0;
		int32 Index = 0;
		FString Tag;
	};
	TMap<uint64, FArrivalWatch> Arrivals;
	void WatchArrivals(FMassEntityManager& EntityManager);
	static FBlockReport LastBlockReport;
	/** Order serial of the last block march started (its body's report, block 0, is LastBlockReport). */
	static uint32 LastBlockSerial;
	/**
	 * A group's move in formation made a block march (J7a.4) unless it is short (within the block's size): its one path,
	 * its rows around its centre facing the path's start, its units' offsets in the block, their arrival places (Places,
	 * given again from where the block will stand), the clusters' requests (FirstRequest, NumClusters) dropped; the block
	 * starts at once at full pace, its point at its units' mean depth (playtest note 1 of J7b, the user's 5th trial: files
	 * on the move, rows at the arrival).
	 */
	void StartBlockMarch(FMassEntityManager& EntityManager, UWorld& World, UNavigationSystemV1* NavSystem, const ANavigationData* NavData, uint32 OrderSerial,
		TConstArrayView<FMassEntityHandle> Entities, TConstArrayView<FVector2D> Positions, TConstArrayView<int32> KindOfUnit, bool bPeasantsOnly,
		const FVector2D& Target, const FVector2D& Forward, double ColumnSpacing, double RowSpacing, int32 FrontWidth, TConstArrayView<FVector> Places,
		float Pace, int32 FirstRequest, int32 NumClusters);
	/** The blocks' step: the march along the path (its units sliding to their files on the move), then each unit to its arrival place (its final leg). */
	void AdvanceBlocks(FMassEntityManager& EntityManager, float DeltaTime, const ANavigationData* NavData);
	/** A block's corridor (MV.5) for its files spanning [MinY, MaxY] across: its path sampled, the free width across read on the navmesh at height Z. */
	static void BuildCorridor(FBlockMarch& Block, double MinY, double MaxY, double Z, const ANavigationData* NavData);

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

	/** A move given with Shift (J7a.4), waiting for its units' walks before it: the order, and its units (D223 identifiers) still in it. */
	struct FQueuedMove
	{
		FHOLMoveOrder Order;
		TArray<uint32> Members;
	};
	/** The queued moves, by identifier, and each unit's queue of them, head first. */
	TMap<uint32, FQueuedMove> QueuedMoves;
	TMap<uint32, TArray<uint32>> MoveQueueOfUnit;
	uint32 NextQueuedMoveId = 1;
	/** Queued moves started at the last step: applied first by the next ApplyQueuedOrders, before the new orders. */
	TArray<FHOLMoveOrder> StartedQueuedMoves;
	/**
	 * Units given a drop-off by ApplyQueuedOrders in this step (D485, EtapeDefini L.5b, L.6): the gathering applies it after
	 * this processor, so StartQueuedMoves sees them neither dropping off nor at work yet. Their current order is not over (the
	 * next queued move waits: the user's 2nd trial, P, D, B: B started in the same step, then the gathering's walk to D
	 * replaced it), and their queue of moves is kept (Shift + D then Shift + B at once: B was forgotten as for a gatherer).
	 */
	TSet<uint32> DroppingNow;
	/**
	 * Units given work (a source, a site) by ApplyQueuedOrders in this step (L.6): the gathering and the builders apply it
	 * after this processor. Their current order is not over (the next queued move does not start in the same step); a
	 * further Shift work order of the same step goes to their queue of work, after it.
	 */
	TSet<uint32> WorkStartedNow;
	/**
	 * A work order (Gather, Build: L.6) applied to Units: each one's Shift order to the gathering or the builders (with no
	 * queue of work, at once), under its player's scope with bChain; then, for a unit whose queued order started, the work
	 * orders right after it in its queue of moves go to its queue of work, in the order given; the rest of its queue of moves
	 * waits for its queue of work (FHOLBuildFragment::bMovesAfter, EtapeDefini M.1).
	 */
	void ApplyWorkOrder(FMassEntityManager& EntityManager, const class UHOLUnitSubsystem& Units, UHOLCommandSubsystem& Commands, const FHOLMoveOrder& Order,
		TConstArrayView<uint32> UnitIds);
	/** A unit's queued moves and attacks only (not its current action), appended to OutSteps (after a builder's chain, M.1). */
	void AppendQueuedSteps(uint32 UnitId, TArray<HOLOrderChain::FStep>& OutSteps) const;
	/** Issues a work order (Gather, Build, DropOff) to the queue of work of UnitIds (Shift), as its player gave it. */
	static void IssueToWorkQueue(UHOLCommandSubsystem& Commands, const FHOLMoveOrder& Order, TConstArrayView<uint32> UnitIds);
	/** Its queue of moves forgotten (a new order, gone): taken out of each move it waited for. */
	void ForgetMoveQueue(uint32 UnitId);
	/**
	 * Chains (D328): the current action of each unit with a queue (or whose last queued move still goes on), and since
	 * which step. Set when a unit's first move is queued (its order then, from OrderSteps) and when a queued move starts;
	 * dropped with its queue (a new order without Shift) and once its last step is over (StartQueuedMoves).
	 */
	struct FChainHead
	{
		HOLOrderChain::FStep Step;
		uint32 SinceStep = 0;
	};
	TMap<uint32, FChainHead> ChainHeadOfUnit;
	/** The step of each recent player order (one entry per order, not per unit), by its serial: a unit's current action. */
	TMap<uint32, HOLOrderChain::FStep> OrderSteps;
	static HOLOrderChain::FStep MakeChainStep(const FHOLMoveOrder& Order);
	TWeakObjectPtr<const UWorld> LastWorld;
	/** Each player's chains to its player state (D328, J7b.5b-1b): every 0.25 s at most, only while there are some. */
	void PublishChains(const UWorld& World, float DeltaSeconds);
	double ChainPublishWait = 0.;
	/** Some player state holds chains: published again (emptied) when the last queue goes. */
	bool bChainsPublished = false;
	TWeakObjectPtr<const class UHOLBuildProcessor> ChainBuilds;
	TWeakObjectPtr<const class UHOLGatherProcessor> ChainGathers;
	/** Attack, stop, hold (J7b.3): the units stand, their task, queue and fight forgotten; an attack sets their target. */
	void ApplyCombatOrder(FMassEntityManager& EntityManager, const UWorld& World, const struct FHOLMoveOrder& Order,
		TConstArrayView<FMassEntityHandle> Entities, FMassEntityHandle AttackTarget);
public:
	/** Combat walks dropped because a newer player order came first (J7b.3). */
	static int64 TotalStaleCombatWalks;
protected:

	uint32 NextOrderSerial = 1;
	int64 NumPathsServed = 0;
	static int64 TotalPathsServed;
	static int64 TotalPathsReused;
	static int64 TotalBuildingRepaths;
	static int64 TotalPlaceReturns;
	/** New paths of walkers held on theirs (J6b.3: pushed behind at a corner of the navmesh), since the program started. */
	static int64 TotalStuckRepaths;
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
	/**
	 * The straight way from Start to Target crosses a building's body that blocks the navmesh (with the margin of D310), or
	 * passes within ExtraMargin more of one (D324: a way between two buildings).
	 */
	static bool DoesCrossBuilding(const UWorld& World, const FVector& Start, const FVector& Target, const ANavigationData* NavData, double ExtraMargin = 0.);

	/**
	 * Slots given to units still on their way, by cell of ClaimCellSize: a later order does not give them again (units
	 * sent one by one to a rally point stand side by side). A claim is checked when read (the unit still walks to
	 * that slot) and dropped otherwise; the whole register is swept from time to time.
	 */
	TMap<FIntPoint, TArray<FMassEntityHandle>> SlotClaims;

	/**
	 * Slowest speed (cm/s, the table's) of the walkers of each group's order in formation (J7a.4), by order serial: the
	 * last step's, read this step (FHOLMoveOrderFragment::MarchSpeed), and this step's, built while the units advance.
	 */
	TMap<uint32, float> SlowestOfOrder;
	TMap<uint32, float> NextSlowestOfOrder;
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
