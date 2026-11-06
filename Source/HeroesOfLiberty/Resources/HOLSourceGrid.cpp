// Copyright Epic Games, Inc. All Rights Reserved.

#include "Resources/HOLSourceGrid.h"

#include "Simulation/HOLGatherRules.h"

FIntPoint FHOLSourceGrid::GetCell(const FVector2D& Location)
{
	return FIntPoint(FMath::FloorToInt32(Location.X / CellSize), FMath::FloorToInt32(Location.Y / CellSize));
}

void FHOLSourceGrid::Add(const int32 Index, const EHOLResource Resource, const FVector2D& Location, const float Radius)
{
	const int32 ResourceIndex = static_cast<int32>(Resource);
	if (!ensure(ResourceIndex >= 0 && ResourceIndex < UE_ARRAY_COUNT(ByResource)))
	{
		return;
	}
	FResourceCells& Cells = ByResource[ResourceIndex];
	const FIntPoint Cell = GetCell(Location);
	FEntry& Entry = Cells.Cells.FindOrAdd(Cell).AddDefaulted_GetRef();
	Entry.Index = Index;
	Entry.Location = Location;
	Cells.Min = FIntPoint(FMath::Min(Cells.Min.X, Cell.X), FMath::Min(Cells.Min.Y, Cell.Y));
	Cells.Max = FIntPoint(FMath::Max(Cells.Max.X, Cell.X), FMath::Max(Cells.Max.Y, Cell.Y));
	++Cells.Num;
	++NumEntries;
	MaxRadius = FMath::Max(MaxRadius, Radius);
}

int32 FHOLSourceGrid::ForEachInSquare(const FVector2D& Centre, const double HalfSide, TFunctionRef<void(const FEntry&)> Visit) const
{
	int32 Visits = 0;
	const FIntPoint From = GetCell(Centre - FVector2D(HalfSide, HalfSide));
	const FIntPoint To = GetCell(Centre + FVector2D(HalfSide, HalfSide));
	for (const FResourceCells& Cells : ByResource)
	{
		if (Cells.Num == 0)
		{
			continue;
		}
		const auto VisitCell = [&Visits, &Visit](const TArray<FEntry>& Entries)
		{
			for (const FEntry& Entry : Entries)
			{
				++Visits;
				Visit(Entry);
			}
		};
		// Only the cells of the square that hold something are met: the square, cut to the cells the resource spreads over.
		const int64 NumInSquare = (static_cast<int64>(FMath::Min(To.X, Cells.Max.X)) - FMath::Max(From.X, Cells.Min.X) + 1)
			* (static_cast<int64>(FMath::Min(To.Y, Cells.Max.Y)) - FMath::Max(From.Y, Cells.Min.Y) + 1);
		if (NumInSquare <= 0)
		{
			continue;
		}
		if (NumInSquare > Cells.Cells.Num())
		{
			// A square wider than the map: every cell, tested.
			for (const TPair<FIntPoint, TArray<FEntry>>& Pair : Cells.Cells)
			{
				if (Pair.Key.X >= From.X && Pair.Key.X <= To.X && Pair.Key.Y >= From.Y && Pair.Key.Y <= To.Y)
				{
					VisitCell(Pair.Value);
				}
			}
			continue;
		}
		for (int32 X = FMath::Max(From.X, Cells.Min.X); X <= FMath::Min(To.X, Cells.Max.X); ++X)
		{
			for (int32 Y = FMath::Max(From.Y, Cells.Min.Y); Y <= FMath::Min(To.Y, Cells.Max.Y); ++Y)
			{
				if (const TArray<FEntry>* Entries = Cells.Cells.Find(FIntPoint(X, Y)))
				{
					VisitCell(*Entries);
				}
			}
		}
	}
	return Visits;
}

int32 FHOLSourceGrid::FindNearest(const FHOLGatherDeposit& Deposit, const EHOLResource Resource, const int32 Count, TFunctionRef<bool(int32)> Accept,
	TArray<int32>& OutIndices) const
{
	OutIndices.Reset();
	const int32 ResourceIndex = static_cast<int32>(Resource);
	if (Count <= 0 || ResourceIndex < 0 || ResourceIndex >= UE_ARRAY_COUNT(ByResource) || ByResource[ResourceIndex].Num == 0)
	{
		return 0;
	}
	const FResourceCells& Cells = ByResource[ResourceIndex];
	const double Reach = Deposit.HalfExtent.Size();
	// The best so far, nearest first (a tie to the lower index), at most Count.
	TArray<TPair<double, int32>, TInlineAllocator<16>> Best;
	int32 Visits = 0;
	const auto Consider = [&](const FIntPoint& Cell)
	{
		if (Cell.X < Cells.Min.X || Cell.X > Cells.Max.X || Cell.Y < Cells.Min.Y || Cell.Y > Cells.Max.Y)
		{
			return;
		}
		// A cell whose nearest point is farther from the footprint (less its half diagonal) than the Count-th best holds nothing to keep.
		if (Best.Num() == Count)
		{
			const double DX = FMath::Max3(Cell.X * CellSize - Deposit.Centre.X, Deposit.Centre.X - (Cell.X + 1) * CellSize, 0.);
			const double DY = FMath::Max3(Cell.Y * CellSize - Deposit.Centre.Y, Deposit.Centre.Y - (Cell.Y + 1) * CellSize, 0.);
			if (FMath::Sqrt(DX * DX + DY * DY) - Reach > Best.Last().Key)
			{
				return;
			}
		}
		const TArray<FEntry>* Entries = Cells.Cells.Find(Cell);
		if (!Entries)
		{
			return;
		}
		for (const FEntry& Entry : *Entries)
		{
			++Visits;
			if (!Accept(Entry.Index))
			{
				continue;
			}
			const TPair<double, int32> Candidate(HOLGather::GetDepositDistance(Entry.Location, Deposit), Entry.Index);
			const auto Before = [](const TPair<double, int32>& A, const TPair<double, int32>& B) { return A.Key != B.Key ? A.Key < B.Key : A.Value < B.Value; };
			if (Best.Num() == Count && !Before(Candidate, Best.Last()))
			{
				continue;
			}
			int32 At = Best.Num();
			while (At > 0 && Before(Candidate, Best[At - 1]))
			{
				--At;
			}
			Best.Insert(Candidate, At);
			if (Best.Num() > Count)
			{
				Best.Pop();
			}
		}
	};

	// Ring after ring of cells round the footprint's centre. An entry in a cell Ring cells away (on an axis) is farther than
	// (Ring - 1) cells from the centre, so farther than that less the footprint's half diagonal from the footprint: once the
	// Count-th best is nearer than what the next ring can hold, nothing beyond can take its place.
	const FIntPoint Home = GetCell(Deposit.Centre);
	const int32 LastRing = FMath::Max(FMath::Max(Home.X - Cells.Min.X, Cells.Max.X - Home.X), FMath::Max(Home.Y - Cells.Min.Y, Cells.Max.Y - Home.Y));
	for (int32 Ring = 0; Ring <= LastRing; ++Ring)
	{
		if (Best.Num() == Count && Best.Last().Key < (Ring - 1) * CellSize - Reach)
		{
			break;
		}
		if (Ring == 0)
		{
			Consider(Home);
			continue;
		}
		for (int32 Step = -Ring; Step <= Ring; ++Step)
		{
			Consider(FIntPoint(Home.X + Step, Home.Y - Ring));
			Consider(FIntPoint(Home.X + Step, Home.Y + Ring));
			if (Step > -Ring && Step < Ring)
			{
				Consider(FIntPoint(Home.X - Ring, Home.Y + Step));
				Consider(FIntPoint(Home.X + Ring, Home.Y + Step));
			}
		}
	}
	for (const TPair<double, int32>& Pair : Best)
	{
		OutIndices.Add(Pair.Value);
	}
	return Visits;
}
