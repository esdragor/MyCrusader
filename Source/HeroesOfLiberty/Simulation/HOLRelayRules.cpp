// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLRelayRules.h"

FHOLRelayState HOLRelay::Step(const FHOLRelayState& State, const TArray<int32>& UnitsBySlot, const TArray<int32>& TeamBySlot, const int32 HoldSteps,
	const int32 DecaySteps)
{
	FHOLRelayState Next = State;
	const int32 Slots = FMath::Min(UnitsBySlot.Num(), TeamBySlot.Num());
	const int32 OwnerTeam = (State.Owner >= 0 && State.Owner < TeamBySlot.Num()) ? TeamBySlot[State.Owner] : -1;
	bool bDefenders = false;
	int32 AdverseTeam = -1;
	int32 AdverseTeams = 0;
	int32 Taker = INDEX_NONE;
	int32 TakerUnits = 0;
	for (int32 Slot = 0; Slot < Slots; ++Slot)
	{
		if (UnitsBySlot[Slot] <= 0)
		{
			continue;
		}
		const int32 Team = TeamBySlot[Slot];
		if (State.Owner >= 0 && Team == OwnerTeam)
		{
			bDefenders = true;
			continue;
		}
		if (Team != AdverseTeam)
		{
			AdverseTeam = Team;
			++AdverseTeams;
		}
		// The most units, the lowest slot on a tie.
		if (UnitsBySlot[Slot] > TakerUnits)
		{
			Taker = Slot;
			TakerUnits = UnitsBySlot[Slot];
		}
	}
	if (AdverseTeams == 0)
	{
		Next.Held = FMath::Max(0, State.Held - DecaySteps);
		if (Next.Held == 0)
		{
			Next.Taker = INDEX_NONE;
		}
		return Next;
	}
	if (bDefenders || AdverseTeams > 1)
	{
		return Next;
	}
	Next.Taker = Taker;
	Next.Held = State.Held + 1;
	if (Next.Held >= HoldSteps)
	{
		Next.Owner = Taker;
		Next.Taker = INDEX_NONE;
		Next.Held = 0;
	}
	return Next;
}
