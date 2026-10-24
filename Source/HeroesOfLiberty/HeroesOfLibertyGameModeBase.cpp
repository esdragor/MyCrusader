// Copyright Epic Games, Inc. All Rights Reserved.

#include "HeroesOfLibertyGameModeBase.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Engine/GameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "HeroesOfLibertyLog.h"
#include "HOLVersion.h"
#include "Kismet/GameplayStatics.h"
#include "Map/HOLStartSlot.h"
#include "EngineUtils.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchSettings.h"
#include "Match/HOLMatchSubsystem.h"
#include "Match/HOLPlayerState.h"
#include "Net/HOLManagerLink.h"
#include "Engine/NetDriver.h"
#include "Player/HOLPlayerController.h"
#include "Match/HOLStartRules.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "TimerManager.h"
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<float> CVarStartWaitSeconds(TEXT("hol.Match.StartWaitSeconds"), 60.f,
	TEXT("R3.9: the longest wait of a match launched by the lobby manager for its named players, in seconds."));

AHeroesOfLibertyGameModeBase::AHeroesOfLibertyGameModeBase()
{
	PlayerStateClass = AHOLPlayerState::StaticClass();
	GameStateClass = AHOLGameState::StaticClass();
}

void AHeroesOfLibertyGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	const UHOLMatchSubsystem* Match = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHOLMatchSubsystem>() : nullptr;
	const bool bHasConfig = Match && Match->HasConfig();
	MatchConfig = bHasConfig ? Match->GetConfigOrDefault() : FHOLMatchConfig::MakeDefault();
	UE_LOG(LogHOLMatch, Log, TEXT("Match on %s: %s configuration, %d occupied slots"), *MapName,
		bHasConfig ? TEXT("given") : TEXT("default"), MatchConfig.GetNumOccupiedSlots());
	// The camps of the match are locked now, at the launch (D357).
	if (AHOLGameState* HOLGameState = GetGameState<AHOLGameState>())
	{
		HOLGameState->InitVictory(MatchConfig);
	}

	// Before any player logs in: the classes are read when each controller is made.
	if (AHOLStartSlot::WorldHasSlots(GetWorld()))
	{
		PlayerControllerClass = AHOLPlayerController::StaticClass();
		DefaultPawnClass = nullptr;
		UE_LOG(LogHOLMatch, Log, TEXT("Match map: RTS camera, no pawn"));
	}
}

void AHeroesOfLibertyGameModeBase::InitGameState()
{
	Super::InitGameState();
	// Whichever of InitGame and InitGameState comes last gives the game state the configuration of the launch.
	if (AHOLGameState* HOLGameState = GetGameState<AHOLGameState>())
	{
		HOLGameState->InitVictory(MatchConfig);
	}
}

void AHeroesOfLibertyGameModeBase::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId,
	FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	if (!ErrorMessage.IsEmpty())
	{
		return;
	}
	const FString ClientVersion = UGameplayStatics::ParseOption(Options, HOLVersion::UrlOption);
	if (!HOLVersion::IsAccepted(HOLVersion::Get(), ClientVersion, &ErrorMessage))
	{
		UE_LOG(LogHOLMatch, Warning, TEXT("Login refused from %s: %s"), *Address, *ErrorMessage);
	}
}

FString AHeroesOfLibertyGameModeBase::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
	const FString& Options, const FString& Portal)
{
	FString ErrorMessage = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
	// The engine keeps the raw option, cut at 20 characters: the decoded name instead (16 characters at most, PROTOCOL.md).
	if (UGameplayStatics::HasOption(Options, TEXT("Name")))
	{
		const FString Name = FGenericPlatformHttp::UrlDecode(UGameplayStatics::ParseOption(Options, TEXT("Name"))).TrimStartAndEnd();
		if (!Name.IsEmpty())
		{
			LoginNames.Add(NewPlayerController, Name);
			ChangeName(NewPlayerController, Name, false);
		}
	}
	return ErrorMessage;
}

void AHeroesOfLibertyGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AHOLPlayerState* PlayerState = NewPlayer ? NewPlayer->GetPlayerState<AHOLPlayerState>() : nullptr;
	if (!PlayerState)
	{
		return;
	}

	TArray<int32> TakenSlots;
	for (const APlayerState* Other : GameState->PlayerArray)
	{
		if (const AHOLPlayerState* HOLOther = Cast<AHOLPlayerState>(Other); HOLOther && HOLOther != PlayerState)
		{
			TakenSlots.Add(HOLOther->GetPlayerIndex());
		}
	}

	FString LoginName;
	LoginNames.RemoveAndCopyValue(NewPlayer, LoginName);
	const int32 Slot = MatchConfig.FindSlotForName(LoginName, TakenSlots);
	if (Slot == INDEX_NONE)
	{
		// M6-Q8: a name absent from the configuration (or held by a player still here), or no human slot left: it watches.
		PlayerState->SetIsSpectator(true);
		PlayerState->SetIsOnlyASpectator(true);
		UE_LOG(LogHOLMatch, Log, TEXT("%s joined as a spectator (name '%s', %s)"), *PlayerState->GetPlayerName(), *LoginName,
			MatchConfig.HasPlayerNames() ? TEXT("no free slot of that name") : TEXT("every human slot is taken"));
		return;
	}
	PlayerState->SetMatchSlot(Slot, MatchConfig.Slots[Slot].ColorIndex);
	UE_LOG(LogHOLMatch, Log, TEXT("%s holds slot %d (colour %d)"), *PlayerState->GetPlayerName(), Slot,
		MatchConfig.Slots[Slot].ColorIndex);
	// The stock of the start (D283); the population is counted by the production at each step.
	PlayerState->SetResources(GetDefault<UHOLMatchSettings>()->StartingResources);
	AssignBuildingOwners(Slot);
	if (MatchConfig.HasPlayerNames())
	{
		UpdateStartWait();
	}
	else
	{
		SendStartedIfAllHere();
	}
}

void AHeroesOfLibertyGameModeBase::UpdateStartWait()
{
	AHOLGameState* HOLGameState = GetGameState<AHOLGameState>();
	if (!HOLGameState || !HOLGameState->IsWaitingForPlayers())
	{
		return;
	}
	TArray<FString> Expected;
	for (const FHOLMatchSlot& SlotConfig : MatchConfig.Slots)
	{
		if (SlotConfig.State == EHOLSlotState::Human && !SlotConfig.PlayerName.TrimStartAndEnd().IsEmpty())
		{
			Expected.Add(SlotConfig.PlayerName.TrimStartAndEnd());
		}
	}
	TArray<FString> Present;
	for (const APlayerState* PlayerState : GameState->PlayerArray)
	{
		const AHOLPlayerState* HOLPlayerState = Cast<AHOLPlayerState>(PlayerState);
		if (HOLPlayerState && !HOLPlayerState->IsOnlyASpectator() && MatchConfig.Slots.IsValidIndex(HOLPlayerState->GetPlayerIndex()))
		{
			Present.Add(MatchConfig.Slots[HOLPlayerState->GetPlayerIndex()].PlayerName);
		}
	}
	const TArray<FString> Missing = FHOLStartRules::GetMissing(Expected, Present);
	const double Elapsed = GetWorld()->GetTimeSeconds() - StartWaitSince;
	const float MaxWait = CVarStartWaitSeconds.GetValueOnGameThread();
	if (!FHOLStartRules::ShouldStart(Missing.Num(), Elapsed, MaxWait))
	{
		if (Missing != LoggedMissing)
		{
			LoggedMissing = Missing;
			UE_LOG(LogHOLMatch, Log, TEXT("Match waits for: %s"), *FString::Join(Missing, TEXT(", ")));
		}
		HOLGameState->SetWaitingForPlayers(true, Missing, StartWaitSince + MaxWait);
		return;
	}
	GetWorldTimerManager().ClearTimer(StartWaitTimer);
	HOLGameState->SetWaitingForPlayers(false, {});
	const UHOLSimulationSubsystem* Simulation = GetWorld()->GetSubsystem<UHOLSimulationSubsystem>();
	const int64 Step = Simulation ? Simulation->GetStepIndex() : 0;
	if (Missing.IsEmpty())
	{
		UE_LOG(LogHOLMatch, Log, TEXT("Match starts: all here (simulation step %lld)"), Step);
	}
	else
	{
		// R7 (R-Q5): the absent players' disconnection window starts here; until then their slots play with no player.
		UE_LOG(LogHOLMatch, Log, TEXT("Match starts after %.0f s without: %s (simulation step %lld)"), MaxWait,
			*FString::Join(Missing, TEXT(", ")), Step);
	}
	UHOLManagerLink* Link = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHOLManagerLink>() : nullptr;
	if (Link && Link->IsActive() && !Link->HasStarted())
	{
		UE_LOG(LogHOLNet, Log, TEXT("Manager: match started"));
		Link->SendStarted();
	}
}

void AHeroesOfLibertyGameModeBase::SendStartedIfAllHere()
{
	UHOLManagerLink* Link = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHOLManagerLink>() : nullptr;
	if (!Link || !Link->IsActive() || Link->HasStarted())
	{
		return;
	}
	int32 Held = 0;
	for (const APlayerState* PlayerState : GameState->PlayerArray)
	{
		const AHOLPlayerState* HOLPlayerState = Cast<AHOLPlayerState>(PlayerState);
		if (HOLPlayerState && !HOLPlayerState->IsOnlyASpectator() && MatchConfig.Slots.IsValidIndex(HOLPlayerState->GetPlayerIndex())
			&& MatchConfig.Slots[HOLPlayerState->GetPlayerIndex()].State == EHOLSlotState::Human)
		{
			++Held;
		}
	}
	int32 Humans = 0;
	for (const FHOLMatchSlot& SlotConfig : MatchConfig.Slots)
	{
		Humans += SlotConfig.State == EHOLSlotState::Human ? 1 : 0;
	}
	if (Held >= Humans)
	{
		UE_LOG(LogHOLNet, Log, TEXT("Manager: every human slot held (%d), match started"), Humans);
		Link->SendStarted();
	}
}

void AHeroesOfLibertyGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	UHOLManagerLink* Link = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHOLManagerLink>() : nullptr;
	if (!Link || !Link->IsActive() || !Link->HasStarted())
	{
		return;
	}
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (It->Get() && It->Get() != Exiting)
		{
			return;
		}
	}
	UE_LOG(LogHOLNet, Log, TEXT("Manager: last player gone"));
	Link->SendEmpty();
}

void AHeroesOfLibertyGameModeBase::OnMatchOver()
{
	const AHOLGameState* HOLGameState = GetGameState<AHOLGameState>();
	UHOLManagerLink* Link = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHOLManagerLink>() : nullptr;
	if (HOLGameState && Link)
	{
		Link->SendFinished(HOLGameState->GetWinners());
	}
}

void AHeroesOfLibertyGameModeBase::AssignBuildingOwners(const int32 Slot)
{
	APlayerController* Controller = nullptr;
	for (const APlayerState* PlayerState : GameState->PlayerArray)
	{
		const AHOLPlayerState* HOLPlayerState = Cast<AHOLPlayerState>(PlayerState);
		if (HOLPlayerState && HOLPlayerState->GetPlayerIndex() == Slot)
		{
			Controller = HOLPlayerState->GetPlayerController();
			break;
		}
	}
	if (!Controller)
	{
		return;
	}
	for (TActorIterator<AHOLBuilding> It(GetWorld()); It; ++It)
	{
		if (It->GetOwnerPlayer() == Slot)
		{
			It->SetOwner(Controller);
		}
	}
}

void AHeroesOfLibertyGameModeBase::StartPlay()
{
	// R3.9: a match launched by the lobby manager waits for its named players, frozen (R3.10), from the first frame.
	AHOLGameState* WaitingState = MatchConfig.HasPlayerNames() ? GetGameState<AHOLGameState>() : nullptr;
	if (WaitingState)
	{
		StartWaitSince = GetWorld()->GetTimeSeconds();
		WaitingState->SetWaitingForPlayers(true, {}, StartWaitSince + CVarStartWaitSeconds.GetValueOnGameThread());
	}

	Super::StartPlay();

	if (AHOLStartSlot::WorldHasSlots(GetWorld()))
	{
		SpawnMainCentres();
	}

	UHOLManagerLink* Link = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHOLManagerLink>() : nullptr;
	if (Link && Link->IsActive() && GetWorld()->GetNetDriver())
	{
		if (AHOLGameState* HOLGameState = GetGameState<AHOLGameState>())
		{
			HOLGameState->OnMatchOver.AddUObject(this, &AHeroesOfLibertyGameModeBase::OnMatchOver);
		}
		// UIpNetDriver::InitListen writes the port listened on into the world's URL.
		Link->SendListening(GetWorld()->URL.Port);
	}

	if (WaitingState)
	{
		UpdateStartWait();
		if (WaitingState->IsWaitingForPlayers())
		{
			GetWorldTimerManager().SetTimer(StartWaitTimer, this, &AHeroesOfLibertyGameModeBase::UpdateStartWait, 1.f, true);
		}
	}
}

void AHeroesOfLibertyGameModeBase::SpawnMainCentres()
{
	for (int32 Slot = 0; Slot < MatchConfig.Slots.Num(); ++Slot)
	{
		const FHOLMatchSlot& SlotConfig = MatchConfig.Slots[Slot];
		if (SlotConfig.State == EHOLSlotState::Closed)
		{
			continue;
		}
		const AHOLStartSlot* Start = AHOLStartSlot::Find(GetWorld(), Slot);
		if (!Start)
		{
			UE_LOG(LogHOLMatch, Warning, TEXT("No start for slot %d in this map: no main centre"), Slot);
			continue;
		}
		// On the tile grid (D311), facing about the way the start faces (the centre of a symmetric map), turned to the
		// nearest quarter turn: the houses placed tile by tile line up with it.
		const FTransform Transform(FRotator(0.f, Start->GetMainCentreYaw(), 0.f), Start->GetMainCentreLocation());
		AHOLBuilding* Centre = GetWorld()->SpawnActorDeferred<AHOLBuilding>(AHOLBuilding::StaticClass(), Transform, nullptr, nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!Centre)
		{
			continue;
		}
		Centre->Setup(HOLBuildingTypeNames::MainCentre, static_cast<uint8>(Slot), static_cast<uint8>(SlotConfig.ColorIndex));
		// V2 (D350): the centre given at the start is the slot's main forum, the one with lives.
		Centre->SetPrimaryForum(true);
		Centre->FinishSpawning(Transform);
		AssignBuildingOwners(Slot);
		UE_LOG(LogHOLMatch, Log, TEXT("Main centre of slot %d (colour %d) at (%.0f, %.0f), yaw %.0f"), Slot, SlotConfig.ColorIndex,
			Transform.GetLocation().X, Transform.GetLocation().Y, Transform.Rotator().Yaw);
	}
}
