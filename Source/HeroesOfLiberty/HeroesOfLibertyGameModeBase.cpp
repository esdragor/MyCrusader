// Copyright Epic Games, Inc. All Rights Reserved.

#include "HeroesOfLibertyGameModeBase.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Engine/GameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "HeroesOfLibertyLog.h"
#include "Map/HOLStartSlot.h"
#include "EngineUtils.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchSettings.h"
#include "Match/HOLMatchSubsystem.h"
#include "Match/HOLPlayerState.h"
#include "Player/HOLPlayerController.h"

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

	const int32 Slot = MatchConfig.FindFreeHumanSlot(TakenSlots);
	if (Slot == INDEX_NONE)
	{
		UE_LOG(LogHOLMatch, Warning, TEXT("%s joined but every human slot is taken: no slot"), *PlayerState->GetPlayerName());
		return;
	}
	PlayerState->SetMatchSlot(Slot, MatchConfig.Slots[Slot].ColorIndex);
	UE_LOG(LogHOLMatch, Log, TEXT("%s holds slot %d (colour %d)"), *PlayerState->GetPlayerName(), Slot,
		MatchConfig.Slots[Slot].ColorIndex);
	// The stock of the start (D283); the population is counted by the production at each step.
	PlayerState->SetResources(GetDefault<UHOLMatchSettings>()->StartingResources);
	AssignBuildingOwners(Slot);
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
	Super::StartPlay();

	if (AHOLStartSlot::WorldHasSlots(GetWorld()))
	{
		SpawnMainCentres();
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
