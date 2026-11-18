// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLWaitBannerReadLibrary.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLGameState.h"

namespace
{
	const AHOLGameState* WaitBannerGameState(const UObject* WorldContextObject)
	{
		const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
		return World ? World->GetGameState<AHOLGameState>() : nullptr;
	}
}

bool UHOLWaitBannerReadLibrary::GetWaitState(const UObject* WorldContextObject, TArray<FString>& OutMissing, int32& OutSecondsLeft)
{
	OutMissing.Reset();
	OutSecondsLeft = 0;
	const AHOLGameState* GameState = WaitBannerGameState(WorldContextObject);
	if (!GameState)
	{
		return false;
	}
	OutMissing = GameState->GetMissingPlayers();
	OutSecondsLeft = FMath::Max(0, FMath::CeilToInt(GameState->GetWaitEndSeconds() - GameState->GetServerWorldTimeSeconds()));
	return GameState->IsWaitingForPlayers();
}

void UHOLWaitBannerReadLibrary::GetAbsenceState(const UObject* WorldContextObject, TArray<FString>& OutLines, TArray<FString>& OutNames, TArray<bool>& OutBack)
{
	OutLines.Reset();
	OutNames.Reset();
	OutBack.Reset();
	const AHOLGameState* GameState = WaitBannerGameState(WorldContextObject);
	if (!GameState)
	{
		return;
	}
	for (const FText& Line : GameState->GetAbsenceBannerLines(GameState->GetServerWorldTimeSeconds()))
	{
		OutLines.Add(Line.ToString());
	}
	for (const FHOLAbsentNotice& Notice : GameState->GetAbsentNotices())
	{
		OutNames.Add(Notice.Name);
		OutBack.Add(Notice.BackAt > 0.);
	}
}
