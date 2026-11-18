// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map/HOLVisionRulesReadLibrary.h"

#include "Buildings/HOLBuildingTypes.h"
#include "Engine/DataTable.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"

namespace HOLVisionRulesReadLibDetail
{
	FHOLVisionGrid MakeGrid(const int32 Side, const double CellSize)
	{
		return FHOLVisionGrid::MakeCentered(Side, CellSize);
	}

	/** One flag per cell of the grid; cells past the given array are open. */
	TArray<bool> MakeBlocked(const FHOLVisionGrid& Grid, const TArray<bool>& Blocked)
	{
		TArray<bool> Result;
		Result.Init(false, Grid.Num());
		for (int32 Index = 0; Index < Result.Num() && Index < Blocked.Num(); ++Index)
		{
			Result[Index] = Blocked[Index];
		}
		return Result;
	}
}

int32 UHOLVisionRulesReadLibrary::GetGridNum(const int32 Side, const double CellSize)
{
	return HOLVisionRulesReadLibDetail::MakeGrid(Side, CellSize).Num();
}

void UHOLVisionRulesReadLibrary::ToCell(const int32 Side, const double CellSize, const FVector2D& Point, int32& OutX, int32& OutY)
{
	const FIntPoint Cell = HOLVisionRulesReadLibDetail::MakeGrid(Side, CellSize).ToCell(Point);
	OutX = Cell.X;
	OutY = Cell.Y;
}

bool UHOLVisionRulesReadLibrary::IsInsideGrid(const int32 Side, const double CellSize, const int32 X, const int32 Y)
{
	return HOLVisionRulesReadLibDetail::MakeGrid(Side, CellSize).IsInside(FIntPoint(X, Y));
}

FVector2D UHOLVisionRulesReadLibrary::GetCellCenter(const int32 Side, const double CellSize, const int32 X, const int32 Y)
{
	return HOLVisionRulesReadLibDetail::MakeGrid(Side, CellSize).GetCenter(FIntPoint(X, Y));
}

int32 UHOLVisionRulesReadLibrary::GetDiscNum(const int32 Side, const double CellSize, const int32 X, const int32 Y, const double Radius)
{
	return HOLVisionRules::GetDisc(HOLVisionRulesReadLibDetail::MakeGrid(Side, CellSize), FIntPoint(X, Y), Radius).Num();
}

bool UHOLVisionRulesReadLibrary::DiscContains(const int32 Side, const double CellSize, const int32 X, const int32 Y, const double Radius, const int32 ProbeX, const int32 ProbeY)
{
	return HOLVisionRules::GetDisc(HOLVisionRulesReadLibDetail::MakeGrid(Side, CellSize), FIntPoint(X, Y), Radius).Contains(FIntPoint(ProbeX, ProbeY));
}

int32 UHOLVisionRulesReadLibrary::GetSeenCellsNum(const int32 Side, const double CellSize, const TArray<bool>& Blocked, const int32 X, const int32 Y, const double Radius)
{
	const FHOLVisionGrid Grid = HOLVisionRulesReadLibDetail::MakeGrid(Side, CellSize);
	return HOLVisionRules::GetSeenCells(Grid, HOLVisionRulesReadLibDetail::MakeBlocked(Grid, Blocked), FIntPoint(X, Y), Radius).Num();
}

bool UHOLVisionRulesReadLibrary::SeenCellsContains(const int32 Side, const double CellSize, const TArray<bool>& Blocked, const int32 FromX, const int32 FromY, const double Radius, const int32 ToX, const int32 ToY)
{
	const FHOLVisionGrid Grid = HOLVisionRulesReadLibDetail::MakeGrid(Side, CellSize);
	return HOLVisionRules::GetSeenCells(Grid, HOLVisionRulesReadLibDetail::MakeBlocked(Grid, Blocked), FIntPoint(FromX, FromY), Radius).Contains(FIntPoint(ToX, ToY));
}

bool UHOLVisionRulesReadLibrary::IsInLineOfSight(const int32 Side, const double CellSize, const TArray<bool>& Blocked, const int32 FromX, const int32 FromY, const int32 ToX, const int32 ToY)
{
	const FHOLVisionGrid Grid = HOLVisionRulesReadLibDetail::MakeGrid(Side, CellSize);
	return HOLVisionRules::IsInLineOfSight(Grid, HOLVisionRulesReadLibDetail::MakeBlocked(Grid, Blocked), FIntPoint(FromX, FromY), FIntPoint(ToX, ToY));
}

double UHOLVisionRulesReadLibrary::GetSightRadius(const double BaseRadius, const double Height)
{
	return HOLVisionRules::GetSightRadius(BaseRadius, Height, FHOLVisionTuning());
}

double UHOLVisionRulesReadLibrary::GetMaxHeightBonus()
{
	return FHOLVisionTuning().MaxHeightBonus;
}

double UHOLVisionRulesReadLibrary::GetTileSize()
{
	return UHOLBuildingSettings::GetTileSize();
}

bool UHOLVisionRulesReadLibrary::ReadUnitSightRows(TArray<FString>& OutNames, TArray<double>& OutSight, TArray<double>& OutReach, TArray<double>& OutSimSight)
{
	OutNames.Reset();
	OutSight.Reset();
	OutReach.Reset();
	OutSimSight.Reset();
	const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
	if (UnitTypes == nullptr)
	{
		return false;
	}
	UnitTypes->ForeachRow<FHOLUnitTypeRow>(TEXT("Vision read library"), [&](const FName& Name, const FHOLUnitTypeRow& Row)
	{
		OutNames.Add(Name.ToString());
		OutSight.Add(Row.SightRadius);
		OutReach.Add(FMath::Max(Row.AttackRange, Row.AggressionRadius));
		OutSimSight.Add(Row.ToConstants(Name).SightRadius);
	});
	return true;
}

bool UHOLVisionRulesReadLibrary::ReadBuildingSightRows(TArray<FString>& OutNames, TArray<double>& OutSight, TArray<double>& OutHalfSide)
{
	OutNames.Reset();
	OutSight.Reset();
	OutHalfSide.Reset();
	const UDataTable* BuildingTypes = GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous();
	if (BuildingTypes == nullptr)
	{
		return false;
	}
	BuildingTypes->ForeachRow<FHOLBuildingTypeRow>(TEXT("Vision read library"), [&](const FName& Name, const FHOLBuildingTypeRow& Row)
	{
		OutNames.Add(Name.ToString());
		OutSight.Add(Row.SightRadius);
		OutHalfSide.Add(0.5 * FMath::Max(Row.Footprint.X, Row.Footprint.Y));
	});
	return true;
}

UHOLVisionMapHolder* UHOLVisionMapHolder::Create(const int32 Side, const double CellSize, const int32 NumCamps)
{
	UHOLVisionMapHolder* Holder = NewObject<UHOLVisionMapHolder>(GetTransientPackage());
	Holder->Map.Init(FHOLVisionGrid::MakeCentered(Side, CellSize), NumCamps);
	return Holder;
}

FVector2D UHOLVisionMapHolder::GetCellCenter(const int32 X, const int32 Y) const
{
	return Map.GetGrid().GetCenter(FIntPoint(X, Y));
}

int32 UHOLVisionMapHolder::GetVision(const int32 Camp, const int32 X, const int32 Y) const
{
	return static_cast<int32>(Map.GetVision(Camp, FIntPoint(X, Y)));
}

int32 UHOLVisionMapHolder::GetCount(const int32 Camp, const int32 X, const int32 Y) const
{
	return Map.GetCount(Camp, FIntPoint(X, Y));
}

bool UHOLVisionMapHolder::IsVisible(const int32 Camp, const FVector2D& Position) const
{
	return Map.IsVisible(Camp, Position);
}

int32 UHOLVisionMapHolder::AddObserver(const int32 Camp, const FVector2D& Position, const double Radius)
{
	return Map.AddObserver(Camp, Position, Radius);
}

bool UHOLVisionMapHolder::MoveObserver(const int32 Handle, const FVector2D& Position)
{
	return Map.MoveObserver(Handle, Position);
}

void UHOLVisionMapHolder::SetObserverRadius(const int32 Handle, const double Radius)
{
	Map.SetObserverRadius(Handle, Radius);
}

void UHOLVisionMapHolder::RemoveObserver(const int32 Handle)
{
	Map.RemoveObserver(Handle);
}

int32 UHOLVisionMapHolder::GetNumObservers() const
{
	return Map.GetNumObservers();
}

bool UHOLVisionMapHolder::SetBlocked(const int32 X, const int32 Y, const bool bBlocked)
{
	return Map.SetBlocked(FIntPoint(X, Y), bBlocked);
}

bool UHOLVisionMapHolder::IsBlocked(const int32 X, const int32 Y) const
{
	return Map.IsBlocked(FIntPoint(X, Y));
}

int32 UHOLVisionMapHolder::CountVisible(const int32 Camp) const
{
	return Map.CountVisible(Camp);
}

int32 UHOLVisionMapHolder::CountExplored(const int32 Camp) const
{
	return Map.CountExplored(Camp);
}

int32 UHOLVisionMapHolder::CountSeenByRules(const int32 X, const int32 Y, const double Radius) const
{
	const FHOLVisionGrid& Grid = Map.GetGrid();
	TArray<bool> Blocked;
	for (int32 Index = 0; Index < Grid.Num(); ++Index)
	{
		Blocked.Add(Map.IsBlocked(Grid.ToCell(Index)));
	}
	return HOLVisionRules::GetSeenCells(Grid, Blocked, FIntPoint(X, Y), Radius).Num();
}
