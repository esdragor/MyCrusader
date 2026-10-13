// Copyright Epic Games, Inc. All Rights Reserved.

#include "HeroesOfLibertyGameModeBase.h"

#include "Engine/GameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLMatchSubsystem.h"
#include "Match/HOLPlayerState.h"

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
