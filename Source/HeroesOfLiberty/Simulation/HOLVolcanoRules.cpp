// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLVolcanoRules.h"

namespace
{
	/** The length of the segment between two points on the ground plane (Z ignored). */
	float FlatSegment(const FVector& A, const FVector& B)
	{
		return static_cast<float>(FVector::Dist2D(A, B));
	}
}

float HOLVolcano::FFlow::Length() const
{
	float Total = 0.f;
	for (int32 Index = 1; Index < Points.Num(); ++Index)
	{
		Total += FlatSegment(Points[Index - 1], Points[Index]);
	}
	return Total;
}

float HOLVolcano::EruptionDuration(const FFlow& Flow, const float Speed)
{
	return Speed > 0.f ? Flow.Length() / Speed : 0.f;
}

float HOLVolcano::FrontDistance(const FFlow& Flow, const float Now, const float EruptAt, const float Speed)
{
	if (Speed <= 0.f || Now <= EruptAt)
	{
		return 0.f;
	}
	return FMath::Min((Now - EruptAt) * Speed, Flow.Length());
}

FVector HOLVolcano::PointAt(const FFlow& Flow, const float Distance)
{
	if (Flow.Points.IsEmpty())
	{
		return FVector::ZeroVector;
	}
	float Left = FMath::Max(Distance, 0.f);
	for (int32 Index = 1; Index < Flow.Points.Num(); ++Index)
	{
		const float Segment = FlatSegment(Flow.Points[Index - 1], Flow.Points[Index]);
		if (Left <= Segment && Segment > 0.f)
		{
			return FMath::Lerp(Flow.Points[Index - 1], Flow.Points[Index], static_cast<double>(Left / Segment));
		}
		Left -= Segment;
	}
	return Flow.Points.Last();
}

HOLVolcano::FCycle HOLVolcano::MakeCycle(const int32 Seed, const int32 Index, const float PreviousReArmAt, const FHOLVolcanoSettings& Settings, const float EruptionLength)
{
	// One stream per cycle: the same dates on every machine, and a cycle never depends on how often it was asked for.
	FRandomStream Stream(Seed * 7919 + Index * 104729 + 1);
	FCycle Cycle;
	const float WindowOpensAt = PreviousReArmAt + Settings.WindowOpensAfter;
	Cycle.WarningAt = WindowOpensAt + Stream.FRand() * Settings.WindowLength;
	Cycle.EruptAt = Cycle.WarningAt + Stream.FRandRange(Settings.WarningMin, Settings.WarningMax);
	Cycle.EruptEndAt = Cycle.EruptAt + (EruptionLength > 0.f ? EruptionLength : Settings.EruptionLength);
	Cycle.CooledAt = Cycle.EruptEndAt + Stream.FRandRange(Settings.CoolingMin, Settings.CoolingMax);
	Cycle.ReArmAt = Cycle.CooledAt + Stream.FRandRange(Settings.RestMin, Settings.RestMax);
	return Cycle;
}

EHOLVolcanoPhase HOLVolcano::GetPhase(const bool bCatastrophe, const int32 Seed, const float Now, const FHOLVolcanoSettings& Settings, const float EruptionLength)
{
	if (!bCatastrophe)
	{
		return EHOLVolcanoPhase::Dormant;
	}
	float ReArmAt = 0.f;
	// Cycles follow one another; a match lasts well under an hour, so a few turns of the loop at most.
	for (int32 Index = 0; Index < 64; ++Index)
	{
		const FCycle Cycle = MakeCycle(Seed, Index, ReArmAt, Settings, EruptionLength);
		if (Now < Cycle.WarningAt)
		{
			return EHOLVolcanoPhase::Armed;
		}
		if (Now < Cycle.EruptAt)
		{
			return EHOLVolcanoPhase::Warning;
		}
		if (Now < Cycle.EruptEndAt)
		{
			return EHOLVolcanoPhase::Erupting;
		}
		if (Now < Cycle.CooledAt)
		{
			return EHOLVolcanoPhase::Cooling;
		}
		if (Now < Cycle.ReArmAt)
		{
			return EHOLVolcanoPhase::Rest;
		}
		ReArmAt = Cycle.ReArmAt;
	}
	return EHOLVolcanoPhase::Rest;
}
