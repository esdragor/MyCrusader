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
class AHOLBuilding;
class UHOLUnitSubsystem;
enum class EHOLAttackType : uint8;

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
	/** FNV-1a over the arrows in flight, in list order (J7c.4, determinism probe, hol.Simulation.StepHashes). */
	uint64 HashArrows() const;

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
	/** A unit at 0 health: Dying if it is not yet, into Deaths once Dying for DyingSteps (the combat off too). */
	void FinishDying(const UWorld* World, UHOLUnitSubsystem& Units, FFighter& Fighter, int64 StepIndex);

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
		/** D345.2b: an arrow at a building (Target unset): its damage of BuildingAttackType through DamageBuilding when it lands. */
		TWeakObjectPtr<AHOLBuilding> TargetBuilding;
		uint8 BuildingAttackType = 0;
	};
	/** In release order (the outcome does not depend on the chunks). */
	TArray<FArrow> Arrows;

	FMassEntityQuery EntityQuery;

	/**
	 * The step's buildings that can be attacked (D345.2b): standing, not a bare foundation (unseen by the enemy), their body
	 * box (D324). Gathered once a step, before the units are taken; one destroyed in the step is marked gone.
	 */
	struct FStepBuilding
	{
		AHOLBuilding* Building = nullptr;
		FBox2D Box = FBox2D(ForceInit);
		uint32 Id = 0;
		uint8 OwnerPlayer = 0;
		bool bGone = false;
	};
	TArray<FStepBuilding> StepBuildings;
	TArray<HOLCombat::FBuildingCandidate> BuildingCandidates;
	TArray<int32> BuildingCandidateIndices;
	/** The step's entry of a building, INDEX_NONE if it is not one of them (gone, a foundation). */
	int32 FindStepBuilding(const AHOLBuilding* Building) const;

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
	/** MV.4 (coming to aid): GatherNearby on the fighter's own side, itself left out (the dense grid only). */
	void GatherFriends(int32 Index, double Reach, TArray<int32>& Out) const;
	/**
	 * J7c.4, trial feedback (hol.Combat.PreferFreeTarget): the living bodies of every side (the fighter left out) whose
	 * edge stands within Gap of the fighter's on the dense grid, at most Limit counted: the places taken around it.
	 */
	int32 CountBodiesAround(int32 Index, double Gap, int32 Limit) const;
	/** Its count of the step for each fighter, -1 not yet counted. */
	TArray<int32> PlacesTaken;
	/**
	 * Gathered ahead (J7c.4, S3), in parallel, for the units that will likely search this step: the list of each, its
	 * Reach, and the slot of each fighter (INDEX_NONE: none, a search falls back to GatherNearby in the serial pass).
	 */
	TArray<int32> AheadSlotOfFighter;
	TArray<int32> AheadFighters;
	TArray<double> AheadReach;
	TArray<TArray<int32>> AheadLists;
	/**
	 * Perf (hol.Combat.ParallelAhead): the contact list (hol.Combat.StopAtContact) of each unit walking to fight at the
	 * step's start, gathered ahead in parallel with the lists above; its slot by fighter (INDEX_NONE: gathered serially).
	 */
	TArray<int32> ContactSlotOfFighter;
	TArray<int32> ContactFighters;
	TArray<TArray<int32>> ContactLists;
	/**
	 * Perf (hol.Combat.ParallelAhead): the places taken around each living enemy of the melee units gathered ahead,
	 * counted in parallel once the arrows have landed (-1: not counted); read in place of CountBodiesAround while no unit
	 * has died since (the count would then be the same).
	 */
	TArray<int32> AheadPlaces;
	TArray<int32> AheadPlaceFighters;
	/**
	 * Perf (hol.Combat.ParallelSearch): the search of each unit gathered ahead (by its slot), made in parallel on the step's
	 * state once the arrows have landed: its target and last attacker then, the fighter chosen (INDEX_NONE: none), and the
	 * candidates whose places it read, with whether they were full. bDone false: not made (no search, its target kept).
	 */
	struct FAheadSearch
	{
		bool bDone = false;
		int32 Target = INDEX_NONE;
		int32 LastAttacker = INDEX_NONE;
		int32 Chosen = INDEX_NONE;
		TArray<int32> PlaceFighters;
		TArray<bool> PlaceFull;
	};
	TArray<FAheadSearch> AheadSearches;
	/**
	 * Perf (hol.Combat.ParallelSearch): 1 for a fighter whose target, attackers or life the serial pass changed this step
	 * (what a search reads of a candidate, its position aside): weighed again by a search taking a choice made ahead.
	 */
	TArray<uint8> SearchTouched;
	/** A fighter is a candidate of the current search when its stamp is SearchStamp (in place of a scan of the list). */
	TArray<uint32> SeenStamp;
	uint32 SearchStamp = 0;
	/** Kept between steps to spare the allocations. */
	TArray<int32> Nearby;
	/** Scratch of GatherFriends (coming to aid). */
	TArray<int32> Friends;
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
