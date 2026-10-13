// Copyright Epic Games, Inc. All Rights Reserved.

#include "HeroesOfLibertyGameModeBase.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Engine/GameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "HeroesOfLibertyLog.h"
#include "Map/HOLStartSlot.h"
#include "Match/HOLMatchSubsystem.h"
#include "Match/HOLPlayerState.h"
#include "Player/HOLPlayerController.h"

AHeroesOfLibertyGameModeBase::AHeroesOfLibertyGameModeBase()
{
	PlayerStateClass = AHOLPlayerState::StaticClass();
}

void AHeroesOfLibertyGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	const UHOLMatchSubsystem* Match = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHOLMatchSubsystem>() : nullptr;
	const bool bHasConfig = Match && Match->HasConfig();
	MatchConfig = bHasConfig ? Match->GetConfigOrDefault() : FHOLMatchConfig::MakeDefault();
	UE_LOG(LogHOLMatch, Log, TEXT("Match on %s: %s configuration, %d occupied slots"), *MapName,
		bHasConfig ? TEXT("given") : TEXT("default"), MatchConfig.GetNumOccupiedSlots());

	// Before any player logs in: the classes are read when each controller is made.
	if (AHOLStartSlot::WorldHasSlots(GetWorld()))
	{
		PlayerControllerClass = AHOLPlayerController::StaticClass();
		DefaultPawnClass = nullptr;
		UE_LOG(LogHOLMatch, Log, TEXT("Match map: RTS camera, no pawn"));
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
		// Facing the way the start faces (the centre of a symmetric map).
		const FTransform Transform(FRotator(0.f, Start->GetActorRotation().Yaw, 0.f), Start->GetGroundLocation());
		AHOLBuilding* Centre = GetWorld()->SpawnActorDeferred<AHOLBuilding>(AHOLBuilding::StaticClass(), Transform, nullptr, nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!Centre)
		{
			continue;
		}
		Centre->Setup(HOLBuildingTypeNames::MainCentre, static_cast<uint8>(Slot), static_cast<uint8>(SlotConfig.ColorIndex));
		Centre->FinishSpawning(Transform);
		UE_LOG(LogHOLMatch, Log, TEXT("Main centre of slot %d (colour %d) at (%.0f, %.0f)"), Slot, SlotConfig.ColorIndex,
			Transform.GetLocation().X, Transform.GetLocation().Y);
	}
}
