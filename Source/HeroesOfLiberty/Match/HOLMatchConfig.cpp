// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLMatchConfig.h"

#include "Algo/Count.h"
#include "Dom/JsonObject.h"
#include "JsonObjectConverter.h"
#include "Math/RandomStream.h"
#include "Match/HOLPlayerColors.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace UE::HeroesOfLiberty::Match
{
	const TCHAR* DefaultMap = TEXT("/Game/Level/BattleMap.BattleMap");
}

FHOLMatchConfig FHOLMatchConfig::MakeDefault()
{
	FHOLMatchConfig Config;
	Config.Map = FSoftObjectPath(UE::HeroesOfLiberty::Match::DefaultMap);
	Config.Normalize();
	Config.Slots[0].State = EHOLSlotState::Human;
	return Config;
}

void FHOLMatchConfig::Normalize()
{
	// Slots added here get distinct colours, in palette order.
	const int32 NumRead = Slots.Num();
	Slots.SetNum(MaxSlots);
	for (int32 Index = NumRead; Index < MaxSlots; ++Index)
	{
		Slots[Index].ColorIndex = Index % HOLPlayerColors::NumColors;
	}
	for (FHOLMatchSlot& Slot : Slots)
	{
		Slot.ColorIndex = FMath::Clamp(Slot.ColorIndex, 0, HOLPlayerColors::NumColors - 1);
		if (Slot.Team < 0 || Slot.Team > MaxTeams)
		{
			Slot.Team = 0;
		}
	}
	StartingPeasants = FMath::Max(StartingPeasants, -1);
	Version = CurrentVersion;
}

void FHOLMatchConfig::DrawRandomColors(FRandomStream& Stream)
{
	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		FHOLMatchSlot& Slot = Slots[Index];
		if (Slot.ColorIndex != HOLPlayerColors::RandomColor)
		{
			continue;
		}
		if (Slot.State == EHOLSlotState::Closed)
		{
			Slot.ColorIndex = Index % HOLPlayerColors::NumColors;
			continue;
		}
		TArray<int32, TInlineAllocator<HOLPlayerColors::NumColors>> Free;
		for (int32 Color = 0; Color < HOLPlayerColors::NumColors; ++Color)
		{
			if (!Slots.ContainsByPredicate([Color](const FHOLMatchSlot& Other) { return Other.State != EHOLSlotState::Closed && Other.ColorIndex == Color; }))
			{
				Free.Add(Color);
			}
		}
		// Eight colours for eight slots: one is always free.
		Slot.ColorIndex = Free.IsEmpty() ? 0 : Free[Stream.RandHelper(Free.Num())];
	}
}

int32 FHOLMatchConfig::GetNumOccupiedSlots() const
{
	return Algo::CountIf(Slots, [](const FHOLMatchSlot& Slot) { return Slot.State != EHOLSlotState::Closed; });
}

int32 FHOLMatchConfig::FindFreeHumanSlot(const TConstArrayView<int32> TakenSlots) const
{
	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		if (Slots[Index].State == EHOLSlotState::Human && !TakenSlots.Contains(Index))
		{
			return Index;
		}
	}
	return INDEX_NONE;
}

bool FHOLMatchConfig::HasPlayerNames() const
{
	return Slots.ContainsByPredicate([](const FHOLMatchSlot& Slot)
	{
		return Slot.State == EHOLSlotState::Human && !Slot.PlayerName.TrimStartAndEnd().IsEmpty();
	});
}

int32 FHOLMatchConfig::FindSlotForName(const FString& Name, const TConstArrayView<int32> TakenSlots) const
{
	if (!HasPlayerNames())
	{
		return FindFreeHumanSlot(TakenSlots);
	}
	const FString Wanted = Name.TrimStartAndEnd();
	if (Wanted.IsEmpty())
	{
		return INDEX_NONE;
	}
	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		if (Slots[Index].State == EHOLSlotState::Human && Slots[Index].PlayerName.TrimStartAndEnd().Equals(Wanted, ESearchCase::IgnoreCase))
		{
			// A name held by a player still here: the second login watches (its slot is freed when the first one leaves).
			return TakenSlots.Contains(Index) ? INDEX_NONE : Index;
		}
	}
	return INDEX_NONE;
}

FString FHOLMatchConfig::ToJson() const
{
	FString Json;
	FJsonObjectConverter::UStructToJsonObjectString(*this, Json);
	return Json;
}

bool FHOLMatchConfig::FromJson(const FString& Json, FHOLMatchConfig& OutConfig, FString* OutError)
{
	const auto Fail = [OutError](const FString& Error)
	{
		if (OutError)
		{
			*OutError = Error;
		}
		return false;
	};

	TSharedPtr<FJsonObject> Object;
	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Object) || !Object.IsValid())
	{
		return Fail(TEXT("not a JSON object"));
	}

	// A file without a version predates versioning: version 1. Field names are matched without regard to case.
	int32 FileVersion = 1;
	Object->TryGetNumberField(TEXT("version"), FileVersion);
	if (FileVersion > CurrentVersion)
	{
		return Fail(FString::Printf(TEXT("version %d is newer than this build (%d)"), FileVersion, CurrentVersion));
	}

	// Start from a blank configuration so a field missing from an older file keeps its default.
	FHOLMatchConfig Config;
	FText Reason;
	if (!FJsonObjectConverter::JsonObjectToUStruct(Object.ToSharedRef(), &Config, 0, 0, false, &Reason))
	{
		return Fail(Reason.ToString());
	}
	Config.Normalize();
	OutConfig = MoveTemp(Config);
	return true;
}
