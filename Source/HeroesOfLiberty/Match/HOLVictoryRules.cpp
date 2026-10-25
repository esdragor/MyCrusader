// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLVictoryRules.h"

#include "Engine/World.h"
#include "Match/HOLGameState.h"

TArray<int32> FHOLVictoryResult::GetWinners() const
{
	TArray<int32> Winners;
	for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
	{
		if (WinnerMask & (1 << Slot))
		{
			Winners.Add(Slot);
		}
	}
	return Winners;
}

int32 HOLVictoryRules::GetCamp(const FHOLMatchConfig& Config, const int32 Slot)
{
	if (!Config.Slots.IsValidIndex(Slot) || Config.Slots[Slot].State == EHOLSlotState::Closed)
	{
		return INDEX_NONE;
	}
	const int32 Team = Config.Slots[Slot].Team;
	return Team >= 1 && Team <= FHOLMatchConfig::MaxTeams ? Team : FHOLMatchConfig::MaxTeams + 1 + Slot;
}

FHOLCampTable::FHOLCampTable()
{
	for (int32 Owner = 0; Owner < 256; ++Owner)
	{
		Camps[Owner] = static_cast<uint8>(Owner);
	}
}

FHOLCampTable::FHOLCampTable(const FHOLMatchConfig& Config)
	: FHOLCampTable()
{
	for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
	{
		const int32 Camp = HOLVictoryRules::GetCamp(Config, Slot);
		if (Camp == INDEX_NONE || Camp > FHOLMatchConfig::MaxTeams)
		{
			continue;
		}
		// The team's first occupied slot stands for it (a slot's own index: no clash with an owner alone).
		for (int32 First = 0; First <= Slot; ++First)
		{
			if (HOLVictoryRules::GetCamp(Config, First) == Camp)
			{
				Camps[Slot] = static_cast<uint8>(First);
				break;
			}
		}
	}
}

FHOLCampTable FHOLCampTable::FromWorld(const UWorld* World)
{
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	return GameState ? FHOLCampTable(GameState->GetLaunchConfig()) : FHOLCampTable();
}

bool HOLVictoryRules::AreHostile(const FHOLMatchConfig& Config, const int32 SlotA, const int32 SlotB)
{
	return FHOLCampTable(Config).AreHostile(SlotA, SlotB);
}

FHOLVictoryResult HOLVictoryRules::Evaluate(const FHOLMatchConfig& Config, const uint8 DefeatedMask)
{
	TSet<int32> Camps;
	TSet<int32> StandingCamps;
	for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
	{
		const int32 Camp = GetCamp(Config, Slot);
		if (Camp == INDEX_NONE)
		{
			continue;
		}
		Camps.Add(Camp);
		if (!(DefeatedMask & (1 << Slot)))
		{
			StandingCamps.Add(Camp);
		}
	}

	FHOLVictoryResult Result;
	if (Camps.IsEmpty())
	{
		return Result;
	}
	// Nobody stands: over, no winner. A single camp standing wins only if it had opponents (D357).
	if (StandingCamps.IsEmpty())
	{
		Result.bOver = true;
	}
	else if (StandingCamps.Num() == 1 && Camps.Num() >= 2)
	{
		Result.bOver = true;
		const int32 Winner = *StandingCamps.CreateConstIterator();
		for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
		{
			if (GetCamp(Config, Slot) == Winner)
			{
				Result.WinnerMask |= 1 << Slot;
			}
		}
	}
	return Result;
}
