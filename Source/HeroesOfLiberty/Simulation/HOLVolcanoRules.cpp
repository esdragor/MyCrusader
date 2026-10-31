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

int32 HOLVolcano::DrawFlow(const int32 Seed, const int32 Index, const int32 FlowCount)
{
	if (FlowCount <= 0)
	{
		return INDEX_NONE;
	}
	// A stream of its own (another salt than the cycle's dates): drawing a flow never moves a date. No memory of the previous eruption.
	// The seed is hashed first: the stream's generator is linear, so neighbouring seeds (cycle 0, cycle 1) would draw alike otherwise.
	FRandomStream Stream(static_cast<int32>(MurmurFinalize32(HashCombineFast(MurmurFinalize32(static_cast<uint32>(Seed)), MurmurFinalize32(static_cast<uint32>(Index) + 31337u)))));
	return Stream.RandRange(0, FlowCount - 1);
}

int32 HOLVolcano::SiteSeed(const int32 MatchSeed, const int32 SiteIndex)
{
	// Hashed, not added: the cycles' streams are linear in the seed, so neighbouring seeds would erupt alike.
	return static_cast<int32>(HashCombineFast(MurmurFinalize32(static_cast<uint32>(MatchSeed)), MurmurFinalize32(static_cast<uint32>(SiteIndex) + 977u)));
}

HOLVolcano::FSiteState HOLVolcano::SiteStateAt(const bool bCatastrophe, const int32 MatchSeed, const int32 SiteIndex, const FSite& Site, const float Now,
	const FHOLVolcanoSettings& Settings, const float ManAtArmsSpeed)
{
	FSiteState State;
	if (!bCatastrophe)
	{
		return State;
	}
	const int32 Seed = SiteSeed(MatchSeed, SiteIndex);
	const float Speed = Settings.LavaSpeed(ManAtArmsSpeed);
	float ReArmAt = 0.f;
	// Cycles follow one another; a match lasts well under an hour, so a few turns of the loop at most.
	for (int32 Index = 0; Index < 64; ++Index)
	{
		const int32 FlowIndex = DrawFlow(Seed, Index, Site.Flows.Num());
		const FFlow* Flow = FlowIndex != INDEX_NONE ? &Site.Flows[FlowIndex] : nullptr;
		const FCycle Cycle = MakeCycle(Seed, Index, ReArmAt, Settings, Flow ? EruptionDuration(*Flow, Speed) : 0.f);
		if (Now >= Cycle.ReArmAt)
		{
			ReArmAt = Cycle.ReArmAt;
			continue;
		}
		State.CycleIndex = Index;
		State.FlowIndex = FlowIndex;
		State.WarningAt = Cycle.WarningAt;
		State.EruptAt = Cycle.EruptAt;
		if (Now < Cycle.WarningAt)
		{
			State.Phase = EHOLVolcanoPhase::Armed;
		}
		else if (Now < Cycle.EruptAt)
		{
			State.Phase = EHOLVolcanoPhase::Warning;
			State.SecondsToErupt = Cycle.EruptAt - Now;
		}
		else if (Now < Cycle.EruptEndAt)
		{
			State.Phase = EHOLVolcanoPhase::Erupting;
		}
		else
		{
			State.Phase = Now < Cycle.CooledAt ? EHOLVolcanoPhase::Cooling : EHOLVolcanoPhase::Rest;
		}
		if (Flow)
		{
			State.FrontDistance = FrontDistance(*Flow, Now, Cycle.EruptAt, Speed);
			State.FrontPoint = PointAt(*Flow, State.FrontDistance);
		}
		return State;
	}
	State.Phase = EHOLVolcanoPhase::Rest;
	return State;
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
