// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HOLManagerLink.generated.h"

/** The calls of a match server to the lobby manager (Manager/PROTOCOL.md, "Routes du serveur Unreal"). */
struct HEROESOFLIBERTY_API FHOLManagerRules
{
	/** {ManagerUrl}/matches/{MatchId}/{Event}, one slash between the parts. */
	static FString MakeRouteUrl(const FString& ManagerUrl, const FString& MatchId, const FString& Event);
	/** { "port": 7778 } */
	static FString MakeListeningBody(int32 Port);
	/** { "winners": [0, 2] } (slots) */
	static FString MakeFinishedBody(TConstArrayView<int32> Winners);
	/** { "playerName": "Bob" } (left, M6-Q14) */
	static FString MakeLeftBody(const FString& PlayerName);
	/** { "playerName": "Bob" } (back, M6-Q15) */
	static FString MakeBackBody(const FString& PlayerName) { return MakeLeftBody(PlayerName); }
	/** {} (started, empty) */
	static FString MakeEmptyBody() { return TEXT("{}"); }

	enum class EQuit : uint8 { None, Empty };
	/**
	 * R3.8 (M6-Q9: one match per server): a server launched by the manager quits only once empty after the match began;
	 * after the end, the players stay in the finished match as long as they want (2026-10-28: no quit N s after the end).
	 */
	static EQuit ShouldQuit(bool bManaged, bool bStarted, int32 PlayersLeft);
};

/**
 * The match server's link to the lobby manager (R3.6, R3.7): active only with -ManagerUrl= and -MatchId= on the command
 * line (the manager's launch); without them (solo, tests, net_match.ps1) nothing is sent. Each call is asynchronous,
 * logged with its answer, and never holds the match. Started, finished and empty are sent once each; empty is the last
 * call: it leaves once every call before it (the players' "left") is answered, or hol.Net.QuitEmptyWait s after it was
 * asked (two calls in flight at once reached the manager in either order, quick_check's "empty once, last" red on
 * 2026-10-28).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLManagerLink : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	bool IsActive() const { return !ManagerUrl.IsEmpty() && !MatchId.IsEmpty(); }

	void SendListening(int32 Port);
	void SendStarted();
	void SendFinished(TConstArrayView<int32> Winners);
	void SendEmpty();
	/** M6-Q14: a named player left the server (after "started"); each time. */
	void SendLeft(const FString& PlayerName);
	/** M6-Q15: a named player back on the server within R7's window (his absent slot taken again); each time. */
	void SendBack(const FString& PlayerName);
	bool HasStarted() const { return bStartedSent; }

	/** POST {ManagerUrl}/matches/{MatchId}/{Event} with this JSON body. */
	void Send(const FString& Event, const FString& JsonBody);

private:
	/**
	 * R3.8: quits the server by FHOLManagerRules::ShouldQuit; "empty" leaves first: asked by the game mode (the frame after
	 * the last Logout), sent once the calls before it are answered, then answered; each wait at most hol.Net.QuitEmptyWait s.
	 */
	bool TickQuit(float DeltaTime);
	/** Sends the "empty" asked once no other call is in flight (or hol.Net.QuitEmptyWait s after it was asked). */
	void TrySendEmpty(double Now);

	FString ManagerUrl;
	FString MatchId;
	bool bStartedSent = false;
	bool bFinishedSent = false;
	/** SendEmpty was called; bEmptySent once the request left. */
	bool bEmptyWanted = false;
	bool bEmptySent = false;
	bool bEmptyAnswered = false;
	bool bQuitting = false;
	double EmptyWantedTime = -1.0;
	double EmptySentTime = -1.0;
	/** When TickQuit first found the server empty after the start (the game mode's "empty" may not be asked yet). */
	double EmptySeenTime = -1.0;
	/** Calls sent and not answered yet (failed ones count as answered). */
	int32 PendingSends = 0;
	FTSTicker::FDelegateHandle QuitTicker;
};
