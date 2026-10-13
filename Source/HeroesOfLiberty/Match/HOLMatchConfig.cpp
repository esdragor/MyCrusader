// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLMatchConfig.h"

#include "Algo/Count.h"
#include "Dom/JsonObject.h"
#include "JsonObjectConverter.h"
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
	Version = CurrentVersion;
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
