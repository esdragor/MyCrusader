// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map/HOLMapSizeReadLibrary.h"

#include "Map/HOLMapSizeRules.h"
#include "Match/HOLMatchConfig.h"

namespace
{
	EHOLMapSize ToSize(const int32 Size)
	{
		return Size >= static_cast<int32>(EHOLMapSize::None) && Size <= static_cast<int32>(EHOLMapSize::Huge) ? static_cast<EHOLMapSize>(Size) : EHOLMapSize::None;
	}

	/** The eight slots of a match from the arrays (the slots past them closed, a state outside 1..2 closed). */
	TArray<FHOLMatchSlot> ToSlots(const TArray<int32>& States, const TArray<int32>& Teams, const TArray<int32>* Colors = nullptr)
	{
		TArray<FHOLMatchSlot> Slots;
		Slots.SetNum(FHOLMatchConfig::MaxSlots);
		for (int32 Index = 0; Index < Slots.Num(); ++Index)
		{
			const int32 State = States.IsValidIndex(Index) ? States[Index] : 0;
			Slots[Index].State = State == 1 ? EHOLSlotState::Human : State == 2 ? EHOLSlotState::AI : EHOLSlotState::Closed;
			Slots[Index].Team = Teams.IsValidIndex(Index) ? Teams[Index] : 0;
			if (Colors != nullptr)
			{
				Slots[Index].ColorIndex = Colors->IsValidIndex(Index) ? (*Colors)[Index] : 0;
			}
		}
		return Slots;
	}

	FHOLMatchConfig ToConfig(const int32 Seed, const int32 Size, const bool bDrawPlaces, const bool bTeamsTogether, const TArray<int32>& States, const TArray<int32>& Teams)
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		Config.Slots = ToSlots(States, Teams);
		Config.Seed = Seed;
		Config.MapSize = ToSize(Size);
		Config.bDrawPlaces = bDrawPlaces;
		Config.bTeamsTogether = bTeamsTogether;
		return Config;
	}
}

int32 UHOLMapSizeReadLibrary::GetNumSizes()
{
	return FHOLMapSizeRules::GetSizes().Num();
}

int32 UHOLMapSizeReadLibrary::GetSizeAt(const int32 Index)
{
	const TConstArrayView<FHOLMapSizeInfo> Sizes = FHOLMapSizeRules::GetSizes();
	return Sizes.IsValidIndex(Index) ? static_cast<int32>(Sizes[Index].Size) : 0;
}

bool UHOLMapSizeReadLibrary::FindSize(const int32 Size, FString& OutKey, int32& OutNumPlaces, int32& OutSideTiles)
{
	const FHOLMapSizeInfo* Info = FHOLMapSizeRules::Find(ToSize(Size));
	if (Info == nullptr)
	{
		return false;
	}
	OutKey = Info->Key;
	OutNumPlaces = Info->NumPlaces;
	OutSideTiles = Info->SideTiles;
	return true;
}

int32 UHOLMapSizeReadLibrary::FindByKey(const FString& Key)
{
	return static_cast<int32>(FHOLMapSizeRules::FindByKey(Key));
}

int32 UHOLMapSizeReadLibrary::GetNumPlaces(const int32 Size)
{
	return FHOLMapSizeRules::GetNumPlaces(ToSize(Size));
}

int32 UHOLMapSizeReadLibrary::GetSideTiles(const int32 Size)
{
	return FHOLMapSizeRules::GetSideTiles(ToSize(Size));
}

bool UHOLMapSizeReadLibrary::IsAllowed(const int32 Size, const int32 NumPlayers)
{
	return FHOLMapSizeRules::IsAllowed(ToSize(Size), NumPlayers);
}

TArray<int32> UHOLMapSizeReadLibrary::GetAllowedSizes(const int32 NumPlayers)
{
	TArray<int32> Result;
	for (const EHOLMapSize Size : FHOLMapSizeRules::GetAllowedSizes(NumPlayers))
	{
		Result.Add(static_cast<int32>(Size));
	}
	return Result;
}

int32 UHOLMapSizeReadLibrary::GetDefaultSize(const int32 NumPlayers)
{
	return static_cast<int32>(FHOLMapSizeRules::GetDefaultSize(NumPlayers));
}

FString UHOLMapSizeReadLibrary::GetMapFile(const FString& Map, const int32 Size)
{
	return FHOLMapSizeRules::GetMapFile(FSoftObjectPath(Map), ToSize(Size)).ToString();
}

TArray<int32> UHOLMapSizeReadLibrary::GetFixedPlaces(const TArray<int32>& States, const TArray<int32>& Teams)
{
	return FHOLMapSizeRules::GetFixedPlaces(ToSlots(States, Teams));
}

TArray<int32> UHOLMapSizeReadLibrary::DrawPlaces(const int32 Seed, const TArray<int32>& States, const TArray<int32>& Teams, const int32 NumPlaces, const bool bTeamsTogether)
{
	return FHOLMapSizeRules::DrawPlaces(Seed, ToSlots(States, Teams), NumPlaces, bTeamsTogether);
}

int32 UHOLMapSizeReadLibrary::GetConfigNumPlaces(const int32 Size)
{
	return FHOLMapSizeRules::GetNumPlaces(ToConfig(0, Size, false, true, {}, {}));
}

TArray<int32> UHOLMapSizeReadLibrary::GetConfigPlaces(const int32 Seed, const int32 Size, const bool bDrawPlaces, const bool bTeamsTogether, const TArray<int32>& States, const TArray<int32>& Teams)
{
	return FHOLMapSizeRules::GetPlaces(ToConfig(Seed, Size, bDrawPlaces, bTeamsTogether, States, Teams));
}

bool UHOLMapSizeReadLibrary::CheckConfigPlaces(const int32 Seed, const int32 Size, const bool bDrawPlaces, const bool bTeamsTogether, const TArray<int32>& States, const TArray<int32>& Teams, FString& OutError)
{
	return FHOLMapSizeRules::CheckPlaces(ToConfig(Seed, Size, bDrawPlaces, bTeamsTogether, States, Teams), &OutError);
}

bool UHOLMapSizeReadLibrary::FitSlotsToSize(TArray<int32>& States, TArray<int32>& Teams, TArray<int32>& Colors, const int32 Size)
{
	TArray<FHOLMatchSlot> Slots = ToSlots(States, Teams, &Colors);
	if (!FHOLMapSizeRules::FitSlotsToSize(Slots, ToSize(Size)))
	{
		return false;
	}
	States.Reset();
	Teams.Reset();
	Colors.Reset();
	for (const FHOLMatchSlot& Slot : Slots)
	{
		States.Add(static_cast<int32>(Slot.State));
		Teams.Add(Slot.Team);
		Colors.Add(Slot.ColorIndex);
	}
	return true;
}
