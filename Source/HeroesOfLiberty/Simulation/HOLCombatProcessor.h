// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityQuery.h"
#include "MassProcessor.h"
#include "Simulation/HOLCombatRules.h"
#include "HOLCombatProcessor.generated.h"

struct FHOLCombatFragment;
struct FHOLMoveOrderFragment;
struct FHOLUnitStateFragment;
struct FHOLUnitTypeConstants;
struct FMassDesiredMovementFragment;
struct FMassMoveTargetFragment;
class UHOLUnitSubsystem;

/**
 * The melee (J7b.2, J7-Q4: a processor of our own, AoE4-like rules in HOLCombat), on the server. Last of the Tasks group,
 * after the orders, the gathering and the building: it sees the step's walks applied, and the walk it gives a unit to
 * its target is a task's move order, applied at the next step as the gathering's are. The positions it reads are those
 * the movement of the step before left, the same the separation ended on.
 *
 * Each unit looks for an enemy about 5 times a second (one step of 4, staggered by its identifier), in its search radius
 * (HOLCombat::GetSearchRadius) through a hashed grid of the step's positions, plus the last enemy that hit it (riposte),
 * and takes the one HOLCombat::ChooseTarget gives. Out of reach, it walks to it (a new walk at most at the search rate,
 * once the target moved away from where it walked to); in reach, it stops and strikes at the table's cadence
 * (HOLCombat::StartAttack, TickAttack), each hit taking HOLCombat::GetDamage off the target's health. A unit at 0 is
 * queued for destruction at the end of the step (D224, UHOLUnitSubsystem: its population freed); its attackers look for
 * another at once. Units are taken in identifier order, so the outcome does not depend on the chunks.
 *
 * A shooter's hit (HOLCombat::IsRangedAttack, J7c.2) is an arrow: at the hit's moment it is queued with its damage and
 * its landing step (HOLCombat::GetArrowFlightSteps), and lands at the start of that step, before the units are taken:
 * on its target if it still stands (lost otherwise). A unit killed by an arrow dies as one killed in melee (KillUnit).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLCombatProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHOLCombatProcessor();

	/** Hits struck, damage dealt and units killed since the program started (hol.Combat.Report). */
	static int64 GetTotalHits() { return TotalHits; }
	static int64 GetTotalKills() { return TotalKills; }
	/** Arrows released, landed on their target and lost (target dead in flight) since the program started. */
	static int64 GetTotalArrowsReleased() { return TotalArrowsReleased; }
	static int64 GetTotalArrowsLanded() { return TotalArrowsLanded; }
	static int64 GetTotalArrowsLost() { return TotalArrowsLost; }
	/** Arrows in flight (the tests). */
	int32 GetArrowsInFlight() const { return Arrows.Num(); }

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	/** A unit of the step. The pointers stay valid through Execute: nothing changes the archetypes meanwhile. */
	struct FFighter
	{
		FMassEntityHandle Entity;
		uint32 UnitId = 0;
		FVector Location = FVector::ZeroVector;
		FHOLCombatFragment* Combat = nullptr;
		FHOLUnitStateFragment* State = nullptr;
		FHOLMoveOrderFragment* Order = nullptr;
		FMassMoveTargetFragment* MoveTarget = nullptr;
		FMassDesiredMovementFragment* Movement = nullptr;
		const FHOLUnitTypeConstants* Constants = nullptr;
		/** It gathers or builds: a worker at its task takes on nobody by itself (J7b.3 gives orders). */
		bool bBusy = false;
	};

	/** Stops it dead where it stands (as the order processor's arrival, D290): its walk to the target is over. */
	void StopWalk(const UWorld& World, FFighter& Fighter) const;
	static int32 BucketOf(const FIntPoint& Cell);
	/**
	 * The dying path (J7b.4, J7-Q5), whatever killed it (a melee hit, an arrow, another way): stopped dead, out of the
	 * fight and of its task, Dying, its population freed at once, destroyed DyingSteps later.
	 */
	void KillUnit(const UWorld* World, UHOLUnitSubsystem& Units, FFighter& Victim, int64 StepIndex) const;

	/** An arrow in flight (J7c.2): its damage falls on Target at the start of ArrivalStep, if it still stands. */
	struct FArrow
	{
		FMassEntityHandle Shooter;
		FMassEntityHandle Target;
		uint32 ShooterId = 0;
		uint32 TargetId = 0;
		FName ShooterType;
		int32 ShooterSlot = INDEX_NONE;
		int32 Damage = 0;
		int64 ReleaseStep = 0;
		int64 ArrivalStep = 0;
	};
	/** In release order (the outcome does not depend on the chunks). */
	TArray<FArrow> Arrows;

	FMassEntityQuery EntityQuery;

	/** The step's units in identifier order, and the index of each entity. */
	TArray<FFighter> Fighters;
	TMap<FMassEntityHandle, int32> IndexOfEntity;
	/**
	 * The fast paths' index of each entity (J7c.4, S3, hol.Combat.FastPaths): a flat table by the entity's index, an entry
	 * being the step's when its stamp is LookupStamp and its serial the entity's. Kept between steps (it only grows).
	 */
	struct FEntitySlot
	{
		int32 SerialNumber = 0;
		int32 Fighter = INDEX_NONE;
		uint32 Stamp = 0;
	};
	TArray<FEntitySlot> FighterOfEntity;
	uint32 LookupStamp = 0;
	/** The fast paths' identifier sort (J7c.4, S3): (identifier, index) keys, then the fighters moved in their order. */
	TArray<uint64> SortKeys;
	TArray<FFighter> SortedFighters;
	/** Cells of the positions, hashed into buckets (as UHOLUnitSeparationProcessor's). */
	TArray<int32> CellHeads;
	TArray<int32> NextInCell;
	TArray<FIntPoint> Cells;
	float MaxRadius = 1.f;
	/**
	 * The fast paths' search grid (J7c.4, S3), rebuilt each step over the units' box, in the very cells of the hashed one
	 * (CellSize): the units sorted by (side, cell, identifier) by a counting sort, so a searcher walks each enemy side's
	 * row of cells as one contiguous range of plain position, radius and fighter values. False when the box is too large
	 * (the hashed grid then, as before).
	 */
	bool BuildDenseGrid();
	struct FGridUnit
	{
		double X = 0.;
		double Y = 0.;
		float Radius = 0.f;
		int32 Fighter = INDEX_NONE;
	};
	bool bDenseGrid = false;
	TArray<FGridUnit> GridUnits;
	/** Start of each (side, cell) range in GridUnits, one more at the end. */
	TArray<int32> GridStarts;
	TArray<int32> GridCursor;
	/** The OwnerPlayer of each side, and the side of each fighter. */
	TArray<int32> Sides;
	TArray<int32> SideOfFighter;
	FIntPoint GridMin = FIntPoint::ZeroValue;
	int32 GridWidth = 0;
	int32 GridHeight = 0;
	/**
	 * The dense grid's units of other sides whose edge distance to the fighter's is at most Reach, in the hashed walk's
	 * cells (Position +- Reach): the step's constants only (positions, radii, owners), not the health, so it can run
	 * ahead of the serial pass, in parallel.
	 */
	void GatherNearby(int32 Index, double Reach, TArray<int32>& Out) const;
	/**
	 * Gathered ahead (J7c.4, S3), in parallel, for the units that will likely search this step: the list of each, its
	 * Reach, and the slot of each fighter (INDEX_NONE: none, a search falls back to GatherNearby in the serial pass).
	 */
	TArray<int32> AheadSlotOfFighter;
	TArray<int32> AheadFighters;
	TArray<double> AheadReach;
	TArray<TArray<int32>> AheadLists;
	/** A fighter is a candidate of the current search when its stamp is SearchStamp (in place of a scan of the list). */
	TArray<uint32> SeenStamp;
	uint32 SearchStamp = 0;
	/** Kept between steps to spare the allocations. */
	TArray<int32> Nearby;
	TArray<HOLCombat::FCandidate> Candidates;
	/** The fighter of each candidate. */
	TArray<int32> CandidateFighters;
	TArray<uint32> Deaths;

	static int64 TotalHits;
	static int64 TotalKills;
	static int64 TotalArrowsReleased;
	static int64 TotalArrowsLanded;
	static int64 TotalArrowsLost;
};
