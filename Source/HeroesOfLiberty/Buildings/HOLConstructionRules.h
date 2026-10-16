// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLResources.h"

/**
 * Pure rules of construction (J6, D311, D312), applied by the server (and the client's ghost, J6a.3): where a
 * foundation may go, what it costs and gives back, how fast its builders raise it, what it gives when finished. No
 * engine state: the caller gathers the footprints, sources, map, enemy centres and navmesh answer and passes them in;
 * tested on their own (HeroesOfLiberty.Construction.*). Lengths in cm, on the ground plane (X, Y).
 */

/** A resource source as an obstacle to placement: its ground point and body radius (D297). */
struct FHOLPlacementCircle
{
	FVector2D Centre = FVector2D::ZeroVector;
	double Radius = 0.0;
};

/** Everything placing one foundation is checked against (D311). */
struct FHOLPlacementQuery
{
	/** The footprint to place, already snapped to the grid (HOLConstruction::SnapToGrid). */
	FBox2D Footprint = FBox2D(ForceInit);
	/** Footprints of the buildings and foundations already there, every player's. */
	TArrayView<const FBox2D> Buildings;
	/** Resource sources still on the map. */
	TArrayView<const FHOLPlacementCircle> Sources;
	/** Playable area of the map (AHOLMapBounds); an invalid box: no limit. */
	FBox2D MapArea = FBox2D(ForceInit);
	/** Ground points of the enemy players' main centres (D22). */
	TArrayView<const FVector2D> EnemyMainCentres;
	/** UHOLBuildingSettings::AntiRushRadius. */
	double AntiRushRadius = 0.0;
	/** The caller's answer: the footprint lies on the navmesh (its corners and middle project onto it). */
	bool bOnNavMesh = true;
};

/** Why a foundation may not go there; the first failed check, in this order. */
enum class EHOLPlacementRefusal : uint8
{
	None,
	/** Partly or wholly out of the map's playable area. */
	OutOfMap,
	/** Overlaps a building or a foundation (touching is fine, D311). */
	Building,
	/** Overlaps a resource source. */
	Source,
	/** Within the anti-rush radius of an enemy main centre (D22). */
	EnemyCentre,
	/** Not on the navmesh (cliff, water, unreachable ground). */
	NavMesh,
};

namespace HOLConstruction
{
	/**
	 * Fixed-point work of construction: one builder adds WorkPerBuilder per simulation step, so a building of
	 * BuildSteps steps needs BuildSteps * WorkPerBuilder. Integers: the server's count never drifts, and 3 builders at
	 * 2000 per step finish in exactly half the steps.
	 */
	inline constexpr int32 WorkPerBuilder = 1000;

	/** Overlap below this (cm) is touching: rounding never refuses two buildings side by side. */
	inline constexpr double TouchTolerance = 0.5;

	/**
	 * The footprint of size Footprint whose middle is nearest Point with its lower corner on the grid of tiles of side
	 * Grid (D311, UHOLBuildingSettings::TileSize, the grid lines through the world's origin): the edges of every
	 * footprint fall on grid lines, so a footprint of an odd number of tiles has its middle in the middle of a tile and
	 * an even one on a grid point, and two buildings side by side touch exactly. Grid <= 0: no snapping.
	 */
	HEROESOFLIBERTY_API FBox2D SnapToGrid(const FVector2D& Point, const FVector2D& Footprint, double Grid);

	/**
	 * Yaw (degrees) of a building on the grid: the nearest quarter turn, so its footprint stays along the grid lines
	 * (a main centre facing a diagonal start turns by 45 degrees; a square looks the same).
	 */
	HEROESOFLIBERTY_API float SnapYaw(float Yaw);

	/** The footprint of size Footprint centred on Centre. */
	HEROESOFLIBERTY_API FBox2D MakeFootprint(const FVector2D& Centre, const FVector2D& Footprint);

	/** The two footprints share some area (more than TouchTolerance deep on both axes); touching edges do not. */
	HEROESOFLIBERTY_API bool Overlaps(const FBox2D& A, const FBox2D& B);

	/** The footprint and the disc share some area: the nearest point of the box is strictly inside the disc. */
	HEROESOFLIBERTY_API bool Overlaps(const FBox2D& Footprint, const FHOLPlacementCircle& Circle);

	/** Ground distance from Point to the nearest point of Footprint (0 inside). */
	HEROESOFLIBERTY_API double GetDistance(const FBox2D& Footprint, const FVector2D& Point);

	/**
	 * Whether the foundation may go there (D311, D22): inside the map, on no building nor source, out of the anti-rush
	 * radius of every enemy main centre (measured from its middle to the nearest point of the footprint), on the
	 * navmesh. The server checks again what the client's ghost showed (D288).
	 */
	HEROESOFLIBERTY_API EHOLPlacementRefusal CheckPlacement(const FHOLPlacementQuery& Query);

	/** Takes Cost from Stock if it covers all of it; otherwise leaves Stock unchanged (D311: never reserve more than owned). */
	HEROESOFLIBERTY_API bool TryPay(FHOLResources& Stock, const FHOLResources& Cost);

	/**
	 * What cancelling a foundation gives back (D311): all of Cost before its first work, half of each resource after
	 * (rounded down: 50 wood -> 25).
	 */
	HEROESOFLIBERTY_API FHOLResources GetRefund(const FHOLResources& Cost, bool bWorkStarted);

	/** Simulation steps a single builder takes, at least 1 (BuildTime and StepSeconds in seconds). */
	HEROESOFLIBERTY_API int32 GetBuildSteps(float BuildTime, float StepSeconds);

	/** Work a building of BuildSteps steps needs (BuildSteps * WorkPerBuilder). */
	HEROESOFLIBERTY_API int64 GetTotalWork(int32 BuildSteps);

	/**
	 * Work Builders builders add per step together, with diminishing returns (D312):
	 *   WorkPerBuilder * Builders ^ Exponent, rounded; 0 with no builder.
	 * Exponent 0.63 (UHOLBuildingSettings::BuilderExponent, ln 2 / ln 3): 3 builders ~2x as fast as one.
	 */
	HEROESOFLIBERTY_API int32 GetWorkPerStep(int32 Builders, float Exponent);

	/** Steps Builders builders take from nothing to finished (Builders >= 1), for the HUD and the tests. */
	HEROESOFLIBERTY_API int32 GetStepsToFinish(int32 BuildSteps, int32 Builders, float Exponent);

	/**
	 * One step of construction: adds the builders' work to Work, capped at TotalWork. True when this step finishes it
	 * (and only that step).
	 */
	HEROESOFLIBERTY_API bool AddWork(int64& Work, int64 TotalWork, int32 Builders, float Exponent);

	/** Share of the construction done, 0 to 1 (the building rises from the ground with it, D312). */
	HEROESOFLIBERTY_API float GetProgress(int64 Work, int64 TotalWork);

	/**
	 * Health of a building with Work done and never hurt (D312): MaxHealth * Work / TotalWork rounded down, at least 1
	 * (a bare foundation can be destroyed by one blow). The caller adds the difference of two of these at each step,
	 * so damage taken during construction stays (M5), and an unhurt building ends at exactly MaxHealth.
	 */
	HEROESOFLIBERTY_API int32 GetHealth(int32 MaxHealth, int64 Work, int64 TotalWork);

	/** Population places a building gives its owner (D312: only once finished; D83: centres and houses). */
	HEROESOFLIBERTY_API int32 GetPopulationProvided(int32 TypePopulation, bool bFinished);
}
