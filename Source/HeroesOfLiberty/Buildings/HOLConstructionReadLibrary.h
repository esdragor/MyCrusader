// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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
UCLASS()
class HEROESOFLIBERTY_API UHOLConstructionReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** A valid box from two corners. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Construction")
	static FHOLConstructionBox MakeBox(const FVector2D& Min, const FVector2D& Max);

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
};
