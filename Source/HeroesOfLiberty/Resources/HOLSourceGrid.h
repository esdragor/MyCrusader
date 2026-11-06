// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLResources.h"

struct FHOLGatherDeposit;

/**
 * The places of the resource sources on a grid of cells (2026-11-05, Forest Medium: 142 935 sources, a walk over all of them
 * by the AI's summary and by the gatherers' stand points took 33 ms and 1.7 s). One map of cells per resource, so the search
 * of gold meets no tree. Sources are only added (a used-up one stays, its owner tests it standing), never moved.
 * Read-only for a query: safe to read from the simulation while nobody adds.
 */
class HEROESOFLIBERTY_API FHOLSourceGrid
{
public:
	/** Side (cm) of a cell: a Forest's wood has a few dozen trees in one. */
	static constexpr double CellSize = 1000.;

	struct FEntry
	{
		int32 Index = INDEX_NONE;
		FVector2D Location = FVector2D::ZeroVector;
	};

	void Add(int32 Index, EHOLResource Resource, const FVector2D& Location, float Radius);

	int32 Num() const { return NumEntries; }
	/** The greatest body radius added: a query by the edge of a source widens its search by it. */
	float GetMaxRadius() const { return MaxRadius; }

	/**
	 * Calls Visit for every entry (any resource) in the cells that touch the square of side 2 * HalfSide round Centre: a superset
	 * of the ones within HalfSide, the caller measures. Returns the number of entries visited.
	 */
	int32 ForEachInSquare(const FVector2D& Centre, double HalfSide, TFunctionRef<void(const FEntry&)> Visit) const;

	/**
	 * The Count entries of Resource accepted by Accept that are nearest the deposit's footprint (HOLGather::GetDepositDistance),
	 * a tie to the lower index: OutIndices, nearest first. Cells are met ring after ring round the footprint and the search
	 * stops once no entry beyond can beat the Count-th. Returns the number of entries visited.
	 */
	int32 FindNearest(const FHOLGatherDeposit& Deposit, EHOLResource Resource, int32 Count, TFunctionRef<bool(int32)> Accept, TArray<int32>& OutIndices) const;

private:
	static FIntPoint GetCell(const FVector2D& Location);

	struct FResourceCells
	{
		TMap<FIntPoint, TArray<FEntry>> Cells;
		FIntPoint Min = FIntPoint(MAX_int32, MAX_int32);
		FIntPoint Max = FIntPoint(MIN_int32, MIN_int32);
		int32 Num = 0;
	};

	FResourceCells ByResource[4];
	int32 NumEntries = 0;
	float MaxRadius = 0.f;
};
