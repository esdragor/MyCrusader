// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLSiteRules.h"

void HOLSite::ResetCounts(TArray<int32>& Takers, TArray<int32>& Disturbers, const int32 Slots)
{
	Takers.Init(0, FMath::Max(0, Slots));
	Disturbers.Init(0, FMath::Max(0, Slots));
}

void HOLSite::AddUnit(TArray<int32>& Takers, TArray<int32>& Disturbers, const int32 Slot, const bool bCanTake, const bool bIsPeasant)
{
	if (bIsPeasant || Slot < 0 || Slot >= Takers.Num() || Slot >= Disturbers.Num())
	{
		return;
	}
	++Disturbers[Slot];
	if (bCanTake)
	{
		++Takers[Slot];
	}
}

FHOLSiteState HOLSite::Step(const FHOLSiteState& State, const TArray<int32>& Takers, const TArray<int32>& Disturbers,
	const TArray<int32>& TeamBySlot, const int32 HoldSteps, const int32 DecaySteps)
{
	FHOLSiteState Next = State;
	const int32 Slots = FMath::Min(FMath::Min(Takers.Num(), Disturbers.Num()), TeamBySlot.Num());
	const int32 OwnerTeam = (State.Owner >= 0 && State.Owner < TeamBySlot.Num()) ? TeamBySlot[State.Owner] : -1;
	const int32 TakerTeam = (State.Taker >= 0 && State.Taker < TeamBySlot.Num()) ? TeamBySlot[State.Taker] : -1;
	bool bDefenders = false;
	TArray<int32> AdverseTeams;
	int32 Taker = INDEX_NONE;
	int32 TakerUnits = 0;
	for (int32 Slot = 0; Slot < Slots; ++Slot)
	{
		if (Takers[Slot] <= 0 && Disturbers[Slot] <= 0)
		{
			continue;
		}
		const int32 Team = TeamBySlot[Slot];
		if (State.Owner >= 0 && Team == OwnerTeam)
		{
			// A disturber of the owner's camp (the owner himself or an ally) defends; a taker of it takes nothing.
			bDefenders = true;
			continue;
		}
		AdverseTeams.AddUnique(Team);
		// The taker: the most takers, the lowest slot on a tie (strictly more to replace).
		if (Takers[Slot] > TakerUnits)
		{
			Taker = Slot;
			TakerUnits = Takers[Slot];
		}
	}
	if (AdverseTeams.Num() == 0)
	{
		// Nobody adverse: the count recedes.
		Next.Held = FMath::Max(0, State.Held - DecaySteps);
		if (Next.Held == 0)
		{
			Next.Taker = INDEX_NONE;
		}
		return Next;
	}
	if (bDefenders || AdverseTeams.Num() > 1)
	{
		// Contested: the count is frozen.
		return Next;
	}
	if (Taker == INDEX_NONE)
	{
		// One adverse camp alone, but no taker among it: it takes nothing. It freezes a count another camp had begun, and lets a count of
		// its own camp recede.
		if (State.Held > 0 && TakerTeam != AdverseTeams[0])
		{
			return Next;
		}
		Next.Held = FMath::Max(0, State.Held - DecaySteps);
		if (Next.Held == 0)
		{
			Next.Taker = INDEX_NONE;
		}
		return Next;
	}
	// One adverse camp alone with a taker: the count goes up. A taker of another camp than the one who began the count starts it again
	// from zero, he inherits nothing (Q2 = B, answered 2026-11-05); an ally's slot goes on.
	const int32 StartedBy = (State.Held > 0 && State.Taker >= 0 && State.Taker < TeamBySlot.Num()) ? TeamBySlot[State.Taker] : -1;
	const int32 Held = (StartedBy >= 0 && StartedBy != TeamBySlot[Taker]) ? 0 : State.Held;
	Next.Taker = Taker;
	Next.Held = Held + 1;
	if (Next.Held >= HoldSteps)
	{
		Next.Owner = Taker;
		Next.Taker = INDEX_NONE;
		Next.Held = 0;
	}
	return Next;
}
