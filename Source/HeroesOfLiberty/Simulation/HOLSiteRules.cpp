// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLSiteRules.h"

FHOLSiteView HOLSite::MakeView(const int32 Index, const FVector2D& Location, const float Radius, const FHOLSiteState& State, const int32 HoldSteps)
{
	FHOLSiteView View;
	View.Index = static_cast<uint8>(FMath::Clamp(Index, 0, 255));
	View.X = static_cast<float>(Location.X);
	View.Y = static_cast<float>(Location.Y);
	View.Radius = Radius;
	View.Owner = static_cast<int8>(State.Owner);
	View.Taker = static_cast<int8>(State.Taker);
	View.FillPercent = static_cast<uint8>(HoldSteps > 0 ? FMath::Clamp(static_cast<int32>(static_cast<int64>(State.Held) * 100 / HoldSteps), 0, 99) : 0);
	return View;
}

int32 HOLSite::GoldStep(int32& Carry, const int32 Sites, const int32 RatePerMinute, const int32 StepsPerSecond)
{
	const int32 StepsPerMinute = 60 * FMath::Max(1, StepsPerSecond);
	Carry += FMath::Max(0, Sites) * FMath::Max(0, RatePerMinute);
	const int32 Award = Carry / StepsPerMinute;
	Carry -= Award * StepsPerMinute;
	return Award;
}

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

FHOLSiteVictoryState HOLSite::VictoryStep(const FHOLSiteVictoryState& State, const TArray<int32>& OwnerBySite,
	const TArray<int32>& ContestedBySite, const TArray<int32>& TeamBySlot, const int32 VictorySteps)
{
	// The winner once set stays.
	if (State.Winner >= 0)
	{
		return State;
	}
	FHOLSiteVictoryState Next = State;
	Next.LastOwners = OwnerBySite;
	// The camp holding every site: the team of the owners' slots when every site has an owner and all the owners are of one team (a team below zero is no camp, as the script's HoldingCamp).
	int32 Camp = INDEX_NONE;
	bool bHolding = OwnerBySite.Num() > 0;
	for (int32 Site = 0; bHolding && Site < OwnerBySite.Num(); ++Site)
	{
		const int32 Owner = OwnerBySite[Site];
		if (Owner < 0 || Owner >= TeamBySlot.Num())
		{
			bHolding = false;
			break;
		}
		const int32 Team = TeamBySlot[Owner];
		if (Team < 0)
		{
			bHolding = false;
			break;
		}
		if (Site == 0)
		{
			Camp = Team;
		}
		else if (Team != Camp)
		{
			bHolding = false;
		}
	}
	if (!bHolding)
	{
		Next.Leader = INDEX_NONE;
		Next.Held = 0;
		return Next;
	}
	// A new holding camp, or any site changed hands (even to a slot of the same camp, or a site appeared or went): the count starts again.
	if (Camp != State.Leader || State.LastOwners != OwnerBySite)
	{
		Next.Leader = Camp;
		Next.Held = 0;
	}
	// A site with an adverse unit in its zone freezes the count (it never recedes).
	for (const int32 Contested : ContestedBySite)
	{
		if (Contested != 0)
		{
			return Next;
		}
	}
	Next.Held = Next.Held + 1;
	if (Next.Held >= VictorySteps)
	{
		Next.Winner = Next.Leader;
	}
	return Next;
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
