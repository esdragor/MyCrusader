// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLGroupMoveRules.h"

namespace HOLGroupMove
{
	namespace
	{
		int32 FindRoot(TArray<int32>& Parents, int32 Index)
		{
			while (Parents[Index] != Index)
			{
				Parents[Index] = Parents[Parents[Index]];
				Index = Parents[Index];
			}
			return Index;
		}

		/** Mean of points, and their root-mean-square distance to it (at least 1 cm). */
		FVector2D GetCentre(TConstArrayView<FVector2D> Points, double& OutSpread)
		{
			FVector2D Centre = FVector2D::ZeroVector;
			for (const FVector2D& Point : Points)
			{
				Centre += Point;
			}
			Centre /= FMath::Max(Points.Num(), 1);
			double SquaredSum = 0.;
			for (const FVector2D& Point : Points)
			{
				SquaredSum += FVector2D::DistSquared(Point, Centre);
			}
			OutSpread = FMath::Max(FMath::Sqrt(SquaredSum / FMath::Max(Points.Num(), 1)), 1.);
			return Centre;
		}
	}

	int32 MakeClusters(TConstArrayView<FVector2D> Positions, const double LinkDistance, TArray<int32>& OutClusterOfUnit)
	{
		// Union of the neighbours, every pair: an order holds a player's selection (175 units at most, D03).
		const int32 Num = Positions.Num();
		TArray<int32> Parents;
		Parents.SetNumUninitialized(Num);
		for (int32 Index = 0; Index < Num; ++Index)
		{
			Parents[Index] = Index;
		}
		const double LinkSquared = LinkDistance * LinkDistance;
		for (int32 First = 0; First < Num; ++First)
		{
			for (int32 Second = First + 1; Second < Num; ++Second)
			{
				if (FVector2D::DistSquared(Positions[First], Positions[Second]) <= LinkSquared)
				{
					const int32 FirstRoot = FindRoot(Parents, First);
					const int32 SecondRoot = FindRoot(Parents, Second);
					if (FirstRoot != SecondRoot)
					{
						Parents[FMath::Max(FirstRoot, SecondRoot)] = FMath::Min(FirstRoot, SecondRoot);
					}
				}
			}
		}

		// Clusters numbered in the order of their first unit.
		OutClusterOfUnit.SetNumUninitialized(Num);
		TMap<int32, int32> ClusterOfRoot;
		for (int32 Index = 0; Index < Num; ++Index)
		{
			const int32 Root = FindRoot(Parents, Index);
			const int32* Cluster = ClusterOfRoot.Find(Root);
			OutClusterOfUnit[Index] = Cluster ? *Cluster : ClusterOfRoot.Add(Root, ClusterOfRoot.Num());
		}
		return ClusterOfRoot.Num();
	}

	void MakeSlots(const int32 Count, const double Spacing, TArray<FVector2D>& OutOffsets)
	{
		OutOffsets.Reset();
		if (Count <= 0)
		{
			return;
		}
		// Honeycomb rows Spacing * sqrt(3) / 2 apart, every other row shifted by half a gap; the disc that holds Count
		// cells (each sqrt(3) / 2 * Spacing^2) with a margin of two rings.
		const double RowGap = Spacing * UE_HALF_SQRT_3;
		const int32 Rings = FMath::CeilToInt32(FMath::Sqrt(Count / (UE_PI * UE_HALF_SQRT_3))) + 2;
		TArray<FVector2D> Cells;
		for (int32 Row = -Rings; Row <= Rings; ++Row)
		{
			const double Shift = (Row & 1) ? Spacing * 0.5 : 0.;
			for (int32 Column = -Rings - 1; Column <= Rings; ++Column)
			{
				Cells.Emplace(Row * RowGap, Column * Spacing + Shift);
			}
		}
		// Nearest first; on a tie, the one in front, then the one on the left: the same order every time.
		Cells.Sort([](const FVector2D& A, const FVector2D& B)
		{
			const double DistanceA = A.SizeSquared();
			const double DistanceB = B.SizeSquared();
			if (!FMath::IsNearlyEqual(DistanceA, DistanceB, 1.))
			{
				return DistanceA < DistanceB;
			}
			if (!FMath::IsNearlyEqual(A.X, B.X, 0.1))
			{
				return A.X > B.X;
			}
			return A.Y < B.Y;
		});
		OutOffsets.Append(Cells.GetData(), FMath::Min(Count, Cells.Num()));
	}

	FVector2D ToWorld(const FVector2D& Offset, const FVector2D& Forward)
	{
		const FVector2D Front = Forward.GetSafeNormal().IsNearlyZero() ? FVector2D(1., 0.) : Forward.GetSafeNormal();
		const FVector2D Right(-Front.Y, Front.X);
		return Front * Offset.X + Right * Offset.Y;
	}

	void AssignSlots(TConstArrayView<FVector2D> Positions, TConstArrayView<FVector2D> Slots, TArray<int32>& OutSlotOfUnit)
	{
		const int32 NumUnits = Positions.Num();
		const int32 NumSlots = Slots.Num();
		OutSlotOfUnit.Init(INDEX_NONE, NumUnits);
		if (NumUnits == 0 || NumSlots == 0)
		{
			return;
		}

		// Both sets brought to the same centre and spread: what is compared is the shape, not where it is.
		double UnitSpread = 1.;
		double SlotSpread = 1.;
		const FVector2D UnitCentre = GetCentre(Positions, UnitSpread);
		const FVector2D SlotCentre = GetCentre(Slots, SlotSpread);

		struct FPair
		{
			double DistanceSquared;
			int32 Unit;
			int32 Slot;
		};
		TArray<FPair> Pairs;
		Pairs.Reserve(NumUnits * NumSlots);
		for (int32 Unit = 0; Unit < NumUnits; ++Unit)
		{
			const FVector2D UnitShape = (Positions[Unit] - UnitCentre) / UnitSpread;
			for (int32 Slot = 0; Slot < NumSlots; ++Slot)
			{
				const FVector2D SlotShape = (Slots[Slot] - SlotCentre) / SlotSpread;
				Pairs.Add({ FVector2D::DistSquared(UnitShape, SlotShape), Unit, Slot });
			}
		}
		// Closest pairs first; ties by unit then slot, so that the result never depends on the sort.
		Pairs.Sort([](const FPair& A, const FPair& B)
		{
			if (A.DistanceSquared != B.DistanceSquared)
			{
				return A.DistanceSquared < B.DistanceSquared;
			}
			return A.Unit != B.Unit ? A.Unit < B.Unit : A.Slot < B.Slot;
		});

		TBitArray<> SlotTaken(false, NumSlots);
		int32 Assigned = 0;
		for (const FPair& Pair : Pairs)
		{
			if (OutSlotOfUnit[Pair.Unit] != INDEX_NONE || SlotTaken[Pair.Slot])
			{
				continue;
			}
			OutSlotOfUnit[Pair.Unit] = Pair.Slot;
			SlotTaken[Pair.Slot] = true;
			if (++Assigned == FMath::Min(NumUnits, NumSlots))
			{
				break;
			}
		}
	}
}
