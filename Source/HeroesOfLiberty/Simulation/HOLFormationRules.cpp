// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLFormationRules.h"

#include "Simulation/HOLGroupMoveRules.h"
#include "Algo/StableSort.h"

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
		// The smallest odd width at least sqrt(N): a unit in the middle, on the march's axis (the user's rule, 2026-10-20).
		const int32 Root = CeilSqrt(Count);
		MakeRowsOfWidth(Count, Root + ((Root & 1) ? 0 : 1), OutWidths);
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

	void PlaceKinds(TArray<int32>& InOutRowWidths, TConstArrayView<int32> CountOfKind, TArray<int32>& OutKindOfPlace)
	{
		OutKindOfPlace.Reset();
		const TArray<int32> Given = InOutRowWidths;
		InOutRowWidths.Reset();
		int32 Total = 0;
		bool bAnyMelee = false;
		for (int32 Kind = 0; Kind < CountOfKind.Num(); ++Kind)
		{
			Total += FMath::Max(CountOfKind[Kind], 0);
			bAnyMelee |= CountOfKind[Kind] > 0 && GetRowClass(Kind) == ERowClass::Melee;
		}
		// The rows' widths: the ones given, then (what a cut pushes back) rows as wide as the next-to-last one given: the
		// last one given is the shorter remainder, filled anyway when nothing is cut.
		const int32 BackWidth = Given.IsEmpty() ? FMath::Max(Total, 1) : FMath::Max(Given[FMath::Max(Given.Num() - 2, 0)], 1);
		auto WidthOfRow = [&Given, BackWidth](const int32 Row)
		{
			return Row < Given.Num() - 1 ? FMath::Max(Given[Row], 0) : BackWidth;
		};

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
		int32 Placed = 0;
		for (int32 Row = 0; Row < Given.Num() || Placed < Total; ++Row)
		{
			const int32 Width = WidthOfRow(Row);
			// The row's parts, in the kinds' order; the first one ends here when its kind began in a row in front. A shooter
			// never in the front row when there is melee; the siege never shares its row.
			Parts.Reset();
			const bool bFirstEnds = Kind < CountOfKind.Num() && KindLeft > 0 && KindLeft < CountOfKind[Kind];
			int32 Taken = 0;
			while (Taken < Width && KindLeft > 0)
			{
				const ERowClass Class = GetRowClass(Kind);
				if ((Row == 0 && bAnyMelee && Class != ERowClass::Melee)
					|| (!Parts.IsEmpty() && (Class == ERowClass::Siege) != (GetRowClass(Parts.Last().Kind) == ERowClass::Siege)))
				{
					break;
				}
				const int32 Count = FMath::Min(KindLeft, Width - Taken);
				Parts.Add({ Kind, Count });
				Taken += Count;
				KindLeft -= Count;
				SkipEmptyKinds();
			}
			if (Taken == 0 && Row >= Given.Num())
			{
				break;
			}
			// Once every unit has its place, a row given keeps its width (its places left free, INDEX_NONE); a row cut
			// short, or one more at the back, keeps only its units' places.
			const int32 RowWidth = KindLeft == 0 && Row < Given.Num() ? FMath::Max(Given[Row], Taken) : Taken;
			Placed += Taken;
			InOutRowWidths.Add(RowWidth);
			const int32 RowStart = OutKindOfPlace.Num();
			OutKindOfPlace.AddUninitialized(RowWidth);
			for (int32 Column = 0; Column < RowWidth; ++Column)
			{
				OutKindOfPlace[RowStart + Column] = INDEX_NONE;
			}
			// The kinds that start here hold the centre, the kind that ends here the wings; across classes, the class
			// further back holds the centre (shooters in the middle, the melee left over on the wings).
			if (bFirstEnds && Parts.Num() > 1)
			{
				const FPart Ending = Parts[0];
				Parts.RemoveAt(0);
				Parts.Add(Ending);
			}
			Algo::StableSort(Parts, [](const FPart& A, const FPart& B) { return GetRowClass(A.Kind) > GetRowClass(B.Kind); });

			// Centred blocks that grow outwards, part after part; an odd wing has its extra place on the left, so a block
			// of K places in the row starts at ceil((Width - K) / 2).
			int32 Held = 0;
			for (const FPart& Part : Parts)
			{
				const int32 OldFirst = (RowWidth - Held + 1) / 2;
				const int32 OldEnd = OldFirst + Held;
				Held += Part.Count;
				const int32 First = (RowWidth - Held + 1) / 2;
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

		FVector2D Front;
		FVector2D Right;
		GetFrame(Forward, Front, Right);
		auto Depth = [&Front](const FVector2D& Point) { return Point | Front; };
		auto Across = [&Right](const FVector2D& Point) { return Point | Right; };
		// The front one first, then from left to right.
		auto SortByDepth = [&Depth, &Across](TConstArrayView<FVector2D> Points, TArray<int32>& OutOrder)
		{
			OutOrder.SetNumUninitialized(Points.Num());
			for (int32 Index = 0; Index < Points.Num(); ++Index)
			{
				OutOrder[Index] = Index;
			}
			OutOrder.Sort([&Points, &Depth, &Across](const int32 A, const int32 B)
			{
				const double DepthA = Depth(Points[A]);
				const double DepthB = Depth(Points[B]);
				if (!FMath::IsNearlyEqual(DepthA, DepthB, 0.1))
				{
					return DepthA > DepthB;
				}
				const double AcrossA = Across(Points[A]);
				const double AcrossB = Across(Points[B]);
				if (!FMath::IsNearlyEqual(AcrossA, AcrossB, 0.1))
				{
					return AcrossA < AcrossB;
				}
				return A < B;
			});
		};
		auto SortAcross = [&Across](TConstArrayView<FVector2D> Points, TArray<int32>& InOutIndices)
		{
			InOutIndices.Sort([&Points, &Across](const int32 A, const int32 B)
			{
				const double AcrossA = Across(Points[A]);
				const double AcrossB = Across(Points[B]);
				return FMath::IsNearlyEqual(AcrossA, AcrossB, 0.1) ? A < B : AcrossA < AcrossB;
			});
		};
		auto Length = [&Positions, &Places](const int32 Unit, const int32 Place)
		{
			return FVector2D::Distance(Positions[Unit], Places[Place]);
		};

		// The places' rows, whatever they hold (rows cut short or with holes): a new row starts when a place lies more than
		// PlaceRowTolerance behind the one before it, front first; each row from left to right.
		constexpr double PlaceRowTolerance = 10.;
		TArray<int32> PlaceOrder;
		SortByDepth(Places, PlaceOrder);
		TArray<TArray<int32>> Rows;
		double LastDepth = 0.;
		for (const int32 Place : PlaceOrder)
		{
			const double PlaceDepth = Depth(Places[Place]);
			if (Rows.IsEmpty() || LastDepth - PlaceDepth > PlaceRowTolerance)
			{
				Rows.AddDefaulted();
			}
			Rows.Last().Add(Place);
			LastDepth = PlaceDepth;
		}
		for (TArray<int32>& Row : Rows)
		{
			SortAcross(Places, Row);
		}

		// The rows filled front first by the units furthest forward (the user's rule: on a half turn, the last row turns
		// on the spot, the one before it fills the new front row, and so on). Units within UnitTieTolerance of the last
		// one a row takes are a tie: the row takes those nearest across to its places that the surer units leave free.
		constexpr double UnitTieTolerance = 60.;
		TArray<int32> UnitOrder;
		SortByDepth(Positions, UnitOrder);
		TBitArray<> UnitTaken(false, NumUnits);
		TArray<int32> Remaining;
		TArray<int32> Chosen;
		TArray<int32> Candidates;
		TArray<double> GapOfUnit;
		GapOfUnit.Init(0., NumUnits);
		for (const TArray<int32>& Row : Rows)
		{
			Remaining.Reset();
			for (const int32 Unit : UnitOrder)
			{
				if (!UnitTaken[Unit])
				{
					Remaining.Add(Unit);
				}
			}
			if (Remaining.IsEmpty())
			{
				break;
			}
			const int32 NumSlots = Row.Num();
			const int32 Take = FMath::Min(NumSlots, Remaining.Num());
			const double BoundaryDepth = Depth(Positions[Remaining[Take - 1]]);
			Chosen.Reset();
			Candidates.Reset();
			for (const int32 Unit : Remaining)
			{
				const double UnitDepth = Depth(Positions[Unit]);
				if (UnitDepth > BoundaryDepth + UnitTieTolerance)
				{
					Chosen.Add(Unit);
				}
				else if (UnitDepth >= BoundaryDepth - UnitTieTolerance)
				{
					Candidates.Add(Unit);
				}
			}
			const int32 Needed = Take - Chosen.Num();
			if (Candidates.Num() > Needed)
			{
				// The places the sure units take (each, from left to right, the free one nearest across); the candidates
				// nearest across to a place left over come first, then the one further forward.
				TBitArray<> Claimed(false, NumSlots);
				SortAcross(Positions, Chosen);
				for (const int32 Unit : Chosen)
				{
					int32 Best = INDEX_NONE;
					for (int32 Slot = 0; Slot < NumSlots; ++Slot)
					{
						if (!Claimed[Slot] && (Best == INDEX_NONE
							|| FMath::Abs(Across(Places[Row[Slot]]) - Across(Positions[Unit])) < FMath::Abs(Across(Places[Row[Best]]) - Across(Positions[Unit]))))
						{
							Best = Slot;
						}
					}
					Claimed[Best] = true;
				}
				for (const int32 Unit : Candidates)
				{
					double Nearest = TNumericLimits<double>::Max();
					for (int32 Slot = 0; Slot < NumSlots; ++Slot)
					{
						if (!Claimed[Slot])
						{
							Nearest = FMath::Min(Nearest, FMath::Abs(Across(Places[Row[Slot]]) - Across(Positions[Unit])));
						}
					}
					GapOfUnit[Unit] = Nearest;
				}
				Candidates.Sort([&GapOfUnit, &Positions, &Depth](const int32 A, const int32 B)
				{
					if (!FMath::IsNearlyEqual(GapOfUnit[A], GapOfUnit[B], 0.1))
					{
						return GapOfUnit[A] < GapOfUnit[B];
					}
					const double DepthA = Depth(Positions[A]);
					const double DepthB = Depth(Positions[B]);
					return FMath::IsNearlyEqual(DepthA, DepthB, 0.1) ? A < B : DepthA > DepthB;
				});
			}
			for (int32 Pick = 0; Pick < Needed; ++Pick)
			{
				Chosen.Add(Candidates[Pick]);
			}
			for (const int32 Unit : Chosen)
			{
				UnitTaken[Unit] = true;
			}

			// In the row, units and places paired in their order from left to right; a row with more places than units
			// leaves free the ones that make the paths shortest in total (keeping that order).
			SortAcross(Positions, Chosen);
			const int32 NumChosen = Chosen.Num();
			TArray<double> Cost;
			Cost.Init(TNumericLimits<double>::Max(), (NumChosen + 1) * (NumSlots + 1));
			auto At = [NumSlots](const int32 I, const int32 J) { return I * (NumSlots + 1) + J; };
			for (int32 J = 0; J <= NumSlots; ++J)
			{
				Cost[At(0, J)] = 0.;
			}
			for (int32 I = 1; I <= NumChosen; ++I)
			{
				for (int32 J = I; J <= NumSlots; ++J)
				{
					const double Skip = J > I ? Cost[At(I, J - 1)] : TNumericLimits<double>::Max();
					const double Pair = Cost[At(I - 1, J - 1)] + Length(Chosen[I - 1], Row[J - 1]);
					Cost[At(I, J)] = FMath::Min(Skip, Pair);
				}
			}
			TBitArray<> SlotTaken(false, NumSlots);
			for (int32 I = NumChosen, J = NumSlots; I > 0; --J)
			{
				if (J > I && Cost[At(I, J)] == Cost[At(I, J - 1)])
				{
					continue;
				}
				OutPlaceOfUnit[Chosen[I - 1]] = Row[J - 1];
				SlotTaken[J - 1] = true;
				--I;
			}

			// Then, within the row only, shorter in total swap by swap (and unit by unit towards a free place of the row
			// nearer): when no swap helps, no two paths of the row cross (two crossing paths are always longer than the
			// same units swapped). Each change shortens the total, so this ends; the cap only guards against rounding.
			constexpr double Tolerance = 0.01;
			constexpr int32 MaxPasses = 200;
			bool bChanged = true;
			for (int32 Pass = 0; bChanged && Pass < MaxPasses; ++Pass)
			{
				bChanged = false;
				for (int32 First = 0; First < NumChosen; ++First)
				{
					for (int32 Second = First + 1; Second < NumChosen; ++Second)
					{
						const int32 UnitA = Chosen[First];
						const int32 UnitB = Chosen[Second];
						const int32 PlaceA = OutPlaceOfUnit[UnitA];
						const int32 PlaceB = OutPlaceOfUnit[UnitB];
						if (Length(UnitA, PlaceB) + Length(UnitB, PlaceA) < Length(UnitA, PlaceA) + Length(UnitB, PlaceB) - Tolerance
							|| Cross(Positions[UnitA], Places[PlaceA], Positions[UnitB], Places[PlaceB]))
						{
							Swap(OutPlaceOfUnit[UnitA], OutPlaceOfUnit[UnitB]);
							bChanged = true;
						}
					}
				}
				if (NumSlots > NumChosen)
				{
					for (const int32 Unit : Chosen)
					{
						for (int32 Slot = 0; Slot < NumSlots; ++Slot)
						{
							if (!SlotTaken[Slot] && Length(Unit, Row[Slot]) < Length(Unit, OutPlaceOfUnit[Unit]) - Tolerance)
							{
								SlotTaken[Row.IndexOfByKey(OutPlaceOfUnit[Unit])] = false;
								SlotTaken[Slot] = true;
								OutPlaceOfUnit[Unit] = Row[Slot];
								bChanged = true;
							}
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

	int32 SettleBlockedPlaces(TArray<FVector2D>& Places, TConstArrayView<bool> IsBlocked, const FVector2D& Anchor,
		const double Spacing, TFunctionRef<bool(const FVector2D&)> IsFree, TArray<bool>& OutSettled)
	{
		OutSettled.Init(false, Places.Num());
		if (Spacing <= 0.)
		{
			return 0;
		}
		const double MinGapSquared = FMath::Square(SettleMinGapInSpacings * Spacing);
		const double Step = Spacing * 0.5;
		const int32 Rings = FMath::RoundToInt32(SettleMaxRadiusInSpacings * Spacing / Step);
		auto Blocked = [&IsBlocked](const int32 Index) { return IsBlocked.IsValidIndex(Index) && IsBlocked[Index]; };
		TArray<FVector2D> Candidates;
		int32 Settled = 0;
		for (int32 Index = 0; Index < Places.Num(); ++Index)
		{
			if (!Blocked(Index))
			{
				continue;
			}
			const FVector2D Origin = Places[Index];
			// Clear of the unblocked places and of the ones already settled.
			auto KeepsGap = [&](const FVector2D& Point)
			{
				for (int32 Other = 0; Other < Places.Num(); ++Other)
				{
					if (Other != Index && (!Blocked(Other) || OutSettled[Other])
						&& FVector2D::DistSquared(Places[Other], Point) < MinGapSquared)
					{
						return false;
					}
				}
				return true;
			};
			for (int32 Ring = 1; Ring <= Rings && !OutSettled[Index]; ++Ring)
			{
				const double Radius = Step * Ring;
				const int32 Count = FMath::Max(12, FMath::CeilToInt32(UE_DOUBLE_TWO_PI * Radius / Step));
				Candidates.Reset();
				for (int32 Angle = 0; Angle < Count; ++Angle)
				{
					const double Theta = UE_DOUBLE_TWO_PI * Angle / Count;
					const FVector2D Point = Origin + Radius * FVector2D(FMath::Cos(Theta), FMath::Sin(Theta));
					if (KeepsGap(Point))
					{
						Candidates.Add(Point);
					}
				}
				Candidates.Sort([&Anchor](const FVector2D& A, const FVector2D& B)
				{
					return FVector2D::DistSquared(A, Anchor) < FVector2D::DistSquared(B, Anchor);
				});
				for (const FVector2D& Point : Candidates)
				{
					if (IsFree(Point))
					{
						Places[Index] = Point;
						OutSettled[Index] = true;
						++Settled;
						break;
					}
				}
			}
		}
		return Settled;
	}

	void PlaceGroup(TConstArrayView<FVector2D> Positions, TConstArrayView<int32> KindOfUnit, const bool bPeasantsOnly,
		const FVector2D& Target, const FVector2D& Forward, const double ColumnSpacing, const double RowSpacing,
		TFunctionRef<FVector2D(const FVector2D&)> ProjectPlace, TArray<FVector2D>& OutPlaces, TArray<int32>& OutPlaceOfUnit,
		const int32 FrontWidth)
	{
		PlaceGroup(Positions, KindOfUnit, bPeasantsOnly, Target, Forward, ColumnSpacing, RowSpacing,
			[](const FVector2D&) { return true; }, ProjectPlace, OutPlaces, OutPlaceOfUnit, FrontWidth);
	}

	void PlaceGroup(TConstArrayView<FVector2D> Positions, TConstArrayView<int32> KindOfUnit, const bool bPeasantsOnly,
		const FVector2D& Target, const FVector2D& Forward, const double ColumnSpacing, const double RowSpacing,
		TFunctionRef<bool(const FVector2D&)> IsFree, TFunctionRef<FVector2D(const FVector2D&)> ProjectPlace,
		TArray<FVector2D>& OutPlaces, TArray<int32>& OutPlaceOfUnit, const int32 FrontWidth)
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
		TArray<bool> IsBlocked;
		bool bAnyBlocked = false;
		for (const FVector2D& Place : OutPlaces)
		{
			bAnyBlocked |= IsBlocked.Add_GetRef(!IsFree(Place));
		}
		if (bAnyBlocked)
		{
			TArray<bool> Settled;
			SettleBlockedPlaces(OutPlaces, IsBlocked, Target, ColumnSpacing, IsFree, Settled);
		}
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

	int32 FindPackets(TConstArrayView<FVector2D> Positions, const double LinkDistance, TArray<int32>& OutPacketOfUnit, TArray<int32>& OutPacketSizes)
	{
		const int32 Num = Positions.Num();
		// Each unit's cluster, grown from one unit through every unit within LinkDistance of one already in it.
		TArray<int32> ClusterOf;
		ClusterOf.Init(INDEX_NONE, Num);
		TArray<int32> Sizes;
		TArray<int32> Open;
		const double LinkSquared = FMath::Square(LinkDistance);
		for (int32 Seed = 0; Seed < Num; ++Seed)
		{
			if (ClusterOf[Seed] != INDEX_NONE)
			{
				continue;
			}
			const int32 Cluster = Sizes.Add(0);
			ClusterOf[Seed] = Cluster;
			Open.Reset();
			Open.Add(Seed);
			while (!Open.IsEmpty())
			{
				const int32 Unit = Open.Pop(EAllowShrinking::No);
				++Sizes[Cluster];
				for (int32 Other = 0; Other < Num; ++Other)
				{
					if (ClusterOf[Other] == INDEX_NONE && FVector2D::DistSquared(Positions[Unit], Positions[Other]) <= LinkSquared)
					{
						ClusterOf[Other] = Cluster;
						Open.Add(Other);
					}
				}
			}
		}
		// Largest first; clusters are made in the order of their first unit, so a stable sort keeps ties that way.
		TArray<int32> Order;
		for (int32 Cluster = 0; Cluster < Sizes.Num(); ++Cluster)
		{
			Order.Add(Cluster);
		}
		Algo::StableSort(Order, [&Sizes](const int32 A, const int32 B) { return Sizes[A] > Sizes[B]; });
		TArray<int32> Rank;
		Rank.SetNumZeroed(Sizes.Num());
		OutPacketSizes.Reset();
		for (int32 Index = 0; Index < Order.Num(); ++Index)
		{
			Rank[Order[Index]] = Index;
			OutPacketSizes.Add(Sizes[Order[Index]]);
		}
		OutPacketOfUnit.SetNum(Num);
		for (int32 Unit = 0; Unit < Num; ++Unit)
		{
			OutPacketOfUnit[Unit] = Rank[ClusterOf[Unit]];
		}
		return OutPacketSizes.Num();
	}

	int32 FindStragglers(TConstArrayView<FVector2D> Positions, const double LinkDistance, TArray<bool>& OutIsStraggler)
	{
		const int32 Num = Positions.Num();
		OutIsStraggler.Init(false, Num);
		TArray<int32> PacketOfUnit;
		TArray<int32> PacketSizes;
		// Everyone apart (no packet of two): no body, the whole group one block.
		if (FindPackets(Positions, LinkDistance, PacketOfUnit, PacketSizes) < 2 || PacketSizes[0] < 2)
		{
			return 0;
		}
		int32 Stragglers = 0;
		for (int32 Unit = 0; Unit < Num; ++Unit)
		{
			OutIsStraggler[Unit] = PacketOfUnit[Unit] != 0 && PacketSizes[PacketOfUnit[Unit]] == 1;
			Stragglers += OutIsStraggler[Unit] ? 1 : 0;
		}
		return Stragglers;
	}

	int32 ReserveRearSlots(TConstArrayView<FVector2D> Slots, TConstArrayView<FVector2D> Positions, TConstArrayView<int32> KindOfUnit,
		TConstArrayView<bool> IsStraggler, TArray<int32>& OutSlotOfUnit, TArray<bool>& OutQueued)
	{
		const int32 Num = FMath::Min3(Slots.Num(), Positions.Num(), FMath::Min(KindOfUnit.Num(), IsStraggler.Num()));
		OutSlotOfUnit.Init(INDEX_NONE, Slots.Num());
		OutQueued.Init(false, Slots.Num());
		double RearX = TNumericLimits<double>::Max();
		for (int32 Slot = 0; Slot < Num; ++Slot)
		{
			RearX = FMath::Min(RearX, Slots[Slot].X);
		}
		TArray<bool> Taken;
		Taken.Init(false, Num);
		int32 Queued = 0;
		// Each straggler: the free slot of its kind furthest back (rows within RearRowTolerance of each other are one), then
		// the nearest to it across the march.
		for (int32 Unit = 0; Unit < Num; ++Unit)
		{
			if (!IsStraggler[Unit])
			{
				continue;
			}
			int32 Best = INDEX_NONE;
			for (int32 Slot = 0; Slot < Num; ++Slot)
			{
				if (Taken[Slot] || KindOfUnit[Slot] != KindOfUnit[Unit])
				{
					continue;
				}
				if (Best == INDEX_NONE || Slots[Slot].X < Slots[Best].X - RearRowTolerance
					|| (Slots[Slot].X <= Slots[Best].X + RearRowTolerance
						&& FMath::Abs(Slots[Slot].Y - Positions[Unit].Y) < FMath::Abs(Slots[Best].Y - Positions[Unit].Y)))
				{
					Best = Slot;
				}
			}
			if (Best == INDEX_NONE)
			{
				// No free slot of its kind (cannot happen: one slot per unit): its own if free, else the first free one.
				Best = !Taken[Unit] ? Unit : Taken.Find(false);
			}
			if (Best == INDEX_NONE)
			{
				continue;
			}
			Taken[Best] = true;
			OutSlotOfUnit[Unit] = Best;
			OutQueued[Unit] = Slots[Best].X > RearX + RearRowTolerance;
			Queued += OutQueued[Unit] ? 1 : 0;
		}
		// The body: each kind's units the other slots of their kind.
		TArray<int32> Kinds;
		for (int32 Unit = 0; Unit < Num; ++Unit)
		{
			Kinds.AddUnique(KindOfUnit[Unit]);
		}
		TArray<int32> Units, FreeSlots, SlotOfKindUnit;
		TArray<FVector2D> KindPositions, KindSlots;
		for (const int32 Kind : Kinds)
		{
			Units.Reset();
			FreeSlots.Reset();
			KindPositions.Reset();
			KindSlots.Reset();
			for (int32 Index = 0; Index < Num; ++Index)
			{
				if (KindOfUnit[Index] != Kind)
				{
					continue;
				}
				if (!IsStraggler[Index])
				{
					Units.Add(Index);
					KindPositions.Add(Positions[Index]);
				}
				if (!Taken[Index])
				{
					FreeSlots.Add(Index);
					KindSlots.Add(Slots[Index]);
				}
			}
			AssignPlaces(KindPositions, KindSlots, FVector2D(1., 0.), SlotOfKindUnit);
			for (int32 Index = 0; Index < Units.Num(); ++Index)
			{
				const int32 Slot = SlotOfKindUnit.IsValidIndex(Index) && FreeSlots.IsValidIndex(SlotOfKindUnit[Index]) ? FreeSlots[SlotOfKindUnit[Index]] : INDEX_NONE;
				if (Slot != INDEX_NONE)
				{
					Taken[Slot] = true;
				}
				OutSlotOfUnit[Units[Index]] = Slot;
			}
		}
		// Any unit left without one (cannot happen): the first free slot.
		for (int32 Unit = 0; Unit < Num; ++Unit)
		{
			if (OutSlotOfUnit[Unit] == INDEX_NONE)
			{
				OutSlotOfUnit[Unit] = Taken.Find(false);
				if (OutSlotOfUnit[Unit] != INDEX_NONE)
				{
					Taken[OutSlotOfUnit[Unit]] = true;
				}
			}
		}
		return Queued;
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

	int32 GetRowPriority(const FName TypeName, const int32 CarryCapacity, const bool bRanged, const bool bSiege)
	{
		static const FName Spearman(TEXT("Spearman"));
		static const FName ManAtArms(TEXT("ManAtArms"));
		static const FName Archer(TEXT("Archer"));
		if (CarryCapacity > 0)
		{
			return PeasantRowPriority;
		}
		if (bSiege)
		{
			return SiegeRowPriority;
		}
		if (bRanged)
		{
			return TypeName == Archer ? ArcherRowPriority : OtherRangedRowPriority;
		}
		return TypeName == Spearman ? 0 : TypeName == ManAtArms ? 1 : OtherMeleeRowPriority;
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

	bool ShouldYieldHeldPlace(const FVector2D& HeldPlace, TConstArrayView<FVector2D> NewPlacesByX, const double RadiusSum)
	{
		// The first new place no farther left than RadiusSum, then those up to RadiusSum to the right.
		int32 Low = 0;
		int32 High = NewPlacesByX.Num();
		const double MinX = HeldPlace.X - RadiusSum;
		while (Low < High)
		{
			const int32 Mid = (Low + High) / 2;
			if (NewPlacesByX[Mid].X < MinX)
			{
				Low = Mid + 1;
			}
			else
			{
				High = Mid;
			}
		}
		const double MaxX = HeldPlace.X + RadiusSum;
		for (int32 Index = Low; Index < NewPlacesByX.Num() && NewPlacesByX[Index].X <= MaxX; ++Index)
		{
			if (FVector2D::DistSquared(HeldPlace, NewPlacesByX[Index]) < FMath::Square(RadiusSum))
			{
				return true;
			}
		}
		return false;
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

	void MakeCorridor(TConstArrayView<double> FreeLeft, TConstArrayView<double> FreeRight, const double SampleStep,
		const double OffsetMinY, const double OffsetMaxY, const double MaxShift, const double MaxSlope, TArray<double>& OutShift, TArray<double>& OutSqueeze)
	{
		const int32 Count = FMath::Min(FreeLeft.Num(), FreeRight.Num());
		OutShift.Init(0., Count);
		OutSqueeze.Init(1., Count);
		if (Count == 0)
		{
			return;
		}
		const double MinY = FMath::Min(OffsetMinY, 0.);
		const double MaxY = FMath::Max(OffsetMaxY, 0.);
		const double Span = MaxY - MinY;
		const double Reach = FMath::Max(-MinY, MaxY);
		// The largest squeeze each sample allows with a shift of MaxShift at most.
		for (int32 Sample = 0; Sample < Count; ++Sample)
		{
			const double Left = FMath::Max(FreeLeft[Sample], 0.);
			const double Right = FMath::Max(FreeRight[Sample], 0.);
			double Squeeze = 1.;
			if (Span > UE_KINDA_SMALL_NUMBER)
			{
				Squeeze = FMath::Min(Squeeze, (Left + Right) / Span);
			}
			if (MinY < 0.)
			{
				Squeeze = FMath::Min(Squeeze, (MaxShift + Left) / -MinY);
			}
			if (MaxY > 0.)
			{
				Squeeze = FMath::Min(Squeeze, (MaxShift + Right) / MaxY);
			}
			OutSqueeze[Sample] = FMath::Clamp(Squeeze, 0., 1.);
		}
		// The squeeze ramps: down ahead of the obstacle, up after it (a distance transform of the slope).
		const double SqueezeStep = Reach > UE_KINDA_SMALL_NUMBER ? MaxSlope * SampleStep / Reach : 1.;
		for (int32 Sample = 1; Sample < Count; ++Sample)
		{
			OutSqueeze[Sample] = FMath::Min(OutSqueeze[Sample], OutSqueeze[Sample - 1] + SqueezeStep);
		}
		for (int32 Sample = Count - 2; Sample >= 0; --Sample)
		{
			OutSqueeze[Sample] = FMath::Min(OutSqueeze[Sample], OutSqueeze[Sample + 1] + SqueezeStep);
		}
		// Each sample's interval of shifts keeping its files free, then the slope-limited envelopes of its bounds; the
		// shift is the one nearest the centre line between them.
		TArray<double> Lower;
		TArray<double> Upper;
		Lower.SetNumUninitialized(Count);
		Upper.SetNumUninitialized(Count);
		for (int32 Sample = 0; Sample < Count; ++Sample)
		{
			Lower[Sample] = FMath::Max(-FMath::Max(FreeLeft[Sample], 0.) - OutSqueeze[Sample] * MinY, -MaxShift);
			Upper[Sample] = FMath::Min(FMath::Max(FreeRight[Sample], 0.) - OutSqueeze[Sample] * MaxY, MaxShift);
		}
		const double ShiftStep = MaxSlope * SampleStep;
		for (int32 Sample = 1; Sample < Count; ++Sample)
		{
			Lower[Sample] = FMath::Max(Lower[Sample], Lower[Sample - 1] - ShiftStep);
			Upper[Sample] = FMath::Min(Upper[Sample], Upper[Sample - 1] + ShiftStep);
		}
		for (int32 Sample = Count - 2; Sample >= 0; --Sample)
		{
			Lower[Sample] = FMath::Max(Lower[Sample], Lower[Sample + 1] - ShiftStep);
			Upper[Sample] = FMath::Min(Upper[Sample], Upper[Sample + 1] + ShiftStep);
		}
		for (int32 Sample = 0; Sample < Count; ++Sample)
		{
			// Bounds crossed (no shift fits within the slope): their middle.
			OutShift[Sample] = Lower[Sample] <= Upper[Sample] ? FMath::Clamp(0., Lower[Sample], Upper[Sample])
				: FMath::Clamp(0.5 * (Lower[Sample] + Upper[Sample]), -MaxShift, MaxShift);
		}
	}

	void GetCorridorAt(TConstArrayView<double> Shift, TConstArrayView<double> Squeeze, const double SampleStep, const double Distance,
		double& OutShift, double& OutSqueeze)
	{
		const int32 Count = FMath::Min(Shift.Num(), Squeeze.Num());
		OutShift = 0.;
		OutSqueeze = 1.;
		if (Count == 0 || SampleStep <= 0.)
		{
			return;
		}
		const double At = FMath::Clamp(Distance / SampleStep, 0., double(Count - 1));
		const int32 First = FMath::Min(FMath::FloorToInt32(At), Count - 1);
		const int32 Next = FMath::Min(First + 1, Count - 1);
		const double Alpha = At - First;
		OutShift = FMath::Lerp(Shift[First], Shift[Next], Alpha);
		OutSqueeze = FMath::Lerp(Squeeze[First], Squeeze[Next], Alpha);
	}
}
