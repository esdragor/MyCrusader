// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLConstructionRules.h"

#include "Algo/StableSort.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Simulation/HOLUnitTypes.h"

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

FVector2D HOLConstruction::GetBodySize(const FVector2D& Footprint, const double Inset)
{
	const auto Side = [Inset](const double Tiles)
	{
		return FMath::Max(Tiles - 2.0 * FMath::Max(0.0, Inset), FMath::Min(Tiles, MinBodySide));
	};
	return FVector2D(Side(Footprint.X), Side(Footprint.Y));
}

FBox2D HOLConstruction::GetBody(const FBox2D& Footprint, const double Inset)
{
	return MakeFootprint(Footprint.GetCenter(), GetBodySize(Footprint.GetSize(), Inset));
}

void HOLConstruction::FindWays(const TConstArrayView<FBox2D> Bodies, const double MaxWidth, TArray<FBox2D>& OutWays)
{
	OutWays.Reset();
	for (int32 First = 0; First < Bodies.Num(); ++First)
	{
		for (int32 Second = First + 1; Second < Bodies.Num(); ++Second)
		{
			const FBox2D& A = Bodies[First];
			const FBox2D& B = Bodies[Second];
			const double AcrossX = FMath::Min(A.Max.X, B.Max.X) - FMath::Max(A.Min.X, B.Min.X);
			const double AcrossY = FMath::Min(A.Max.Y, B.Max.Y) - FMath::Max(A.Min.Y, B.Min.Y);
			// Side by side along X (a way running along Y), or along Y (a way running along X); never on a diagonal.
			const double GapX = FMath::Max(B.Min.X - A.Max.X, A.Min.X - B.Max.X);
			const double GapY = FMath::Max(B.Min.Y - A.Max.Y, A.Min.Y - B.Max.Y);
			if (GapX > 0.0 && GapX <= MaxWidth && AcrossY > 0.0)
			{
				const double West = A.Max.X <= B.Min.X ? A.Max.X : B.Max.X;
				OutWays.Emplace(FVector2D(West, FMath::Max(A.Min.Y, B.Min.Y)), FVector2D(West + GapX, FMath::Min(A.Max.Y, B.Max.Y)));
			}
			else if (GapY > 0.0 && GapY <= MaxWidth && AcrossX > 0.0)
			{
				const double South = A.Max.Y <= B.Min.Y ? A.Max.Y : B.Max.Y;
				OutWays.Emplace(FVector2D(FMath::Max(A.Min.X, B.Min.X), South), FVector2D(FMath::Min(A.Max.X, B.Max.X), South + GapY));
			}
		}
	}
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

FBox2D HOLConstruction::SnapOutward(const FBox2D& Box, const double Grid)
{
	if (Grid <= 0.0)
	{
		return Box;
	}
	// Within TouchTolerance of a grid line is on it: a footprint already on the grid never grows by a tile.
	const auto Down = [Grid](const double Value) { return FMath::FloorToDouble((Value + TouchTolerance) / Grid) * Grid; };
	const auto Up = [Grid](const double Value) { return FMath::CeilToDouble((Value - TouchTolerance) / Grid) * Grid; };
	return FBox2D(FVector2D(Down(Box.Min.X), Down(Box.Min.Y)), FVector2D(Up(Box.Max.X), Up(Box.Max.Y)));
}

bool HOLConstruction::FindObstacle(const FHOLPlacementQuery& Query, const FVector2D& Aim, const double Grid, FBox2D& OutObstacle)
{
	bool bFound = false;
	bool bContains = false;
	double Nearest = TNumericLimits<double>::Max();
	// The one containing the aim wins; else the nearest; the first of two as near.
	const auto Consider = [&](const FBox2D& Box, const bool bInside, const double Distance)
	{
		if ((bInside && !bContains) || (bInside == bContains && Distance < Nearest))
		{
			bFound = true;
			bContains = bInside;
			Nearest = Distance;
			OutObstacle = Box;
		}
	};
	for (const FBox2D& Building : Query.Buildings)
	{
		if (Overlaps(Query.Footprint, Building))
		{
			const double Distance = GetDistance(Building, Aim);
			Consider(Building, Distance <= 0.0, Distance);
		}
	}
	for (const FHOLPlacementCircle& Source : Query.Sources)
	{
		if (Overlaps(Query.Footprint, Source))
		{
			const double Distance = FMath::Max(0.0, FVector2D::Distance(Source.Centre, Aim) - Source.Radius);
			const FVector2D Half(Source.Radius, Source.Radius);
			Consider(FBox2D(Source.Centre - Half, Source.Centre + Half), Distance <= 0.0, Distance);
		}
	}
	if (bFound)
	{
		OutObstacle = SnapOutward(OutObstacle, Grid);
	}
	return bFound;
}

bool HOLConstruction::FindFreeFootprint(const FHOLPlacementQuery& Query, const FBox2D& Obstacle, const FVector2D& Aim, const double Grid,
	const int32 MaxRings, TFunctionRef<bool(const FBox2D&)> IsOnNavMesh, FBox2D& OutFootprint, int32* OutRing)
{
	if (Grid <= 0.0 || MaxRings < 0)
	{
		return false;
	}
	const FVector2D Size = Query.Footprint.GetSize();
	const FBox2D Box = SnapOutward(Obstacle, Grid);
	// In tiles: the obstacle's box and the footprint's size (a footprint is a whole number of tiles, D311).
	const int64 MinX = FMath::RoundToInt64(Box.Min.X / Grid), MinY = FMath::RoundToInt64(Box.Min.Y / Grid);
	const int64 MaxX = FMath::RoundToInt64(Box.Max.X / Grid), MaxY = FMath::RoundToInt64(Box.Max.Y / Grid);
	const int64 Width = FMath::Max<int64>(1, FMath::CeilToInt64(Size.X / Grid - TouchTolerance / Grid));
	const int64 Height = FMath::Max<int64>(1, FMath::CeilToInt64(Size.Y / Grid - TouchTolerance / Grid));

	// What may refuse a candidate, gathered once: only the buildings and sources near the search area (the obstacle's box
	// grown by MaxRings tiles and the footprint's size); the enemy centres and the map as they are.
	const FBox2D Area = Box.ExpandBy(FVector2D(MaxRings * Grid + Size.X, MaxRings * Grid + Size.Y));
	TArray<FBox2D> Buildings;
	for (const FBox2D& Building : Query.Buildings)
	{
		if (Overlaps(Area, Building))
		{
			Buildings.Add(Building);
		}
	}
	TArray<FHOLPlacementCircle> Sources;
	for (const FHOLPlacementCircle& Source : Query.Sources)
	{
		if (Overlaps(Area, Source))
		{
			Sources.Add(Source);
		}
	}
	FHOLPlacementQuery Candidate = Query;
	Candidate.Buildings = Buildings;
	Candidate.Sources = Sources;
	Candidate.bOnNavMesh = true;

	TArray<TPair<double, FBox2D>> Ring;
	for (int32 Gap = 0; Gap <= MaxRings; ++Gap)
	{
		// Every lower corner whose footprint leaves exactly Gap tiles to the box (the larger of the gaps along X and Y).
		Ring.Reset();
		for (int64 X = MinX - Width - Gap; X <= MaxX + Gap; ++X)
		{
			const int64 GapX = FMath::Max(MinX - (X + Width), X - MaxX);
			for (int64 Y = MinY - Height - Gap; Y <= MaxY + Gap; ++Y)
			{
				const int64 GapY = FMath::Max(MinY - (Y + Height), Y - MaxY);
				if (FMath::Max(GapX, GapY) != Gap)
				{
					continue;
				}
				const FVector2D Corner(X * Grid, Y * Grid);
				const FBox2D Footprint(Corner, Corner + Size);
				Ring.Emplace(FVector2D::DistSquared(Footprint.GetCenter(), Aim), Footprint);
			}
		}
		Algo::StableSortBy(Ring, [](const TPair<double, FBox2D>& Entry) { return Entry.Key; });
		for (const TPair<double, FBox2D>& Entry : Ring)
		{
			Candidate.Footprint = Entry.Value;
			if (CheckPlacement(Candidate) == EHOLPlacementRefusal::None && IsOnNavMesh(Entry.Value))
			{
				OutFootprint = Entry.Value;
				if (OutRing)
				{
					*OutRing = Gap;
				}
				return true;
			}
		}
	}
	return false;
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

FHOLResources HOLConstruction::GetRefund(const FHOLResources& Cost, const int64 Work, const int64 TotalWork)
{
	if (TotalWork <= 0 || Work <= 0)
	{
		return Cost;
	}
	const int64 Left = TotalWork - FMath::Min(Work, TotalWork);
	const auto Share = [Left, TotalWork](const int32 Amount) { return static_cast<int32>(static_cast<int64>(Amount) * Left / TotalWork); };
	return FHOLResources(Share(Cost.Food), Share(Cost.Wood), Share(Cost.Stone), Share(Cost.Gold));
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

int32 HOLConstruction::GetHealthWithDamage(const int32 MaxHealth, const int64 Work, const int64 TotalWork, const int32 Damage)
{
	return FMath::Max(0, GetHealth(MaxHealth, Work, TotalWork) - FMath::Max(Damage, 0));
}

int32 HOLConstruction::GetBuildingDamage(const int32 Damage, const EHOLAttackType AttackType, const int32 RangedArmor)
{
	const int32 Armor = AttackType == EHOLAttackType::Ranged ? FMath::Max(RangedArmor, 0) : 0;
	return FMath::Max(1, Damage - Armor);
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

FHOLResources HOLConstruction::GetCancelRefund(const FHOLResources& Cost, const EHOLConstructionState State, const int64 Work, const int64 TotalWork)
{
	return CanCancel(State) ? GetRefund(Cost, Work, TotalWork) : FHOLResources();
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

bool HOLConstruction::SendsBuildersToGather(const FName TypeName, const TConstArrayView<EHOLResource> Accepts)
{
	return !Accepts.IsEmpty() && TypeName != HOLBuildingTypeNames::MainCentre;
}

double HOLConstruction::GetSourceGap(const FBox2D& Footprint, const FVector2D& Centre, const double Radius)
{
	return FMath::Max(0.0, GetDistance(Footprint, Centre) - Radius);
}

int32 HOLConstruction::FindCampSource(const FBox2D& Camp, const TConstArrayView<EHOLResource> Accepts, const TConstArrayView<FHOLCampSource> Sources,
	const double Range, double* OutGap)
{
	int32 Found = INDEX_NONE;
	double Nearest = Range;
	for (int32 Index = 0; Index < Sources.Num(); ++Index)
	{
		const FHOLCampSource& Source = Sources[Index];
		if (!Source.bStanding || !Accepts.Contains(Source.Resource))
		{
			continue;
		}
		const double Gap = GetSourceGap(Camp, Source.Centre, Source.Radius);
		// Strictly nearer than the one kept: the first of two as near (the same choice on every run).
		if (Gap <= Range && (Found == INDEX_NONE || Gap < Nearest))
		{
			Found = Index;
			Nearest = Gap;
		}
	}
	if (OutGap && Found != INDEX_NONE)
	{
		*OutGap = Nearest;
	}
	return Found;
}

double HOLConstruction::GetSegmentDistance(const FBox2D& Footprint, const FVector2D& A, const FVector2D& B)
{
	// Through the box, by its two slabs: the part of the segment inside each one, overlapping on both axes.
	const FVector2D Delta = B - A;
	double Enter = 0.0;
	double Exit = 1.0;
	bool bMeets = true;
	for (int32 Axis = 0; Axis < 2 && bMeets; ++Axis)
	{
		const double Start = A[Axis];
		const double Step = Delta[Axis];
		if (FMath::IsNearlyZero(Step))
		{
			bMeets = Start >= Footprint.Min[Axis] && Start <= Footprint.Max[Axis];
			continue;
		}
		double In = (Footprint.Min[Axis] - Start) / Step;
		double Out = (Footprint.Max[Axis] - Start) / Step;
		if (In > Out)
		{
			Swap(In, Out);
		}
		Enter = FMath::Max(Enter, In);
		Exit = FMath::Min(Exit, Out);
		bMeets = Enter <= Exit;
	}
	if (bMeets)
	{
		return 0.0;
	}
	// Apart, the nearest points are an end of the segment or a corner of the box.
	double Distance = FMath::Min(GetDistance(Footprint, A), GetDistance(Footprint, B));
	for (const FVector2D& Corner : { Footprint.Min, Footprint.Max, FVector2D(Footprint.Min.X, Footprint.Max.Y), FVector2D(Footprint.Max.X, Footprint.Min.Y) })
	{
		Distance = FMath::Min(Distance, FVector2D::Distance(Corner, FMath::ClosestPointOnSegment2D(Corner, A, B)));
	}
	return Distance;
}

bool HOLConstruction::DoesWalkCross(const FBox2D& Footprint, const double Margin, const TConstArrayView<FVector2D> Points)
{
	if (Points.Num() == 1)
	{
		return GetDistance(Footprint, Points[0]) < Margin;
	}
	for (int32 Index = 0; Index + 1 < Points.Num(); ++Index)
	{
		if (GetSegmentDistance(Footprint, Points[Index], Points[Index + 1]) < Margin)
		{
			return true;
		}
	}
	return false;
}

FVector2D HOLConstruction::PushOutOfFootprint(const FBox2D& Footprint, const double Margin, const FVector2D& Point)
{
	if (GetDistance(Footprint, Point) >= Margin)
	{
		return Point;
	}
	const FBox2D Widened = Footprint.ExpandBy(Margin);
	const FVector2D Centre = Footprint.GetCenter();
	FVector2D Direction = Point - Centre;
	if (!Direction.Normalize())
	{
		Direction = FVector2D(1.0, 0.0);
	}
	// The nearest exit of the widened box along the ray: the smaller of the distances to its two facing sides.
	const FVector2D Half = Widened.GetExtent();
	const double ToX = FMath::IsNearlyZero(Direction.X) ? TNumericLimits<double>::Max() : Half.X / FMath::Abs(Direction.X);
	const double ToY = FMath::IsNearlyZero(Direction.Y) ? TNumericLimits<double>::Max() : Half.Y / FMath::Abs(Direction.Y);
	return Centre + Direction * FMath::Min(ToX, ToY);
}

double HOLConstruction::GetKeepOutInsideGap(const double Radius, const double Clearance, const double VisualGap)
{
	// The farther of the two, its whole body beyond it, and 1 cm: the rounding does not leave it a hair short.
	return FMath::Max(Clearance, VisualGap) + Radius + 1.0;
}

bool HOLConstruction::KeepOutOfBody(const FVector2D& HalfExtent, const double InsideGap, const double OutsideGap, FVector2D& Local)
{
	const FVector2D Outside(FMath::Abs(Local.X) - HalfExtent.X, FMath::Abs(Local.Y) - HalfExtent.Y);
	if (Outside.X <= 0. && Outside.Y <= 0.)
	{
		// Its centre inside: out by the nearest side.
		if (Outside.X >= Outside.Y)
		{
			Local.X = (Local.X >= 0. ? 1. : -1.) * (HalfExtent.X + InsideGap);
		}
		else
		{
			Local.Y = (Local.Y >= 0. ? 1. : -1.) * (HalfExtent.Y + InsideGap);
		}
		return true;
	}
	// Its centre outside, its body too near: away from the body's nearest point.
	const FVector2D Nearest(FMath::Clamp(Local.X, -HalfExtent.X, HalfExtent.X), FMath::Clamp(Local.Y, -HalfExtent.Y, HalfExtent.Y));
	const FVector2D Away = Local - Nearest;
	const double Distance = Away.Size();
	if (Distance >= OutsideGap - UE_KINDA_SMALL_NUMBER)
	{
		return false;
	}
	Local = Nearest + Away / Distance * OutsideGap;
	return true;
}
