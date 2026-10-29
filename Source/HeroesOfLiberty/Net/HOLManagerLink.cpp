// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLManagerLink.h"

#include "HeroesOfLibertyLog.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Misc/CommandLine.h"

namespace
{
	// R3.8: the longest wait for the manager's answer to "empty" before quitting.
	float QuitEmptyWait = 2.f;
	FAutoConsoleVariableRef CVarQuitEmptyWait(TEXT("hol.Net.QuitEmptyWait"), QuitEmptyWait,
		TEXT("R3.8: seconds a server waits for the answer to empty before quitting"));
}

FString FHOLManagerRules::MakeRouteUrl(const FString& ManagerUrl, const FString& MatchId, const FString& Event)
{
	FString Base = ManagerUrl;
	Base.RemoveFromEnd(TEXT("/"));
	return FString::Printf(TEXT("%s/matches/%s/%s"), *Base, *MatchId, *Event);
}

FString FHOLManagerRules::MakeListeningBody(const int32 Port)
{
	return FString::Printf(TEXT("{\"port\":%d}"), Port);
}

FString FHOLManagerRules::MakeFinishedBody(const TConstArrayView<int32> Winners)
{
	return FString::Printf(TEXT("{\"winners\":[%s]}"),
		*FString::JoinBy(Winners, TEXT(","), [](const int32 Winner) { return FString::FromInt(Winner); }));
}

FString FHOLManagerRules::MakeLeftBody(const FString& PlayerName)
{
	return FString::Printf(TEXT("{\"playerName\":\"%s\"}"), *PlayerName.ReplaceCharWithEscapedChar());
}

FHOLManagerRules::EQuit FHOLManagerRules::ShouldQuit(const bool bManaged, const bool bStarted, const int32 PlayersLeft)
{
	return bManaged && bStarted && PlayersLeft <= 0 ? EQuit::Empty : EQuit::None;
}

void UHOLManagerLink::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FParse::Value(FCommandLine::Get(), TEXT("-ManagerUrl="), ManagerUrl);
	FParse::Value(FCommandLine::Get(), TEXT("-MatchId="), MatchId);
	if (IsActive())
	{
		UE_LOG(LogHOLNet, Log, TEXT("Manager: %s, match %s"), *ManagerUrl, *MatchId);
		QuitTicker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UHOLManagerLink::TickQuit), 0.25f);
	}
}

void UHOLManagerLink::Deinitialize()
{
	FTSTicker::RemoveTicker(QuitTicker);
	Super::Deinitialize();
}

bool UHOLManagerLink::TickQuit(float)
{
	const UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (bQuitting || !World)
	{
		return true;
	}
	const double Now = FPlatformTime::Seconds();
	const FHOLManagerRules::EQuit Quit = FHOLManagerRules::ShouldQuit(IsActive(), bStartedSent, World->GetNumPlayerControllers());
	if (Quit == FHOLManagerRules::EQuit::None)
	{
		EmptySeenTime = -1.0;
		return true;
	}
	// "empty" last and before the quit (quick_check's "manager: empty once, last", 2026-10-28): the game mode asks it the
	// frame after the last Logout (this ticker may run in between: waited for), it leaves once the "left" calls before it
	// are answered (TrySendEmpty), and the quit waits for its answer; each wait at most hol.Net.QuitEmptyWait s.
	if (EmptySeenTime < 0.0)
	{
		EmptySeenTime = Now;
	}
	TrySendEmpty(Now);
	if ((!bEmptySent && Now - EmptySeenTime < QuitEmptyWait) || (bEmptySent && !bEmptyAnswered && Now - EmptySentTime < QuitEmptyWait))
	{
		return true;
	}
	bQuitting = true;
	UE_LOG(LogHOLNet, Log, TEXT("Server quits: empty after the match"));
	RequestEngineExit(TEXT("HOLManagerLink R3.8"));
	return true;
}

void UHOLManagerLink::SendListening(const int32 Port)
{
	Send(TEXT("listening"), FHOLManagerRules::MakeListeningBody(Port));
}

void UHOLManagerLink::SendStarted()
{
	if (!bStartedSent)
	{
		bStartedSent = true;
		Send(TEXT("started"), FHOLManagerRules::MakeEmptyBody());
	}
}

void UHOLManagerLink::SendFinished(const TConstArrayView<int32> Winners)
{
	if (!bFinishedSent)
	{
		bFinishedSent = true;
		Send(TEXT("finished"), FHOLManagerRules::MakeFinishedBody(Winners));
	}
}

void UHOLManagerLink::SendLeft(const FString& PlayerName)
{
	Send(TEXT("left"), FHOLManagerRules::MakeLeftBody(PlayerName));
}

void UHOLManagerLink::SendBack(const FString& PlayerName)
{
	Send(TEXT("back"), FHOLManagerRules::MakeBackBody(PlayerName));
}

void UHOLManagerLink::SendEmpty()
{
	if (!bEmptyWanted)
	{
		bEmptyWanted = true;
		EmptyWantedTime = FPlatformTime::Seconds();
		TrySendEmpty(EmptyWantedTime);
	}
}

void UHOLManagerLink::TrySendEmpty(const double Now)
{
	// The calls before it (the last players' "left", sent the same frame or the one before) answered first: two requests
	// in flight at once reach the manager in either order, and "empty" must be its last call (PROTOCOL.md).
	if (!bEmptyWanted || bEmptySent || (PendingSends > 0 && Now - EmptyWantedTime < QuitEmptyWait))
	{
		return;
	}
	if (PendingSends > 0)
	{
		UE_LOG(LogHOLNet, Warning, TEXT("Manager: empty sent with %d calls still unanswered after %.1f s"), PendingSends, Now - EmptyWantedTime);
	}
	bEmptySent = true;
	EmptySentTime = Now;
	Send(TEXT("empty"), FHOLManagerRules::MakeEmptyBody());
}

void UHOLManagerLink::Send(const FString& Event, const FString& JsonBody)
{
	if (!IsActive())
	{
		return;
	}
	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetVerb(TEXT("POST"));
	Request->SetURL(FHOLManagerRules::MakeRouteUrl(ManagerUrl, MatchId, Event));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
	Request->SetContentAsString(JsonBody);
	Request->SetTimeout(10.f);
	Request->OnProcessRequestComplete().BindWeakLambda(this, [this, Event, JsonBody](FHttpRequestPtr, const FHttpResponsePtr Response, const bool bConnected)
	{
		PendingSends = FMath::Max(0, PendingSends - 1);
		bEmptyAnswered |= Event == TEXT("empty");
		if (bConnected && Response.IsValid())
		{
			UE_LOG(LogHOLNet, Log, TEXT("Manager: %s %s sent -> %d"), *Event, *JsonBody, Response->GetResponseCode());
		}
		else
		{
			UE_LOG(LogHOLNet, Warning, TEXT("Manager: %s %s failed (no answer)"), *Event, *JsonBody);
		}
		// The "empty" waiting for this answer leaves now.
		TrySendEmpty(FPlatformTime::Seconds());
	});
	++PendingSends;
	Request->ProcessRequest();
}
