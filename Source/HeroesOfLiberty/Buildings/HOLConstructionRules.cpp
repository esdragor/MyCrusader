// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLConstructionRules.h"

#include "Algo/StableSort.h"
#include "Buildings/HOLBuildingTypes.h"

FBox2D HOLConstruction::SnapToGrid(const FVector2D& Point, const FVector2D& Footprint, const double Grid)
{
	const FVector2D Half = Footprint * 0.5;
	if (Grid <= 0.0)
	{
		return FBox2D(Point - Half, Point + Half);
	}
	// The lower corner on the nearest grid point: the footprint's edges are then on grid lines whatever its size.
	const FVector2D Corner(FMath::RoundToDouble((Point.X - Half.X) / Grid) * Grid, FMath::RoundToDouble((Point.Y - Half.Y) / Grid) * Grid);
	return FBox2D(Corner, Corner + Footprint);
}

float HOLConstruction::SnapYaw(const float Yaw)
{
	return FRotator::NormalizeAxis(FMath::RoundToFloat(Yaw / 90.f) * 90.f);
}

FBox2D HOLConstruction::MakeFootprint(const FVector2D& Centre, const FVector2D& Footprint)
{
	const FVector2D Half = Footprint * 0.5;
	return FBox2D(Centre - Half, Centre + Half);
}

bool HOLConstruction::Overlaps(const FBox2D& A, const FBox2D& B)
{
	return FMath::Min(A.Max.X, B.Max.X) - FMath::Max(A.Min.X, B.Min.X) > TouchTolerance
		&& FMath::Min(A.Max.Y, B.Max.Y) - FMath::Max(A.Min.Y, B.Min.Y) > TouchTolerance;
}

double HOLConstruction::GetDistance(const FBox2D& Footprint, const FVector2D& Point)
{
	const FVector2D Nearest(FMath::Clamp(Point.X, Footprint.Min.X, Footprint.Max.X), FMath::Clamp(Point.Y, Footprint.Min.Y, Footprint.Max.Y));
	return FVector2D::Distance(Nearest, Point);
}

bool HOLConstruction::Overlaps(const FBox2D& Footprint, const FHOLPlacementCircle& Circle)
{
	return GetDistance(Footprint, Circle.Centre) < Circle.Radius - TouchTolerance;
}

EHOLPlacementRefusal HOLConstruction::CheckPlacement(const FHOLPlacementQuery& Query)
{
	const FBox2D& Footprint = Query.Footprint;
	if (Query.MapArea.bIsValid
		&& (Footprint.Min.X < Query.MapArea.Min.X - TouchTolerance || Footprint.Min.Y < Query.MapArea.Min.Y - TouchTolerance
			|| Footprint.Max.X > Query.MapArea.Max.X + TouchTolerance || Footprint.Max.Y > Query.MapArea.Max.Y + TouchTolerance))
	{
		return EHOLPlacementRefusal::OutOfMap;
	}
	for (const FBox2D& Building : Query.Buildings)
	{
		if (Overlaps(Footprint, Building))
		{
			return EHOLPlacementRefusal::Building;
		}
	}
	for (const FHOLPlacementCircle& Source : Query.Sources)
	{
		if (Overlaps(Footprint, Source))
		{
			return EHOLPlacementRefusal::Source;
		}
	}
	for (const FVector2D& Centre : Query.EnemyMainCentres)
	{
		if (GetDistance(Footprint, Centre) < Query.AntiRushRadius)
		{
			return EHOLPlacementRefusal::EnemyCentre;
		}
	}
	return Query.bOnNavMesh ? EHOLPlacementRefusal::None : EHOLPlacementRefusal::NavMesh;
}

bool HOLConstruction::TryPay(FHOLResources& Stock, const FHOLResources& Cost)
{
	if (!Stock.Covers(Cost))
	{
		return false;
	}
	Stock -= Cost;
	return true;
}

FHOLResources HOLConstruction::GetRefund(const FHOLResources& Cost, const bool bWorkStarted)
{
	if (!bWorkStarted)
	{
		return Cost;
	}
	return FHOLResources(Cost.Food / 2, Cost.Wood / 2, Cost.Stone / 2, Cost.Gold / 2);
}

int32 HOLConstruction::GetBuildSteps(const float BuildTime, const float StepSeconds)
{
	return FMath::Max(1, FMath::RoundToInt(BuildTime / FMath::Max(StepSeconds, UE_KINDA_SMALL_NUMBER)));
}

int64 HOLConstruction::GetTotalWork(const int32 BuildSteps)
{
	return static_cast<int64>(FMath::Max(BuildSteps, 1)) * WorkPerBuilder;
}

int32 HOLConstruction::GetWorkPerStep(const int32 Builders, const float Exponent)
{
	if (Builders <= 0)
	{
		return 0;
	}
	return FMath::RoundToInt(WorkPerBuilder * FMath::Pow(static_cast<float>(Builders), FMath::Clamp(Exponent, 0.f, 1.f)));
}

int32 HOLConstruction::GetStepsToFinish(const int32 BuildSteps, const int32 Builders, const float Exponent)
{
	const int64 PerStep = FMath::Max(GetWorkPerStep(FMath::Max(Builders, 1), Exponent), 1);
	return static_cast<int32>((GetTotalWork(BuildSteps) + PerStep - 1) / PerStep);
}

bool HOLConstruction::AddWork(int64& Work, const int64 TotalWork, const int32 Builders, const float Exponent)
{
	if (Work >= TotalWork)
	{
		return false;
	}
	Work = FMath::Min(Work + GetWorkPerStep(Builders, Exponent), TotalWork);
	return Work >= TotalWork;
}

float HOLConstruction::GetProgress(const int64 Work, const int64 TotalWork)
{
	return TotalWork > 0 ? static_cast<float>(FMath::Clamp(static_cast<double>(Work) / static_cast<double>(TotalWork), 0.0, 1.0)) : 1.f;
}

int32 HOLConstruction::GetHealth(const int32 MaxHealth, const int64 Work, const int64 TotalWork)
{
	if (TotalWork <= 0)
	{
		return FMath::Max(MaxHealth, 1);
	}
	const int64 Done = FMath::Clamp(Work, int64(0), TotalWork);
	return FMath::Max(1, static_cast<int32>(static_cast<int64>(MaxHealth) * Done / TotalWork));
}

int32 HOLConstruction::GetPopulationProvided(const int32 TypePopulation, const bool bFinished)
{
	return bFinished ? FMath::Max(TypePopulation, 0) : 0;
}

bool HOLConstruction::BlocksNavigation(const EHOLConstructionState State)
{
	return State != EHOLConstructionState::Foundation;
}

bool HOLConstruction::IsServing(const EHOLConstructionState State)
{
	return State == EHOLConstructionState::Finished;
}

bool HOLConstruction::CanCancel(const EHOLConstructionState State)
{
	return State != EHOLConstructionState::Finished;
}

bool HOLConstruction::HasWorkStarted(const EHOLConstructionState State, const int64 Work)
{
	return State != EHOLConstructionState::Foundation || Work > 0;
}

FHOLResources HOLConstruction::GetCancelRefund(const FHOLResources& Cost, const EHOLConstructionState State, const int64 Work)
{
	return CanCancel(State) ? GetRefund(Cost, HasWorkStarted(State, Work)) : FHOLResources();
}

FBox2D HOLConstruction::GetFootprintBounds(const FVector2D& Centre, const FVector2D& Footprint, const float Yaw)
{
	double Sin = 0.0, Cos = 1.0;
	FMath::SinCos(&Sin, &Cos, FMath::DegreesToRadians(static_cast<double>(Yaw)));
	// Exact on the quarter turns: no rounding left by the sine of 90 degrees.
	Sin = FMath::IsNearlyZero(Sin, 1e-6) ? 0.0 : Sin;
	Cos = FMath::IsNearlyZero(Cos, 1e-6) ? 0.0 : Cos;
	const FVector2D Half = Footprint * 0.5;
	const FVector2D Reach(FMath::Abs(Cos) * Half.X + FMath::Abs(Sin) * Half.Y, FMath::Abs(Sin) * Half.X + FMath::Abs(Cos) * Half.Y);
	return FBox2D(Centre - Reach, Centre + Reach);
}

void HOLConstruction::GetNavProbes(const FBox2D& Footprint, const double Tile, const double CornerInset, TArray<FVector2D>& OutPoints)
{
	OutPoints.Reset();
	const FVector2D Size = Footprint.GetSize();
	const int32 CountX = Tile > 0.0 ? FMath::Max(1, FMath::RoundToInt(Size.X / Tile)) : 1;
	const int32 CountY = Tile > 0.0 ? FMath::Max(1, FMath::RoundToInt(Size.Y / Tile)) : 1;
	for (int32 X = 0; X < CountX; ++X)
	{
		for (int32 Y = 0; Y < CountY; ++Y)
		{
			OutPoints.Emplace(Footprint.Min.X + (X + 0.5) * Size.X / CountX, Footprint.Min.Y + (Y + 0.5) * Size.Y / CountY);
		}
	}
	if (CornerInset > 0.0)
	{
		const FVector2D Inset(FMath::Min(CornerInset, Size.X * 0.5), FMath::Min(CornerInset, Size.Y * 0.5));
		OutPoints.Emplace(Footprint.Min.X + Inset.X, Footprint.Min.Y + Inset.Y);
		OutPoints.Emplace(Footprint.Max.X - Inset.X, Footprint.Min.Y + Inset.Y);
		OutPoints.Emplace(Footprint.Min.X + Inset.X, Footprint.Max.Y - Inset.Y);
		OutPoints.Emplace(Footprint.Max.X - Inset.X, Footprint.Max.Y - Inset.Y);
	}
}

void HOLConstruction::GetBuildSpots(const FBox2D& Footprint, const double Offset, const double Spacing, const FVector2D& From, TArray<FVector2D>& OutSpots)
{
	OutSpots.Reset();
	const FVector2D Size = Footprint.GetSize();
	const FVector2D Centre = Footprint.GetCenter();
	const FVector2D Half = Size * 0.5;
	// Along each side, centred on it, as many as fit between its corners (Spacing apart).
	const auto AddSide = [&OutSpots, &Centre, Offset, Spacing](const FVector2D& Outward, const FVector2D& Along, const double HalfDepth, const double HalfLength)
	{
		const int32 Count = Spacing > 0.0 ? FMath::FloorToInt32(2.0 * HalfLength / Spacing) + 1 : 1;
		for (int32 Index = 0; Index < Count; ++Index)
		{
			const double Position = (Index - (Count - 1) * 0.5) * Spacing;
			OutSpots.Add(Centre + Outward * (HalfDepth + Offset) + Along * Position);
		}
	};
	AddSide(FVector2D(1.0, 0.0), FVector2D(0.0, 1.0), Half.X, Half.Y);
	AddSide(FVector2D(-1.0, 0.0), FVector2D(0.0, 1.0), Half.X, Half.Y);
	AddSide(FVector2D(0.0, 1.0), FVector2D(1.0, 0.0), Half.Y, Half.X);
	AddSide(FVector2D(0.0, -1.0), FVector2D(1.0, 0.0), Half.Y, Half.X);
	// The nearest From first (a stable sort: the same order on every machine and at every call).
	Algo::StableSortBy(OutSpots, [&From](const FVector2D& Spot) { return FVector2D::DistSquared(Spot, From); });
}

bool HOLConstruction::IsAtSite(const FBox2D& Footprint, const FVector2D& Position, const double BodyRadius, const double Gap)
{
	return GetDistance(Footprint, Position) - BodyRadius <= Gap;
}

double HOLConstruction::GetGap(const FBox2D& A, const FBox2D& B)
{
	const double GapX = FMath::Max3(0.0, A.Min.X - B.Max.X, B.Min.X - A.Max.X);
	const double GapY = FMath::Max3(0.0, A.Min.Y - B.Max.Y, B.Min.Y - A.Max.Y);
	return FMath::Sqrt(GapX * GapX + GapY * GapY);
}

int32 HOLConstruction::FindNextSite(const FBox2D& Finished, const TConstArrayView<FBox2D> Sites, const double Range)
{
	int32 Found = INDEX_NONE;
	double Nearest = Range;
	for (int32 Index = 0; Index < Sites.Num(); ++Index)
	{
		const double Gap = GetGap(Finished, Sites[Index]);
		// Strictly nearer than the one kept: the first of two as near (the same choice on every run).
		if (Gap <= Range && (Found == INDEX_NONE || Gap < Nearest))
		{
			Found = Index;
			Nearest = Gap;
		}
	}
	return Found;
}
