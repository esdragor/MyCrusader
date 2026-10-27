// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLResources.h"

enum class EHOLUnitActivity : uint8;
enum class EHOLMovePhase : uint8;

enum class EHOLConstructionState : uint8;
enum class EHOLAttackType : uint8;

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

/**
 * A resource source as the builders of a camp just finished choose one (J6b.1, D312): its ground point, body radius and
 * resource (D297), standing or used up.
 */
struct FHOLCampSource
{
	FVector2D Centre = FVector2D::ZeroVector;
	double Radius = 0.0;
	EHOLResource Resource = EHOLResource::Wood;
	bool bStanding = true;
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

	/** The smallest side of a building's body (cm, D324): a tiny footprint keeps a body this wide (or its own size). */
	inline constexpr double MinBodySide = 100.0;

	/**
	 * The body of a building whose tiles measure Footprint (D324): the building itself, inset by Inset (cm,
	 * UHOLBuildingSettings::BodyInset) on every side of its tiles, whatever its size (a house of 2 x 2 tiles at 105 cm: 3.9 m
	 * of 6, a main centre of 4 x 4: 9.9 m of 12); a side never below MinBodySide (nor above the footprint's).
	 */
	HEROESOFLIBERTY_API FVector2D GetBodySize(const FVector2D& Footprint, double Inset);

	/** The body (GetBodySize) of the footprint box Footprint, on the same middle (D324). */
	HEROESOFLIBERTY_API FBox2D GetBody(const FBox2D& Footprint, double Inset);

	/**
	 * The ways between neighbouring buildings (D324): for each pair of Bodies (boxes on the ground, along the grid) facing each
	 * other along X or Y, no farther apart than MaxWidth and sharing some length across, the box of ground between their facing
	 * sides. Two houses whose tiles touch leave a way 2 x BodyInset wide, as long as their facing sides.
	 */
	HEROESOFLIBERTY_API void FindWays(TConstArrayView<FBox2D> Bodies, double MaxWidth, TArray<FBox2D>& OutWays);

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

	// A free spot beside the obstacle under the cursor (the user's trial of J7b, feedback 3).

	/** The box Box grown outward to the grid lines of tiles of side Grid (a footprint already on them stays as it is). */
	HEROESOFLIBERTY_API FBox2D SnapOutward(const FBox2D& Box, double Grid);

	/**
	 * The obstacle a refused footprint is moved away from (the user's trial of J7b, feedback 3): among the buildings and
	 * sources Query.Footprint overlaps, the one containing Aim (the cursor's ground point), else the nearest to it, as a
	 * box snapped outward to the grid (SnapOutward: a source's disc becomes its bounding box grown to the grid lines). False
	 * when the footprint overlaps none of them.
	 */
	HEROESOFLIBERTY_API bool FindObstacle(const FHOLPlacementQuery& Query, const FVector2D& Aim, double Grid, FBox2D& OutObstacle);

	/**
	 * The first footprint of Query.Footprint's size that may go beside Obstacle (the user's trial of J7b, feedback 3): on the
	 * grid of tiles of side Grid, not overlapping the obstacle's box, in rings around it, ring k leaving a gap of k tiles
	 * between them (the larger of the gaps along X and along Y), from ring 0 (touching) to MaxRings; all of a ring before
	 * the next (around first, then farther), within a ring the one whose middle is nearest Aim first (the first enumerated,
	 * X then Y, of two as near). Each one is checked as CheckPlacement does, against the buildings and sources of Query
	 * near the search area only (a map has many trees), then by IsOnNavMesh (the caller's probes, only for those that pass
	 * the rest). False if none is free within MaxRings (Grid <= 0: none is looked for).
	 */
	HEROESOFLIBERTY_API bool FindFreeFootprint(const FHOLPlacementQuery& Query, const FBox2D& Obstacle, const FVector2D& Aim, double Grid, int32 MaxRings,
		TFunctionRef<bool(const FBox2D&)> IsOnNavMesh, FBox2D& OutFootprint, int32* OutRing = nullptr);

	/** Takes Cost from Stock if it covers all of it; otherwise leaves Stock unchanged (D311: never reserve more than owned). */
	HEROESOFLIBERTY_API bool TryPay(FHOLResources& Stock, const FHOLResources& Cost);

	/**
	 * What cancelling or losing a site gives back (D345, replacing D311's half): the share of Cost not yet built,
	 * Cost * (TotalWork - Work) / TotalWork, rounded down for each resource (100 wood at a quarter of its work -> 75);
	 * all of it for a bare foundation (Work 0) or without a total.
	 */
	HEROESOFLIBERTY_API FHOLResources GetRefund(const FHOLResources& Cost, int64 Work, int64 TotalWork);

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
	 * so damage taken during construction stays (M5), and an unhurt building ends at exactly MaxHealth. AHOLBuilding keeps
	 * the damage taken instead and shows this minus it (GetHealthWithDamage): the same sum, the steps added up.
	 */
	HEROESOFLIBERTY_API int32 GetHealth(int32 MaxHealth, int64 Work, int64 TotalWork);

	/** Health of a building with Work done that took Damage (D312, D345): GetHealth minus Damage, 0 at least. */
	HEROESOFLIBERTY_API int32 GetHealthWithDamage(int32 MaxHealth, int64 Work, int64 TotalWork, int32 Damage);

	/**
	 * Damage one attack deals a building (D345): a ranged attack loses the building's RangedArmor (40 for every building
	 * today), 1 at least (an arrow always scratches it: 5 -> 1, 50 -> 10); melee, siege and hero attacks meet no armour
	 * (full damage, 1 at least). The hero's x0.3 against buildings has no carrier yet.
	 */
	HEROESOFLIBERTY_API int32 GetBuildingDamage(int32 Damage, EHOLAttackType AttackType, int32 RangedArmor);

	/** Population places a building gives its owner (D312: only once finished; D83: centres and houses). */
	HEROESOFLIBERTY_API int32 GetPopulationProvided(int32 TypePopulation, bool bFinished);

	// The construction states (J6a.3, EHOLConstructionState).

	/** The building's footprint is a forbidden area of the navmesh (D312: from its first work on, never as a bare foundation). */
	HEROESOFLIBERTY_API bool BlocksNavigation(EHOLConstructionState State);

	/** The building serves its owner: population, deposit, production (D312: only once finished). */
	HEROESOFLIBERTY_API bool IsServing(EHOLConstructionState State);

	/** Its owner may cancel it and be refunded (D311): a foundation or a building under construction, never a finished one. */
	HEROESOFLIBERTY_API bool CanCancel(EHOLConstructionState State);

	/** Some work has been done on it (D345: the refund is then the share not built): under construction, or Work above 0. */
	HEROESOFLIBERTY_API bool HasWorkStarted(EHOLConstructionState State, int64 Work);

	/**
	 * What cancelling a building in State with Work done of TotalWork gives back, or losing it (D345: GetRefund, the
	 * share not built); nothing for a finished one.
	 */
	HEROESOFLIBERTY_API FHOLResources GetCancelRefund(const FHOLResources& Cost, EHOLConstructionState State, int64 Work, int64 TotalWork);

	/**
	 * The ground box a footprint of size Footprint covers, centred on Centre and turned by Yaw (degrees): the footprint
	 * itself on the grid's quarter turns (a turned square is the same square), its bounds otherwise.
	 */
	HEROESOFLIBERTY_API FBox2D GetFootprintBounds(const FVector2D& Centre, const FVector2D& Footprint, float Yaw);

	/**
	 * Where the navmesh is probed under a footprint (D311, the caller's answer of FHOLPlacementQuery::bOnNavMesh): the
	 * middle of each of its tiles of side Tile (Tile <= 0: its middle), then its four corners moved CornerInset inside
	 * (none if CornerInset <= 0). For 3 m tiles the middles stand 1.5 m inside the edges, and with an inset of 1 m the
	 * corners too stay out of the forbidden area of a neighbour it touches (its footprint + 25 cm + the agent's 60 cm,
	 * D310), while a rock across a corner is seen.
	 */
	HEROESOFLIBERTY_API void GetNavProbes(const FBox2D& Footprint, double Tile, double CornerInset, TArray<FVector2D>& OutPoints);

	// The builders (J6a.4, D312).

	/**
	 * The builders' spots around a footprint (D312, as the gatherers' spots around a source, D309), in the order to try
	 * them: Offset cm out from each of its four sides (a body's radius, the clearance every body keeps from a footprint,
	 * D310, and a small gap), Spacing apart along each side, centred on it (no spot at the corners: two bodies there would
	 * overlap), on every side; the nearest From first, so the side facing the builders fills first and the others after.
	 * The caller keeps the first one free (not taken by another builder, on the navmesh, clear of the other buildings).
	 */
	HEROESOFLIBERTY_API void GetBuildSpots(const FBox2D& Footprint, double Offset, double Spacing, const FVector2D& From, TArray<FVector2D>& OutSpots);

	/** A body of BodyRadius at Position works on the site of Footprint: its body within Gap of the footprint (D312, at contact). */
	HEROESOFLIBERTY_API bool IsAtSite(const FBox2D& Footprint, const FVector2D& Position, double BodyRadius, double Gap);

	/**
	 * A build order given to a builder (D311, D312): with bAppend (Shift), Site goes at the end of its queue (unless it is
	 * already in it), the sites before it built first; without, Site alone replaces its queue.
	 */
	template<typename SiteType>
	void AddToBuildQueue(TArray<SiteType>& Queue, const SiteType& Site, const bool bAppend)
	{
		if (!bAppend)
		{
			Queue.Reset();
		}
		Queue.AddUnique(Site);
	}

	/**
	 * The head of a builder's queue is the site it builds: the sites no longer open (finished, cancelled) are dropped from
	 * the head until one is, the next of its queue (D312: a house finished, its builders go to the next site of their
	 * queue). False when none is left: the builder goes idle.
	 */
	template<typename SiteType>
	bool AdvanceBuildQueue(TArray<SiteType>& Queue, TFunctionRef<bool(const SiteType&)> IsOpen)
	{
		int32 Dropped = 0;
		while (Dropped < Queue.Num() && !IsOpen(Queue[Dropped]))
		{
			++Dropped;
		}
		Queue.RemoveAt(0, Dropped, EAllowShrinking::No);
		return Queue.Num() > 0;
	}

	/**
	 * One item of a worker's queue (Shift, D311; the user's trial of J6a.4): a site to build, a source to gather (Shift +
	 * right click on a resource) or a deposit to unload at (Shift + right click on one of its finished buildings that take
	 * deliveries, D485) once the items before it are done. A gathering lasts until its massif is used up (no spot left in
	 * it nor in its nearby groves, D309), an unloading until it has unloaded (nothing taken carried: at once), then the
	 * next item comes.
	 */
	template<typename SiteType>
	struct TWorkItem
	{
		/** The site to build; with bDeposit, the building to unload at. */
		SiteType Site = SiteType();
		/** The source to gather (an index of UHOLSourceSubsystem); INDEX_NONE: a site or a deposit. */
		int32 SourceIndex = INDEX_NONE;
		/** A deposit to unload at (D485): Site is the building, finished; SourceIndex INDEX_NONE. */
		bool bDeposit = false;

		static TWorkItem MakeBuild(const SiteType& InSite)
		{
			TWorkItem Item;
			Item.Site = InSite;
			return Item;
		}
		static TWorkItem MakeGather(const int32 InSourceIndex)
		{
			TWorkItem Item;
			Item.SourceIndex = InSourceIndex;
			return Item;
		}
		static TWorkItem MakeDeposit(const SiteType& InBuilding)
		{
			TWorkItem Item;
			Item.Site = InBuilding;
			Item.bDeposit = true;
			return Item;
		}
		bool IsGather() const { return SourceIndex != INDEX_NONE; }
		bool IsDeposit() const { return bDeposit; }
		/** A site to build: neither a source nor a deposit. */
		bool IsBuild() const { return !IsGather() && !IsDeposit(); }
		bool operator==(const TWorkItem& Other) const
		{
			return Site == Other.Site && SourceIndex == Other.SourceIndex && bDeposit == Other.bDeposit;
		}
		bool operator!=(const TWorkItem& Other) const { return !(*this == Other); }
	};

	/**
	 * A worker's queue moves on (the user's trial of J6a.4): its head done when bHeadDone (a gathering at the head ends only
	 * when the gathering stops: its massif used up; an unloading, once unloaded, D485), then the sites no longer open
	 * (finished, cancelled) dropped from the head as AdvanceBuildQueue does; a gathering or a deposit waiting its turn stays
	 * (its massif is searched, its load judged, when it comes). False when nothing is left.
	 */
	template<typename SiteType>
	bool AdvanceWorkQueue(TArray<TWorkItem<SiteType>>& Queue, const bool bHeadDone, TFunctionRef<bool(const SiteType&)> IsSiteOpen)
	{
		if (bHeadDone && Queue.Num() > 0)
		{
			Queue.RemoveAt(0, 1, EAllowShrinking::No);
		}
		return AdvanceBuildQueue<TWorkItem<SiteType>>(Queue, [&IsSiteOpen](const TWorkItem<SiteType>& Item)
		{
			return !Item.IsBuild() || IsSiteOpen(Item.Site);
		});
	}

	/** Ground distance between two footprints (0 when they touch or overlap). */
	HEROESOFLIBERTY_API double GetGap(const FBox2D& A, const FBox2D& B);

	/**
	 * The site a builder goes on to when it finishes one with nothing left in its queue (the user's trial of J6a.4): the
	 * nearest of Sites (its player's sites not finished, bare foundations too: its own plans), footprint to footprint from
	 * the finished one, within Range (cm, hol.Build.NextSiteRange); the first of two as near. INDEX_NONE if none is that
	 * near: it stands idle.
	 */
	HEROESOFLIBERTY_API int32 FindNextSite(const FBox2D& Finished, TConstArrayView<FBox2D> Sites, double Range);

	// The camps (J6b.1, D312).

	/**
	 * A building of this type sends its builders to gather once it is finished (D312: a camp): it takes deliveries (Accepts
	 * not empty, D296) and is not a main centre (it takes the four resources: no source would be « its » own). A house, or
	 * any other: they go on to the next site or stand idle.
	 */
	HEROESOFLIBERTY_API bool SendsBuildersToGather(FName TypeName, TConstArrayView<EHOLResource> Accepts);

	/** Ground distance from a footprint to the edge of a source's body of Radius at Centre (0 when they touch or overlap). */
	HEROESOFLIBERTY_API double GetSourceGap(const FBox2D& Footprint, const FVector2D& Centre, double Radius);

	/**
	 * The source the builders of a camp just finished, with nothing left in their queue, go on to gather (D312, J6b.1): the
	 * nearest standing one of Sources whose resource the camp accepts (Accepts: a mining camp takes the nearer of stone and
	 * gold), measured from the camp's footprint to the source's edge, within Range (cm, hol.Build.CampGatherRange); the first
	 * of two as near. INDEX_NONE if none is that near: they go on as after a house (the nearby site, else idle). OutGap, when
	 * given, holds the gap of the one found.
	 */
	HEROESOFLIBERTY_API int32 FindCampSource(const FBox2D& Camp, TConstArrayView<EHOLResource> Accepts, TConstArrayView<FHOLCampSource> Sources, double Range,
		double* OutGap = nullptr);

	// Ways cut by a new footprint (J6b.2, D314).

	/** Ground distance from the segment A-B to the nearest point of Footprint (0 when it meets or enters it). */
	HEROESOFLIBERTY_API double GetSegmentDistance(const FBox2D& Footprint, const FVector2D& A, const FVector2D& B);

	/**
	 * A walk along Points (in order; a single point: a body standing there) passes closer than Margin to Footprint
	 * (J6b.2, D314): Margin is a body's radius, the clearance every body keeps from a footprint (D310) and its place across
	 * its group's corridor. When the footprint becomes forbidden on the navmesh, such a walker gets a new path and such a
	 * kept path of the gathering (D305) is dropped; a walk passing farther goes on as it is.
	 */
	HEROESOFLIBERTY_API bool DoesWalkCross(const FBox2D& Footprint, double Margin, TConstArrayView<FVector2D> Points);

	/**
	 * A destination that a new footprint covers, moved out of it (J6b.2: a slot of an order given before, a spot now under
	 * a building): along the ray from the footprint's middle through Point, onto the edge of the footprint widened by
	 * Margin; the slots of a group so keep their spread around it instead of meeting on one point. Point in the middle:
	 * toward +X. A point already out of the widened footprint is returned as it is.
	 */
	HEROESOFLIBERTY_API FVector2D PushOutOfFootprint(const FBox2D& Footprint, double Margin, const FVector2D& Point);

	/**
	 * Gap (cm) from a building's body at which a body of Radius whose centre stood inside it is put back (D310; trial of
	 * 2026-10-22: a site started under a unit left it in the walls): beyond the navmesh's margin (Clearance, D310) and the
	 * visible wall (VisualGap: from the body to the visible shape, D324), so that it stands neither in the walls nor off the
	 * navmesh.
	 */
	HEROESOFLIBERTY_API double GetKeepOutInsideGap(double Radius, double Clearance, double VisualGap);

	/**
	 * The footprints' guard on one building's body (D310), in its frame (its middle at the origin, HalfExtent): a centre
	 * inside goes out by the nearest side, InsideGap beyond it; a centre outside nearer than OutsideGap goes away from the
	 * body's nearest point, to OutsideGap. True when Local moved.
	 */
	HEROESOFLIBERTY_API bool KeepOutOfBody(const FVector2D& HalfExtent, double InsideGap, double OutsideGap, FVector2D& Local);

	/**
	 * A unit standing still (EtapeDefini C.1: idle, no move order, not fighting) is kept wholly beyond a building's visible
	 * wall. A walker, a worker (building, gathering, at a deposit) or a fighter keeps D324's shoulder: its middle up to the
	 * wall's line, so that a Peasant goes between two houses side by side.
	 */
	HEROESOFLIBERTY_API bool StandsClearOfVisibleWall(EHOLUnitActivity Activity, EHOLMovePhase Phase, bool bFighting);

	/**
	 * KeepOutOfBody, then a body of Radius standing still (bStandsStill) nearer than the visible wall (VisualGap from the
	 * body) and its radius goes away from the body's nearest point, MaxStep a step at most (0: at once), to VisualGap +
	 * Radius + 1 cm. True when Local moved; bOutEased (if given) true when that move was the easing out of the visible wall
	 * alone (C.2: not a body put back, counted apart).
	 */
	HEROESOFLIBERTY_API bool KeepOutOfVisibleWall(const FVector2D& HalfExtent, double InsideGap, double OutsideGap, double Radius,
		double VisualGap, bool bStandsStill, double MaxStep, FVector2D& Local, bool* bOutEased = nullptr);

	/** A building's body in the world: its middle, half its size, its yaw's cosine and sine. */
	struct FWallBody
	{
		FVector2D Centre = FVector2D::ZeroVector;
		FVector2D HalfExtent = FVector2D::ZeroVector;
		double Cos = 1.;
		double Sin = 0.;
	};

	/**
	 * EtapeDefini C.3: a unit standing still (out of the bodies) in the visible wall of one of Bodies goes away from it, MaxStep
	 * a step at most, to VisualGap + Radius + 1 cm, if its way out leaves it clear of every other wall. In two walls at once
	 * (the passage between two houses side by side), or led into another, it stays: D324's shoulder. True when it moved.
	 * bSquareCorners (EtapeDefini J loop 9): the visible wall is the body's box widened by VisualGap, square at its corners
	 * as the slab drawn (the load test's rule), not VisualGap + Radius + 1 cm round the body's corner (which leaves a body
	 * up to ~35 cm in the slab's corner); the side is the same either way.
	 */
	HEROESOFLIBERTY_API bool EaseOutOfVisibleWalls(TConstArrayView<FWallBody> Bodies, double Radius, double VisualGap, double MaxStep, FVector2D& Position,
		bool bSquareCorners = false);
}
