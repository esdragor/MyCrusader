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
	// R3.8: seconds after the match's end before the server quits with players left (R4.8 sends them to the lobby).
	float QuitAfterFinished = 120.f;
	FAutoConsoleVariableRef CVarQuitAfterFinished(TEXT("hol.Net.QuitAfterFinished"), QuitAfterFinished,
		TEXT("R3.8: seconds after the match's end before a server launched by the manager quits"));
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

FHOLManagerRules::EQuit FHOLManagerRules::ShouldQuit(const bool bManaged, const bool bStarted, const int32 PlayersLeft,
	const double SecondsSinceFinished, const double FinishedDelay)
{
	if (!bManaged || !bStarted)
	{
		return EQuit::None;
	}
	if (PlayersLeft <= 0)
	{
		return EQuit::Empty;
	}
	return SecondsSinceFinished >= 0.0 && SecondsSinceFinished >= FinishedDelay ? EQuit::Finished : EQuit::None;
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
	const FHOLManagerRules::EQuit Quit = FHOLManagerRules::ShouldQuit(IsActive(), bStartedSent, World->GetNumPlayerControllers(),
		FinishedTime < 0.0 ? -1.0 : Now - FinishedTime, QuitAfterFinished);
	if (Quit == FHOLManagerRules::EQuit::None
		|| (Quit == FHOLManagerRules::EQuit::Empty && bEmptySent && !bEmptyAnswered && Now - EmptySentTime < QuitEmptyWait))
	{
		return true;
	}
	bQuitting = true;
	UE_LOG(LogHOLNet, Log, TEXT("Server quits: %s"), Quit == FHOLManagerRules::EQuit::Empty
		? TEXT("empty after the match") : *FString::Printf(TEXT("%.0f s after the end of the match"), QuitAfterFinished));
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
		FinishedTime = FPlatformTime::Seconds();
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
	if (!bEmptySent)
	{
		bEmptySent = true;
		EmptySentTime = FPlatformTime::Seconds();
		Send(TEXT("empty"), FHOLManagerRules::MakeEmptyBody());
	}
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
		bEmptyAnswered |= Event == TEXT("empty");
		if (bConnected && Response.IsValid())
		{
			UE_LOG(LogHOLNet, Log, TEXT("Manager: %s %s sent -> %d"), *Event, *JsonBody, Response->GetResponseCode());
		}
		else
		{
			UE_LOG(LogHOLNet, Warning, TEXT("Manager: %s %s failed (no answer)"), *Event, *JsonBody);
		}
	});
	Request->ProcessRequest();
}
