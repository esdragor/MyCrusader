// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLManagerClientReadLibrary.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Net/HOLManagerClientRules.h"
#include "Player/HOLGameMenuRules.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
	FString Bool(const bool bValue) { return bValue ? TEXT("true") : TEXT("false"); }
	FString Int(const int64 Value) { return FString::Printf(TEXT("%lld"), Value); }

	const TCHAR* StateName(const EHOLLobbyState State)
	{
		switch (State)
		{
		case EHOLLobbyState::Lobby: return TEXT("lobby");
		case EHOLLobbyState::Starting: return TEXT("starting");
		default: return TEXT("in_game");
		}
	}

	FString EnumName(const UEnum* Enum, const int64 Value)
	{
		return Enum ? Enum->GetNameStringByValue(Value) : Int(Value);
	}

	TOptional<int32> OptInt(const bool bHas, const int32 Value) { return bHas ? TOptional<int32>(Value) : TOptional<int32>(); }
	TOptional<bool> OptBool(const bool bHas, const bool bValue) { return bHas ? TOptional<bool>(bValue) : TOptional<bool>(); }

	/** The value of a JSON field as text; "" when absent. */
	FString ValueText(const TSharedPtr<FJsonValue>& Value)
	{
		if (!Value.IsValid())
		{
			return FString();
		}
		switch (Value->Type)
		{
		case EJson::String: return Value->AsString();
		case EJson::Number: return Int(static_cast<int64>(Value->AsNumber()));
		case EJson::Boolean: return Bool(Value->AsBool());
		default: return FString();
		}
	}

	TSharedPtr<FJsonObject> ReadManagerJsonObject(const FString& Json)
	{
		TSharedPtr<FJsonObject> Object;
		FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Object);
		return Object;
	}
}

bool UHOLManagerClientReadLibrary::IsStatus(const FString& Json)
{
	FHOLManagerStatus Status;
	return FHOLManagerClientRules::ParseStatus(Json, Status);
}

FString UHOLManagerClientReadLibrary::StatusField(const FString& Json, const FString& Field)
{
	FHOLManagerStatus Status;
	if (!FHOLManagerClientRules::ParseStatus(Json, Status)) { return FString(); }
	if (Field == TEXT("ProtocolVersion")) { return Int(Status.ProtocolVersion); }
	if (Field == TEXT("GameVersion")) { return Status.GameVersion; }
	if (Field == TEXT("Lobbies")) { return Int(Status.Lobbies); }
	if (Field == TEXT("Servers")) { return Int(Status.Servers); }
	if (Field == TEXT("MaxServers")) { return Int(Status.MaxServers); }
	return FString();
}

int32 UHOLManagerClientReadLibrary::GetNumLobbies(const FString& Json)
{
	TArray<FHOLLobbySummary> Rows;
	return FHOLManagerClientRules::ParseLobbies(Json, Rows) ? Rows.Num() : -1;
}

FString UHOLManagerClientReadLibrary::LobbiesField(const FString& Json, const int32 Row, const FString& Field)
{
	TArray<FHOLLobbySummary> Rows;
	if (!FHOLManagerClientRules::ParseLobbies(Json, Rows) || !Rows.IsValidIndex(Row)) { return FString(); }
	const FHOLLobbySummary& L = Rows[Row];
	if (Field == TEXT("Id")) { return L.Id; }
	if (Field == TEXT("Name")) { return L.Name; }
	if (Field == TEXT("Map")) { return L.Map; }
	if (Field == TEXT("Size")) { return L.Size; }
	if (Field == TEXT("Players")) { return Int(L.Players); }
	if (Field == TEXT("MaxPlayers")) { return Int(L.MaxPlayers); }
	if (Field == TEXT("State")) { return StateName(L.State); }
	if (Field == TEXT("Minutes")) { return Int(L.Minutes); }
	if (Field == TEXT("CanJoin")) { return Bool(L.bCanJoin); }
	if (Field == TEXT("CanWatch")) { return Bool(L.bCanWatch); }
	if (Field == TEXT("Away")) { return FString::Join(L.Away, TEXT(",")); }
	if (Field == TEXT("Spectators")) { return Int(L.Spectators); }
	if (Field == TEXT("CanSpectate")) { return Bool(FHOLManagerClientRules::CanSpectate(L)); }
	return FString();
}

int32 UHOLManagerClientReadLibrary::ChooseLobbyAction(const FString& Json, const int32 Row, const FString& MyName)
{
	TArray<FHOLLobbySummary> Rows;
	if (!FHOLManagerClientRules::ParseLobbies(Json, Rows) || !Rows.IsValidIndex(Row)) { return -1; }
	return static_cast<int32>(FHOLManagerClientRules::ChooseLobbyAction(Rows[Row], MyName));
}

bool UHOLManagerClientReadLibrary::IsLobby(const FString& Json)
{
	FHOLLobby Lobby;
	return FHOLManagerClientRules::ParseLobby(Json, Lobby);
}

FString UHOLManagerClientReadLibrary::LobbyField(const FString& Json, const FString& Field, const int32 SlotIndex)
{
	FHOLLobby L;
	if (!FHOLManagerClientRules::ParseLobby(Json, L)) { return FString(); }
	if (Field == TEXT("Id")) { return L.Id; }
	if (Field == TEXT("Name")) { return L.Name; }
	if (Field == TEXT("State")) { return StateName(L.State); }
	if (Field == TEXT("Minutes")) { return Int(L.Minutes); }
	if (Field == TEXT("Leader")) { return L.Leader; }
	if (Field == TEXT("You")) { return L.You; }
	if (Field == TEXT("Map")) { return L.Map; }
	if (Field == TEXT("Size")) { return L.Size; }
	if (Field == TEXT("TeamsTogether")) { return Bool(L.bTeamsTogether); }
	if (Field == TEXT("Fog")) { return L.Fog; }
	if (Field == TEXT("Biome")) { return L.Biome; }
	if (Field == TEXT("Mode")) { return L.Mode; }
	if (Field == TEXT("Nomad")) { return Bool(L.bNomad); }
	if (Field == TEXT("Regicide")) { return Bool(L.bRegicide); }
	if (Field == TEXT("Catastrophe")) { return Bool(L.bCatastrophe); }
	if (Field == TEXT("SacredSites")) { return Bool(L.bSacredSites); }
	if (Field == TEXT("Maps")) { return FString::Join(L.Maps, TEXT(",")); }
	if (Field == TEXT("HasMatch")) { return Bool(L.bHasMatch); }
	if (Field == TEXT("MatchId")) { return L.MatchId; }
	if (Field == TEXT("MatchPort")) { return Int(L.MatchPort); }
	if (Field == TEXT("MatchPlayerName")) { return L.MatchPlayerName; }
	if (Field == TEXT("MatchGameVersion")) { return L.MatchGameVersion; }
	if (Field == TEXT("Notice")) { return L.Notice; }
	if (Field == TEXT("Spectators")) { return FString::Join(L.Spectators, TEXT(",")); }
	if (Field == TEXT("NumSlots")) { return Int(L.Slots.Num()); }
	if (Field == TEXT("NumPlayers")) { return Int(L.GetNumPlayers()); }
	if (Field == TEXT("NumAIs")) { return Int(L.GetNumAIs()); }
	if (Field == TEXT("MapSize")) { return Int(static_cast<int32>(L.GetMapSize())); }
	if (Field == TEXT("FogMode")) { return EnumName(StaticEnum<EHOLFogMode>(), static_cast<int64>(L.GetFogMode())); }
	if (Field == TEXT("BiomeKey")) { return L.GetBiomeKey(); }
	if (Field == TEXT("MatchMode")) { return EnumName(StaticEnum<EHOLMatchMode>(), static_cast<int64>(L.GetMatchMode())); }
	if (Field == TEXT("NumPlaces")) { return Int(L.GetNumPlaces()); }
	if (Field == TEXT("FindYourSlot")) { return Int(L.FindYourSlot()); }
	if (Field == TEXT("IsYouSpectator")) { return Bool(L.IsYouSpectator()); }
	if (Field == TEXT("HasOpenSlot")) { return Bool(L.HasOpenSlot()); }
	if (Field == TEXT("IsYouLeader")) { return Bool(L.IsYouLeader()); }
	if (Field == TEXT("CanStart")) { return Bool(L.CanStart()); }
	if (Field == TEXT("CanEditSlot")) { return Bool(L.CanEditSlot(SlotIndex)); }
	if (Field == TEXT("CanToggleSlot")) { return Bool(L.CanToggleSlot(SlotIndex)); }
	if (Field == TEXT("CanEditAISlot")) { return Bool(L.CanEditAISlot(SlotIndex)); }
	return FString();
}

FString UHOLManagerClientReadLibrary::LobbySlotField(const FString& Json, const int32 SlotIndex, const FString& Field)
{
	FHOLLobby L;
	if (!FHOLManagerClientRules::ParseLobby(Json, L) || !L.Slots.IsValidIndex(SlotIndex)) { return FString(); }
	const FHOLLobbySlot& S = L.Slots[SlotIndex];
	if (Field == TEXT("Index")) { return Int(S.Index); }
	if (Field == TEXT("State"))
	{
		switch (S.State)
		{
		case EHOLLobbySlotState::Open: return TEXT("open");
		case EHOLLobbySlotState::Closed: return TEXT("closed");
		case EHOLLobbySlotState::Player: return TEXT("player");
		default: return TEXT("ai");
		}
	}
	if (Field == TEXT("PlayerName")) { return S.PlayerName; }
	if (Field == TEXT("ColorIndex")) { return Int(S.ColorIndex); }
	if (Field == TEXT("Team")) { return Int(S.Team); }
	if (Field == TEXT("Faction")) { return S.Faction; }
	if (Field == TEXT("Ready")) { return Bool(S.bReady); }
	if (Field == TEXT("Level")) { return EnumName(StaticEnum<EHOLAIDifficulty>(), static_cast<int64>(S.Level)); }
	return FString();
}

bool UHOLManagerClientReadLibrary::IsJoin(const FString& Json)
{
	FString Token;
	int32 Slot = INDEX_NONE;
	FHOLLobby Lobby;
	return FHOLManagerClientRules::ParseJoin(Json, Token, Slot, Lobby);
}

FString UHOLManagerClientReadLibrary::JoinField(const FString& Json, const FString& Field)
{
	FString Token;
	int32 Slot = INDEX_NONE;
	FHOLLobby Lobby;
	if (!FHOLManagerClientRules::ParseJoin(Json, Token, Slot, Lobby)) { return FString(); }
	if (Field == TEXT("Token")) { return Token; }
	if (Field == TEXT("Slot")) { return Int(Slot); }
	if (Field == TEXT("LobbyId")) { return Lobby.Id; }
	return FString();
}

bool UHOLManagerClientReadLibrary::IsWatch(const FString& Json)
{
	FString MatchId;
	int32 Port = 0;
	return FHOLManagerClientRules::ParseWatch(Json, MatchId, Port);
}

FString UHOLManagerClientReadLibrary::WatchField(const FString& Json, const FString& Field)
{
	FString MatchId;
	int32 Port = 0;
	if (!FHOLManagerClientRules::ParseWatch(Json, MatchId, Port)) { return FString(); }
	if (Field == TEXT("MatchId")) { return MatchId; }
	if (Field == TEXT("Port")) { return Int(Port); }
	return FString();
}

bool UHOLManagerClientReadLibrary::IsProtocolError(const FString& Json)
{
	FHOLManagerError Error;
	return FHOLManagerClientRules::ParseError(Json, Error);
}

FString UHOLManagerClientReadLibrary::ErrorField(const FString& Json, const FString& Field)
{
	FHOLManagerError Error;
	if (!FHOLManagerClientRules::ParseError(Json, Error)) { return FString(); }
	if (Field == TEXT("Code")) { return Error.Code; }
	if (Field == TEXT("Message")) { return Error.Message; }
	return FString();
}

FString UHOLManagerClientReadLibrary::FailureField(const bool bConnected, const int32 HttpCode, const FString& Body, const FString& Field)
{
	const FHOLManagerError Error = FHOLManagerClientRules::ReadFailure(bConnected, HttpCode, Body);
	if (Field == TEXT("Kind"))
	{
		switch (Error.Kind)
		{
		case FHOLManagerError::EKind::None: return TEXT("None");
		case FHOLManagerError::EKind::Unreachable: return TEXT("Unreachable");
		case FHOLManagerError::EKind::Unreadable: return TEXT("Unreadable");
		default: return TEXT("Protocol");
		}
	}
	if (Field == TEXT("HttpCode")) { return Int(Error.HttpCode); }
	if (Field == TEXT("Code")) { return Error.Code; }
	if (Field == TEXT("Message")) { return Error.Message; }
	return FString();
}

int32 UHOLManagerClientReadLibrary::GetNumServers(const FString& Json)
{
	TArray<FHOLManagerServer> Rows;
	return FHOLManagerClientRules::ParseServers(Json, Rows) ? Rows.Num() : -1;
}

FString UHOLManagerClientReadLibrary::ServersField(const FString& Json, const int32 Row, const FString& Field)
{
	TArray<FHOLManagerServer> Rows;
	if (!FHOLManagerClientRules::ParseServers(Json, Rows) || !Rows.IsValidIndex(Row)) { return FString(); }
	if (Field == TEXT("Name")) { return Rows[Row].Name; }
	if (Field == TEXT("Url")) { return Rows[Row].Url; }
	if (Field == TEXT("Key")) { return Rows[Row].Key; }
	return FString();
}

FString UHOLManagerClientReadLibrary::MakeMeBody(const bool bHasColorIndex, const int32 ColorIndex, const bool bHasTeam, const int32 Team,
	const bool bHasReady, const bool bReady, const bool bHasSpectator, const bool bSpectator, const FString& Faction)
{
	return FHOLManagerClientRules::MakeMeBody(OptInt(bHasColorIndex, ColorIndex), OptInt(bHasTeam, Team), OptBool(bHasReady, bReady), OptBool(bHasSpectator, bSpectator), Faction);
}

FString UHOLManagerClientReadLibrary::MakeJoinBody(const FString& PlayerName, const FString& GameVersion, const bool bSpectator)
{
	return FHOLManagerClientRules::MakeJoinBody(PlayerName, GameVersion, bSpectator);
}

FString UHOLManagerClientReadLibrary::MakeSettingsBody(const FString& Map, const bool bNomad, const bool bRegicide, const bool bCatastrophe, const FString& Size,
	const bool bHasTeamsTogether, const bool bTeamsTogether, const FString& Fog, const FString& Mode, const FString& Biome, const bool bHasSacredSites, const bool bSacredSites)
{
	return FHOLManagerClientRules::MakeSettingsBody(Map, bNomad, bRegicide, bCatastrophe, Size, OptBool(bHasTeamsTogether, bTeamsTogether), Fog, Mode, Biome,
		OptBool(bHasSacredSites, bSacredSites));
}

FString UHOLManagerClientReadLibrary::MakeSlotBody(const bool bOpen)
{
	return FHOLManagerClientRules::MakeSlotBody(bOpen);
}

FString UHOLManagerClientReadLibrary::MakeAISlotBody(const bool bHasColorIndex, const int32 ColorIndex, const bool bHasTeam, const int32 Team, const FString& Faction)
{
	return FHOLManagerClientRules::MakeAISlotBody(OptInt(bHasColorIndex, ColorIndex), OptInt(bHasTeam, Team), Faction);
}

bool UHOLManagerClientReadLibrary::JsonHasField(const FString& Json, const FString& Field)
{
	const TSharedPtr<FJsonObject> Object = ReadManagerJsonObject(Json);
	return Object.IsValid() && Object->HasField(Field);
}

FString UHOLManagerClientReadLibrary::JsonField(const FString& Json, const FString& Field)
{
	const TSharedPtr<FJsonObject> Object = ReadManagerJsonObject(Json);
	return Object.IsValid() ? ValueText(Object->TryGetField(Field)) : FString();
}

int32 UHOLManagerClientReadLibrary::JsonNumFields(const FString& Json)
{
	const TSharedPtr<FJsonObject> Object = ReadManagerJsonObject(Json);
	return Object.IsValid() ? Object->Values.Num() : -1;
}

FString UHOLManagerClientReadLibrary::MakeMatchAddress(const FString& ManagerUrl, const int32 Port, const FString& PlayerName, const FString& GameVersion)
{
	return FHOLManagerClientRules::MakeMatchAddress(ManagerUrl, Port, PlayerName, GameVersion);
}

int32 UHOLManagerClientReadLibrary::GetMatchOrigin(const bool bHasToken, const bool bRejoinedLobby, const bool bNamed, const bool bInMatch)
{
	return static_cast<int32>(FHOLManagerClientRules::GetMatchOrigin(bHasToken, bRejoinedLobby, bNamed, bInMatch));
}

FString UHOLManagerClientReadLibrary::MakeUrl(const FString& Url, const FString& Route)
{
	return FHOLManagerClientRules::MakeUrl(Url, Route);
}

FString UHOLManagerClientReadLibrary::FormatReconnectLine(const double SecondsLeft)
{
	return FHOLReconnectRules::FormatLine(SecondsLeft).ToString();
}

double UHOLManagerClientReadLibrary::GetReconnectSecondsLeft(const double LostAt, const double Now, const double WindowSeconds)
{
	return FHOLReconnectRules::GetSecondsLeft(LostAt, Now, WindowSeconds);
}
