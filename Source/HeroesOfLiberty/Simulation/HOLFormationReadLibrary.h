// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLFormationReadLibrary.generated.h"

/** HOLFormation::FPlaceHold as a struct the script can hold (same fields). */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLPlaceHoldView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Formation")
	FVector2D LastLocation = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Formation")
	int32 RestSteps = 0;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Formation")
	int32 FailedReturns = 0;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Formation")
	bool bHeld = false;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Formation")
	bool bReturning = false;
};

/**
 * MIG-C-Formation (D830, D836, D839): the rules of the simple formation (HOLFormation, Simulation/HOLFormationRules.h, one C++ source read
 * by the simulation) made callable from script, for the script port of HOLFormationTests.cpp (Script/Tests/HOLFormation_Test.as). Thin
 * wrappers, no rule of its own. Not exposed yet (listed in the report): the by-depth assignment, the block (packets, stragglers, rear slots,
 * blend, line, corridor offsets), the settle of blocked places, the overload of PlaceGroup with IsFree.
 * NOT COMPILED when written (GALADRIEL-1117p).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLFormationReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** A constant of HOLFormation by name (Priority constants, BlockBreakDistance, ReturnRestSteps, MaxReturnTries, BlockMemberStuckSteps, FileSlideMaxAngle...); 0 if unknown. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Formation")
	static double GetConstant(FName Name);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Formation")
	static void MakePeasantRows(int32 Count, TArray<int32>& OutWidths);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Formation")
	static void MakeSquareRows(int32 Count, TArray<int32>& OutWidths);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Formation")
	static void MakeMixedRows(int32 Count, TArray<int32>& OutWidths);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Formation")
	static void MakeRowsOfWidth(int32 Count, int32 Width, TArray<int32>& OutWidths);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Formation")
	static void MakeGroupRows(const TArray<int32>& CountOfKind, bool bPeasantsOnly, TArray<int32>& OutWidths);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Formation")
	static void PlaceKinds(UPARAM(ref) TArray<int32>& InOutRowWidths, const TArray<int32>& CountOfKind, TArray<int32>& OutKindOfPlace);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Formation")
	static int32 GetRowPriority(FName TypeName, int32 CarryCapacity, bool bRanged, bool bSiege);

	/** HOLFormation::GetRowClass as an int: 0 Melee, 1 Ranged, 2 Siege, 3 Worker (ERowClass order). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Formation")
	static int32 GetRowClass(int32 RowPriority);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Formation")
	static void MakePlaces(const TArray<int32>& RowWidths, const FVector2D& Target, const FVector2D& Forward, double ColumnSpacing,
		double RowSpacing, TArray<FVector2D>& OutPlaces);

	/** HOLFormation::PlaceGroup with the identity as ProjectPlace (no ground to project on). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Formation")
	static void PlaceGroup(const TArray<FVector2D>& Positions, const TArray<int32>& KindOfUnit, bool bPeasantsOnly, const FVector2D& Target,
		const FVector2D& Forward, double ColumnSpacing, double RowSpacing, int32 FrontWidth, TArray<FVector2D>& OutPlaces,
		TArray<int32>& OutPlaceOfUnit);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Formation")
	static void AssignPlaces(const TArray<FVector2D>& Positions, const TArray<FVector2D>& Places, const FVector2D& Forward,
		TArray<int32>& OutPlaceOfUnit);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Formation")
	static int32 GetFrontWidth(double Length, double ColumnSpacing, int32 Count);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Formation")
	static FVector2D GetFrontFacing(const FVector2D& Arrow);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Formation")
	static float GetMarchSpeed(float OwnSpeed, float Slowest);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Formation")
	static void EndWalkToPlace(UPARAM(ref) FHOLPlaceHoldView& Hold, const FVector2D& Location, const FVector2D& Place, double Radius);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Formation")
	static bool WatchPlace(UPARAM(ref) FHOLPlaceHoldView& Hold, const FVector2D& Location, const FVector2D& Place, double Radius);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Formation")
	static bool ShouldYieldHeldPlace(const FVector2D& HeldPlace, const TArray<FVector2D>& NewPlacesByX, double RadiusSum);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Formation")
	static bool ShouldBreakBlock(bool bPlainMove, double ClosestEnemy, double ClosestFightingFriend);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Formation")
	static bool IsFightingFriend(bool bAttacking, bool bApproaching, bool bHasTarget);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Formation")
	static bool IsMemberStuck(int32 Steps);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Formation")
	static void MakeCorridor(const TArray<double>& FreeLeft, const TArray<double>& FreeRight, double SampleStep, double OffsetMinY,
		double OffsetMaxY, double MaxShift, double MaxSlope, TArray<double>& OutShift, TArray<double>& OutSqueeze);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Formation")
	static void GetCorridorAt(const TArray<double>& Shift, const TArray<double>& Squeeze, double SampleStep, double Distance,
		double& OutShift, double& OutSqueeze);
};
