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
	/** {} (started, empty) */
	static FString MakeEmptyBody() { return TEXT("{}"); }

	enum class EQuit : uint8 { None, Empty, Finished };
	/**
	 * R3.8 (M6-Q9: one match per server): a server launched by the manager quits once empty after the match began, or
	 * FinishedDelay seconds after the match's end even with players left. SecondsSinceFinished < 0: not finished.
	 */
	static EQuit ShouldQuit(bool bManaged, bool bStarted, int32 PlayersLeft, double SecondsSinceFinished, double FinishedDelay);
};

/**
 * The match server's link to the lobby manager (R3.6, R3.7): active only with -ManagerUrl= and -MatchId= on the command
 * line (the manager's launch); without them (solo, tests, net_match.ps1) nothing is sent. Each call is asynchronous,
 * logged with its answer, and never holds the match. Started, finished and empty are sent once each.
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
	bool HasStarted() const { return bStartedSent; }

	/** POST {ManagerUrl}/matches/{MatchId}/{Event} with this JSON body. */
	void Send(const FString& Event, const FString& JsonBody);

private:
	/** R3.8: quits the server by FHOLManagerRules::ShouldQuit; "empty" leaves first (answered, or hol.Net.QuitEmptyWait s). */
	bool TickQuit(float DeltaTime);

	FString ManagerUrl;
	FString MatchId;
	bool bStartedSent = false;
	bool bFinishedSent = false;
	bool bEmptySent = false;
	bool bEmptyAnswered = false;
	bool bQuitting = false;
	double EmptySentTime = -1.0;
	double FinishedTime = -1.0;
	FTSTicker::FDelegateHandle QuitTicker;
};
