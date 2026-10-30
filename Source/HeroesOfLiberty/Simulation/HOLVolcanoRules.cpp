// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLVolcanoRules.h"

HOLVolcano::FCycle HOLVolcano::MakeCycle(const int32 Seed, const int32 Index, const float PreviousReArmAt, const FHOLVolcanoSettings& Settings)
{
	// One stream per cycle: the same dates on every machine, and a cycle never depends on how often it was asked for.
	FRandomStream Stream(Seed * 7919 + Index * 104729 + 1);
	FCycle Cycle;
	const float WindowOpensAt = PreviousReArmAt + Settings.WindowOpensAfter;
	Cycle.WarningAt = WindowOpensAt + Stream.FRand() * Settings.WindowLength;
	Cycle.EruptAt = Cycle.WarningAt + Stream.FRandRange(Settings.WarningMin, Settings.WarningMax);
	Cycle.EruptEndAt = Cycle.EruptAt + Settings.EruptionLength;
	Cycle.CooledAt = Cycle.EruptEndAt + Stream.FRandRange(Settings.CoolingMin, Settings.CoolingMax);
	Cycle.ReArmAt = Cycle.CooledAt + Stream.FRandRange(Settings.RestMin, Settings.RestMax);
	return Cycle;
}

EHOLVolcanoPhase HOLVolcano::GetPhase(const bool bCatastrophe, const int32 Seed, const float Now, const FHOLVolcanoSettings& Settings)
{
	if (!bCatastrophe)
	{
		return EHOLVolcanoPhase::Dormant;
	}
	float ReArmAt = 0.f;
	// Cycles follow one another; a match lasts well under an hour, so a few turns of the loop at most.
	for (int32 Index = 0; Index < 64; ++Index)
	{
		const FCycle Cycle = MakeCycle(Seed, Index, ReArmAt, Settings);
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
