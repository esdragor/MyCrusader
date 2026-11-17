// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLMatchConfigReadLibrary.h"

#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerColors.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
	/** A configuration read from its JSON (the same FromJson the game uses); false if refused. */
	bool ReadMatchConfig(const FString& Json, FHOLMatchConfig& OutConfig)
	{
		return FHOLMatchConfig::FromJson(Json, OutConfig);
	}

	EHOLSlotState ToSlotState(const int32 State)
	{
		return State == 1 ? EHOLSlotState::Human : State == 2 ? EHOLSlotState::AI : EHOLSlotState::Closed;
	}

	EHOLFaction ToFaction(const int32 Faction)
	{
		return Faction >= 0 && Faction <= static_cast<int32>(EHOLFaction::Templars) ? static_cast<EHOLFaction>(Faction) : EHOLFaction::DawnOrder;
	}
}

int32 UHOLMatchConfigReadLibrary::GetCurrentVersion()
{
	return FHOLMatchConfig::CurrentVersion;
}

int32 UHOLMatchConfigReadLibrary::GetMaxSlots()
{
	return FHOLMatchConfig::MaxSlots;
}

int32 UHOLMatchConfigReadLibrary::GetNumColors()
{
	return HOLPlayerColors::NumColors;
}

int32 UHOLMatchConfigReadLibrary::GetRandomColor()
{
	return HOLPlayerColors::RandomColor;
}

FString UHOLMatchConfigReadLibrary::MakeDefaultJson()
{
	return FHOLMatchConfig::MakeDefault().ToJson();
}

bool UHOLMatchConfigReadLibrary::CanRead(const FString& Json, FString& OutError)
{
	FHOLMatchConfig Config;
	OutError.Reset();
	return FHOLMatchConfig::FromJson(Json, Config, &OutError);
}

FString UHOLMatchConfigReadLibrary::SetSlotJson(const FString& Json, const int32 Index, const int32 State, const int32 Faction, const int32 ColorIndex, const int32 Team, const FString& PlayerName)
{
	FHOLMatchConfig Config;
	if (!ReadMatchConfig(Json, Config) || !Config.Slots.IsValidIndex(Index))
	{
		return FString();
	}
	FHOLMatchSlot& Slot = Config.Slots[Index];
	Slot.State = ToSlotState(State);
	Slot.Faction = ToFaction(Faction);
	Slot.ColorIndex = ColorIndex;
	Slot.Team = Team;
	Slot.PlayerName = PlayerName;
	return Config.ToJson();
}

FString UHOLMatchConfigReadLibrary::SetOptionsJson(const FString& Json, const int32 Seed, const FString& Map, const bool bNomad, const bool bRegicide, const bool bCatastrophe, const bool bSacredSites)
{
	FHOLMatchConfig Config;
	if (!ReadMatchConfig(Json, Config))
	{
		return FString();
	}
	Config.Seed = Seed;
	Config.Map = FSoftObjectPath(Map);
	Config.bNomad = bNomad;
	Config.bRegicide = bRegicide;
	Config.bCatastrophe = bCatastrophe;
	Config.bSacredSites = bSacredSites;
	return Config.ToJson();
}

bool UHOLMatchConfigReadLibrary::RoundTripsLosslessly(const FString& Json)
{
	FHOLMatchConfig First;
	FHOLMatchConfig Second;
	return ReadMatchConfig(Json, First) && ReadMatchConfig(First.ToJson(), Second) && First == Second;
}

bool UHOLMatchConfigReadLibrary::AreEqual(const FString& JsonA, const FString& JsonB)
{
	FHOLMatchConfig A;
	FHOLMatchConfig B;
	return ReadMatchConfig(JsonA, A) && ReadMatchConfig(JsonB, B) && A == B;
}

bool UHOLMatchConfigReadLibrary::ReadLeavesOutputUntouchedOnFailure(const FString& BaseJson, const FString& BadJson, bool& bOutParsed)
{
	FHOLMatchConfig Read;
	bOutParsed = false;
	if (!ReadMatchConfig(BaseJson, Read))
	{
		return false;
	}
	const FHOLMatchConfig Before = Read;
	bOutParsed = FHOLMatchConfig::FromJson(BadJson, Read);
	return Read == Before;
}

bool UHOLMatchConfigReadLibrary::Describe(const FString& Json, int32& OutVersion, int32& OutSeed, FString& OutMap, int32& OutMapSize, bool& bOutDrawPlaces, int32& OutMode,
	bool& bOutNomad, bool& bOutRegicide, bool& bOutCatastrophe, bool& bOutSacredSites, TArray<int32>& OutStates, TArray<int32>& OutFactions,
	TArray<int32>& OutColors, TArray<int32>& OutTeams, TArray<FString>& OutNames)
{
	FHOLMatchConfig Config;
	if (!ReadMatchConfig(Json, Config))
	{
		return false;
	}
	OutVersion = Config.Version;
	OutSeed = Config.Seed;
	OutMap = Config.Map.IsNull() ? FString() : Config.Map.ToString();
	OutMapSize = static_cast<int32>(Config.MapSize);
	bOutDrawPlaces = Config.bDrawPlaces;
	OutMode = static_cast<int32>(Config.Mode);
	bOutNomad = Config.bNomad;
	bOutRegicide = Config.bRegicide;
	bOutCatastrophe = Config.bCatastrophe;
	bOutSacredSites = Config.bSacredSites;
	OutStates.Reset();
	OutFactions.Reset();
	OutColors.Reset();
	OutTeams.Reset();
	OutNames.Reset();
	for (const FHOLMatchSlot& Slot : Config.Slots)
	{
		OutStates.Add(static_cast<int32>(Slot.State));
		OutFactions.Add(static_cast<int32>(Slot.Faction));
		OutColors.Add(Slot.ColorIndex);
		OutTeams.Add(Slot.Team);
		OutNames.Add(Slot.PlayerName);
	}
	return true;
}

int32 UHOLMatchConfigReadLibrary::GetNumOccupiedSlots(const FString& Json)
{
	FHOLMatchConfig Config;
	return ReadMatchConfig(Json, Config) ? Config.GetNumOccupiedSlots() : -1;
}

int32 UHOLMatchConfigReadLibrary::FindFreeHumanSlot(const FString& Json, const TArray<int32>& TakenSlots)
{
	FHOLMatchConfig Config;
	return ReadMatchConfig(Json, Config) ? Config.FindFreeHumanSlot(TakenSlots) : -2;
}

bool UHOLMatchConfigReadLibrary::HasPlayerNames(const FString& Json)
{
	FHOLMatchConfig Config;
	return ReadMatchConfig(Json, Config) && Config.HasPlayerNames();
}

int32 UHOLMatchConfigReadLibrary::FindSlotForName(const FString& Json, const FString& Name, const TArray<int32>& TakenSlots)
{
	FHOLMatchConfig Config;
	return ReadMatchConfig(Json, Config) ? Config.FindSlotForName(Name, TakenSlots) : -2;
}

TArray<int32> UHOLMatchConfigReadLibrary::DrawRandomColors(const int32 Seed, const TArray<int32>& States, const TArray<int32>& Colors)
{
	FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
	for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
	{
		Config.Slots[Index].State = ToSlotState(States.IsValidIndex(Index) ? States[Index] : 0);
		Config.Slots[Index].ColorIndex = Colors.IsValidIndex(Index) ? Colors[Index] : Config.Slots[Index].ColorIndex;
	}
	FRandomStream Stream(Seed);
	Config.DrawRandomColors(Stream);
	TArray<int32> Result;
	for (const FHOLMatchSlot& Slot : Config.Slots)
	{
		Result.Add(Slot.ColorIndex);
	}
	return Result;
}

bool UHOLMatchConfigReadLibrary::LoadProjectFileText(const FString& RelativePath, FString& OutText)
{
	return FFileHelper::LoadFileToString(OutText, *FPaths::Combine(FPaths::ProjectDir(), RelativePath));
}
