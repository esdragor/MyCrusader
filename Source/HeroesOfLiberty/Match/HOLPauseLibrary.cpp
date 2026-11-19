// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLPauseLibrary.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchFlowSubsystem.h"

namespace
{
	const AHOLGameState* GetGameStateOf(const UObject* WorldContextObject)
	{
		const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
		return World ? World->GetGameState<AHOLGameState>() : nullptr;
	}

	bool IsInMatch(const AHOLGameState& GameState, const int32 Slot)
	{
		return Slot != INDEX_NONE && !GameState.IsSlotDefeated(Slot) && !GameState.IsMatchOver();
	}
}

bool UHOLPauseLibrary::IsMatchPaused(const UObject* WorldContextObject)
{
	const AHOLGameState* GameState = GetGameStateOf(WorldContextObject);
	return GameState && GameState->IsMatchPaused();
}

FString UHOLPauseLibrary::GetPausedByName(const UObject* WorldContextObject)
{
	const AHOLGameState* GameState = GetGameStateOf(WorldContextObject);
	return GameState && GameState->IsMatchPaused() ? GameState->GetPausedByName() : FString();
}

int32 UHOLPauseLibrary::GetPausedBySlot(const UObject* WorldContextObject)
{
	const AHOLGameState* GameState = GetGameStateOf(WorldContextObject);
	return GameState ? GameState->GetPausedBySlot() : INDEX_NONE;
}

int32 UHOLPauseLibrary::GetPausesLeft(const UObject* WorldContextObject)
{
	const AHOLGameState* GameState = GetGameStateOf(WorldContextObject);
	return GameState ? GameState->GetPausesLeft() : -1;
}

int32 UHOLPauseLibrary::GetTimerSecondsLeft(const UObject* WorldContextObject)
{
	const AHOLGameState* GameState = GetGameStateOf(WorldContextObject);
	return GameState ? FMath::CeilToInt32(GameState->GetPauseTimerLeft(GameState->GetServerWorldTimeSeconds())) : 0;
}

bool UHOLPauseLibrary::IsPauseFree(const UObject* WorldContextObject)
{
	const AHOLGameState* GameState = GetGameStateOf(WorldContextObject);
	return GameState && GameState->IsPauseFree();
}

bool UHOLPauseLibrary::CanPauseNow(const UObject* WorldContextObject, const int32 Slot)
{
	const AHOLGameState* GameState = GetGameStateOf(WorldContextObject);
	// The script's rule (HOLMatchFlow::CanPause), as the server will decide; no board (-1) = no.
	return GameState && UHOLMatchFlowSubsystem::AskCanPause(GameState->GetWorld(), GameState->IsPauseFree(), GameState->GetPausedBySlot(), GameState->GetPausesLeft(),
		IsInMatch(*GameState, Slot)) == UHOLMatchFlowSubsystem::PauseAnswerAllowed;
}

bool UHOLPauseLibrary::CanResumeNow(const UObject* WorldContextObject, const int32 Slot)
{
	const AHOLGameState* GameState = GetGameStateOf(WorldContextObject);
	if (!GameState)
	{
		return false;
	}
	// The start of the pause as the rule sees it at the server time Now (from the timer this machine counts).
	const double Now = GameState->GetServerWorldTimeSeconds();
	const double PausedAt = Now - (AHOLGameState::PauseTimerSeconds - GameState->GetPauseTimerLeft(Now));
	return UHOLMatchFlowSubsystem::AskCanResume(GameState->GetWorld(), GameState->IsPauseFree(), GameState->GetPausedBySlot(), PausedAt, Slot, IsInMatch(*GameState, Slot), Now)
		== UHOLMatchFlowSubsystem::PauseAnswerAllowed;
}
