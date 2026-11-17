// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLVictoryReadLibrary.h"

#include "Match/HOLMatchConfig.h"
#include "Match/HOLVictoryRules.h"

namespace
{
	EHOLSlotState ToSlotState(const int32 State)
	{
		return State == 1 ? EHOLSlotState::Human : State == 2 ? EHOLSlotState::AI : EHOLSlotState::Closed;
	}

	/** The default configuration whose slots are States and Teams (completed to eight, closed, team 0): the C++ test's MakeConfig. */
	FHOLMatchConfig MakeConfig(const TArray<int32>& States, const TArray<int32>& Teams)
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
		{
			Config.Slots[Index].State = ToSlotState(States.IsValidIndex(Index) ? States[Index] : 0);
			Config.Slots[Index].Team = Teams.IsValidIndex(Index) ? Teams[Index] : 0;
		}
		return Config;
	}
}

bool UHOLVictoryReadLibrary::Evaluate(const TArray<int32>& States, const TArray<int32>& Teams, const int32 DefeatedMask, int32& OutWinnerMask, TArray<int32>& OutWinners)
{
	const FHOLVictoryResult Result = HOLVictoryRules::Evaluate(MakeConfig(States, Teams), static_cast<uint8>(DefeatedMask));
	OutWinnerMask = Result.WinnerMask;
	OutWinners = Result.GetWinners();
	return Result.bOver;
}

int32 UHOLVictoryReadLibrary::GetCamp(const TArray<int32>& States, const TArray<int32>& Teams, const int32 Slot)
{
	return HOLVictoryRules::GetCamp(MakeConfig(States, Teams), Slot);
}

bool UHOLVictoryReadLibrary::AreHostile(const TArray<int32>& States, const TArray<int32>& Teams, const int32 SlotA, const int32 SlotB)
{
	return HOLVictoryRules::AreHostile(MakeConfig(States, Teams), SlotA, SlotB);
}
