// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Match/HOLResources.h"
#include "HOLConstructionReadLibrary.generated.h"

/** A box on the ground plane for the script (FBox2D is not exposed). bValid false (the default) = no box (e.g. no map limit); the functions below and MakeBox give valid ones. Lengths in cm. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLConstructionBox
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Construction")
	FVector2D Min = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Construction")
	FVector2D Max = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Construction")
	bool bValid = false;
};

/**
 * The pure rules of construction (HOLConstruction, Buildings/HOLConstructionRules.h, C++, applied by the server) for Angelscript
 * (namespace HOLConstructionRead there), so that their test (HOLConstructionTests.cpp) can be ported to script (MIG-C-Construction,
 * D830, D839, D836). Wrappers only: no rule here. Enums are passed as ints: EHOLConstructionState 0 Foundation, 1 UnderConstruction,
 * 2 Finished; EHOLAttackType 0 Melee, 1 Ranged, 2 Siege, 3 Hero; EHOLPlacementRefusal 0 None, 1 OutOfMap, 2 Building, 3 Source,
 * 4 EnemyCentre, 5 NavMesh. A resource source as an obstacle is an FVector (X, Y = ground point, Z = body radius).
 * The queues (AddToBuildQueue, AdvanceBuildQueue, AdvanceWorkQueue) are exposed on queues of ints (sites by number, work items encoded by
 * MakeWorkBuild / MakeWorkGather / MakeWorkDeposit; the open test is a list of closed sites): the templates themselves stay in C++.
 */
/** FHOLCampSource for the script (Resource as an int: 0 Food, 1 Wood, 2 Stone, 3 Gold). Lengths in cm. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLConstructionCampSource
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Construction")
	FVector2D Centre = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Construction")
	double Radius = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "Construction")
	int32 Resource = 1;

	UPROPERTY(BlueprintReadWrite, Category = "Construction")
	bool bStanding = true;
};

/** HOLConstruction::FWallBody for the script: a building's body in the world (middle, half its size, its yaw's cosine and sine). */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLConstructionWallBody
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Construction")
	FVector2D Centre = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Construction")
	FVector2D HalfExtent = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Construction")
	double Cos = 1.0;

	UPROPERTY(BlueprintReadWrite, Category = "Construction")
	double Sin = 0.0;
};

UCLASS()
class HEROESOFLIBERTY_API UHOLConstructionReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** A valid box from two corners. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static FHOLConstructionBox MakeBox(const FVector2D& Min, const FVector2D& Max);

	/** HOLConstruction::SnapWallPiece (D898 G4): SnapToGrid with the footprint and the grid divided by SubDivision. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static FHOLConstructionBox SnapWallPiece(const FVector2D& Point, const FVector2D& Footprint, double Grid, int32 SubDivision);

	/** HOLConstruction::SnapToGrid. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static FHOLConstructionBox SnapToGrid(const FVector2D& Point, const FVector2D& Footprint, double Grid);

	/** HOLConstruction::SnapYaw. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static float SnapYaw(float Yaw);

	/** HOLConstruction::MakeFootprint. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static FHOLConstructionBox MakeFootprint(const FVector2D& Centre, const FVector2D& Footprint);

	/** HOLConstruction::Overlaps (two boxes). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static bool Overlaps(const FHOLConstructionBox& A, const FHOLConstructionBox& B);

	/** HOLConstruction::GetDistance. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static double GetDistance(const FHOLConstructionBox& Footprint, const FVector2D& Point);

	/** HOLConstruction::GetGap. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static double GetGap(const FHOLConstructionBox& A, const FHOLConstructionBox& B);

	/** HOLConstruction::CheckPlacement; returns EHOLPlacementRefusal as an int. MapArea invalid (bValid false): no limit. Sources: X, Y ground point, Z body radius. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static int32 CheckPlacement(const FHOLConstructionBox& Footprint, const TArray<FHOLConstructionBox>& Buildings, const TArray<FVector>& Sources,
		const FHOLConstructionBox& MapArea, const TArray<FVector2D>& EnemyMainCentres, double AntiRushRadius, bool bOnNavMesh);

	/**
	 * NON COMPILE. F4 wall line, the world the script cannot read: the tile size (returned), the first and last valid cell of the map (OutMapMin, OutMapMax,
	 * both included; a wide range when the map has no limit) and every cell a building or foundation touches (OutOccupied). Cell = floor(coordinate / tile),
	 * grid origin at the world origin (UHOLConstructionSubsystem::SnapFootprint). Read on the machine that calls it (the server for ServerPlaceWallLine).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Construction", meta = (WorldContext = "WorldContextObject"))
	static double GetWallLineWorld(const UObject* WorldContextObject, FIntPoint& OutMapMin, FIntPoint& OutMapMax, TArray<FIntPoint>& OutOccupied);

	/** HOLConstruction::IsAgeOpen. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static bool IsAgeOpen(int32 BuildingAge, int32 Tier);

	/** HOLConstruction::TryPay. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Construction")
	static bool TryPay(UPARAM(ref) FHOLResources& Stock, const FHOLResources& Cost);

	/** HOLConstruction::GetRefund. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static FHOLResources GetRefund(const FHOLResources& Cost, int64 Work, int64 TotalWork);

	/** HOLConstruction::GetBuildSteps. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static int32 GetBuildSteps(float BuildTime, float StepSeconds);

	/** HOLConstruction::GetTotalWork. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static int64 GetTotalWork(int32 BuildSteps);

	/** HOLConstruction::WorkPerBuilder. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static int32 GetWorkPerBuilder();

	/** HOLConstruction::GetWorkPerStep. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static int32 GetWorkPerStep(int32 Builders, float Exponent);

	/** HOLConstruction::GetStepsToFinish. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static int32 GetStepsToFinish(int32 BuildSteps, int32 Builders, float Exponent);

	/** HOLConstruction::AddWork (Work is changed). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Construction")
	static bool AddWork(UPARAM(ref) int64& Work, int64 TotalWork, int32 Builders, float Exponent);

	/** HOLConstruction::GetProgress. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static float GetProgress(int64 Work, int64 TotalWork);

	/** HOLConstruction::GetHealth. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static int32 GetHealth(int32 MaxHealth, int64 Work, int64 TotalWork);

	/** HOLConstruction::GetHealthWithDamage. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static int32 GetHealthWithDamage(int32 MaxHealth, int64 Work, int64 TotalWork, int32 Damage);

	/** HOLConstruction::GetBuildingDamage (AttackType: 0 Melee, 1 Ranged, 2 Siege, 3 Hero). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static int32 GetBuildingDamage(int32 Damage, int32 AttackType, int32 RangedArmor);

	/** HOLConstruction::GetPopulationProvided. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static int32 GetPopulationProvided(int32 TypePopulation, bool bFinished);

	/** HOLConstruction::BlocksNavigation (State: 0 Foundation, 1 UnderConstruction, 2 Finished). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static bool BlocksNavigation(int32 State, bool bWalkable);

	/** HOLConstruction::IsServing. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static bool IsServing(int32 State);

	/** HOLConstruction::CanCancel. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static bool CanCancel(int32 State);

	/** HOLConstruction::HasWorkStarted. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static bool HasWorkStarted(int32 State, int64 Work);

	/** HOLConstruction::GetCancelRefund. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static FHOLResources GetCancelRefund(const FHOLResources& Cost, int32 State, int64 Work, int64 TotalWork);

	/** HOLConstruction::GetFootprintBounds. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static FHOLConstructionBox GetFootprintBounds(const FVector2D& Centre, const FVector2D& Footprint, float Yaw);

	/** HOLConstruction::GetNavProbes (the points are returned). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static TArray<FVector2D> GetNavProbes(const FHOLConstructionBox& Footprint, double Tile, double CornerInset);

	/** HOLConstruction::GetBuildSpots (the spots are returned). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static TArray<FVector2D> GetBuildSpots(const FHOLConstructionBox& Footprint, double Offset, double Spacing, const FVector2D& From);

	/** HOLConstruction::IsAtSite. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static bool IsAtSite(const FHOLConstructionBox& Footprint, const FVector2D& Position, double BodyRadius, double Gap);

	/** HOLConstruction::MinBodySide. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static double GetMinBodySide();

	/** HOLConstruction::GetBodySize. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static FVector2D GetBodySize(const FVector2D& Footprint, double Inset);

	/** HOLConstruction::GetBody. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static FHOLConstructionBox GetBody(const FHOLConstructionBox& Footprint, double Inset);

	/** HOLConstruction::FindWays (the ways are returned). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static TArray<FHOLConstructionBox> FindWays(const TArray<FHOLConstructionBox>& Bodies, double MaxWidth);

	/** HOLConstruction::TouchTolerance. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static double GetTouchTolerance();

	/** HOLConstruction::GetSegmentDistance. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static double GetSegmentDistance(const FHOLConstructionBox& Footprint, const FVector2D& A, const FVector2D& B);

	/** HOLConstruction::DoesWalkCross. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static bool DoesWalkCross(const FHOLConstructionBox& Footprint, double Margin, const TArray<FVector2D>& Points);

	/** HOLConstruction::PushOutOfFootprint. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static FVector2D PushOutOfFootprint(const FHOLConstructionBox& Footprint, double Margin, const FVector2D& Point);

	/** HOLConstruction::GetKeepOutInsideGap. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static double GetKeepOutInsideGap(double Radius, double Clearance, double VisualGap);

	/** HOLConstruction::KeepOutOfBody (Local is changed; true when it moved). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Construction")
	static bool KeepOutOfBody(const FVector2D& HalfExtent, double InsideGap, double OutsideGap, UPARAM(ref) FVector2D& Local);

	/** HOLConstruction::FindNextSite. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static int32 FindNextSite(const FHOLConstructionBox& Finished, const TArray<FHOLConstructionBox>& Sites, double Range);

	/** HOLConstruction::GetSourceGap. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static double GetSourceGap(const FHOLConstructionBox& Footprint, const FVector2D& Centre, double Radius);

	/** HOLConstruction::SendsBuildersToGather (Accepts: resources as ints, 0 Food, 1 Wood, 2 Stone, 3 Gold). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static bool SendsBuildersToGather(FName TypeName, const TArray<int32>& Accepts);

	/** HOLConstruction::AddToBuildQueue on a queue of ints (sites by number, or work items encoded by MakeWorkBuild / MakeWorkGather / MakeWorkDeposit). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Construction")
	static void AddToBuildQueue(UPARAM(ref) TArray<int32>& Queue, int32 Item, bool bAppend);

	/** HOLConstruction::AdvanceBuildQueue on sites by number; ClosedSites = the sites no longer open (finished, cancelled). True when a site is left at the head. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Construction")
	static bool AdvanceBuildQueue(UPARAM(ref) TArray<int32>& Queue, const TArray<int32>& ClosedSites);

	/** A work item (HOLConstruction::TWorkItem<int32>) as an int: a site to build (n * 4), a source to gather (n * 4 + 1), a deposit (n * 4 + 2); n >= 0. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static int32 MakeWorkBuild(int32 Site);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static int32 MakeWorkGather(int32 SourceIndex);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static int32 MakeWorkDeposit(int32 Building);

	/** The kind of an encoded work item, read by TWorkItem::IsBuild / IsGather / IsDeposit: 0 build, 1 gather, 2 deposit. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static int32 GetWorkItemKind(int32 Item);

	/** HOLConstruction::AdvanceWorkQueue on encoded work items; ClosedSites = the sites no longer open. True when an item is left at the head. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Construction")
	static bool AdvanceWorkQueue(UPARAM(ref) TArray<int32>& Queue, bool bHeadDone, const TArray<int32>& ClosedSites);

	// UHOLBuildingSettings and the building type table (Tables, Body: the game's settings).

	/** UHOLBuildingSettings::GetTileSize. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static double GetSettingTileSize();

	/** UHOLBuildingSettings::AntiRushRadius. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static float GetSettingAntiRushRadius();

	/** UHOLBuildingSettings::BuilderExponent. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static float GetSettingBuilderExponent();

	/** UHOLBuildingSettings::GetBodyInset. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static double GetSettingBodyInset();

	/** UHOLBuildingSettings::GetBodySize. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static FVector2D GetSettingBodySize(const FVector2D& Footprint);

	/** UHOLBuildingSettings::GetVisualInset. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static double GetSettingVisualInset();

	/** UHOLBuildingSettings::GetVisualSize. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static FVector2D GetSettingVisualSize(const FVector2D& Footprint);

	/** UHOLBuildingSettings::FindType: a copy of the row in Out; false when the type is not in the table. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Construction")
	static bool FindBuildingType(FName TypeName, UPARAM(ref) FHOLBuildingTypeRow& Out);

	/** FHOLBuildingTypeRow::IsBuildable. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static bool IsBuildingTypeBuildable(const FHOLBuildingTypeRow& Row);

	/** FHOLBuildingTypeRow::Category as an int: 0 Economic, 1 Military, 2 Technological. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static int32 GetBuildingTypeCategory(const FHOLBuildingTypeRow& Row);

	/** FHOLBuildingTypeRow::Accepts as ints (0 Food, 1 Wood, 2 Stone, 3 Gold). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static TArray<int32> GetBuildingTypeAccepts(const FHOLBuildingTypeRow& Row);

	/** UHOLSourceSettings::FindType: a source type's Linked flag and its resource as an int; false when it is not in the table. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Construction")
	static bool FindSourceTypeInfo(FName SourceName, UPARAM(ref) bool& bLinked, UPARAM(ref) int32& Resource);

	// CampSource, IdleWall, FreeFootprint.

	/** HOLConstruction::FindCampSource (Accepts as ints; OutGap = the gap of the one found). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Construction")
	static int32 FindCampSource(const FHOLConstructionBox& Camp, const TArray<int32>& Accepts, const TArray<FHOLConstructionCampSource>& Sources, double Range,
		UPARAM(ref) double& OutGap);

	/** HOLConstruction::StandsClearOfVisibleWall (Activity: 0 Idle, 1 Moving, 2 Gathering, 3 GatherTrip, 4 Building, 5 Attacking, 6 Dying; Phase: 0 None, 1 WaitingPath, 2 Path, 3 FinalLeg, 4 FormUp, 5 Block). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static bool StandsClearOfVisibleWall(int32 Activity, int32 Phase, bool bFighting);

	/** HOLConstruction::KeepOutOfVisibleWall (Local is changed; bOutEased is set; true when Local moved). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Construction")
	static bool KeepOutOfVisibleWall(const FVector2D& HalfExtent, double InsideGap, double OutsideGap, double Radius, double VisualGap, bool bStandsStill,
		double MaxStep, UPARAM(ref) FVector2D& Local, UPARAM(ref) bool& bOutEased);

	/** HOLConstruction::EaseOutOfVisibleWalls (Position is changed; true when it moved). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Construction")
	static bool EaseOutOfVisibleWalls(const TArray<FHOLConstructionWallBody>& Bodies, double Radius, double VisualGap, double MaxStep,
		UPARAM(ref) FVector2D& Position, bool bSquareCorners);

	/** HOLConstruction::SnapOutward. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static FHOLConstructionBox SnapOutward(const FHOLConstructionBox& Box, double Grid);

	/** HOLConstruction::FindObstacle on a query of Footprint, Buildings, Sources (X, Y ground point, Z body radius), MapArea (invalid: no limit), AntiRushRadius. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Construction")
	static bool FindObstacle(const FHOLConstructionBox& Footprint, const TArray<FHOLConstructionBox>& Buildings, const TArray<FVector>& Sources,
		const FHOLConstructionBox& MapArea, double AntiRushRadius, const FVector2D& Aim, double Grid, UPARAM(ref) FHOLConstructionBox& OutObstacle);

	/**
	 * HOLConstruction::FindFreeFootprint on the same query. The navmesh's answer (a TFunctionRef in C++) is a list of boxes: a candidate whose middle lies in
	 * one of OffNavMesh is off the navmesh. OutRing = the ring found (-1 if none).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Construction")
	static bool FindFreeFootprint(const FHOLConstructionBox& Footprint, const TArray<FHOLConstructionBox>& Buildings, const TArray<FVector>& Sources,
		const FHOLConstructionBox& MapArea, double AntiRushRadius, const FHOLConstructionBox& Obstacle, const FVector2D& Aim, double Grid, int32 MaxRings,
		const TArray<FHOLConstructionBox>& OffNavMesh, UPARAM(ref) FHOLConstructionBox& OutFootprint, UPARAM(ref) int32& OutRing);
};
