// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLManagerClientRules.h"

#include "Dom/JsonObject.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Player/HOLGameMenuRules.h"

const TCHAR* FHOLManagerClientRules::DefaultUrl = TEXT("http://127.0.0.1:7700");

namespace
{
	TSharedPtr<FJsonObject> ReadObject(const FString& Json)
	{
		TSharedPtr<FJsonObject> Object;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
		return FJsonSerializer::Deserialize(Reader, Object) ? Object : nullptr;
	}

	bool ReadState(const FString& Text, EHOLLobbyState& Out)
	{
		if (Text == TEXT("lobby")) { Out = EHOLLobbyState::Lobby; return true; }
		if (Text == TEXT("starting")) { Out = EHOLLobbyState::Starting; return true; }
		if (Text == TEXT("in_game")) { Out = EHOLLobbyState::InGame; return true; }
		return false;
	}
}

bool FHOLManagerClientRules::ParseStatus(const FString& Json, FHOLManagerStatus& Out)
{
	const TSharedPtr<FJsonObject> Object = ReadObject(Json);
	return Object.IsValid()
		&& Object->TryGetNumberField(TEXT("protocolVersion"), Out.ProtocolVersion)
		&& Object->TryGetStringField(TEXT("gameVersion"), Out.GameVersion)
		&& Object->TryGetNumberField(TEXT("lobbies"), Out.Lobbies)
		&& Object->TryGetNumberField(TEXT("servers"), Out.Servers)
		&& Object->TryGetNumberField(TEXT("maxServers"), Out.MaxServers);
}

bool FHOLManagerClientRules::ParseLobbies(const FString& Json, TArray<FHOLLobbySummary>& Out)
{
	Out.Reset();
	const TSharedPtr<FJsonObject> Object = ReadObject(Json);
	const TArray<TSharedPtr<FJsonValue>>* Rows = nullptr;
	if (!Object.IsValid() || !Object->TryGetArrayField(TEXT("lobbies"), Rows))
	{
		return false;
	}
	for (const TSharedPtr<FJsonValue>& Value : *Rows)
	{
		const TSharedPtr<FJsonObject>* Row = nullptr;
		FHOLLobbySummary Lobby;
		FString State;
		if (!Value.IsValid() || !Value->TryGetObject(Row)
			|| !(*Row)->TryGetStringField(TEXT("id"), Lobby.Id)
			|| !(*Row)->TryGetStringField(TEXT("name"), Lobby.Name)
			|| !(*Row)->TryGetNumberField(TEXT("players"), Lobby.Players)
			|| !(*Row)->TryGetNumberField(TEXT("maxPlayers"), Lobby.MaxPlayers)
			|| !(*Row)->TryGetStringField(TEXT("state"), State) || !ReadState(State, Lobby.State)
			|| !(*Row)->TryGetBoolField(TEXT("canJoin"), Lobby.bCanJoin)
			|| !(*Row)->TryGetBoolField(TEXT("canWatch"), Lobby.bCanWatch))
		{
			Out.Reset();
			return false;
		}
		// null (or absent) outside in_game.
		if (!(*Row)->TryGetNumberField(TEXT("minutes"), Lobby.Minutes))
		{
			Lobby.Minutes = -1;
		}
		Lobby.Away.Reset();
		// R7.6, added to protocol 1: absent from an older manager.
		if (const TArray<TSharedPtr<FJsonValue>>* Away = nullptr; (*Row)->TryGetArrayField(TEXT("away"), Away))
		{
			for (const TSharedPtr<FJsonValue>& Name : *Away)
			{
				FString Text;
				if (Name.IsValid() && Name->TryGetString(Text))
				{
					Lobby.Away.Add(MoveTemp(Text));
				}
			}
		}
		Out.Add(MoveTemp(Lobby));
	}
	return true;
}

EHOLLobbyAction FHOLManagerClientRules::ChooseLobbyAction(const FHOLLobbySummary& Lobby, const FString& MyName)
{
	if (Lobby.State == EHOLLobbyState::Lobby && Lobby.bCanJoin)
	{
		return EHOLLobbyAction::Join;
	}
	if (Lobby.State != EHOLLobbyState::InGame || !Lobby.bCanWatch)
	{
		return EHOLLobbyAction::None;
	}
	// R7.6: the manager's names are the match's (PROTOCOL.md 8: compared case ignored, trimmed).
	const FString Name = MyName.TrimStartAndEnd();
	const bool bAway = !Name.IsEmpty() && Lobby.Away.ContainsByPredicate([&Name](const FString& Each)
		{ return Each.TrimStartAndEnd().Equals(Name, ESearchCase::IgnoreCase); });
	return bAway ? EHOLLobbyAction::Rejoin : EHOLLobbyAction::Watch;
}

int32 FHOLLobby::GetNumPlayers() const
{
	return Slots.FilterByPredicate([](const FHOLLobbySlot& Slot) { return Slot.State == EHOLLobbySlotState::Player; }).Num();
}

int32 FHOLLobby::FindYourSlot() const
{
	return You.IsEmpty() ? INDEX_NONE : Slots.IndexOfByPredicate([this](const FHOLLobbySlot& Slot)
	{
		return Slot.State == EHOLLobbySlotState::Player && Slot.PlayerName == You;
	});
}

bool FHOLLobby::IsYouLeader() const
{
	return !You.IsEmpty() && You == Leader;
}

bool FHOLLobby::CanEditSlot(const int32 SlotIndex) const
{
	return SlotIndex != INDEX_NONE && SlotIndex == FindYourSlot();
}

bool FHOLLobby::CanToggleSlot(const int32 SlotIndex) const
{
	return IsYouLeader() && Slots.IsValidIndex(SlotIndex) && Slots[SlotIndex].State != EHOLLobbySlotState::Player;
}

bool FHOLLobby::CanStart() const
{
	return IsYouLeader() && GetNumPlayers() >= 2
		&& !Slots.ContainsByPredicate([](const FHOLLobbySlot& Slot) { return Slot.State == EHOLLobbySlotState::Player && !Slot.bReady; });
}

namespace
{
	FString WriteBody(const TSharedRef<FJsonObject>& Object)
	{
		FString Body;
		const TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Body);
		FJsonSerializer::Serialize(Object, Writer);
		return Body;
	}
}

FString FHOLManagerClientRules::MakeMeBody(const TOptional<int32> ColorIndex, const TOptional<int32> Team, const TOptional<bool> bReady)
{
	const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
	if (ColorIndex.IsSet())
	{
		Object->SetNumberField(TEXT("colorIndex"), ColorIndex.GetValue());
	}
	if (Team.IsSet())
	{
		Object->SetNumberField(TEXT("team"), Team.GetValue());
	}
	if (bReady.IsSet())
	{
		Object->SetBoolField(TEXT("ready"), bReady.GetValue());
	}
	return WriteBody(Object);
}

FString FHOLManagerClientRules::MakeSettingsBody(const FString& Map, const bool bNomad, const bool bRegicide, const bool bCatastrophe)
{
	const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
	Object->SetStringField(TEXT("map"), Map);
	Object->SetBoolField(TEXT("nomad"), bNomad);
	Object->SetBoolField(TEXT("regicide"), bRegicide);
	Object->SetBoolField(TEXT("catastrophe"), bCatastrophe);
	return WriteBody(Object);
}

FString FHOLManagerClientRules::MakeSlotBody(const bool bOpen)
{
	const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
	Object->SetStringField(TEXT("state"), bOpen ? TEXT("open") : TEXT("closed"));
	return WriteBody(Object);
}

namespace
{
	bool ReadSlotState(const FString& Text, EHOLLobbySlotState& Out)
	{
		if (Text == TEXT("open")) { Out = EHOLLobbySlotState::Open; return true; }
		if (Text == TEXT("closed")) { Out = EHOLLobbySlotState::Closed; return true; }
		if (Text == TEXT("player")) { Out = EHOLLobbySlotState::Player; return true; }
		return false;
	}

	bool ReadLobbyObject(const FJsonObject& Object, FHOLLobby& Out)
	{
		Out = FHOLLobby();
		FString State;
		const TSharedPtr<FJsonObject>* Settings = nullptr;
		const TArray<TSharedPtr<FJsonValue>>* Slots = nullptr;
		if (!Object.TryGetStringField(TEXT("id"), Out.Id) || !Object.TryGetStringField(TEXT("name"), Out.Name)
			|| !Object.TryGetStringField(TEXT("state"), State) || !ReadState(State, Out.State)
			|| !Object.TryGetObjectField(TEXT("settings"), Settings) || !(*Settings)->TryGetStringField(TEXT("map"), Out.Map)
			|| !Object.TryGetArrayField(TEXT("slots"), Slots) || Slots->Num() != 8)
		{
			return false;
		}
		// null (or absent): -1 and empty.
		if (!Object.TryGetNumberField(TEXT("minutes"), Out.Minutes))
		{
			Out.Minutes = -1;
		}
		Object.TryGetStringField(TEXT("leader"), Out.Leader);
		Object.TryGetStringField(TEXT("you"), Out.You);
		Object.TryGetStringField(TEXT("notice"), Out.Notice);
		(*Settings)->TryGetBoolField(TEXT("nomad"), Out.bNomad);
		(*Settings)->TryGetBoolField(TEXT("regicide"), Out.bRegicide);
		(*Settings)->TryGetBoolField(TEXT("catastrophe"), Out.bCatastrophe);
		const TArray<TSharedPtr<FJsonValue>>* Maps = nullptr;
		if (Object.TryGetArrayField(TEXT("maps"), Maps))
		{
			for (const TSharedPtr<FJsonValue>& Value : *Maps)
			{
				const TSharedPtr<FJsonObject>* Map = nullptr;
				FString Name;
				if (Value.IsValid() && Value->TryGetObject(Map) && (*Map)->TryGetStringField(TEXT("name"), Name))
				{
					Out.Maps.Add(MoveTemp(Name));
				}
			}
		}
		for (const TSharedPtr<FJsonValue>& Value : *Slots)
		{
			const TSharedPtr<FJsonObject>* Row = nullptr;
			FHOLLobbySlot Slot;
			if (!Value.IsValid() || !Value->TryGetObject(Row) || !(*Row)->TryGetNumberField(TEXT("index"), Slot.Index)
				|| !(*Row)->TryGetStringField(TEXT("state"), State) || !ReadSlotState(State, Slot.State))
			{
				return false;
			}
			if (Slot.State == EHOLLobbySlotState::Player
				&& (!(*Row)->TryGetStringField(TEXT("playerName"), Slot.PlayerName) || !(*Row)->TryGetNumberField(TEXT("colorIndex"), Slot.ColorIndex)
					|| !(*Row)->TryGetNumberField(TEXT("team"), Slot.Team) || !(*Row)->TryGetBoolField(TEXT("ready"), Slot.bReady)))
			{
				return false;
			}
			(*Row)->TryGetStringField(TEXT("faction"), Slot.Faction);
			Out.Slots.Add(MoveTemp(Slot));
		}
		const TSharedPtr<FJsonObject>* Match = nullptr;
		Out.bHasMatch = Object.TryGetObjectField(TEXT("match"), Match);
		if (Out.bHasMatch)
		{
			(*Match)->TryGetStringField(TEXT("matchId"), Out.MatchId);
			(*Match)->TryGetNumberField(TEXT("port"), Out.MatchPort);
			(*Match)->TryGetStringField(TEXT("playerName"), Out.MatchPlayerName);
			(*Match)->TryGetStringField(TEXT("gameVersion"), Out.MatchGameVersion);
		}
		return true;
	}
}

bool FHOLManagerClientRules::ParseLobby(const FString& Json, FHOLLobby& Out)
{
	const TSharedPtr<FJsonObject> Object = ReadObject(Json);
	if (!Object.IsValid() || !ReadLobbyObject(*Object, Out))
	{
		Out = FHOLLobby();
		return false;
	}
	return true;
}

bool FHOLManagerClientRules::ParseJoin(const FString& Json, FString& OutToken, int32& OutSlot, FHOLLobby& OutLobby)
{
	const TSharedPtr<FJsonObject> Object = ReadObject(Json);
	const TSharedPtr<FJsonObject>* Lobby = nullptr;
	return Object.IsValid() && Object->TryGetStringField(TEXT("token"), OutToken) && !OutToken.IsEmpty()
		&& Object->TryGetNumberField(TEXT("slot"), OutSlot) && Object->TryGetObjectField(TEXT("lobby"), Lobby)
		&& ReadLobbyObject(**Lobby, OutLobby);
}

bool FHOLManagerClientRules::ParseError(const FString& Json, FHOLManagerError& Out)
{
	const TSharedPtr<FJsonObject> Object = ReadObject(Json);
	if (!Object.IsValid() || !Object->TryGetStringField(TEXT("error"), Out.Code) || Out.Code.IsEmpty())
	{
		return false;
	}
	Object->TryGetStringField(TEXT("message"), Out.Message);
	return true;
}

bool FHOLManagerClientRules::ParseServers(const FString& Json, TArray<FHOLManagerServer>& Out)
{
	Out.Reset();
	const TSharedPtr<FJsonObject> Object = ReadObject(Json);
	const TArray<TSharedPtr<FJsonValue>>* Rows = nullptr;
	if (!Object.IsValid() || !Object->TryGetArrayField(TEXT("servers"), Rows))
	{
		return false;
	}
	for (const TSharedPtr<FJsonValue>& Value : *Rows)
	{
		const TSharedPtr<FJsonObject>* Row = nullptr;
		FHOLManagerServer Server;
		if (Value.IsValid() && Value->TryGetObject(Row) && (*Row)->TryGetStringField(TEXT("url"), Server.Url) && !Server.Url.IsEmpty())
		{
			(*Row)->TryGetStringField(TEXT("name"), Server.Name);
			(*Row)->TryGetStringField(TEXT("key"), Server.Key);
			Server.Key.TrimStartAndEndInline();
			Out.Add(MoveTemp(Server));
		}
	}
	return true;
}

FHOLManagerError FHOLManagerClientRules::ReadFailure(const bool bConnected, const int32 HttpCode, const FString& Body)
{
	FHOLManagerError Error;
	Error.HttpCode = HttpCode;
	if (!bConnected || HttpCode <= 0)
	{
		Error.Kind = FHOLManagerError::EKind::Unreachable;
	}
	else if (HttpCode < 200 || HttpCode >= 300)
	{
		Error.Kind = ParseError(Body, Error) ? FHOLManagerError::EKind::Protocol : FHOLManagerError::EKind::Unreadable;
		if (Error.Kind == FHOLManagerError::EKind::Unreadable)
		{
			Error.Message = Body.Left(200);
		}
	}
	return Error;
}

EHOLMatchOrigin FHOLManagerClientRules::GetMatchOrigin(const bool bHasToken, const bool bRejoinedLobby, const bool bNamed, const bool bInMatch)
{
	if ((bHasToken || bRejoinedLobby) && bNamed)
	{
		return EHOLMatchOrigin::Lobby;
	}
	return bInMatch ? EHOLMatchOrigin::Manager : EHOLMatchOrigin::Local;
}

FString FHOLManagerClientRules::MakeUrl(const FString& Url, const FString& Route)
{
	FString Base = Url;
	Base.RemoveFromEnd(TEXT("/"));
	return Route.StartsWith(TEXT("/")) ? Base + Route : Base + TEXT("/") + Route;
}

bool FHOLManagerClientRules::ParseWatch(const FString& Json, FString& OutMatchId, int32& OutPort)
{
	const TSharedPtr<FJsonObject> Object = ReadObject(Json);
	OutPort = 0;
	if (!Object.IsValid() || !Object->TryGetNumberField(TEXT("port"), OutPort) || OutPort <= 0)
	{
		OutPort = 0;
		return false;
	}
	Object->TryGetStringField(TEXT("matchId"), OutMatchId);
	return true;
}

FString FHOLManagerClientRules::MakeMatchAddress(const FString& ManagerUrl, const int32 Port, const FString& PlayerName, const FString& GameVersion)
{
	// The manager's host: no scheme, no path, no port.
	FString Host = ManagerUrl;
	const int32 Scheme = Host.Find(TEXT("://"));
	if (Scheme != INDEX_NONE)
	{
		Host.RightChopInline(Scheme + 3);
	}
	int32 Cut = INDEX_NONE;
	if (Host.FindChar(TEXT('/'), Cut))
	{
		Host.LeftInline(Cut);
	}
	if (Host.FindLastChar(TEXT(':'), Cut))
	{
		Host.LeftInline(Cut);
	}
	if (Host.IsEmpty() || Port <= 0)
	{
		return FString();
	}
	FString Address = FString::Printf(TEXT("%s:%d?"), *Host, Port);
	if (!PlayerName.IsEmpty())
	{
		Address += TEXT("Name=") + FGenericPlatformHttp::UrlEncode(PlayerName) + TEXT("?");
	}
	return Address + TEXT("Version=") + FGenericPlatformHttp::UrlEncode(GameVersion);
}
