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
}
