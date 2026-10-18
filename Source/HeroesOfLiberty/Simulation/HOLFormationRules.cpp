// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLFormationRules.h"

#include "Simulation/HOLGroupMoveRules.h"

namespace HOLFormation
{
	namespace
	{
		/** The smallest whole number whose square is at least Value (Value >= 0). */
		int32 CeilSqrt(const int32 Value)
		{
			int32 Root = FMath::Max(FMath::FloorToInt32(FMath::Sqrt(static_cast<double>(Value))), 0);
			while (Root * Root < Value)
			{
				++Root;
			}
			while (Root > 0 && (Root - 1) * (Root - 1) >= Value)
			{
				--Root;
			}
			return Root;
		}

		/** The march's frame: Front normalised (+X when Forward is null), Right as HOLGroupMove::ToWorld turns it. */
		void GetFrame(const FVector2D& Forward, FVector2D& OutFront, FVector2D& OutRight)
		{
			OutFront = Forward.GetSafeNormal().IsNearlyZero() ? FVector2D(1., 0.) : Forward.GetSafeNormal();
			OutRight = FVector2D(-OutFront.Y, OutFront.X);
		}

		/** Whether the paths A-B and C-D cross (strictly: touching at an end does not count). */
		bool Cross(const FVector2D& A, const FVector2D& B, const FVector2D& C, const FVector2D& D)
		{
			const double ABC = FVector2D::CrossProduct(B - A, C - A);
			const double ABD = FVector2D::CrossProduct(B - A, D - A);
			const double CDA = FVector2D::CrossProduct(D - C, A - C);
			const double CDB = FVector2D::CrossProduct(D - C, B - C);
			return ABC * ABD < 0. && CDA * CDB < 0.;
		}
	}

	void MakePeasantRows(const int32 Count, TArray<int32>& OutWidths)
	{
		MakeRowsOfWidth(Count, Count <= 5 ? Count : CeilSqrt(2 * Count), OutWidths);
	}

	void MakeSquareRows(const int32 Count, TArray<int32>& OutWidths)
	{
		MakeRowsOfWidth(Count, CeilSqrt(Count), OutWidths);
	}

	void MakeMixedRows(const int32 Count, TArray<int32>& OutWidths)
	{
		OutWidths.Reset();
		if (Count <= 0)
		{
			return;
		}
		// Odd rows: a unit in the middle, on the march's axis.
		int32 Width = CeilSqrt(Count);
		Width += (Width & 1) ? 0 : 1;
		const int32 Rows = Count / Width;
		const int32 Remainder = Count - Rows * Width;
		OutWidths.Init(Width, Rows);
		if (Remainder == 0)
		{
			return;
		}
		if (Rows >= 2 && Remainder <= Rows)
		{
			// The remainder spread on the front rows, two by two (each row stays odd), the odd one last.
			for (int32 Row = 0; Row < Remainder / 2; ++Row)
			{
				OutWidths[Row] += 2;
			}
			if (Remainder & 1)
			{
				OutWidths[Remainder / 2] += 1;
			}
			return;
		}
		OutWidths.Add(Remainder);
	}

	void MakeRowsOfWidth(const int32 Count, const int32 Width, TArray<int32>& OutWidths)
	{
		OutWidths.Reset();
		if (Count <= 0)
		{
			return;
		}
		const int32 Clamped = FMath::Clamp(Width, 1, Count);
		for (int32 Left = Count; Left > 0; Left -= Clamped)
		{
			OutWidths.Add(FMath::Min(Clamped, Left));
		}
	}

	void PlaceKinds(TConstArrayView<int32> RowWidths, TConstArrayView<int32> CountOfKind, TArray<int32>& OutKindOfPlace)
	{
		OutKindOfPlace.Reset();
		int32 Kind = 0;
		int32 KindLeft = CountOfKind.Num() > 0 ? FMath::Max(CountOfKind[0], 0) : 0;
		auto SkipEmptyKinds = [&]()
		{
			while (KindLeft == 0 && Kind + 1 < CountOfKind.Num())
			{
				KindLeft = FMath::Max(CountOfKind[++Kind], 0);
			}
		};
		SkipEmptyKinds();

		struct FPart
		{
			int32 Kind;
			int32 Count;
		};
		TArray<FPart, TInlineAllocator<4>> Parts;
		for (const int32 Width : RowWidths)
		{
			const int32 RowStart = OutKindOfPlace.Num();
			OutKindOfPlace.AddUninitialized(FMath::Max(Width, 0));
			for (int32 Column = 0; Column < Width; ++Column)
			{
				OutKindOfPlace[RowStart + Column] = INDEX_NONE;
			}

			// The row's parts, in the kinds' order; the first one ends here when its kind began in a row in front.
			Parts.Reset();
			const bool bFirstEnds = Kind < CountOfKind.Num() && KindLeft > 0 && KindLeft < CountOfKind[Kind];
			for (int32 Taken = 0; Taken < Width && KindLeft > 0;)
			{
				const int32 Count = FMath::Min(KindLeft, Width - Taken);
				Parts.Add({ Kind, Count });
				Taken += Count;
				KindLeft -= Count;
				SkipEmptyKinds();
			}
			// The kinds that start here hold the centre, the kind that ends here the wings.
			if (bFirstEnds && Parts.Num() > 1)
			{
				const FPart Ending = Parts[0];
				Parts.RemoveAt(0);
				Parts.Add(Ending);
			}

			// Centred blocks that grow outwards, part after part; an odd wing has its extra place on the left, so a block
			// of K places in the row starts at ceil((Width - K) / 2).
			int32 Held = 0;
			for (const FPart& Part : Parts)
			{
				const int32 OldFirst = (Width - Held + 1) / 2;
				const int32 OldEnd = OldFirst + Held;
				Held += Part.Count;
				const int32 First = (Width - Held + 1) / 2;
				for (int32 Column = First; Column < First + Held; ++Column)
				{
					// The blocks are nested: the old one sits inside the new one.
					if (Column < OldFirst || Column >= OldEnd)
					{
						OutKindOfPlace[RowStart + Column] = Part.Kind;
					}
				}
			}
		}
	}

	void MakePlaces(TConstArrayView<int32> RowWidths, const FVector2D& Target, const FVector2D& Forward,
		const double ColumnSpacing, const double RowSpacing, TArray<FVector2D>& OutPlaces)
	{
		OutPlaces.Reset();
		for (int32 Row = 0; Row < RowWidths.Num(); ++Row)
		{
			const int32 Width = RowWidths[Row];
			for (int32 Column = 0; Column < Width; ++Column)
			{
				// X forward, Y to the right: the left end of the row first.
				const FVector2D Offset(-Row * RowSpacing, (Column - (Width - 1) * 0.5) * ColumnSpacing);
				OutPlaces.Add(Target + HOLGroupMove::ToWorld(Offset, Forward));
			}
		}
	}

	void AssignPlaces(TConstArrayView<FVector2D> Positions, TConstArrayView<FVector2D> Places, const FVector2D& Forward,
		TArray<int32>& OutPlaceOfUnit)
	{
		const int32 NumUnits = Positions.Num();
		const int32 NumPlaces = Places.Num();
		OutPlaceOfUnit.Init(INDEX_NONE, NumUnits);
		if (NumUnits == 0 || NumPlaces == 0)
		{
			return;
		}

		// A first guess: units and places in their order from left to right (the front one first on a tie).
		FVector2D Front;
		FVector2D Right;
		GetFrame(Forward, Front, Right);
		auto SortAcross = [&Front, &Right](TConstArrayView<FVector2D> Points, TArray<int32>& OutOrder)
		{
			OutOrder.SetNumUninitialized(Points.Num());
			for (int32 Index = 0; Index < Points.Num(); ++Index)
			{
				OutOrder[Index] = Index;
			}
			OutOrder.Sort([&Points, &Front, &Right](const int32 A, const int32 B)
			{
				const double AcrossA = Points[A] | Right;
				const double AcrossB = Points[B] | Right;
				if (!FMath::IsNearlyEqual(AcrossA, AcrossB, 0.1))
				{
					return AcrossA < AcrossB;
				}
				const double AlongA = Points[A] | Front;
				const double AlongB = Points[B] | Front;
				if (!FMath::IsNearlyEqual(AlongA, AlongB, 0.1))
				{
					return AlongA > AlongB;
				}
				return A < B;
			});
		};
		TArray<int32> UnitOrder;
		TArray<int32> PlaceOrder;
		SortAcross(Positions, UnitOrder);
		SortAcross(Places, PlaceOrder);
		TBitArray<> PlaceTaken(false, NumPlaces);
		for (int32 Rank = 0; Rank < FMath::Min(NumUnits, NumPlaces); ++Rank)
		{
			OutPlaceOfUnit[UnitOrder[Rank]] = PlaceOrder[Rank];
			PlaceTaken[PlaceOrder[Rank]] = true;
		}

		// Then shorter in total, swap by swap (and unit by unit towards a free place nearer): when no swap helps, no two
		// paths cross, since two crossing paths are always longer than the same units swapped (triangle inequality).
		// Two long paths almost in line may cross for a gain below the tolerance: those swap too. Each change shortens
		// the total, so this ends; the cap only guards against rounding.
		constexpr double Tolerance = 0.01;
		constexpr int32 MaxPasses = 200;
		auto Length = [&Positions, &Places](const int32 Unit, const int32 Place)
		{
			return FVector2D::Distance(Positions[Unit], Places[Place]);
		};
		bool bChanged = true;
		for (int32 Pass = 0; bChanged && Pass < MaxPasses; ++Pass)
		{
			bChanged = false;
			for (int32 First = 0; First < NumUnits; ++First)
			{
				if (OutPlaceOfUnit[First] == INDEX_NONE)
				{
					continue;
				}
				for (int32 Second = First + 1; Second < NumUnits; ++Second)
				{
					const int32 FirstPlace = OutPlaceOfUnit[First];
					const int32 SecondPlace = OutPlaceOfUnit[Second];
					if (SecondPlace == INDEX_NONE)
					{
						continue;
					}
					if (Length(First, SecondPlace) + Length(Second, FirstPlace) < Length(First, FirstPlace) + Length(Second, SecondPlace) - Tolerance
						|| Cross(Positions[First], Places[FirstPlace], Positions[Second], Places[SecondPlace]))
					{
						Swap(OutPlaceOfUnit[First], OutPlaceOfUnit[Second]);
						bChanged = true;
					}
				}
			}
			if (NumPlaces > NumUnits)
			{
				for (int32 Unit = 0; Unit < NumUnits; ++Unit)
				{
					for (int32 Place = 0; Place < NumPlaces; ++Place)
					{
						if (!PlaceTaken[Place] && Length(Unit, Place) < Length(Unit, OutPlaceOfUnit[Unit]) - Tolerance)
						{
							PlaceTaken[OutPlaceOfUnit[Unit]] = false;
							PlaceTaken[Place] = true;
							OutPlaceOfUnit[Unit] = Place;
							bChanged = true;
						}
					}
				}
			}
		}
	}

	void MakeGroupRows(TConstArrayView<int32> CountOfKind, const bool bPeasantsOnly, TArray<int32>& OutWidths)
	{
		int32 Count = 0;
		int32 Kinds = 0;
		for (const int32 KindCount : CountOfKind)
		{
			Count += FMath::Max(KindCount, 0);
			Kinds += KindCount > 0 ? 1 : 0;
		}
		if (bPeasantsOnly)
		{
			MakePeasantRows(Count, OutWidths);
		}
		else if (Kinds <= 1)
		{
			MakeSquareRows(Count, OutWidths);
		}
		else
		{
			MakeMixedRows(Count, OutWidths);
		}
	}

	void PlaceGroup(TConstArrayView<FVector2D> Positions, TConstArrayView<int32> KindOfUnit, const bool bPeasantsOnly,
		const FVector2D& Target, const FVector2D& Forward, const double ColumnSpacing, const double RowSpacing,
		TFunctionRef<FVector2D(const FVector2D&)> ProjectPlace, TArray<FVector2D>& OutPlaces, TArray<int32>& OutPlaceOfUnit,
		const int32 FrontWidth)
	{
		OutPlaces.Reset();
		OutPlaceOfUnit.Init(INDEX_NONE, Positions.Num());
		int32 NumKinds = 0;
		for (const int32 Kind : KindOfUnit)
		{
			NumKinds = FMath::Max(NumKinds, Kind + 1);
		}
		TArray<int32, TInlineAllocator<8>> CountOfKind;
		CountOfKind.Init(0, NumKinds);
		for (const int32 Kind : KindOfUnit)
		{
			if (Kind >= 0)
			{
				++CountOfKind[Kind];
			}
		}
		TArray<int32> Widths;
		if (FrontWidth > 0)
		{
			MakeRowsOfWidth(KindOfUnit.Num(), FrontWidth, Widths);
		}
		else
		{
			MakeGroupRows(CountOfKind, bPeasantsOnly, Widths);
		}
		TArray<int32> KindOfPlace;
		PlaceKinds(Widths, CountOfKind, KindOfPlace);
		MakePlaces(Widths, Target, Forward, ColumnSpacing, RowSpacing, OutPlaces);
		for (FVector2D& Place : OutPlaces)
		{
			Place = ProjectPlace(Place);
		}

		TArray<FVector2D> KindPositions;
		TArray<FVector2D> KindPlaces;
		TArray<int32> KindUnits;
		TArray<int32> KindPlaceIndices;
		TArray<int32> PlaceOfKindUnit;
		for (int32 Kind = 0; Kind < NumKinds; ++Kind)
		{
			KindPositions.Reset();
			KindUnits.Reset();
			for (int32 Unit = 0; Unit < Positions.Num(); ++Unit)
			{
				if (KindOfUnit.IsValidIndex(Unit) && KindOfUnit[Unit] == Kind)
				{
					KindPositions.Add(Positions[Unit]);
					KindUnits.Add(Unit);
				}
			}
			KindPlaces.Reset();
			KindPlaceIndices.Reset();
			for (int32 Place = 0; Place < KindOfPlace.Num() && Place < OutPlaces.Num(); ++Place)
			{
				if (KindOfPlace[Place] == Kind)
				{
					KindPlaces.Add(OutPlaces[Place]);
					KindPlaceIndices.Add(Place);
				}
			}
			if (KindUnits.IsEmpty() || KindPlaces.IsEmpty())
			{
				continue;
			}
			AssignPlaces(KindPositions, KindPlaces, Forward, PlaceOfKindUnit);
			for (int32 Index = 0; Index < KindUnits.Num() && Index < PlaceOfKindUnit.Num(); ++Index)
			{
				if (KindPlaceIndices.IsValidIndex(PlaceOfKindUnit[Index]))
				{
					OutPlaceOfUnit[KindUnits[Index]] = KindPlaceIndices[PlaceOfKindUnit[Index]];
				}
			}
		}
	}

	int32 GetFrontWidth(const double Length, const double ColumnSpacing, const int32 Count)
	{
		const int32 Columns = ColumnSpacing > 0. ? FMath::FloorToInt32(FMath::Max(Length, 0.) / ColumnSpacing + 1e-6) + 1 : 1;
		return FMath::Clamp(Columns, 1, FMath::Max(Count, 1));
	}

	double GetBlockSoftPace(const double MaxGap)
	{
		return FMath::Clamp(1. - (MaxGap - BlockLagDistance) / (BlockSoftHoldDistance - BlockLagDistance), 0., 1.);
	}

	double GetBlockBlendDistance(const double BlockSize, const double PathLength)
	{
		return FMath::Max(FMath::Min(BlockSize + BlockBlendMargin, PathLength * 0.5), 0.);
	}

	double GetBlockBlend(const double Remaining, const double BlendDistance)
	{
		return BlendDistance <= 0. ? 1. : FMath::Clamp(1. - Remaining / BlendDistance, 0., 1.);
	}

	void AssignPlacesByKind(TConstArrayView<FVector2D> Positions, TConstArrayView<int32> KindOfUnit, TConstArrayView<FVector2D> Places,
		const FVector2D& Forward, TArray<int32>& OutPlaceOfUnit)
	{
		const int32 Num = FMath::Min3(Positions.Num(), KindOfUnit.Num(), Places.Num());
		OutPlaceOfUnit.Init(INDEX_NONE, Positions.Num());
		TArray<int32> Kinds;
		for (int32 Unit = 0; Unit < Num; ++Unit)
		{
			Kinds.AddUnique(KindOfUnit[Unit]);
		}
		TArray<int32> Units;
		TArray<FVector2D> KindPositions, KindPlaces;
		TArray<int32> PlaceOfKindUnit;
		for (const int32 Kind : Kinds)
		{
			Units.Reset();
			KindPositions.Reset();
			KindPlaces.Reset();
			for (int32 Unit = 0; Unit < Num; ++Unit)
			{
				if (KindOfUnit[Unit] == Kind)
				{
					Units.Add(Unit);
					KindPositions.Add(Positions[Unit]);
					KindPlaces.Add(Places[Unit]);
				}
			}
			AssignPlaces(KindPositions, KindPlaces, Forward, PlaceOfKindUnit);
			for (int32 Index = 0; Index < Units.Num(); ++Index)
			{
				OutPlaceOfUnit[Units[Index]] = PlaceOfKindUnit.IsValidIndex(Index) && Units.IsValidIndex(PlaceOfKindUnit[Index]) ? Units[PlaceOfKindUnit[Index]] : Units[Index];
			}
		}
	}

	FVector2D GetFrontFacing(const FVector2D& Arrow)
	{
		const FVector2D Direction = Arrow.GetSafeNormal();
		return Direction.IsNearlyZero() ? FVector2D(1., 0.) : Direction;
	}

	int32 GetRowPriority(const FName TypeName, const int32 CarryCapacity)
	{
		static const FName Spearman(TEXT("Spearman"));
		static const FName ManAtArms(TEXT("ManAtArms"));
		static const FName Archer(TEXT("Archer"));
		if (CarryCapacity > 0)
		{
			return PeasantRowPriority;
		}
		return TypeName == Spearman ? 0 : TypeName == ManAtArms ? 1 : TypeName == Archer ? 2 : 3;
	}

	void EndWalkToPlace(FPlaceHold& Hold, const FVector2D& Location, const FVector2D& Place, const double Radius)
	{
		const bool bWasReturning = Hold.bReturning;
		Hold.bReturning = false;
		Hold.RestSteps = 0;
		Hold.LastLocation = Location;
		const bool bOnPlace = FVector2D::DistSquared(Location, Place) <= FMath::Square(Radius * ReturnDistanceInRadii);
		if (bOnPlace)
		{
			Hold.bHeld = true;
			Hold.FailedReturns = 0;
			return;
		}
		// Short of it: an arrival lets it go; a walk back tries again, a few times in a row at most.
		Hold.FailedReturns = bWasReturning && Hold.bHeld ? Hold.FailedReturns + 1 : 0;
		Hold.bHeld = bWasReturning && Hold.bHeld && Hold.FailedReturns < MaxReturnTries;
		if (!Hold.bHeld)
		{
			Hold.FailedReturns = 0;
		}
	}

	bool WatchPlace(FPlaceHold& Hold, const FVector2D& Location, const FVector2D& Place, const double Radius)
	{
		if (!Hold.bHeld || Hold.bReturning)
		{
			return false;
		}
		const double Distance = FVector2D::Distance(Location, Place);
		const double LastDistance = FVector2D::Distance(Hold.LastLocation, Place);
		const bool bPushed = Distance > LastDistance + RestStepDistance
			|| FVector2D::DistSquared(Location, Hold.LastLocation) > FMath::Square(PushStepDistance);
		Hold.LastLocation = Location;
		const bool bAway = Distance > Radius * ReturnDistanceInRadii;
		Hold.RestSteps = bAway && !bPushed ? Hold.RestSteps + 1 : 0;
		if (Hold.RestSteps < ReturnRestSteps)
		{
			return false;
		}
		Hold.RestSteps = 0;
		Hold.bReturning = true;
		return true;
	}

	double GetLineLength(TConstArrayView<FVector2D> Points)
	{
		double Length = 0.;
		for (int32 Index = 1; Index < Points.Num(); ++Index)
		{
			Length += FVector2D::Distance(Points[Index - 1], Points[Index]);
		}
		return Length;
	}

	FVector2D GetPointOnLine(TConstArrayView<FVector2D> Points, double Distance)
	{
		if (Points.Num() == 0)
		{
			return FVector2D::ZeroVector;
		}
		if (Distance <= 0.)
		{
			return Points[0];
		}
		for (int32 Index = 1; Index < Points.Num(); ++Index)
		{
			const double Segment = FVector2D::Distance(Points[Index - 1], Points[Index]);
			if (Distance <= Segment && Segment > 0.)
			{
				return FMath::Lerp(Points[Index - 1], Points[Index], Distance / Segment);
			}
			Distance -= Segment;
		}
		return Points.Last();
	}

	FVector2D GetDirectionOnLine(TConstArrayView<FVector2D> Points, const double Distance, const double Window, const FVector2D& Fallback)
	{
		const double Length = GetLineLength(Points);
		const FVector2D Behind = GetPointOnLine(Points, FMath::Clamp(Distance - Window, 0., Length));
		const FVector2D Ahead = GetPointOnLine(Points, FMath::Clamp(Distance + Window, 0., Length));
		const FVector2D Direction = Ahead - Behind;
		return Direction.IsNearlyZero() ? Fallback : Direction.GetSafeNormal();
	}

	FVector2D BlockOffsetToWorld(const FVector2D& Offset, const FVector2D& Forward)
	{
		const FVector2D Right(-Forward.Y, Forward.X);
		return Forward * Offset.X + Right * Offset.Y;
	}

	FVector2D WorldToBlockOffset(const FVector2D& Delta, const FVector2D& Forward)
	{
		const FVector2D Right(-Forward.Y, Forward.X);
		return FVector2D(FVector2D::DotProduct(Delta, Forward), FVector2D::DotProduct(Delta, Right));
	}

	FVector2D GetSlotOnPath(TConstArrayView<FVector2D> Points, const double Distance, const FVector2D& Offset, const FVector2D& Fallback)
	{
		const FVector2D Direction = GetDirectionOnLine(Points, Distance, BlockTurnWindow, Fallback);
		return GetPointOnLine(Points, Distance) + BlockOffsetToWorld(Offset, Direction);
	}
}
