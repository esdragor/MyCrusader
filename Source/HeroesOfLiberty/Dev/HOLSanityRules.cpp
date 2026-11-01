// Copyright Epic Games, Inc. All Rights Reserved.

#include "Dev/HOLSanityRules.h"

#include "Match/HOLPlayerColors.h"

namespace HOLSanity
{
	TArray<FString> FindColourClashes(const TConstArrayView<int32> Neutrals, const TConstArrayView<int32> PlayerSlots, const TConstArrayView<uint8> SlotColorIndices)
	{
		TArray<FString> Clashes;
		for (const int32 Neutral : Neutrals)
		{
			const FVector3f NeutralColor = HOLPlayerColors::GetOwnerLinearColor(Neutral, SlotColorIndices);
			for (const int32 Slot : PlayerSlots)
			{
				if (HOLPlayerColors::GetOwnerLinearColor(Slot, SlotColorIndices) == NeutralColor)
				{
					Clashes.Add(FString::Printf(TEXT("neutral units (owner %d) are drawn in the colour of slot %d (%s)"), Neutral, Slot,
						SlotColorIndices.IsValidIndex(Slot) ? *HOLPlayerColors::GetColorName(SlotColorIndices[Slot]).ToString() : TEXT("none")));
				}
			}
		}
		return Clashes;
	}

	void FUnitTrack::Add(const double Time, const FVector2D& Location, const EUnitMotion Motion, const FSanityRules& Rules)
	{
		FSample Sample;
		Sample.Time = Time;
		Sample.Location = Location;
		Sample.Motion = Motion;
		Samples.Add(Sample);
		// Keep the window, plus one sample before it (its start).
		int32 Drop = 0;
		while (Drop + 1 < Samples.Num() && Samples[Drop + 1].Time <= Time - Rules.WindowSeconds)
		{
			++Drop;
		}
		if (Drop > 0)
		{
			Samples.RemoveAt(0, Drop, EAllowShrinking::No);
		}
	}

	FString FUnitTrack::Judge(const FSanityRules& Rules) const
	{
		if (Samples.Num() < 3 || Samples.Last().Time - Samples[0].Time < Rules.WindowSeconds * 0.99)
		{
			return FString();
		}
		bool bAllOrdered = true;
		bool bAllIdle = true;
		double Path = 0.;
		int32 Fast = 0;
		for (int32 Index = 1; Index < Samples.Num(); ++Index)
		{
			bAllOrdered &= Samples[Index].Motion == EUnitMotion::Ordered;
			bAllIdle &= Samples[Index].Motion == EUnitMotion::Idle;
			const double Step = FVector2D::Distance(Samples[Index].Location, Samples[Index - 1].Location);
			Path += Step;
			Fast += Step > Rules.ShakeStepCm ? 1 : 0;
		}
		const double Net = FVector2D::Distance(Samples.Last().Location, Samples[0].Location);

		if (bAllOrdered && Path >= Rules.SpinMinPathCm)
		{
			// Headings taken each time the unit moved a segment from the last anchor: the total turn of the window.
			double Turn = 0.;
			FVector2D Anchor = Samples[0].Location;
			bool bHaveHeading = false;
			double Heading = 0.;
			for (int32 Index = 1; Index < Samples.Num(); ++Index)
			{
				const FVector2D Delta = Samples[Index].Location - Anchor;
				if (Delta.Size() >= Rules.CourseSegmentCm)
				{
					const double NewHeading = FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X));
					if (bHaveHeading)
					{
						Turn += FMath::Abs(FMath::UnwindDegrees(NewHeading - Heading));
					}
					Heading = NewHeading;
					bHaveHeading = true;
					Anchor = Samples[Index].Location;
				}
			}
			if (Turn >= Rules.SpinTurnDegrees && Net < Rules.SpinAdvanceShare * Path)
			{
				return FString::Printf(TEXT("spins: turned %.0f degrees in %.1f s walking %.0f cm and got %.0f cm away from where it was"),
					Turn, Rules.WindowSeconds, Path, Net);
			}
		}
		if (bAllIdle)
		{
			const int32 Steps = Samples.Num() - 1;
			if (Fast >= Rules.ShakeStepShare * Steps && Path > 0. && Net < Rules.ShakeAdvanceShare * Path)
			{
				return FString::Printf(TEXT("shakes: moved more than %.1f cm in %d of %d steps with no order and went %.0f cm in all (%.0f cm walked)"),
					Rules.ShakeStepCm, Fast, Steps, Net, Path);
			}
		}
		return FString();
	}
}
