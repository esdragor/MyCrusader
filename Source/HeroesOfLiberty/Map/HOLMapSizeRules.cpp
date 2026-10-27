// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map/HOLMapSizeRules.h"

#include "Algo/Find.h"
#include "Match/HOLMatchConfig.h"
#include "Math/RandomStream.h"
#include "Misc/PackageName.h"

namespace UE::HeroesOfLiberty::MapSize
{
	/**
	 * TC-Q4, TC-Q5: places and side in 4 m tiles (AoE2 proportions brought to the 880 m of D344). Tools/Data/MapSizes.csv
	 * says the same (test HeroesOfLiberty.MapSize.Csv).
	 */
	const FHOLMapSizeInfo Sizes[] = {
		{ EHOLMapSize::Small, TEXT("Small"), 2, 110 },
		{ EHOLMapSize::Medium, TEXT("Medium"), 4, 154 },
		{ EHOLMapSize::Normal, TEXT("Normal"), 6, 183 },
		{ EHOLMapSize::Large, TEXT("Large"), 8, 202 },
		{ EHOLMapSize::Huge, TEXT("Huge"), 8, 220 },
	};

	/** Fisher-Yates on the match's stream: the same seed, the same order. */
	template <typename T>
	void Shuffle(TArray<T>& Items, FRandomStream& Stream)
	{
		for (int32 Index = Items.Num() - 1; Index > 0; --Index)
		{
			Items.Swap(Index, Stream.RandHelper(Index + 1));
		}
	}
}

TConstArrayView<FHOLMapSizeInfo> FHOLMapSizeRules::GetSizes()
{
	return UE::HeroesOfLiberty::MapSize::Sizes;
}

const FHOLMapSizeInfo* FHOLMapSizeRules::Find(const EHOLMapSize Size)
{
	return Algo::FindBy(GetSizes(), Size, &FHOLMapSizeInfo::Size);
}

EHOLMapSize FHOLMapSizeRules::FindByKey(const FString& Key)
{
	const FHOLMapSizeInfo* Info = Algo::FindByPredicate(GetSizes(), [&Key](const FHOLMapSizeInfo& Line)
	{
		return Key.TrimStartAndEnd().Equals(Line.Key, ESearchCase::IgnoreCase);
	});
	return Info ? Info->Size : EHOLMapSize::None;
}

int32 FHOLMapSizeRules::GetNumPlaces(const EHOLMapSize Size)
{
	const FHOLMapSizeInfo* Info = Find(Size);
	return Info ? Info->NumPlaces : 0;
}

int32 FHOLMapSizeRules::GetSideTiles(const EHOLMapSize Size)
{
	const FHOLMapSizeInfo* Info = Find(Size);
	return Info ? Info->SideTiles : 0;
}

bool FHOLMapSizeRules::IsAllowed(const EHOLMapSize Size, const int32 NumPlayers)
{
	return Size != EHOLMapSize::None && GetNumPlaces(Size) >= NumPlayers;
}

TArray<EHOLMapSize> FHOLMapSizeRules::GetAllowedSizes(const int32 NumPlayers)
{
	TArray<EHOLMapSize> Allowed;
	for (const FHOLMapSizeInfo& Line : GetSizes())
	{
		if (IsAllowed(Line.Size, NumPlayers))
		{
			Allowed.Add(Line.Size);
		}
	}
	return Allowed;
}

EHOLMapSize FHOLMapSizeRules::GetDefaultSize(const int32 NumPlayers)
{
	// The lines go from the smallest up.
	const TArray<EHOLMapSize> Allowed = GetAllowedSizes(NumPlayers);
	return Allowed.IsEmpty() ? EHOLMapSize::None : Allowed[0];
}

FSoftObjectPath FHOLMapSizeRules::GetMapFile(const FSoftObjectPath& Map, const EHOLMapSize Size)
{
	const FHOLMapSizeInfo* Info = Find(Size);
	if (!Info || Map.IsNull())
	{
		return Map;
	}
	// TC-Q3: one file per size, next to the map, its name followed by the size's key.
	const FString Package = Map.GetLongPackageName() + TEXT("_") + Info->Key;
	return FSoftObjectPath(Package + TEXT(".") + FPackageName::GetShortName(Package));
}

TArray<int32> FHOLMapSizeRules::GetFixedPlaces(const TConstArrayView<FHOLMatchSlot> Slots)
{
	TArray<int32> Places;
	Places.Reserve(Slots.Num());
	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		Places.Add(Slots[Index].State != EHOLSlotState::Closed ? Index : INDEX_NONE);
	}
	return Places;
}

TArray<int32> FHOLMapSizeRules::DrawPlaces(const int32 Seed, const TConstArrayView<FHOLMatchSlot> Slots, const int32 NumPlaces, const bool bTeamsTogether)
{
	TArray<int32> Occupied;
	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		if (Slots[Index].State != EHOLSlotState::Closed)
		{
			Occupied.Add(Index);
		}
	}
	const int32 NumPlayers = Occupied.Num();
	if (NumPlaces <= 0 || NumPlayers > NumPlaces)
	{
		return {};
	}

	FRandomStream Stream(Seed);
	// Where the first seat is: the whole draw turns around the circle.
	const int32 Turn = Stream.RandHelper(NumPlaces);

	// Who sits on the seats, in order around the circle.
	TArray<int32> Order;
	if (bTeamsTogether)
	{
		// A team is a group, a slot without a team (0) a group of its own; the groups in a drawn order, their members too.
		TArray<TArray<int32>> Groups;
		TMap<int32, int32> GroupOfTeam;
		for (const int32 Slot : Occupied)
		{
			const int32 Team = Slots[Slot].Team;
			if (Team <= 0)
			{
				Groups.Add({ Slot });
				continue;
			}
			if (const int32* Group = GroupOfTeam.Find(Team))
			{
				Groups[*Group].Add(Slot);
			}
			else
			{
				GroupOfTeam.Add(Team, Groups.Num());
				Groups.Add({ Slot });
			}
		}
		UE::HeroesOfLiberty::MapSize::Shuffle(Groups, Stream);
		for (TArray<int32>& Group : Groups)
		{
			UE::HeroesOfLiberty::MapSize::Shuffle(Group, Stream);
			Order.Append(Group);
		}
	}
	else
	{
		Order = Occupied;
		UE::HeroesOfLiberty::MapSize::Shuffle(Order, Stream);
	}

	// Seats as evenly spread as can be: seat k at k * NumPlaces / NumPlayers, two gaps differ by one place at most.
	TArray<int32> Places;
	Places.Init(INDEX_NONE, Slots.Num());
	for (int32 Seat = 0; Seat < NumPlayers; ++Seat)
	{
		Places[Order[Seat]] = (Turn + Seat * NumPlaces / NumPlayers) % NumPlaces;
	}
	return Places;
}

TArray<int32> FHOLMapSizeRules::GetPlaces(const FHOLMatchConfig& Config)
{
	if (!Config.bDrawPlaces)
	{
		return GetFixedPlaces(Config.Slots);
	}
	return DrawPlaces(Config.Seed, Config.Slots, GetNumPlaces(Config), Config.bTeamsTogether);
}

int32 FHOLMapSizeRules::GetNumPlaces(const FHOLMatchConfig& Config)
{
	return Config.MapSize == EHOLMapSize::None ? FHOLMatchConfig::MaxSlots : GetNumPlaces(Config.MapSize);
}

bool FHOLMapSizeRules::CheckPlaces(const FHOLMatchConfig& Config, FString* OutError)
{
	const int32 NumPlaces = GetNumPlaces(Config);
	const int32 NumPlayers = Config.GetNumOccupiedSlots();
	FString Error;
	if (NumPlayers > NumPlaces)
	{
		Error = FString::Printf(TEXT("%d players for the %d places of the map"), NumPlayers, NumPlaces);
	}
	else if (!Config.bDrawPlaces)
	{
		// Fixed places: slot i at place i, which the map must have.
		for (int32 Slot = NumPlaces; Slot < Config.Slots.Num() && Error.IsEmpty(); ++Slot)
		{
			if (Config.Slots[Slot].State != EHOLSlotState::Closed)
			{
				Error = FString::Printf(TEXT("slot %d occupied, past the %d places of the map (fixed places)"), Slot, NumPlaces);
			}
		}
	}
	if (OutError)
	{
		*OutError = Error;
	}
	return Error.IsEmpty();
}

bool FHOLMapSizeRules::FitSlotsToSize(TArray<FHOLMatchSlot>& Slots, const EHOLMapSize Size)
{
	const int32 NumPlaces = FMath::Min(Size == EHOLMapSize::None ? FHOLMatchConfig::MaxSlots : GetNumPlaces(Size), Slots.Num());
	const int32 NumPlayers = Slots.FilterByPredicate([](const FHOLMatchSlot& Slot) { return Slot.State != EHOLSlotState::Closed; }).Num();
	if (NumPlayers > NumPlaces)
	{
		return false;
	}
	// Each player past the places onto the first closed slot below them, in order.
	int32 Free = 0;
	for (int32 Index = NumPlaces; Index < Slots.Num(); ++Index)
	{
		if (Slots[Index].State == EHOLSlotState::Closed)
		{
			continue;
		}
		while (Slots[Free].State != EHOLSlotState::Closed)
		{
			++Free;
		}
		Slots[Free] = Slots[Index];
		Slots[Index].State = EHOLSlotState::Closed;
		Slots[Index].PlayerName.Reset();
	}
	return true;
}
