// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Net/HOLManagerClientRules.h"
#include "Player/HOLGameMenuRules.h"
#include "Engine/EngineBaseTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HOLManagerClient.generated.h"

DECLARE_DELEGATE_TwoParams(FHOLManagerStatusDone, const FHOLManagerStatus& /*Status*/, const FHOLManagerError& /*Error*/);
DECLARE_DELEGATE_TwoParams(FHOLManagerLobbyDone, const FHOLLobby& /*Lobby*/, const FHOLManagerError& /*Error*/);
DECLARE_DELEGATE_TwoParams(FHOLManagerStartDone, const FString& /*MatchId*/, const FHOLManagerError& /*Error, its HttpCode also on success*/);
DECLARE_DELEGATE_ThreeParams(FHOLManagerWatchDone, const FString& /*MatchId*/, int32 /*Port*/, const FHOLManagerError& /*Error*/);
DECLARE_DELEGATE_TwoParams(FHOLManagerLobbiesDone,const TArray<FHOLLobbySummary>& /*Lobbies*/, const FHOLManagerError& /*Error*/);

/**
 * The game's client of the lobby manager (M6 R4.1, Manager/PROTOCOL.md): in the interface module, so never on a
 * dedicated server (D247). The managers come from servers.json, never shown (M6-Q5):
 *   { "servers": [ { "name": "PC de Baudouin", "url": "http://100.x.y.z:7700", "key": "..." } ] }
 * The optional key (launcher L11) goes in the X-HOL-Key header of every call and, through HOLAccessKey::SetClientKey, in
 * the login options of the match servers (?HOLKey=).
 * (Tools\Manager\servers.example.json). Read from the project folder in the editor, beside HeroesOfLiberty.exe in a
 * package (FPaths::RootDir()), or from -ServersFile=<path>; without one, http://127.0.0.1:7700. The first server is
 * the one called. Each call is asynchronous and answers once, with the structures or the error.
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLManagerClient : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** GET /status. */
	void GetStatus(FHOLManagerStatusDone Done);
	/** GET /lobbies. */
	void GetLobbies(FHOLManagerLobbiesDone Done);

	/** POST /lobbies (R4.4): creates a lobby and enters it; its token is kept. */
	void CreateLobby(const FString& LobbyName, const FString& PlayerName, FHOLManagerLobbyDone Done);
	/** POST /lobbies/{id}/join with this game's version; its token is kept. */
	void JoinLobby(const FString& LobbyId, const FString& PlayerName, FHOLManagerLobbyDone Done);
	/** GET /lobbies/{id} with the kept token, which is the presence (5 s without a read: out). Out of the lobby
	 *  (not_in_lobby, lobby_not_found), the token is dropped. */
	void ReadLobby(FHOLManagerLobbyDone Done);
	/** POST /lobbies/{id}/leave; the token is dropped at once. */
	void LeaveLobby();
	/** PUT /lobbies/{id}/me (R4.5): my colour, team, Ready; only the fields set. Answers the lobby's state. */
	void UpdateMe(TOptional<int32> ColorIndex, TOptional<int32> Team, TOptional<bool> bReady, FHOLManagerLobbyDone Done);
	/** PUT /lobbies/{id}/settings (leader): the map by name and the options. */
	void UpdateSettings(const FString& Map, bool bNomad, bool bRegicide, bool bCatastrophe, FHOLManagerLobbyDone Done,
		const FString& Size = FString(), TOptional<bool> bTeamsTogether = {});
	/** PUT /lobbies/{id}/slots/{index} (leader): opens or closes a slot nobody holds. */
	void SetSlot(int32 SlotIndex, bool bOpen, FHOLManagerLobbyDone Done);
	/** POST /lobbies/{id}/start (leader): 202 and the match's id, the lobby then "starting". */
	void StartMatch(FHOLManagerStartDone Done);
	/** GET /lobbies/{id}/watch (no token): the match's id and port of a lobby in_game (no_match otherwise). */
	void WatchLobby(const FString& InLobbyId, FHOLManagerWatchDone Done);
	/**
	 * Opens the match's server, on the manager's host (R4.7): as PlayerName, or as a spectator if empty (M6-Q9). The
	 * lobby's token is kept (back to the lobby after the match, R4.8). False without an address.
	 */
	bool OpenMatch(int32 Port, const FString& PlayerName, const FString& GameVersion);
	bool IsInLobby() const { return !LobbyToken.IsEmpty(); }
	/** In a match opened from a lobby with a name (R4.8): its end banner offers the way back. */
	bool IsInLobbyMatch() const { return GetMatchOrigin() == EHOLMatchOrigin::Lobby; }
	/**
	 * Where the match now played was opened from (M6-Q10): Lobby for a lobby's player, Manager for any other match
	 * opened by OpenMatch (the list's « Regarder »: no lobby), Local otherwise. Its end banner's way out follows.
	 */
	EHOLMatchOrigin GetMatchOrigin() const
	{
		return FHOLManagerClientRules::GetMatchOrigin(IsInLobby(), bRejoinedLobby, !MatchPlayerName.IsEmpty(), bInMatch);
	}
	/**
	 * Back in a lobby's match by the list's « Revenir » (R7.6, B6c): no token (the lobby is in game), only its id, kept
	 * for « Retour au salon » (ReturnToLobby joins again by name). Before OpenMatch.
	 */
	void KeepRejoinedLobby(const FString& InLobbyId);

	/** Where the menu goes once the match is left (R4.8): read once by the menu's player controller. */
	enum class EAfterMatch : uint8 { None, Lobby, List };
	void SetAfterMatch(EAfterMatch InAfterMatch) { AfterMatch = InAfterMatch; }
	EAfterMatch TakeAfterMatch() { const EAfterMatch Taken = AfterMatch; AfterMatch = EAfterMatch::None; bInMatch = false; return Taken; }
	/** POST /lobbies/{id}/join again with the match's name (PROTOCOL.md, « Retour au salon », within 2 min). */
	void ReturnToLobby(FHOLManagerLobbyDone Done);
	/** « Quitter vers la liste »: the lobby is forgotten here, the manager drops the player by itself. */
	void ForgetLobby();
	const FString& GetLobbyId() const { return LobbyId; }

	/** Why the last lobby was lost, for the Multiplayer screen to show once; None if it was left. */
	void SetLobbyLoss(const FHOLManagerError& Error) { LobbyLoss = Error; }
	FHOLManagerError TakeLobbyLoss() { return MoveTemp(LobbyLoss); }

	const TArray<FHOLManagerServer>& GetServers() const { return Servers; }
	const FString& GetUrl() const { return Servers[0].Url; }
	/** Whether a manager serves this game's version (HOLVersion::Get()). */
	static bool IsSameVersion(const FHOLManagerStatus& Status);

	/** servers.json's path (-ServersFile= first). */
	static FString GetServersFilePath();

private:
	void Get(const FString& Route, TFunction<void(const FHOLManagerError&, const FString&)> Done);
	/** A call: Verb on Route, Body as JSON if not empty, the token's header if not empty. */
	void Send(const FString& Verb, const FString& Route, const FString& Body, const FString& Token,
		TFunction<void(const FHOLManagerError&, const FString&)> Done);
	/** Create and join: the token kept, the lobby given. */
	void Enter(const FString& Route, const FString& Body, FHOLManagerLobbyDone Done);
	/** A PUT with the token that answers the lobby's state. */
	void Write(const FString& Route, const FString& Body, FHOLManagerLobbyDone Done);

	/** A match opened here lost (R4.9): the lobby forgotten, the Multiplayer screen opened with why. */
	void HandleNetworkFailure(UWorld* World, class UNetDriver* NetDriver, ENetworkFailure::Type Failure, const FString& Message);
	void HandleTravelFailure(UWorld* World, ETravelFailure::Type Failure, const FString& Message);
	void LoseMatch(const FHOLManagerError& Error);

	TArray<FHOLManagerServer> Servers;
	/** In a match opened by OpenMatch (player or spectator), until it is lost or the menu is back. */
	bool bInMatch = false;
	FDelegateHandle NetworkFailureHandle;
	FDelegateHandle TravelFailureHandle;
	FString LobbyId;
	FString LobbyToken;
	/** LobbyId kept by KeepRejoinedLobby, without a token: until the lobby is joined again, forgotten or left. */
	bool bRejoinedLobby = false;
	/** The name the match was opened with (R4.7), for the way back (R4.8). */
	FString MatchPlayerName;
	EAfterMatch AfterMatch = EAfterMatch::None;
	FHOLManagerError LobbyLoss;
};
