// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLUnitPicking.h"

namespace HOLUnitPicking
{
	int32 PickAt(TConstArrayView<FHOLPickCandidate> Candidates, const FVector2D& Cursor)
	{
		int32 Best = INDEX_NONE;
		for (int32 Index = 0; Index < Candidates.Num(); ++Index)
		{
			const FHOLPickCandidate& Candidate = Candidates[Index];
			// The body: from the feet to the head, HalfWidth on each side of that line (the camera does not roll).
			const double Top = FMath::Min(Candidate.Foot.Y, Candidate.Head.Y) - Candidate.HalfWidth * 0.5;
			const double Bottom = FMath::Max(Candidate.Foot.Y, Candidate.Head.Y) + Candidate.HalfWidth * 0.5;
			if (Cursor.Y < Top || Cursor.Y > Bottom)
			{
				continue;
			}
			const double Span = Candidate.Head.Y - Candidate.Foot.Y;
			const double Along = FMath::IsNearlyZero(Span) ? 0. : FMath::Clamp((Cursor.Y - Candidate.Foot.Y) / Span, 0., 1.);
			const double CentreX = FMath::Lerp(Candidate.Foot.X, Candidate.Head.X, Along);
			if (FMath::Abs(Cursor.X - CentreX) > Candidate.HalfWidth)
			{
				continue;
			}
			if (Best == INDEX_NONE || Candidate.Depth < Candidates[Best].Depth)
			{
				Best = Index;
			}
		}
		return Best;
	}

	double GetBodyDistance(const FHOLPickCandidate& Candidate, const FVector2D& Cursor)
	{
		// The same body as PickAt: outside it, how far above or below, and how far beside its slanted middle line.
		const double Top = FMath::Min(Candidate.Foot.Y, Candidate.Head.Y) - Candidate.HalfWidth * 0.5;
		const double Bottom = FMath::Max(Candidate.Foot.Y, Candidate.Head.Y) + Candidate.HalfWidth * 0.5;
		const double Vertical = FMath::Max3(Top - Cursor.Y, Cursor.Y - Bottom, 0.);
		const double Span = Candidate.Head.Y - Candidate.Foot.Y;
		const double Along = FMath::IsNearlyZero(Span) ? 0. : FMath::Clamp((Cursor.Y - Candidate.Foot.Y) / Span, 0., 1.);
		const double CentreX = FMath::Lerp(Candidate.Foot.X, Candidate.Head.X, Along);
		const double Horizontal = FMath::Max(FMath::Abs(Cursor.X - CentreX) - Candidate.HalfWidth, 0.);
		return FMath::Sqrt(Vertical * Vertical + Horizontal * Horizontal);
	}

	int32 PickNear(TConstArrayView<FHOLPickCandidate> Candidates, const FVector2D& Cursor, const double Margin, bool& bOutDirect)
	{
		const int32 Direct = PickAt(Candidates, Cursor);
		bOutDirect = Direct != INDEX_NONE;
		if (bOutDirect || Margin <= 0.)
		{
			return Direct;
		}
		int32 Best = INDEX_NONE;
		double BestDistance = 0.;
		for (int32 Index = 0; Index < Candidates.Num(); ++Index)
		{
			const double Distance = GetBodyDistance(Candidates[Index], Cursor);
			if (Distance > Margin)
			{
				continue;
			}
			if (Best == INDEX_NONE || Distance < BestDistance || (Distance == BestDistance && Candidates[Index].Depth < Candidates[Best].Depth))
			{
				Best = Index;
				BestDistance = Distance;
			}
		}
		return Best;
	}

	double GetScaledMargin(const double MarginAt1080, const double ViewportHeight)
	{
		return ViewportHeight > 0. ? MarginAt1080 * ViewportHeight / 1080. : MarginAt1080;
	}

	void PickInBox(TConstArrayView<FHOLPickCandidate> Candidates, const FVector2D& CornerA, const FVector2D& CornerB, TArray<int32>& OutIndices)
	{
		OutIndices.Reset();
		const FVector2D Min(FMath::Min(CornerA.X, CornerB.X), FMath::Min(CornerA.Y, CornerB.Y));
		const FVector2D Max(FMath::Max(CornerA.X, CornerB.X), FMath::Max(CornerA.Y, CornerB.Y));
		for (int32 Index = 0; Index < Candidates.Num(); ++Index)
		{
			const FVector2D Middle = (Candidates[Index].Foot + Candidates[Index].Head) * 0.5;
			if (Middle.X >= Min.X && Middle.X <= Max.X && Middle.Y >= Min.Y && Middle.Y <= Max.Y)
			{
				OutIndices.Add(Index);
			}
		}
	}

	bool IsBox(const FVector2D& Start, const FVector2D& End)
	{
		return FVector2D::Distance(Start, End) > BoxThreshold;
	}

	uint32 NextInCycle(const TConstArrayView<uint32> SortedIds, const uint32 Last)
	{
		for (const uint32 Id : SortedIds)
		{
			if (Id > Last)
			{
				return Id;
			}
		}
		return SortedIds.IsEmpty() ? 0 : SortedIds[0];
	}
}
