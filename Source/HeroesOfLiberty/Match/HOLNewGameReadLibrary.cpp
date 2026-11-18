// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLNewGameReadLibrary.h"

#include "Map/HOLFogMode.h"
#include "Map/HOLMapRules.h"
#include "Map/HOLMapSizeRules.h"
#include "Match/HOLFactionChoice.h"
#include "Match/HOLMatchConfig.h"
#include "Net/HOLManagerClientRules.h"

namespace
{
	/** A configuration read from its JSON (the same FromJson the game uses); false if refused. */
	bool NewGameReadConfig(const FString& Json, FHOLMatchConfig& OutConfig)
	{
		return FHOLMatchConfig::FromJson(Json, OutConfig);
	}

	EHOLFogMode NewGameToFogMode(const int32 Value)
	{
		return Value >= static_cast<int32>(EHOLFogMode::Disabled) && Value <= static_cast<int32>(EHOLFogMode::Normal) ? static_cast<EHOLFogMode>(Value) : EHOLFogMode::Disabled;
	}

	EHOLMapSize NewGameToMapSize(const int32 Value)
	{
		return FHOLMapSizeRules::Find(static_cast<EHOLMapSize>(Value)) ? static_cast<EHOLMapSize>(Value) : EHOLMapSize::None;
	}

	EHOLAIDifficulty NewGameToDifficulty(const int32 Value)
	{
		return Value >= static_cast<int32>(EHOLAIDifficulty::Easy) && Value <= static_cast<int32>(EHOLAIDifficulty::Evolving) ? static_cast<EHOLAIDifficulty>(Value) : EHOLAIDifficulty::Normal;
	}
}

int32 UHOLNewGameReadLibrary::GetSiteCount(const FString& AssetPath)
{
	return FHOLMapRules::GetSiteCount(FSoftObjectPath(AssetPath));
}

bool UHOLNewGameReadLibrary::DescribeGlobals(const FString& Json, int32& OutFogMode, bool& bOutTeamsTogether, FString& OutBiomeKey, int32& OutStartingPeasants,
	TArray<int32>& OutDifficulties, TArray<int32>& OutAIBrains)
{
	FHOLMatchConfig Config;
	if (!NewGameReadConfig(Json, Config))
	{
		return false;
	}
	OutFogMode = static_cast<int32>(Config.FogMode);
	bOutTeamsTogether = Config.bTeamsTogether;
	OutBiomeKey = Config.Biome;
	OutStartingPeasants = Config.StartingPeasants;
	OutDifficulties.Reset();
	OutAIBrains.Reset();
	for (const FHOLMatchSlot& Slot : Config.Slots)
	{
		OutDifficulties.Add(static_cast<int32>(Slot.Difficulty));
		OutAIBrains.Add(Slot.bAIBrain ? 1 : 0);
	}
	return true;
}

FString UHOLNewGameReadLibrary::SetGlobalsJson(const FString& Json, const FString& Map, const int32 MapSize, const bool bDrawPlaces, const bool bTeamsTogether, const int32 FogMode,
	const int32 Mode, const FString& BiomeKey)
{
	FHOLMatchConfig Config;
	if (!NewGameReadConfig(Json, Config))
	{
		return FString();
	}
	Config.Map = FSoftObjectPath(Map);
	Config.MapSize = NewGameToMapSize(MapSize);
	Config.bDrawPlaces = bDrawPlaces;
	Config.bTeamsTogether = bTeamsTogether;
	Config.FogMode = NewGameToFogMode(FogMode);
	Config.Mode = Mode == static_cast<int32>(EHOLMatchMode::Diplomacy) ? EHOLMatchMode::Diplomacy : EHOLMatchMode::Standard;
	Config.Biome = BiomeKey.IsEmpty() ? FString(TEXT("default")) : BiomeKey;
	return Config.ToJson();
}

FString UHOLNewGameReadLibrary::SetSlotAIJson(const FString& Json, const int32 Index, const int32 Difficulty, const bool bAIBrain)
{
	FHOLMatchConfig Config;
	if (!NewGameReadConfig(Json, Config) || !Config.Slots.IsValidIndex(Index))
	{
		return FString();
	}
	Config.Slots[Index].Difficulty = NewGameToDifficulty(Difficulty);
	Config.Slots[Index].bAIBrain = bAIBrain;
	return Config.ToJson();
}

FHOLNewGameLobbyView UHOLNewGameReadLibrary::MakeLobbyView(const FHOLLobby& Lobby)
{
	FHOLNewGameLobbyView View;
	View.Id = Lobby.Id;
	View.Name = Lobby.Name;
	View.State = static_cast<int32>(Lobby.State);
	View.Leader = Lobby.Leader;
	View.You = Lobby.You;
	View.Map = Lobby.Map;
	View.Size = Lobby.Size;
	View.MapSize = static_cast<int32>(Lobby.GetMapSize());
	View.NumPlaces = Lobby.GetNumPlaces();
	View.bTeamsTogether = Lobby.bTeamsTogether;
	View.FogMode = static_cast<int32>(Lobby.GetFogMode());
	View.BiomeKey = Lobby.GetBiomeKey();
	View.Mode = static_cast<int32>(Lobby.GetMatchMode());
	View.bNomad = Lobby.bNomad;
	View.bRegicide = Lobby.bRegicide;
	View.bCatastrophe = Lobby.bCatastrophe;
	View.bSacredSites = Lobby.bSacredSites;
	View.YourSlot = Lobby.FindYourSlot();
	View.bYouLeader = Lobby.IsYouLeader();
	View.bYouSpectator = Lobby.IsYouSpectator();
	View.bHasOpenSlot = Lobby.HasOpenSlot();
	View.bCanStart = Lobby.CanStart();
	View.bHasMatch = Lobby.bHasMatch;
	View.NumPlayers = Lobby.GetNumPlayers();
	View.NumAIs = Lobby.GetNumAIs();
	View.Spectators = Lobby.Spectators;
	for (const FHOLLobbySlot& Row : Lobby.Slots)
	{
		FHOLNewGameLobbySlotView& Slot = View.Slots.AddDefaulted_GetRef();
		Slot.Index = Row.Index;
		Slot.State = static_cast<int32>(Row.State);
		Slot.PlayerName = Row.PlayerName;
		Slot.ColorIndex = Row.ColorIndex;
		Slot.Team = Row.Team;
		Slot.FactionName = Row.Faction;
		EHOLFaction Faction = EHOLFaction::DawnOrder;
		Slot.Faction = HOLFactionChoice::TryFindByName(Row.Faction, Faction) ? static_cast<int32>(Faction) : -1;
		Slot.bReady = Row.bReady;
		Slot.Level = static_cast<int32>(Row.Level);
		Slot.bIsLeader = Row.State == EHOLLobbySlotState::Player && Row.PlayerName == Lobby.Leader;
		Slot.bIsYou = Row.Index == View.YourSlot;
		Slot.bCanEdit = Lobby.CanEditSlot(Row.Index);
		Slot.bCanToggle = Lobby.CanToggleSlot(Row.Index);
		Slot.bCanEditAI = Lobby.CanEditAISlot(Row.Index);
		if (Row.State == EHOLLobbySlotState::Player && Row.bReady)
		{
			++View.NumReady;
		}
	}
	return View;
}
