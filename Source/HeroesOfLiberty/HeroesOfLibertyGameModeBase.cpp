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
#include "Map/HOLMapSizeRules.h"
#include "Map/HOLStartSlot.h"
#include "EngineUtils.h"
#include "Match/HOLDisconnectRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchSettings.h"
#include "Match/HOLMatchSubsystem.h"
#include "Match/HOLPlayerState.h"
#include "Net/HOLAccessKey.h"
#include "Net/HOLCheatRules.h"
#include "Net/HOLManagerLink.h"
#include "Engine/NetDriver.h"
#include "Player/HOLPlayerController.h"
#include "Match/HOLStartRules.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "TimerManager.h"
#include "HAL/IConsoleManager.h"

/** V5.1: a King for a slot by its main forum, whatever the option (for the trials of Regicide). */
static FAutoConsoleCommandWithWorldAndArgs CommandSpawnKing(TEXT("hol.Victory.SpawnKing"),
	TEXT("hol.Victory.SpawnKing [Slot=0]: a King of the slot born by its main forum, server or solo (development only)"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		const int32 Slot = Args.IsEmpty() ? 0 : FCString::Atoi(*Args[0]);
		if (!World || World->GetNetMode() == NM_Client)
		{
			UE_LOG(LogHOLMatch, Warning, TEXT("hol.Victory.SpawnKing: on the server or in solo"));
			return;
		}
		for (TActorIterator<AHOLBuilding> It(World); It; ++It)
		{
			if (It->IsPrimaryForum() && It->GetOwnerPlayer() == Slot)
			{
				AHeroesOfLibertyGameModeBase::SpawnKing(*World, Slot, **It);
				return;
			}
		}
		UE_LOG(LogHOLMatch, Warning, TEXT("hol.Victory.SpawnKing: slot %d has no main forum"), Slot);
	}));

/** V5.2: the slot's King killed by the dying path of the combat (its defeat, reason King, at the end of the step). */
static FAutoConsoleCommandWithWorldAndArgs CommandKillKing(TEXT("hol.Victory.KillKing"),
	TEXT("hol.Victory.KillKing [Slot=0]: the slot's King at 0 health, dying as killed in combat, server or solo (development only)"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		const int32 Slot = Args.IsEmpty() ? 0 : FCString::Atoi(*Args[0]);
		UHOLUnitSubsystem* Units = World && World->GetNetMode() != NM_Client ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
		if (!Units || Slot < 0 || Slot >= FHOLMatchConfig::MaxSlots)
		{
			UE_LOG(LogHOLMatch, Warning, TEXT("hol.Victory.KillKing: on the server or in solo, a slot 0 to %d"), FHOLMatchConfig::MaxSlots - 1);
			return;
		}
		const int32 Killed = Units->KillUnitsOfOwner(static_cast<uint8>(Slot), FHOLStartRules::KingTypeName);
		UE_LOG(LogHOLMatch, Display, TEXT("hol.Victory.KillKing: slot %d, %d King(s) at 0 health"), Slot, Killed);
	}));

/** R7.5: the disconnection banner in solo, for a capture and the trial's scenario (no slot kept, no defeat). */
static FAutoConsoleCommandWithWorldAndArgs CommandFakeAbsent(TEXT("hol.Net.FakeAbsent"),
	TEXT("hol.Net.FakeAbsent [Name] [Seconds=161]: « Name : connexion perdue — m:ss » in the banner, Seconds before its end; ")
	TEXT("Seconds < 0: « Name : de retour » for 5 s; no Name: the fake ones cleared. Server or solo (development only)"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		AHOLGameState* HOLGameState = World && World->GetNetMode() != NM_Client ? World->GetGameState<AHOLGameState>() : nullptr;
		if (!HOLGameState)
		{
			UE_LOG(LogHOLMatch, Warning, TEXT("hol.Net.FakeAbsent: on the server or in solo, in a match"));
			return;
		}
		const FString Name = Args.IsEmpty() ? FString() : Args[0];
		const double Seconds = Args.Num() > 1 ? FCString::Atod(*Args[1]) : 161.;
		HOLGameState->SetFakeAbsentNotice(Name, Seconds);
		UE_LOG(LogHOLMatch, Display, TEXT("hol.Net.FakeAbsent: %s"), Name.IsEmpty() ? TEXT("cleared")
			: *FString::Printf(TEXT("%s, %.0f s"), *Name, Seconds));
	}));

static TAutoConsoleVariable<float> CVarStartWaitSeconds(TEXT("hol.Match.StartWaitSeconds"), 60.f,
	TEXT("R3.9: the longest wait of a match launched by the lobby manager for its named players, in seconds."));

static TAutoConsoleVariable<float> CVarDisconnectWindowSeconds(TEXT("hol.Match.DisconnectWindowSeconds"),
	static_cast<float>(FHOLDisconnectRules::DefaultWindowSeconds),
	TEXT("R7 (R-Q5): how long a named player gone during the match keeps his slot, in seconds; then his slot loses. ")
	TEXT("Also -HOLDisconnectWindow=<s> on the command line (tests)."));

AHeroesOfLibertyGameModeBase::AHeroesOfLibertyGameModeBase()
{
	PlayerStateClass = AHOLPlayerState::StaticClass();
	GameStateClass = AHOLGameState::StaticClass();
}

void AHeroesOfLibertyGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// R7: a shorter window for the scripted checks (the console variable can also be set by the console).
	float WindowSeconds = 0.f;
	if (FParse::Value(FCommandLine::Get(), TEXT("HOLDisconnectWindow="), WindowSeconds) && WindowSeconds > 0.f)
	{
		CVarDisconnectWindowSeconds->Set(WindowSeconds, ECVF_SetByCommandline);
		UE_LOG(LogHOLMatch, Log, TEXT("Disconnection window: %.0f s"), WindowSeconds);
	}

	const UHOLMatchSubsystem* Match = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHOLMatchSubsystem>() : nullptr;
	const bool bHasConfig = Match && Match->HasConfig();
	MatchConfig = bHasConfig ? Match->GetConfigOrDefault() : FHOLMatchConfig::MakeDefault();
	UE_LOG(LogHOLMatch, Log, TEXT("Match on %s: %s configuration, %d occupied slots"), *MapName,
		bHasConfig ? TEXT("given") : TEXT("default"), MatchConfig.GetNumOccupiedSlots());
	// TC3 (TC-Q6): each occupied slot's place, fixed (slot i at place i: scenarios, quick_check) or drawn from the seed;
	// a configuration that does not fit its map is refused: nobody gets a start (AHOLGameState::GetStartPlace).
	FString PlacesError;
	if (!FHOLMapSizeRules::CheckPlaces(MatchConfig, &PlacesError))
	{
		UE_LOG(LogHOLMatch, Error, TEXT("Match config refused on %s: %s; no start for anyone"), *MapName, *PlacesError);
	}
	else if (MatchConfig.bDrawPlaces)
	{
		const TArray<int32> Places = FHOLMapSizeRules::GetPlaces(MatchConfig);
		UE_LOG(LogHOLMatch, Log, TEXT("Places drawn (seed %d, teams together %d): %s"), MatchConfig.Seed, MatchConfig.bTeamsTogether,
			*FString::JoinBy(Places, TEXT(" "), [](const int32 Place) { return FString::FromInt(Place); }));
	}
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
		return;
	}
	// Launcher L11: a server started with -HOLAccessKey= lets in only the kit's games (the key never logged).
	const FString ClientKey = FGenericPlatformHttp::UrlDecode(UGameplayStatics::ParseOption(Options, HOLAccessKey::UrlOption));
	if (!HOLAccessKey::IsAccepted(HOLAccessKey::GetServerKey(), ClientKey, &ErrorMessage))
	{
		UE_LOG(LogHOLMatch, Warning, TEXT("Login refused from %s: %s"), *Address, *ErrorMessage);
		return;
	}
	// M6-Q10: « Regarder » once the match is over: a spectator is refused, nothing left to watch. Every client sends a
	// name (the engine adds its nickname to the login), so a spectator is one whose name holds no free slot (M6-Q8).
	const AHOLGameState* HOLGameState = GetGameState<AHOLGameState>();
	if (!HOLGameState || !HOLGameState->IsMatchOver())
	{
		return;
	}
	TArray<int32> TakenSlots;
	for (const APlayerState* Other : GameState->PlayerArray)
	{
		if (const AHOLPlayerState* HOLOther = Cast<AHOLPlayerState>(Other))
		{
			TakenSlots.Add(HOLOther->GetPlayerIndex());
		}
	}
	const FString Name = FGenericPlatformHttp::UrlDecode(UGameplayStatics::ParseOption(Options, TEXT("Name"))).TrimStartAndEnd();
	if (HOLGameState->FindSlotOnLogin(MatchConfig, Name, TakenSlots, /*bLog*/ false) == INDEX_NONE)
	{
		ErrorMessage = TEXT("The match is over");
		UE_LOG(LogHOLMatch, Warning, TEXT("Login refused from %s: %s (spectator)"), *Address, *ErrorMessage);
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
	// B6b: a defeated slot is never taken again (FindSlotOnLogin), its name watches.
	const AHOLGameState* LoginGameState = GetGameState<AHOLGameState>();
	const int32 Slot = LoginGameState ? LoginGameState->FindSlotOnLogin(MatchConfig, LoginName, TakenSlots)
		: MatchConfig.FindSlotForName(LoginName, TakenSlots);
	if (Slot == INDEX_NONE)
	{
		// M6-Q8: a name absent from the configuration (or held by a player still here, or its slot defeated), or no human
		// slot left: it watches.
		PlayerState->SetIsSpectator(true);
		PlayerState->SetIsOnlyASpectator(true);
		UE_LOG(LogHOLMatch, Log, TEXT("%s joined as a spectator (name '%s', %s)"), *PlayerState->GetPlayerName(), *LoginName,
			MatchConfig.HasPlayerNames() ? TEXT("no free slot of that name") : TEXT("every human slot is taken"));
		return;
	}
	PlayerState->SetMatchSlot(Slot, MatchConfig.Slots[Slot].ColorIndex);
	UE_LOG(LogHOLMatch, Log, TEXT("%s holds slot %d (colour %d)"), *PlayerState->GetPlayerName(), Slot,
		MatchConfig.Slots[Slot].ColorIndex);
	// The stock of the start (D283), or the one kept if he comes back within the window (R7.2); the population is
	// counted by the production at each step.
	if (AHOLGameState* HOLGameState = GetGameState<AHOLGameState>())
	{
		if (HOLGameState->GiveSlotStock(*PlayerState, GetDefault<UHOLMatchSettings>()->StartingResources))
		{
			// M6-Q15: back within the window, he counts as present for the manager (a leader stays leader).
			UHOLManagerLink* Link = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHOLManagerLink>() : nullptr;
			const FString PlayerName = MatchConfig.Slots[Slot].PlayerName.TrimStartAndEnd();
			if (Link && Link->IsActive() && Link->HasStarted() && !PlayerName.IsEmpty())
			{
				UE_LOG(LogHOLNet, Log, TEXT("Manager: %s back"), *PlayerName);
				Link->SendBack(PlayerName);
			}
		}
	}
	else
	{
		PlayerState->SetResources(GetDefault<UHOLMatchSettings>()->StartingResources);
	}
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
		for (int32 Slot = 0; Slot < MatchConfig.Slots.Num(); ++Slot)
		{
			const FString SlotName = MatchConfig.Slots[Slot].PlayerName.TrimStartAndEnd();
			if (Missing.ContainsByPredicate([&SlotName](const FString& Name) { return Name.Equals(SlotName, ESearchCase::IgnoreCase); })
				&& HOLGameState->KeepMissingSlot(Slot, GetDefault<UHOLMatchSettings>()->StartingResources, GetWorld()->GetTimeSeconds(),
					CVarDisconnectWindowSeconds.GetValueOnGameThread()))
			{
				GetWorldTimerManager().SetTimer(AbsenceTimer, this, &AHeroesOfLibertyGameModeBase::UpdateAbsences, 1.f, true);
			}
		}
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

	// R7 (R-Q5): his slot, units, buildings and stock kept for the window (named matches only).
	AHOLGameState* AbsenceState = GetGameState<AHOLGameState>();
	const AHOLPlayerState* LeavingState = Exiting ? Exiting->GetPlayerState<AHOLPlayerState>() : nullptr;
	if (AbsenceState && LeavingState && AbsenceState->KeepAbsentSlot(*LeavingState, GetWorld()->GetTimeSeconds(),
		CVarDisconnectWindowSeconds.GetValueOnGameThread()))
	{
		GetWorldTimerManager().SetTimer(AbsenceTimer, this, &AHeroesOfLibertyGameModeBase::UpdateAbsences, 1.f, true);
	}

	UHOLManagerLink* Link = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHOLManagerLink>() : nullptr;
	if (!Link || !Link->IsActive() || !Link->HasStarted())
	{
		return;
	}
	// M6-Q14: a named player gone; the manager gives the lobby a new leader if he was its away leader.
	const AHOLPlayerState* HOLPlayerState = Exiting ? Exiting->GetPlayerState<AHOLPlayerState>() : nullptr;
	if (HOLPlayerState && !HOLPlayerState->IsOnlyASpectator() && MatchConfig.Slots.IsValidIndex(HOLPlayerState->GetPlayerIndex()))
	{
		const FString PlayerName = MatchConfig.Slots[HOLPlayerState->GetPlayerIndex()].PlayerName.TrimStartAndEnd();
		if (!PlayerName.IsEmpty())
		{
			UE_LOG(LogHOLNet, Log, TEXT("Manager: %s left"), *PlayerName);
			Link->SendLeft(PlayerName);
		}
	}
	if (!bEmptyCheckQueued)
	{
		bEmptyCheckQueued = true;
		GetWorldTimerManager().SetTimerForNextTick(this, &AHeroesOfLibertyGameModeBase::SendEmptyIfNobodyLeft);
	}
}

void AHeroesOfLibertyGameModeBase::SendEmptyIfNobodyLeft()
{
	bEmptyCheckQueued = false;
	UHOLManagerLink* Link = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHOLManagerLink>() : nullptr;
	if (!Link || !Link->IsActive() || !Link->HasStarted())
	{
		return;
	}
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (IsValid(It->Get()))
		{
			return;
		}
	}
	UE_LOG(LogHOLNet, Log, TEXT("Manager: last player gone"));
	Link->SendEmpty();
}

void AHeroesOfLibertyGameModeBase::UpdateAbsences()
{
	AHOLGameState* HOLGameState = GetGameState<AHOLGameState>();
	if (!HOLGameState)
	{
		return;
	}
	HOLGameState->DefeatExpiredAbsences(GetWorld()->GetTimeSeconds(), CVarDisconnectWindowSeconds.GetValueOnGameThread());
	// R7.5: on until the last « de retour » is dropped.
	if (HOLGameState->GetAbsentSlots().IsEmpty() && HOLGameState->GetAbsentNotices().IsEmpty())
	{
		GetWorldTimerManager().ClearTimer(AbsenceTimer);
	}
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

bool AHeroesOfLibertyGameModeBase::AllowPausing(APlayerController* PC)
{
	if (FHOLCheatRules::IsPauseAllowed(GetNetMode()))
	{
		return Super::AllowPausing(PC);
	}
	if (PC)
	{
		const AHOLPlayerState* HOLPlayerState = PC->GetPlayerState<AHOLPlayerState>();
		UE_LOG(LogHOLMatch, Display, TEXT("Pause refused from %s (slot %d)"), HOLPlayerState ? *HOLPlayerState->GetPlayerName() : TEXT("?"),
			HOLPlayerState ? HOLPlayerState->GetPlayerIndex() : INDEX_NONE);
	}
	return false;
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
		// TC3: at the slot's place (AHOLGameState::GetStartPlace, set by InitVictory).
		const AHOLStartSlot* Start = AHOLStartSlot::FindForSlot(GetWorld(), Slot);
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
		// V5.1 (Regicide, D206): each occupied slot's King, by its main forum.
		if (FHOLStartRules::ShouldSpawnKing(MatchConfig, Slot))
		{
			SpawnKing(*GetWorld(), Slot, *Centre);
		}
		// IA1.1: each occupied slot's starting Peasants, in front of its main forum (0 in the test scenarios).
		const int32 NumPeasants = FHOLStartRules::GetNumStartingPeasants(MatchConfig, GetDefault<UHOLMatchSettings>()->StartingPeasants);
		if (NumPeasants > 0)
		{
			SpawnStartingPeasants(*GetWorld(), Slot, *Centre, NumPeasants);
		}
		// J8a.2 (J8a-Q2): each occupied slot's hero, by its main forum (none in the test scenarios).
		if (FHOLStartRules::ShouldSpawnHero(MatchConfig, Slot, GetDefault<UHOLMatchSettings>()->bStartingHero))
		{
			SpawnStartingHero(*GetWorld(), Slot, *Centre);
		}
	}
}

bool AHeroesOfLibertyGameModeBase::SpawnStartingPeasants(UWorld& World, const int32 Slot, const AHOLBuilding& Forum, const int32 Num)
{
	UHOLUnitSubsystem* Units = World.GetSubsystem<UHOLUnitSubsystem>();
	if (!Units)
	{
		return false;
	}
	const FHOLBuildingTypeRow* Type = Forum.GetType();
	const FVector2D Body = Type ? UHOLBuildingSettings::GetBodySize(Type->Footprint) : FVector2D(1200.0);
	const TArray<FVector> Locations =
		FHOLStartRules::GetStartingPeasantLocations(Forum.GetActorLocation(), 0.5f * static_cast<float>(Body.GetMax()), Num);
	for (const FVector& Location : Locations)
	{
		// A tiny radius: put on the navmesh right under the point, like the King.
		Units->RequestUnits(FHOLUnitSpawnRequest{ FHOLStartRules::PeasantTypeName, 1, Location, 1.f, static_cast<uint8>(Slot) });
	}
	UE_LOG(LogHOLMatch, Display, TEXT("Starting Peasants of slot %d: %d in front of its main forum"), Slot, Locations.Num());
	return true;
}

bool AHeroesOfLibertyGameModeBase::SpawnStartingHero(UWorld& World, const int32 Slot, const AHOLBuilding& Forum)
{
	UHOLUnitSubsystem* Units = World.GetSubsystem<UHOLUnitSubsystem>();
	if (!Units)
	{
		return false;
	}
	const FHOLBuildingTypeRow* Type = Forum.GetType();
	const FVector2D Body = Type ? UHOLBuildingSettings::GetBodySize(Type->Footprint) : FVector2D(1200.0);
	const FVector Location = FHOLStartRules::GetHeroLocation(Forum.GetActorLocation(), 0.5f * static_cast<float>(Body.GetMax()));
	// A tiny radius: put on the navmesh right under the point, like the King.
	Units->RequestUnits(FHOLUnitSpawnRequest{ FHOLStartRules::HeroTypeName, 1, Location, 1.f, static_cast<uint8>(Slot) });
	UE_LOG(LogHOLMatch, Display, TEXT("Starting hero of slot %d spawned at (%.0f, %.0f)"), Slot, Location.X, Location.Y);
	return true;
}

bool AHeroesOfLibertyGameModeBase::SpawnKing(UWorld& World, const int32 Slot, const AHOLBuilding& Forum)
{
	UHOLUnitSubsystem* Units = World.GetSubsystem<UHOLUnitSubsystem>();
	if (!Units)
	{
		return false;
	}
	const FHOLBuildingTypeRow* Type = Forum.GetType();
	const FVector2D Body = Type ? UHOLBuildingSettings::GetBodySize(Type->Footprint) : FVector2D(1200.0);
	const FVector Location = FHOLStartRules::GetKingLocation(Forum.GetActorLocation(), 0.5f * static_cast<float>(Body.GetMax()));
	// A tiny radius: put on the navmesh right under the point (FHOLUnitSpawnRequest), like a production's birth.
	Units->RequestUnits(FHOLUnitSpawnRequest{ FHOLStartRules::KingTypeName, 1, Location, 1.f, static_cast<uint8>(Slot) });
	UE_LOG(LogHOLMatch, Display, TEXT("Regicide: king of slot %d spawned at (%.0f, %.0f)"), Slot, Location.X, Location.Y);
	return true;
}
