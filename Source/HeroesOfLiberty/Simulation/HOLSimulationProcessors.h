// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include <atomic>
#include "MassEntityQuery.h"
#include "MassProcessor.h"
#include "Match/HOLVictoryRules.h"
#include "Simulation/HOLAvoidRules.h"
#include "HOLSimulationProcessors.generated.h"

struct FHOLCombatFragment;

/** First processor of each fixed step: keeps the transform the presentation interpolates from. */
UCLASS()
class HEROESOFLIBERTY_API UHOLStorePreviousTransformProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHOLStorePreviousTransformProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};

/**
 * A unit stands or walks at its full speed, nothing in between (the user's trial of J4, D290): Mass' steering eases
 * in, slows down over the last second of the way and overshoots its goal, then the standing steering brings it back
 * slowly. Between the force and the movement, a moving unit's velocity keeps the direction the steering, the
 * avoidance and the navmesh boundary gave it, at the full speed of its order; on the final leg it walks straight to
 * its slot (D294) and its last step ends exactly there. A standing unit does not move at all.
 *
 * Every step, a walker looks ahead for the bodies it would meet on its way within a short time and turns aside from
 * them, the smallest turn first (D293): Mass' avoidance runs one step out of 6 (D220) and turned too late, the walker
 * pushed through. When no way within the largest turn is clear (a crowd, its slot behind other units), the detour is
 * too long: it goes on and pushes its way (UHOLUnitSeparationProcessor, D292).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLFullSpeedProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHOLFullSpeedProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	/** Takes the units' positions, velocities and radii into a hashed grid; false when no unit walks (nothing to do). */
	bool BuildLookAhead(FMassExecutionContext& Context);
	static int32 LookAheadBucketOf(const FIntPoint& Cell);
	/**
	 * The way a walker takes: Direction if nothing is met on it within Horizon (s), else the smallest clear turn, else
	 * Direction; from the detour remembered in Detour while its body is ahead, eased back one turn a step (MV.2). With
	 * bAroundEnemies (a walk that is not a fight's), an enemy in the way is a wall: no turn clear, the largest turn is
	 * taken and held on its hand, never a push straight into it (MV.4). A still body is looked for within StillHorizon (at
	 * least Horizon), and gone along up to hol.Units.AvoidStillMaxAngle (MV.4.4); none touching its goal (Goal, if any:
	 * its slot), where it is to stand beside it (MV.4.5.3).
	 */
	FVector AvoidBodies(FMassEntityHandle Entity, const FVector& Location, float Radius, const FVector& Direction, float Speed, double Horizon, double StillHorizon, uint32 FormUpSerial, uint8 FormUpKind,
		uint8 Owner, struct FHOLDetourFragment& Detour, bool bAroundEnemies, const FVector2D* Goal = nullptr) const;
	/**
	 * J7c.4 loop 19 (hol.Combat.RearSlide): a melee walking to fight, off the front, wanting Wanted: OutVelocity slides along
	 * the friend barring its way (or stops when both sides are shut), smoothed from LastVelocity. False at the front.
	 */
	bool SlideAlongFriends(FMassEntityHandle Entity, const FVector2D& Position, float Radius, const FVector2D& Goal, const FVector2D& Wanted, float Speed,
		const FVector2D& LastVelocity, uint8 Owner, float DeltaTime, FHOLCombatFragment& Combat, FVector2D& OutVelocity, bool& bOutSliding) const;

	FMassEntityQuery EntityQuery;

	/** The step's units before they move (kept between steps to spare the allocations). */
	TArray<FMassEntityHandle> Entities;
	TArray<FVector2D> Positions;
	TArray<FVector2D> Velocities;
	/** Standing (no move under way) and still: gone round further and wider (MV.4.4). */
	TArray<bool> Stills;
	TArray<float> Radii;
	/**
	 * Each unit's GetPassThroughSerial and GetPassThroughKind (J7a.4, J7c): two units for which CanPassThrough holds do not
	 * turn aside for each other.
	 */
	TArray<uint32> FormUpSerials;
	TArray<uint8> FormUpKinds;
	/** Each unit's OwnerPlayer (J7c.4: the middle of each army, hol.Combat.RearSlide; the friends and enemies around a walker). */
	TArray<uint8> Owners;
	/** The owners' camps this step: an enemy is of another camp (an ally's unit is gone round and pushed as a friend). */
	FHOLCampTable CampTable;
	float MaxRadius = 1.f;
	float MaxSpeed = 0.f;
	/** Cells of the positions, hashed into buckets: first unit of each bucket, the next unit of the same bucket, each unit's cell. */
	TArray<int32> CellHeads;
	TArray<int32> NextInCell;
	TArray<FIntPoint> Cells;
	/**
	 * Perf (hol.Units.FastNear): the same cells in a dense table over the units' bounding box when it is not too large,
	 * each cell's units side by side in descending index order (the order of a bucket's chain): its range
	 * LookAheadBodies[LookAheadStarts[Cell] .. LookAheadStarts[Cell + 1]), with their positions and radii for the first tests.
	 */
	struct FLookAheadBody
	{
		FVector2D Position = FVector2D::ZeroVector;
		float Radius = 0.f;
		int32 Index = INDEX_NONE;
	};
	bool bLookAheadDense = false;
	FIntPoint LookAheadMin = FIntPoint::ZeroValue;
	int32 LookAheadWidth = 0;
	int32 LookAheadHeight = 0;
	TArray<int32> LookAheadStarts;
	TArray<FLookAheadBody> LookAheadBodies;
	/** J7c.4 loop 20: each owner's units' mean position this step (the middle of its army: a rear rank slides away from it). */
	TMap<uint8, FVector2D> OwnerCentres;
	/** The bodies of the buildings the navmesh forbids, this step (EtapeDefini E: no detour into them), and their bounding radii. */
	TArray<HOLAvoid::FFootprintBox> Footprints;
	TArray<double> FootprintRadii;
};

/**
 * No unit's body in a building (D310, J6a.1): once the units have moved and parted, a body in a footprint is put back
 * against its wall, out by the nearest way. The navmesh keeps the walkers' paths the clearance away
 * (UHOLBuildingSettings::BodyClearance); what pushed bodies in was the separation of a crowd standing against a wall
 * (D292: 19 Peasants sent onto the main centre stood inside it, up to 4.6 m deep), the look-ahead's turns (D293), and a
 * final leg to a slot by the wall. The same rule for every building, each unit's own radius; a building's body, inset in its
 * tiles (D324), so that a unit walks between two neighbours.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLFootprintKeepOutProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHOLFootprintKeepOutProcessor();

	/** Bodies put back out of a footprint since the program started (hol.Nav.Footprints report). */
	static int64 GetNumPushedOut() { return NumPushedOut.load(); }
	/** Units standing still eased out of a visible wall (EtapeDefini C.1), apart from the bodies put back (C.2). */
	static int64 GetNumEasedOut() { return NumEasedOut.load(); }
	/** Bodies put back out of a fixed obstacle of the map (a rock, a trunk: ridge_army) since the program started. */
	static int64 GetNumPutOutOfObstacles() { return NumPutOutOfObstacles.load(); }

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;

	/** A footprint of the step, in its own frame. */
	struct FFootprint
	{
		FVector2D Centre = FVector2D::ZeroVector;
		FVector2D HalfExtent = FVector2D::ZeroVector;
		/** Its yaw's cosine and sine. */
		double Cos = 1.;
		double Sin = 0.;
		/** Where a body can touch it: its bounds widened by the largest body. */
		FBox2D Reach;
	};
	TArray<FFootprint> Footprints;
	/**
	 * The footprints each cell of a grid over their reaches may concern (S7: 32 buildings), a unit tried against these
	 * alone: cell C's are CellFootprints[CellStarts[C]] to CellFootprints[CellStarts[C + 1]].
	 */
	FBox2D GridBounds;
	FIntPoint GridSize = FIntPoint::ZeroValue;
	TArray<int32> CellStarts;
	TArray<int32> CellFootprints;

	/**
	 * The map's fixed obstacles (rocks, trunks: the static meshes that cut the navmesh, gathered once per world), each one's
	 * bounds widened by the largest body: a unit inside one of them is held on the navmesh, nowhere else (ridge_army).
	 */
	TArray<FBox2D> ObstacleReaches;
	FBox2D ObstacleBounds;
	TWeakObjectPtr<const UWorld> ObstacleWorld;

	static std::atomic<int64> NumPushedOut;
	static std::atomic<int64> NumEasedOut;
	static std::atomic<int64> NumPutOutOfObstacles;
};

/**
 * Orientation, instant (user's choice at the J5a trial, as D290 made the speed): a walking unit faces the way it walks,
 * from the step it starts or turns; a unit at rest keeps its facing (pushed aside, it does not turn). Replaces Mass'
 * smoothed orientation, which blended the walk with the path's direction and turned in ~0.3 s.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLInstantOrientationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHOLInstantOrientationProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};

/**
 * No body walks through another (the user's trial of J4, D292): at full speed the avoidance turns too late, and a
 * standing unit no longer steps aside (D290). Once the units have moved, every overlap a walker takes part in is
 * undone, each unit of the pair taking half: a walker pushes a standing unit out of its way (as in AoE) and the pushed
 * unit moves no further, two walkers part. Standing units among themselves are left as they are, but for a gatherer
 * that walked through every unit to its spot (D309): landed, it parts from a unit standing there. A gatherer at its
 * source and a unit standing beside it part down to the spots' spacing only (100 cm: gatherers pressed against a wood's
 * edge overlap a little, the user's trial of D309). Pairs found in a hashed grid of the step's positions, one
 * pass a step.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLUnitSeparationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHOLUnitSeparationProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;

	/** The step's units, in identifier order (kept between steps to spare the allocations). */
	TArray<FVector2D> Positions;
	TArray<FVector2D> Corrections;
	TArray<float> Radii;
	/** The walkers, and the gatherers that landed at their spot this step (D309): the units that look for overlaps. */
	TArray<bool> Walking;
	/** Gatherers on their round trip (D309): they pass through every unit and none pushes them. */
	TArray<bool> Ghosts;
	/**
	 * Each unit's GetPassThroughSerial and GetPassThroughKind (J7a.4, J7c): two units for which CanPassThrough holds pass
	 * through each other (rows of other kinds turned to a new march cross; three bodies pressed together held each other still).
	 */
	TArray<uint32> FormUpSerials;
	TArray<uint8> FormUpKinds;
	/** Units walking an order (not a gatherer settling at its spot). */
	TArray<bool> Moving;
	/** Each unit's player: two enemies never push each other back (HOLCombat::GetOverlapShare). */
	TArray<uint8> Owners;
	/** The owners' camps this step: two units are enemies when of two camps (allies push each other as friends). */
	FHOLCampTable CampTable;
	/**
	 * Gatherers at their source (D309): with a unit standing beside them, they keep the spots' spacing only (100 cm,
	 * hol.Gather.SpotSpacing), not two bodies; a walker still pushes them out of its way.
	 */
	TArray<bool> AtSource;
	/** Soldiers (a unit type that carries nothing: not a Peasant): the units of a line of contact (J7c.4, hol.Combat.RearFlank). */
	TArray<bool> Soldiers;
	/** J7c.4, loop 18 (hol.Simulation.FrontHoldsFriends): units that strike or touch an enemy, never pushed by a friend. */
	TArray<bool> Front;
	/**
	 * EtapeDefini O.7 (a crowd, AoE4): a melee standing and striking a building, its way in (FHOLCombatFragment::
	 * BuildingInward); zero for the others. Not of the front: a friend pushing it moves it only that way, never outwards.
	 */
	TArray<FVector2D> PushIn;
	/** EtapeDefini O.7: how far (cm) each PushIn unit may be pushed in this step (FHOLCombatFragment::BuildingRoom). */
	TArray<double> PushInRoom;
	/**
	 * J7c.4 loop 30 (hol.Simulation.FrontNeedsCombat): units concerned by a fight (a target, striking, approaching, on an
	 * attack-move or its detour, flanking); an enemy contact makes the front only when one of the two is.
	 */
	TArray<bool> Engaged;
	/** Units that looked for their neighbours this step, and those to look (the walkers, then the units they pushed). */
	TArray<bool> Looked;
	TArray<bool> Queued;
	TArray<int32> ToLook;
	/** Cells of the positions, hashed into buckets: first unit of each bucket, the next unit of the same bucket, each unit's cell. */
	TArray<int32> CellHeads;
	TArray<int32> NextInCell;
	TArray<FIntPoint> Cells;
	/**
	 * Or the dense table over the units' bounding box (J7c.4, hol.Simulation.FastSeparation): each cell's range in CellUnits
	 * (its start, and the next cell's start), its units in descending index order as in a bucket's chain; each unit's cell.
	 */
	TArray<int32> CellStarts;
	TArray<int32> CellUnits;
	TArray<int32> DenseCells;
	/**
	 * J7c.4, determinism: the units are put in identifier order once gathered, as the combat's fighters, so the cells' lists,
	 * the units to look and the in-place corrections do not depend on the chunks' order (which differs between runs for the
	 * same state). Each unit's identifier, the sorted (identifier, gather index) keys, each gathered unit's sorted index, and
	 * a scratch buffer for the moves.
	 */
	TArray<uint32> UnitIds;
	TArray<uint64> SortKeys;
	TArray<int32> SortedIndexOf;
	TArray<uint8> PermuteScratch;
};
