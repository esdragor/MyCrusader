// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLManagerClientReadLibrary.generated.h"

/**
 * MIG-C-ManagerClient (CELEBORN-1117l): the script's read of FHOLManagerClientRules (Net/HOLManagerClientRules.h, C++, stays: the manager
 * client calls it), ENVELOPES ONLY (no rule here, a call and a copy each). The pure part only: JSON of the manager in, FLAT values out
 * (the structures FHOLLobby, FHOLLobbySummary... are plain C++ structs that cannot cross to the script). The HTTP calls (FHOLManagerLink)
 * and the stateful FHOLReconnect stay C++ (network, Tick on a world clock). Script name: HOLManagerClientRead::<Function>.
 *
 * A value is its text: a bool "true" / "false", a number its digits, an enum its name; a field of a JSON that cannot be read is "".
 * The lobby's fields (LobbyField): Id, Name, State (lobby, starting, in_game), Minutes, Leader, You, Map, Size, TeamsTogether, Fog, Biome,
 * Mode, Nomad, Regicide, Catastrophe, SacredSites, Maps (joined by ","), HasMatch, MatchId, MatchPort, MatchPlayerName, MatchGameVersion,
 * Notice, Spectators (joined by ","), NumSlots, NumPlayers, NumAIs, MapSize (EHOLMapSize as a number: 0 None, 1 Small .. 5 Huge), FogMode,
 * BiomeKey, MatchMode, NumPlaces, FindYourSlot, IsYouSpectator, HasOpenSlot, IsYouLeader, CanStart; with a SlotIndex: CanEditSlot,
 * CanToggleSlot, CanEditAISlot.
 * A slot's fields (LobbySlotField): Index, State (open, closed, player, ai), PlayerName, ColorIndex, Team, Faction, Ready, Level.
 * A row of the list (LobbiesField): Id, Name, Map, Size, Players, MaxPlayers, State, Minutes, CanJoin, CanWatch, Away (joined by ","),
 * Spectators, CanSpectate.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLManagerClientReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** ParseStatus is true. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static bool IsStatus(const FString& Json);
	/** Of the status: ProtocolVersion, GameVersion, Lobbies, Servers, MaxServers. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static FString StatusField(const FString& Json, const FString& Field);

	/** The number of rows of ParseLobbies; -1 when it is false. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static int32 GetNumLobbies(const FString& Json);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static FString LobbiesField(const FString& Json, int32 Row, const FString& Field);
	/** ChooseLobbyAction of a row: 0 None, 1 Join, 2 Watch, 3 Rejoin (EHOLLobbyAction); -1 without the row. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static int32 ChooseLobbyAction(const FString& Json, int32 Row, const FString& MyName);

	/** ParseLobby is true. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static bool IsLobby(const FString& Json);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static FString LobbyField(const FString& Json, const FString& Field, int32 SlotIndex = -1);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static FString LobbySlotField(const FString& Json, int32 SlotIndex, const FString& Field);

	/** ParseJoin is true. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static bool IsJoin(const FString& Json);
	/** Of the answer of join: Token, Slot, LobbyId. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static FString JoinField(const FString& Json, const FString& Field);

	/** ParseWatch is true. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static bool IsWatch(const FString& Json);
	/** Of the answer of watch: MatchId, Port. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static FString WatchField(const FString& Json, const FString& Field);

	/** ParseError is true. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static bool IsProtocolError(const FString& Json);
	/** Of ParseError: Code, Message. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static FString ErrorField(const FString& Json, const FString& Field);
	/** Of ReadFailure: Kind (None, Unreachable, Unreadable, Protocol), HttpCode, Code, Message. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static FString FailureField(bool bConnected, int32 HttpCode, const FString& Body, const FString& Field);

	/** The number of servers of ParseServers; -1 when it is false. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static int32 GetNumServers(const FString& Json);
	/** Of a server: Name, Url, Key. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static FString ServersField(const FString& Json, int32 Row, const FString& Field);

	/** MakeMeBody; a field is sent only when its bHas is true. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static FString MakeMeBody(bool bHasColorIndex, int32 ColorIndex, bool bHasTeam, int32 Team, bool bHasReady, bool bReady, bool bHasSpectator, bool bSpectator, const FString& Faction);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static FString MakeJoinBody(const FString& PlayerName, const FString& GameVersion, bool bSpectator);
	/** MakeSettingsBody; teamsTogether and sacredSites are sent only when their bHas is true. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static FString MakeSettingsBody(const FString& Map, bool bNomad, bool bRegicide, bool bCatastrophe, const FString& Size, bool bHasTeamsTogether, bool bTeamsTogether,
		const FString& Fog, const FString& Mode, const FString& Biome, bool bHasSacredSites, bool bSacredSites);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static FString MakeSlotBody(bool bOpen);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static FString MakeAISlotBody(bool bHasColorIndex, int32 ColorIndex, bool bHasTeam, int32 Team, const FString& Faction);

	/** The fields of a body (or any JSON object) as the receiver sees them: is Field there, its value as text, the number of fields. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static bool JsonHasField(const FString& Json, const FString& Field);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static FString JsonField(const FString& Json, const FString& Field);
	/** -1 when Json is not an object. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static int32 JsonNumFields(const FString& Json);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static FString MakeMatchAddress(const FString& ManagerUrl, int32 Port, const FString& PlayerName, const FString& GameVersion);
	/** GetMatchOrigin: 0 Local, 1 Manager, 2 Lobby (EHOLMatchOrigin). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static int32 GetMatchOrigin(bool bHasToken, bool bRejoinedLobby, bool bNamed, bool bInMatch);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static FString MakeUrl(const FString& Url, const FString& Route);

	/** FHOLReconnectRules::FormatLine and GetSecondsLeft. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static FString FormatReconnectLine(double SecondsLeft);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|ManagerClient")
	static double GetReconnectSecondsLeft(double LostAt, double Now, double WindowSeconds);
};
