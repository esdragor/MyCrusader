// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLSiteRulesLibrary.h"

#include "Simulation/HOLSiteRules.h"
#include "Engine/World.h"
#include "Match/HOLGameState.h"

namespace
{
	void CountUnits(const int32 Slots, const TArray<int32>& UnitSlots, const TArray<int32>& UnitCanTake, const TArray<int32>& UnitIsPeasant,
		TArray<int32>& OutTakers, TArray<int32>& OutDisturbers)
	{
		HOLSite::ResetCounts(OutTakers, OutDisturbers, Slots);
		for (int32 Index = 0; Index < UnitSlots.Num(); ++Index)
		{
			const bool bCanTake = UnitCanTake.IsValidIndex(Index) && UnitCanTake[Index] != 0;
			const bool bIsPeasant = UnitIsPeasant.IsValidIndex(Index) && UnitIsPeasant[Index] != 0;
			HOLSite::AddUnit(OutTakers, OutDisturbers, UnitSlots[Index], bCanTake, bIsPeasant);
		}
	}
}

void UHOLSiteRulesLibrary::StepSite(const int32 Owner, const int32 Taker, const int32 Held, const TArray<int32>& UnitSlots, const TArray<int32>& UnitCanTake,
	const TArray<int32>& UnitIsPeasant, const TArray<int32>& TeamBySlot, const int32 HoldSteps, const int32 DecaySteps, int32& OutOwner, int32& OutTaker,
	int32& OutHeld)
{
	TArray<int32> Takers;
	TArray<int32> Disturbers;
	CountUnits(TeamBySlot.Num(), UnitSlots, UnitCanTake, UnitIsPeasant, Takers, Disturbers);
	FHOLSiteState State;
	State.Owner = Owner;
	State.Taker = Taker;
	State.Held = Held;
	const FHOLSiteState Next = HOLSite::Step(State, Takers, Disturbers, TeamBySlot, HoldSteps, DecaySteps);
	OutOwner = Next.Owner;
	OutTaker = Next.Taker;
	OutHeld = Next.Held;
}

void UHOLSiteRulesLibrary::CountSiteUnits(const int32 Slots, const TArray<int32>& UnitSlots, const TArray<int32>& UnitCanTake,
	const TArray<int32>& UnitIsPeasant, TArray<int32>& OutTakers, TArray<int32>& OutDisturbers)
{
	CountUnits(Slots, UnitSlots, UnitCanTake, UnitIsPeasant, OutTakers, OutDisturbers);
}

int32 UHOLSiteRulesLibrary::GetHoldSeconds()
{
	return HOLSite::DefaultHoldSeconds;
}

void UHOLSiteRulesLibrary::GetVictoryCount(const UObject* WorldContextObject, int32& OutLeader, int32& OutSecondsLeft, bool& bOutFrozen, int32& OutWinner)
{
	FHOLSiteCountNet Count;
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr)
	{
		Count = GameState->GetSiteCount();
	}
	OutLeader = Count.Leader;
	OutSecondsLeft = Count.SecondsLeft;
	bOutFrozen = Count.bFrozen;
	OutWinner = Count.Winner;
}

int32 UHOLSiteRulesLibrary::GetDecaySteps()
{
	return HOLSite::DefaultDecaySteps;
}
