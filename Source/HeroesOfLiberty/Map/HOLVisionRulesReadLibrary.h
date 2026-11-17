// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Map/HOLVisionRules.h"
#include "HOLVisionRulesReadLibrary.generated.h"

/**
 * The pure rules of the fog of war (HOLVisionRules, FHOLVisionGrid, FHOLVisionMap; Map/HOLVisionRules.h, C++, same module as this
 * library) for Angelscript (namespace HOLVisionRulesRead there): D836 keeps the rule in C++ and the script reads it here, so that the
 * pure tests of HOLVisionTests.cpp (Grid, Disc, Forest, Height, Counts, TreeCut, SightRadius) can be ported to script (D830, D839).
 * Wrappers only: no rule here. A grid is its Side and CellSize (centred on the origin, FHOLVisionGrid::MakeCentered); a cell is X, Y;
 * Blocked is one bool per cell, index Y * Side + X (a forest cell); a cell's vision is the number of EHOLCellVision (0 Unexplored,
 * 1 Explored, 2 Visible). Not exposed (see Docs\Rapports\CELEBORN-1117m.md): the ghost / draw / camp / fog-mode / picture rules and
 * the tests that need a world (classe A).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLVisionRulesReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** FHOLVisionGrid::MakeCentered(Side, CellSize).Num(). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Vision")
	static int32 GetGridNum(int32 Side, double CellSize);

	/** FHOLVisionGrid::ToCell(Point): the cell holding a point of the ground, which may lie outside the grid (see IsInsideGrid). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Vision")
	static void ToCell(int32 Side, double CellSize, const FVector2D& Point, int32& OutX, int32& OutY);

	/** FHOLVisionGrid::IsInside. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Vision")
	static bool IsInsideGrid(int32 Side, double CellSize, int32 X, int32 Y);

	/** FHOLVisionGrid::GetCenter. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Vision")
	static FVector2D GetCellCenter(int32 Side, double CellSize, int32 X, int32 Y);

	/** HOLVisionRules::GetDisc(...).Num(). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Vision")
	static int32 GetDiscNum(int32 Side, double CellSize, int32 X, int32 Y, double Radius);

	/** HOLVisionRules::GetDisc(...).Contains(Probe). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Vision")
	static bool DiscContains(int32 Side, double CellSize, int32 X, int32 Y, double Radius, int32 ProbeX, int32 ProbeY);

	/** HOLVisionRules::GetSeenCells(...).Num() (Blocked: one bool per cell, Y * Side + X; shorter: the rest open). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Vision")
	static int32 GetSeenCellsNum(int32 Side, double CellSize, const TArray<bool>& Blocked, int32 X, int32 Y, double Radius);

	/** HOLVisionRules::GetSeenCells(...).Contains(To). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Vision")
	static bool SeenCellsContains(int32 Side, double CellSize, const TArray<bool>& Blocked, int32 FromX, int32 FromY, double Radius, int32 ToX, int32 ToY);

	/** HOLVisionRules::IsInLineOfSight. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Vision")
	static bool IsInLineOfSight(int32 Side, double CellSize, const TArray<bool>& Blocked, int32 FromX, int32 FromY, int32 ToX, int32 ToY);

	/** HOLVisionRules::GetSightRadius with the default FHOLVisionTuning. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Vision")
	static double GetSightRadius(double BaseRadius, double Height);

	/** The default FHOLVisionTuning::MaxHeightBonus. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Vision")
	static double GetMaxHeightBonus();

	/** UHOLBuildingSettings::GetTileSize(). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Vision")
	static double GetTileSize();

	/**
	 * The unit type table (UHOLSimulationSettings::UnitTypesTable) read for the sight check, one entry per row: its name, SightRadius,
	 * reach (the larger of AttackRange and AggressionRadius) and the SightRadius the simulation reads (Row.ToConstants(Name)). False
	 * (arrays empty) when the table does not load.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Vision")
	static bool ReadUnitSightRows(TArray<FString>& OutNames, TArray<double>& OutSight, TArray<double>& OutReach, TArray<double>& OutSimSight);

	/** The building type table (UHOLBuildingSettings::BuildingTypesTable): per row its name, SightRadius and half its larger footprint side. False when the table does not load. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Vision")
	static bool ReadBuildingSightRows(TArray<FString>& OutNames, TArray<double>& OutSight, TArray<double>& OutHalfSide);
};

/**
 * One FHOLVisionMap for the script (a plain C++ class cannot be held by a script): the map of every camp, wrappers only. Make it with
 * UHOLVisionMapHolder::Create(Side, CellSize, NumCamps).
 */
UCLASS(BlueprintType)
class HEROESOFLIBERTY_API UHOLVisionMapHolder : public UObject
{
	GENERATED_BODY()

public:
	/** A new holder, its map Init(MakeCentered(Side, CellSize), NumCamps). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Vision")
	static UHOLVisionMapHolder* Create(int32 Side, double CellSize, int32 NumCamps);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Vision")
	FVector2D GetCellCenter(int32 X, int32 Y) const;

	/** FHOLVisionMap::GetVision as the number of EHOLCellVision (0 Unexplored, 1 Explored, 2 Visible). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Vision")
	int32 GetVision(int32 Camp, int32 X, int32 Y) const;

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Vision")
	int32 GetCount(int32 Camp, int32 X, int32 Y) const;

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Vision")
	bool IsVisible(int32 Camp, const FVector2D& Position) const;

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Vision")
	int32 AddObserver(int32 Camp, const FVector2D& Position, double Radius);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Vision")
	bool MoveObserver(int32 Handle, const FVector2D& Position);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Vision")
	void SetObserverRadius(int32 Handle, double Radius);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Vision")
	void RemoveObserver(int32 Handle);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Vision")
	int32 GetNumObservers() const;

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Vision")
	bool SetBlocked(int32 X, int32 Y, bool bBlocked);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Vision")
	bool IsBlocked(int32 X, int32 Y) const;

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Vision")
	int32 CountVisible(int32 Camp) const;

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Vision")
	int32 CountExplored(int32 Camp) const;

	/** HOLVisionRules::GetSeenCells(...).Num() over this map's own forest cells (IsBlocked of every cell): what the observers should count. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Vision")
	int32 CountSeenByRules(int32 X, int32 Y, double Radius) const;

private:
	FHOLVisionMap Map;
};
