// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLSiteRules.h"

void HOLSite::ResetCounts(TArray<int32>& Takers, TArray<int32>& Disturbers, const int32 Slots)
{
	Takers.Init(0, FMath::Max(0, Slots));
	Disturbers.Init(0, FMath::Max(0, Slots));
}

void HOLSite::AddUnit(TArray<int32>& Takers, TArray<int32>& Disturbers, const int32 Slot, const bool bCanTake, const bool bIsPeasant)
{
	// S2 stub (red test first): no behaviour yet.
}

FHOLSiteState HOLSite::Step(const FHOLSiteState& State, const TArray<int32>& Takers, const TArray<int32>& Disturbers,
	const TArray<int32>& TeamBySlot, const int32 HoldSteps, const int32 DecaySteps)
{
	// S2 stub (red test first): no behaviour yet.
	return State;
}
