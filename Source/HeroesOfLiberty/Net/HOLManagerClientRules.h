// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Map/HOLMapSizeRules.h"

/** A lobby's state in GET /lobbies (Manager/PROTOCOL.md). */
enum class EHOLLobbyState : uint8 { Lobby, Starting, InGame };

/** GET /status. */
struct FHOLManagerStatus
{
	int32 ProtocolVersion = 0;
	FString GameVersion;
	int32 Lobbies = 0;
	int32 Servers = 0;
	int32 MaxServers = 0;
};

/** A row of GET /lobbies. */
struct FHOLLobbySummary
{
	FString Id;
	FString Name;
	/** TC3: the lobby's map and size keys ("Plain", "Large"); empty from an older manager. */
	FString Map;
	FString Size;
	int32 Players = 0;
	int32 MaxPlayers = 0;
	EHOLLobbyState State = EHOLLobbyState::Lobby;
	/** Minutes of match; -1 for null (outside in_game). */
	int32 Minutes = -1;
	bool bCanJoin = false;
	bool bCanWatch = false;
};

/** Why a call to the manager failed: no answer, an answer that is not the protocol's, or its { error, message }. */
struct FHOLManagerError
{
	enum class EKind : uint8 { None, Unreachable, Unreadable, Protocol };
	EKind Kind = EKind::None;
	int32 HttpCode = 0;
	/** The protocol's stable code (lobby_full, ...): the text shown is chosen from it (R4.3). */
	FString Code;
	/** English, for the log. */
	FString Message;

	bool IsError() const { return Kind != EKind::None; }
};

/** A slot's state in the lobby's state (GET /lobbies/{id}). */
enum class EHOLLobbySlotState : uint8 { Open, Closed, Player };

/** A row of the lobby's state: the player's fields only for Player. */
struct FHOLLobbySlot
{
	int32 Index = 0;
	EHOLLobbySlotState State = EHOLLobbySlotState::Open;
	FString PlayerName;
	int32 ColorIndex = 0;
	int32 Team = 0;
	/** "DawnOrder", "BlackLegions"... (not changeable yet). */
	FString Faction;
	bool bReady = false;
};

/** The lobby's state (GET /lobbies/{id}, also in the answers of create and join). */
struct HEROESOFLIBERTY_API FHOLLobby
{
	FString Id;
	FString Name;
	EHOLLobbyState State = EHOLLobbyState::Lobby;
	/** -1 for null. */
	int32 Minutes = -1;
	FString Leader;
	/** The name of the token; empty without one. */
	FString You;
	FString Map;
	/** TC3 (TC-Q3, TC-Q4): the size's key (Small... Huge, EHOLMapSize by name); empty for a map without sizes. */
	FString Size;
	/** TC3 (TC-Q6): "Équipes ensemble"; true when the manager does not say. */
	bool bTeamsTogether = true;
	bool bNomad = false;
	bool bRegicide = false;
	bool bCatastrophe = false;
	/** The maps the manager offers, by name. */
	TArray<FString> Maps;
	/** Always 8. */
	TArray<FHOLLobbySlot> Slots;
	/** match is not null (in_game): its fields below. */
	bool bHasMatch = false;
	FString MatchId;
	int32 MatchPort = 0;
	FString MatchPlayerName;
	FString MatchGameVersion;
	/** "server_stopped", "leader_changed"; empty for null. */
	FString Notice;

	int32 GetNumPlayers() const;
	/** TC3: the lobby's size; None without one (an older manager, a map without sizes). */
	EHOLMapSize GetMapSize() const;
	/** TC3: the places of the lobby's map at its size; FHOLMatchConfig::MaxSlots without a size. */
	int32 GetNumPlaces() const;
	/** The slot of the token's player; INDEX_NONE without one. */
	int32 FindYourSlot() const;

	/** The token's player leads the lobby (M6-Q3): map, options, slots, Launch. */
	bool IsYouLeader() const;
	/** A line's colour, team and Ready: only my own line (M6-Q4). */
	bool CanEditSlot(int32 SlotIndex) const;
	/** Opening or closing a slot: the leader's, on a slot nobody holds, within the places of the size (TC3). */
	bool CanToggleSlot(int32 SlotIndex) const;
	/** Launch shown active: the leader's, 2 players at least, all Ready (the manager still decides). */
	bool CanStart() const;
};

/** A manager the game may call, from servers.json (its address is never shown, M6-Q5). */
struct FHOLManagerServer
{
	FString Name;
	FString Url;
	/** The shared access key (launcher L11, HOLAccessKey); empty: none (an open manager). Never logged. */
	FString Key;
};

/** The pure rules of the game's client of the lobby manager (R4.1): JSON of the protocol in, structures out. */
struct HEROESOFLIBERTY_API FHOLManagerClientRules
{
	/** Without servers.json: a manager on this PC. */
	static const TCHAR* DefaultUrl;

	static bool ParseStatus(const FString& Json, FHOLManagerStatus& Out);
	static bool ParseLobbies(const FString& Json, TArray<FHOLLobbySummary>& Out);
	/** The lobby's state; false unless its 8 slots are read. */
	static bool ParseLobby(const FString& Json, FHOLLobby& Out);
	/** The answer of create and join: { token, slot, lobby }. */
	static bool ParseJoin(const FString& Json, FString& OutToken, int32& OutSlot, FHOLLobby& OutLobby);
	/** { "error": "...", "message": "..." } into Code and Message. */
	static bool ParseError(const FString& Json, FHOLManagerError& Out);
	/** { "servers": [ { "name": "...", "url": "http://...", "key": "..." } ] } (key optional); rows without a url skipped. */
	static bool ParseServers(const FString& Json, TArray<FHOLManagerServer>& Out);

	/**
	 * The failure of an answered or unanswered call (None for a 2xx answer): Unreachable without an answer, Protocol
	 * for a { error, message } body, Unreadable otherwise.
	 */
	static FHOLManagerError ReadFailure(bool bConnected, int32 HttpCode, const FString& Body);

	/** PUT /lobbies/{id}/me: only the fields given ({ "colorIndex", "team", "ready" }). */
	static FString MakeMeBody(TOptional<int32> ColorIndex, TOptional<int32> Team, TOptional<bool> bReady);
	/**
	 * PUT /lobbies/{id}/settings: { "map", "nomad", "regicide", "catastrophe" }, then (TC3) "size" when Size is not empty and
	 * "teamsTogether" when given.
	 */
	static FString MakeSettingsBody(const FString& Map, bool bNomad, bool bRegicide, bool bCatastrophe, const FString& Size = FString(),
		TOptional<bool> bTeamsTogether = {});
	/** PUT /lobbies/{id}/slots/{index}: { "state": "open" | "closed" }. */
	static FString MakeSlotBody(bool bOpen);

	/** GET /lobbies/{id}/watch: { matchId, port, minutes }; false without a port. */
	static bool ParseWatch(const FString& Json, FString& OutMatchId, int32& OutPort);
	/**
	 * What the client opens to reach a match's server (R4.7): the manager's host, the match's port, then
	 * ?Name=<PlayerName>?Version=<GameVersion>, URL-encoded (an Unreal URL parts its options with '?', not '&'); no Name for
	 * a spectator (M6-Q9). Empty without a host or port.
	 */
	static FString MakeMatchAddress(const FString& ManagerUrl, int32 Port, const FString& PlayerName, const FString& GameVersion);

	/** {Url}{Route}, one slash between them. */
	static FString MakeUrl(const FString& Url, const FString& Route);
};
