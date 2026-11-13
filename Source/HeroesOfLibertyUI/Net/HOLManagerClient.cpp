// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLManagerClient.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "GameFramework/GameStateBase.h"
#include "Match/HOLDisconnectRules.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HOLVersion.h"
#include "Net/HOLAccessKey.h"
#include "HeroesOfLibertyLog.h"
#include "HttpModule.h"
#include "Net/HOLManagerErrorText.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

static TAutoConsoleVariable<float> CVarReconnectInterval(TEXT("hol.Net.ReconnectInterval"), static_cast<float>(FHOLReconnectRules::DefaultIntervalSeconds),
	TEXT("T7 (D721): seconds between two automatic attempts to rejoin a match whose connection was lost."));
static TAutoConsoleVariable<float> CVarReconnectWindow(TEXT("hol.Net.ReconnectWindow"), static_cast<float>(FHOLDisconnectRules::DefaultWindowSeconds),
	TEXT("T7 (D721): seconds the client keeps retrying (the server's window, hol.Match.DisconnectWindowSeconds)."));

// T7 check: the connection of a client in a match is lost as the engine reports a real loss (the same ConnectionLost failure a timeout
// raises: the engine's own handling closes the connection, the server sees a Logout, our handler starts the automatic return).
static FAutoConsoleCommand CommandFakeLoss(TEXT("hol.Net.FakeLoss"),
	TEXT("hol.Net.FakeLoss: on a client in a match, raises ConnectionLost on its connection (the seat is then retried by itself, T7)"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		if (!GEngine)
		{
			return;
		}
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			UWorld* World = Context.World();
			if (Context.WorldType == EWorldType::Game && World && World->GetNetMode() == NM_Client && World->GetNetDriver())
			{
				UE_LOG(LogHOLNet, Display, TEXT("hol.Net.FakeLoss: ConnectionLost raised on the client's connection"));
				GEngine->BroadcastNetworkFailure(World, World->GetNetDriver(), ENetworkFailure::ConnectionLost, TEXT("hol.Net.FakeLoss"));
				return;
			}
		}
		UE_LOG(LogHOLNet, Warning, TEXT("hol.Net.FakeLoss: not a client in a match"));
	}));

namespace
{
	const TCHAR* KindName(const FHOLManagerError::EKind Kind)
	{
		switch (Kind)
		{
		case FHOLManagerError::EKind::Unreachable: return TEXT("unreachable");
		case FHOLManagerError::EKind::Unreadable: return TEXT("unreadable answer");
		case FHOLManagerError::EKind::Protocol: return TEXT("error");
		default: return TEXT("none");
		}
	}

	FString Describe(const FHOLManagerError& Error)
	{
		return FString::Printf(TEXT("%s (HTTP %d) %s %s"), KindName(Error.Kind), Error.HttpCode, *Error.Code, *Error.Message);
	}

	const TCHAR* StateName(const EHOLLobbyState State)
	{
		return State == EHOLLobbyState::Lobby ? TEXT("lobby") : State == EHOLLobbyState::Starting ? TEXT("starting") : TEXT("in_game");
	}

	UHOLManagerClient* FindClient(const UWorld* World)
	{
		const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		return GameInstance ? GameInstance->GetSubsystem<UHOLManagerClient>() : nullptr;
	}

	void LogStatus(const TArray<FString>&, UWorld* World)
	{
		UHOLManagerClient* Client = FindClient(World);
		if (!Client)
		{
			UE_LOG(LogHOLNet, Warning, TEXT("Manager status: no client"));
			return;
		}
		Client->GetStatus(FHOLManagerStatusDone::CreateLambda([](const FHOLManagerStatus& Status, const FHOLManagerError& Error)
		{
			if (Error.IsError())
			{
				UE_LOG(LogHOLNet, Warning, TEXT("Manager status: %s"), *Describe(Error));
				return;
			}
			UE_LOG(LogHOLNet, Log, TEXT("Manager status: protocol %d, game %s, %d lobbies (%s)"), Status.ProtocolVersion,
				*Status.GameVersion, Status.Lobbies, UHOLManagerClient::IsSameVersion(Status) ? TEXT("same version") : TEXT("other version"));
		}));
	}

	void LogLobbies(const TArray<FString>&, UWorld* World)
	{
		UHOLManagerClient* Client = FindClient(World);
		if (!Client)
		{
			UE_LOG(LogHOLNet, Warning, TEXT("Manager lobbies: no client"));
			return;
		}
		Client->GetLobbies(FHOLManagerLobbiesDone::CreateLambda([](const TArray<FHOLLobbySummary>& Lobbies, const FHOLManagerError& Error)
		{
			if (Error.IsError())
			{
				UE_LOG(LogHOLNet, Warning, TEXT("Manager lobbies: %s"), *Describe(Error));
				return;
			}
			UE_LOG(LogHOLNet, Log, TEXT("Manager lobbies: %d"), Lobbies.Num());
			for (const FHOLLobbySummary& Lobby : Lobbies)
			{
				UE_LOG(LogHOLNet, Log, TEXT("  %s \"%s\" %d/%d %s minutes %d join %d watch %d"), *Lobby.Id, *Lobby.Name, Lobby.Players,
					Lobby.MaxPlayers, StateName(Lobby.State), Lobby.Minutes, Lobby.bCanJoin, Lobby.bCanWatch);
			}
		}));
	}

	FAutoConsoleCommandWithWorldAndArgs StatusCommand(TEXT("hol.Manager.Status"),
		TEXT("hol.Manager.Status: asks the lobby manager of servers.json for its status (R4.1), logged in LogHOLNet"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&LogStatus));
	FAutoConsoleCommandWithWorldAndArgs LobbiesCommand(TEXT("hol.Manager.Lobbies"),
		TEXT("hol.Manager.Lobbies: asks the lobby manager of servers.json for its lobbies (R4.1), logged in LogHOLNet"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&LogLobbies));
}

bool UHOLManagerClient::ShouldCreateSubsystem(UObject* Outer) const
{
	return !IsRunningDedicatedServer() && Super::ShouldCreateSubsystem(Outer);
}

FString UHOLManagerClient::GetServersFilePath()
{
	FString Path;
	if (FParse::Value(FCommandLine::Get(), TEXT("-ServersFile="), Path))
	{
		return Path;
	}
#if WITH_EDITOR
	return FPaths::Combine(FPaths::ProjectDir(), TEXT("servers.json"));
#else
	return FPaths::Combine(FPaths::RootDir(), TEXT("servers.json"));
#endif
}

void UHOLManagerClient::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	const FString Path = GetServersFilePath();
	FString Text;
	if (FFileHelper::LoadFileToString(Text, *Path) && !FHOLManagerClientRules::ParseServers(Text, Servers))
	{
		UE_LOG(LogHOLNet, Warning, TEXT("Manager client: %s unreadable, the default manager is used"), *Path);
	}
	if (Servers.IsEmpty())
	{
		Servers.Add({ TEXT("Local"), FHOLManagerClientRules::DefaultUrl });
	}
	// The match servers of this manager want its key at login (UHOLLocalPlayer::GetGameLoginOptions).
	HOLAccessKey::SetClientKey(Servers[0].Key);
	// The address stays out of the screens (M6-Q5), not out of the log.
	UE_LOG(LogHOLNet, Log, TEXT("Manager client: %d server(s), first %s (%s)"), Servers.Num(), *Servers[0].Name,
		Servers[0].Key.IsEmpty() ? TEXT("no access key") : TEXT("access key set"));
	if (GEngine)
	{
		NetworkFailureHandle = GEngine->OnNetworkFailure().AddUObject(this, &UHOLManagerClient::HandleNetworkFailure);
		TravelFailureHandle = GEngine->OnTravelFailure().AddUObject(this, &UHOLManagerClient::HandleTravelFailure);
	}
}

void UHOLManagerClient::Deinitialize()
{
	EndReconnect();
	if (GEngine)
	{
		GEngine->OnNetworkFailure().Remove(NetworkFailureHandle);
		GEngine->OnTravelFailure().Remove(TravelFailureHandle);
	}
	Super::Deinitialize();
}

void UHOLManagerClient::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, const ENetworkFailure::Type Failure, const FString& Message)
{
	if (bInMatch && World && World->GetGameInstance() == GetGameInstance())
	{
		// T7: the failures of the attempts themselves are the retries' business.
		if (Reconnect.IsActive())
		{
			UE_LOG(LogHOLNet, Log, TEXT("Manager: reconnect attempt failed (%s)"), *Message);
			return;
		}
		const FHOLManagerError Error = MakeMatchLossError(Failure, Message);
		if ((Failure == ENetworkFailure::ConnectionLost || Failure == ENetworkFailure::ConnectionTimeout) && StartReconnect(Error))
		{
			return;
		}
		LoseMatch(Error);
	}
}

void UHOLManagerClient::HandleTravelFailure(UWorld* World, const ETravelFailure::Type Failure, const FString& Message)
{
	if (Reconnect.IsActive())
	{
		return;
	}
	if (bInMatch && (!World || World->GetGameInstance() == GetGameInstance()))
	{
		LoseMatch(MakeMatchTravelError(FString::Printf(TEXT("%s: %s"), ETravelFailure::ToString(Failure), *Message)));
	}
}

bool UHOLManagerClient::StartReconnect(const FHOLManagerError& Error)
{
	// A spectator (no name) has no seat to take back.
	if (MatchPlayerName.IsEmpty() || ReconnectAddress.IsEmpty() || CVarReconnectWindow.GetValueOnGameThread() <= 0.f || AfterMatch != EAfterMatch::None)
	{
		return false;
	}
	ReconnectError = Error;
	Reconnect.Start(FPlatformTime::Seconds(), CVarReconnectWindow.GetValueOnGameThread(), FMath::Max(0.5f, CVarReconnectInterval.GetValueOnGameThread()));
	UE_LOG(LogHOLNet, Display, TEXT("Manager: connection lost (%s), retrying the seat for %.0f s"), *Error.Message, CVarReconnectWindow.GetValueOnGameThread());
	ShowReconnectBanner();
	ReconnectTicker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UHOLManagerClient::TickReconnect), 0.5f);
	return true;
}

bool UHOLManagerClient::TickReconnect(float)
{
	const double Now = FPlatformTime::Seconds();
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	// Back in the match: a new connection, open, with its game state, after at least one attempt.
	const UNetDriver* Driver = World ? World->GetNetDriver() : nullptr;
	if (Reconnect.GetAttempts() > 0 && World && World->GetNetMode() == NM_Client && Driver && Driver->ServerConnection
		&& Driver->ServerConnection->GetConnectionState() == USOCK_Open && World->GetGameState())
	{
		UE_LOG(LogHOLNet, Display, TEXT("Manager: back in the match after %d attempt(s)"), Reconnect.GetAttempts());
		EndReconnect();
		return false;
	}
	switch (Reconnect.Tick(Now))
	{
	case FHOLReconnectRules::EStep::Attempt:
		UE_LOG(LogHOLNet, Display, TEXT("Manager: reconnect attempt %d"), Reconnect.GetAttempts());
		if (World && GEngine)
		{
			GEngine->SetClientTravel(World, *ReconnectAddress, TRAVEL_Absolute);
		}
		break;
	case FHOLReconnectRules::EStep::GiveUp:
	{
		UE_LOG(LogHOLNet, Display, TEXT("Manager: the window is over, the seat is not taken back"));
		const FHOLManagerError Error = ReconnectError;
		EndReconnect();
		LoseMatch(Error);
		return false;
	}
	default:
		break;
	}
	if (ReconnectText.IsValid())
	{
		ReconnectText->SetText(FHOLReconnectRules::FormatLine(Reconnect.GetSecondsLeft(Now)));
	}
	return true;
}

void UHOLManagerClient::CancelReconnect()
{
	if (!Reconnect.IsActive())
	{
		return;
	}
	UE_LOG(LogHOLNet, Display, TEXT("Manager: reconnection cancelled"));
	const FHOLManagerError Error = ReconnectError;
	EndReconnect();
	LoseMatch(Error);
}

void UHOLManagerClient::EndReconnect()
{
	Reconnect.Cancel();
	if (ReconnectTicker.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(ReconnectTicker);
		ReconnectTicker.Reset();
	}
	HideReconnectBanner();
}

void UHOLManagerClient::ShowReconnectBanner()
{
	if (!GEngine || !GEngine->GameViewport)
	{
		return;
	}
	HideReconnectBanner();
	const FSlateFontInfo Font = FCoreStyle::GetDefaultFontStyle("Bold", 22);
	ReconnectWidget = SNew(SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(FMargin(0.f, 80.f, 0.f, 0.f))
		[
			SNew(SBorder)
			.Padding(FMargin(20.f, 12.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SAssignNew(ReconnectText, STextBlock)
					.Font(Font)
					.Text(FHOLReconnectRules::FormatLine(Reconnect.GetSecondsLeft(FPlatformTime::Seconds())))
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(24.f, 0.f, 0.f, 0.f))
				[
					SNew(SButton)
					.OnClicked(FOnClicked::CreateWeakLambda(this, [this]() { CancelReconnect(); return FReply::Handled(); }))
					[
						SNew(STextBlock).Font(Font).Text(NSLOCTEXT("HOLNet", "ReconnectCancel", "Annuler"))
					]
				]
			]
		];
	GEngine->GameViewport->AddViewportWidgetContent(ReconnectWidget.ToSharedRef(), 1000);
}

void UHOLManagerClient::HideReconnectBanner()
{
	if (ReconnectWidget.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(ReconnectWidget.ToSharedRef());
	}
	ReconnectWidget.Reset();
	ReconnectText.Reset();
}

void UHOLManagerClient::LoseMatch(const FHOLManagerError& Error)
{
	bInMatch = false;
	// The match was already over and left by its end banner (R4.8): its choice stands.
	if (AfterMatch != EAfterMatch::None)
	{
		UE_LOG(LogHOLNet, Display, TEXT("Manager: match left (%s)"), *Error.Message);
		return;
	}
	UE_LOG(LogHOLNet, Display, TEXT("Manager: match lost (%s: %s)"), *Error.Code, *Error.Message);
	// The engine goes back to the menu's map by itself; its player controller opens the list (R4.9).
	ForgetLobby();
	SetLobbyLoss(Error);
	AfterMatch = EAfterMatch::List;
}

bool UHOLManagerClient::IsSameVersion(const FHOLManagerStatus& Status)
{
	return Status.GameVersion == HOLVersion::Get();
}

void UHOLManagerClient::Get(const FString& Route, TFunction<void(const FHOLManagerError&, const FString&)> Done)
{
	Send(TEXT("GET"), Route, FString(), FString(), MoveTemp(Done));
}

void UHOLManagerClient::Send(const FString& Verb, const FString& Route, const FString& Body, const FString& Token,
	TFunction<void(const FHOLManagerError&, const FString&)> Done)
{
	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetVerb(Verb);
	Request->SetURL(FHOLManagerClientRules::MakeUrl(GetUrl(), Route));
	if (!Body.IsEmpty())
	{
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
		Request->SetContentAsString(Body);
	}
	if (!Token.IsEmpty())
	{
		Request->SetHeader(TEXT("X-Player-Token"), Token);
	}
	// Launcher L11: a wrong or missing key answers 401 bad_key (GetManagerErrorText).
	if (!Servers[0].Key.IsEmpty())
	{
		Request->SetHeader(HOLAccessKey::Header, Servers[0].Key);
	}
	Request->SetTimeout(5.f);
	Request->OnProcessRequestComplete().BindWeakLambda(this, [Done = MoveTemp(Done)](FHttpRequestPtr, const FHttpResponsePtr Response, const bool bConnected)
	{
		const bool bAnswered = bConnected && Response.IsValid();
		const FString Body = bAnswered ? Response->GetContentAsString() : FString();
		Done(FHOLManagerClientRules::ReadFailure(bAnswered, bAnswered ? Response->GetResponseCode() : 0, Body), Body);
	});
	Request->ProcessRequest();
}

void UHOLManagerClient::GetStatus(FHOLManagerStatusDone Done)
{
	Get(TEXT("/status"), [Done = MoveTemp(Done)](const FHOLManagerError& Failure, const FString& Body)
	{
		FHOLManagerError Error = Failure;
		FHOLManagerStatus Status;
		if (!Error.IsError() && !FHOLManagerClientRules::ParseStatus(Body, Status))
		{
			Error.Kind = FHOLManagerError::EKind::Unreadable;
			Error.Message = Body.Left(200);
		}
		Done.ExecuteIfBound(Status, Error);
	});
}

void UHOLManagerClient::GetLobbies(FHOLManagerLobbiesDone Done)
{
	Get(TEXT("/lobbies"), [Done = MoveTemp(Done)](const FHOLManagerError& Failure, const FString& Body)
	{
		FHOLManagerError Error = Failure;
		TArray<FHOLLobbySummary> Lobbies;
		if (!Error.IsError() && !FHOLManagerClientRules::ParseLobbies(Body, Lobbies))
		{
			Error.Kind = FHOLManagerError::EKind::Unreadable;
			Error.Message = Body.Left(200);
		}
		Done.ExecuteIfBound(Lobbies, Error);
	});
}

namespace
{
	FString MakeBody(const TArray<TPair<FString, FString>>& Fields)
	{
		const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		for (const TPair<FString, FString>& Field : Fields)
		{
			Object->SetStringField(Field.Key, Field.Value);
		}
		FString Body;
		const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
		FJsonSerializer::Serialize(Object, Writer);
		return Body;
	}
}

void UHOLManagerClient::Enter(const FString& Route, const FString& Body, FHOLManagerLobbyDone Done)
{
	Send(TEXT("POST"), Route, Body, FString(), [WeakThis = TWeakObjectPtr<UHOLManagerClient>(this), Done = MoveTemp(Done)](const FHOLManagerError& Failure, const FString& Answer)
	{
		FHOLManagerError Error = Failure;
		FHOLLobby Lobby;
		FString Token;
		int32 SlotIndex = INDEX_NONE;
		if (!Error.IsError() && !FHOLManagerClientRules::ParseJoin(Answer, Token, SlotIndex, Lobby))
		{
			Error.Kind = FHOLManagerError::EKind::Unreadable;
			Error.Message = Answer.Left(200);
		}
		if (Error.IsError())
		{
			UE_LOG(LogHOLNet, Warning, TEXT("Manager: lobby not entered: %s"), *Describe(Error));
		}
		else if (WeakThis.IsValid())
		{
			WeakThis->LobbyId = Lobby.Id;
			WeakThis->LobbyToken = Token;
			WeakThis->bLobbySpectator = Lobby.IsYouSpectator();
			WeakThis->bRejoinedLobby = false;
			WeakThis->LobbyLoss = FHOLManagerError();
			UE_LOG(LogHOLNet, Log, TEXT("Manager: in lobby %s as %s, slot %d"), *Lobby.Id, *Lobby.You, SlotIndex);
		}
		Done.ExecuteIfBound(Lobby, Error);
	});
}

void UHOLManagerClient::CreateLobby(const FString& LobbyName, const FString& PlayerName, FHOLManagerLobbyDone Done)
{
	Enter(TEXT("/lobbies"), MakeBody({ { TEXT("name"), LobbyName }, { TEXT("playerName"), PlayerName }, { TEXT("gameVersion"), HOLVersion::Get() } }),
		MoveTemp(Done));
}

void UHOLManagerClient::JoinLobby(const FString& InLobbyId, const FString& PlayerName, FHOLManagerLobbyDone Done, const bool bSpectator)
{
	Enter(FString::Printf(TEXT("/lobbies/%s/join"), *InLobbyId),
		FHOLManagerClientRules::MakeJoinBody(PlayerName, HOLVersion::Get(), bSpectator), MoveTemp(Done));
}

void UHOLManagerClient::ReadLobby(FHOLManagerLobbyDone Done)
{
	if (!IsInLobby())
	{
		FHOLManagerError Error;
		Error.Kind = FHOLManagerError::EKind::Protocol;
		Error.Code = TEXT("not_in_lobby");
		Done.ExecuteIfBound(FHOLLobby(), Error);
		return;
	}
	const FString Token = LobbyToken;
	Send(TEXT("GET"), FString::Printf(TEXT("/lobbies/%s"), *LobbyId), FString(), Token,
		[WeakThis = TWeakObjectPtr<UHOLManagerClient>(this), Token, Done = MoveTemp(Done)](const FHOLManagerError& Failure, const FString& Answer)
	{
		FHOLManagerError Error = Failure;
		FHOLLobby Lobby;
		if (!Error.IsError() && !FHOLManagerClientRules::ParseLobby(Answer, Lobby))
		{
			Error.Kind = FHOLManagerError::EKind::Unreadable;
			Error.Message = Answer.Left(200);
		}
		// SP1: what I am in the lobby (spectator or player), for the way back after the match.
		if (!Error.IsError() && WeakThis.IsValid() && WeakThis->LobbyToken == Token)
		{
			WeakThis->bLobbySpectator = Lobby.IsYouSpectator();
		}
		// Out of the lobby: this token is worth nothing any more (unless a newer one came meanwhile).
		if ((Error.Code == TEXT("not_in_lobby") || Error.Code == TEXT("lobby_not_found")) && WeakThis.IsValid() && WeakThis->LobbyToken == Token)
		{
			WeakThis->LobbyToken.Reset();
		}
		Done.ExecuteIfBound(Lobby, Error);
	});
}

void UHOLManagerClient::Write(const FString& Route, const FString& Body, FHOLManagerLobbyDone Done)
{
	if (!IsInLobby())
	{
		FHOLManagerError Error;
		Error.Kind = FHOLManagerError::EKind::Protocol;
		Error.Code = TEXT("not_in_lobby");
		Done.ExecuteIfBound(FHOLLobby(), Error);
		return;
	}
	Send(TEXT("PUT"), FString::Printf(TEXT("/lobbies/%s%s"), *LobbyId, *Route), Body, LobbyToken,
		[Route, Body, Done = MoveTemp(Done)](const FHOLManagerError& Failure, const FString& Answer)
	{
		FHOLManagerError Error = Failure;
		FHOLLobby Lobby;
		if (!Error.IsError() && !FHOLManagerClientRules::ParseLobby(Answer, Lobby))
		{
			Error.Kind = FHOLManagerError::EKind::Unreadable;
			Error.Message = Answer.Left(200);
		}
		if (Error.IsError())
		{
			UE_LOG(LogHOLNet, Warning, TEXT("Manager: PUT %s %s refused: %s"), *Route, *Body, *Describe(Error));
		}
		Done.ExecuteIfBound(Lobby, Error);
	});
}

void UHOLManagerClient::UpdateMe(const TOptional<int32> ColorIndex, const TOptional<int32> Team, const TOptional<bool> bReady, FHOLManagerLobbyDone Done,
	const TOptional<bool> bSpectator)
{
	Write(TEXT("/me"), FHOLManagerClientRules::MakeMeBody(ColorIndex, Team, bReady, bSpectator), MoveTemp(Done));
}

void UHOLManagerClient::UpdateSettings(const FString& Map, const bool bNomad, const bool bRegicide, const bool bCatastrophe, FHOLManagerLobbyDone Done,
	const FString& Size, const TOptional<bool> bTeamsTogether, const FString& Fog, const FString& Mode, const FString& Biome,
	const TOptional<bool> bSacredSites)
{
	Write(TEXT("/settings"), FHOLManagerClientRules::MakeSettingsBody(Map, bNomad, bRegicide, bCatastrophe, Size, bTeamsTogether, Fog, Mode, Biome,
		bSacredSites), MoveTemp(Done));
}

void UHOLManagerClient::SetSlot(const int32 SlotIndex, const bool bOpen, FHOLManagerLobbyDone Done)
{
	Write(FString::Printf(TEXT("/slots/%d"), SlotIndex), FHOLManagerClientRules::MakeSlotBody(bOpen), MoveTemp(Done));
}

void UHOLManagerClient::SetSlotAI(const int32 SlotIndex, const TOptional<int32> ColorIndex, const TOptional<int32> Team, FHOLManagerLobbyDone Done)
{
	Write(FString::Printf(TEXT("/slots/%d"), SlotIndex), FHOLManagerClientRules::MakeAISlotBody(ColorIndex, Team), MoveTemp(Done));
}

void UHOLManagerClient::StartMatch(FHOLManagerStartDone Done)
{
	if (!IsInLobby())
	{
		FHOLManagerError Error;
		Error.Kind = FHOLManagerError::EKind::Protocol;
		Error.Code = TEXT("not_in_lobby");
		Done.ExecuteIfBound(FString(), Error);
		return;
	}
	Send(TEXT("POST"), FString::Printf(TEXT("/lobbies/%s/start"), *LobbyId), FString(), LobbyToken,
		[Done = MoveTemp(Done)](const FHOLManagerError& Failure, const FString& Answer)
	{
		FString MatchId;
		TSharedPtr<FJsonObject> Object;
		if (!Failure.IsError() && FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Answer), Object) && Object.IsValid())
		{
			Object->TryGetStringField(TEXT("matchId"), MatchId);
		}
		Done.ExecuteIfBound(MatchId, Failure);
	});
}

void UHOLManagerClient::WatchLobby(const FString& InLobbyId, FHOLManagerWatchDone Done)
{
	Get(FString::Printf(TEXT("/lobbies/%s/watch"), *InLobbyId), [Done = MoveTemp(Done)](const FHOLManagerError& Failure, const FString& Answer)
	{
		FString MatchId;
		int32 Port = 0;
		FHOLManagerError Error = Failure;
		if (!Error.IsError() && !FHOLManagerClientRules::ParseWatch(Answer, MatchId, Port))
		{
			Error.Kind = FHOLManagerError::EKind::Unreadable;
			Error.Message = TEXT("watch answer without a port");
		}
		Done.ExecuteIfBound(MatchId, Port, Error);
	});
}

bool UHOLManagerClient::OpenMatch(const int32 Port, const FString& PlayerName, const FString& GameVersion)
{
	const FString Address = FHOLManagerClientRules::MakeMatchAddress(GetUrl(), Port, PlayerName, GameVersion);
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (Address.IsEmpty() || !World || !GEngine)
	{
		UE_LOG(LogHOLNet, Warning, TEXT("Manager: no address for port %d from %s"), Port, *GetUrl());
		return false;
	}
	UE_LOG(LogHOLNet, Display, TEXT("Manager: open %s"), *Address);
	MatchPlayerName = PlayerName;
	ReconnectAddress = Address;
	bInMatch = true;
	// As the console's "open".
	GEngine->SetClientTravel(World, *Address, TRAVEL_Absolute);
	return true;
}

void UHOLManagerClient::KeepRejoinedLobby(const FString& InLobbyId)
{
	UE_LOG(LogHOLNet, Display, TEXT("Manager: back in the match of lobby %s, no token"), *InLobbyId);
	LobbyId = InLobbyId;
	LobbyToken.Reset();
	bRejoinedLobby = true;
}

void UHOLManagerClient::ReturnToLobby(FHOLManagerLobbyDone Done)
{
	const FString Id = LobbyId;
	const FString Name = MatchPlayerName;
	MatchPlayerName.Reset();
	bRejoinedLobby = false;
	UE_LOG(LogHOLNet, Display, TEXT("Manager: back to lobby %s as %s%s"), *Id, *Name, bLobbySpectator ? TEXT(" (spectator)") : TEXT(""));
	JoinLobby(Id, Name, MoveTemp(Done), bLobbySpectator);
}

void UHOLManagerClient::ForgetLobby()
{
	UE_LOG(LogHOLNet, Display, TEXT("Manager: lobby %s forgotten"), *LobbyId);
	LobbyToken.Reset();
	bRejoinedLobby = false;
	MatchPlayerName.Reset();
}

void UHOLManagerClient::LeaveLobby()
{
	bRejoinedLobby = false;
	if (!IsInLobby())
	{
		return;
	}
	const FString Id = LobbyId;
	Send(TEXT("POST"), FString::Printf(TEXT("/lobbies/%s/leave"), *Id), FString(), LobbyToken, [Id](const FHOLManagerError& Error, const FString&)
	{
		if (Error.IsError())
		{
			UE_LOG(LogHOLNet, Warning, TEXT("Manager: leaving lobby %s: %s"), *Id, *Describe(Error));
		}
		else
		{
			UE_LOG(LogHOLNet, Log, TEXT("Manager: left lobby %s"), *Id);
		}
	});
	LobbyToken.Reset();
}
