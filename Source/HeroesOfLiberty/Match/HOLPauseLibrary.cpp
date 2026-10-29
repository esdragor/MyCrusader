// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLPauseLibrary.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLGameState.h"
#include "Net/HOLPauseRules.h"

namespace
{
	const AHOLGameState* GetGameStateOf(const UObject* WorldContextObject)
	{
		const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
		return World ? World->GetGameState<AHOLGameState>() : nullptr;
	}

	/** The game state's pause as the rule sees it at the server time Now (its start from the timer this machine counts). */
	FHOLPauseRules::FState MakeState(const AHOLGameState& GameState, const double Now)
	{
		FHOLPauseRules::FState State;
		State.PausesLeft = GameState.GetPausesLeft();
		State.PausedBySlot = GameState.GetPausedBySlot();
		State.PausedAtSeconds = Now - (FHOLPauseRules::DefaultTimerSeconds - GameState.GetPauseTimerLeft(Now));
		return State;
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
	return GameState && FHOLPauseRules::CanPause(GameState->GetNetMode(), MakeState(*GameState, GameState->GetServerWorldTimeSeconds()), IsInMatch(*GameState, Slot))
		== FHOLPauseRules::EAnswer::Allowed;
}

bool UHOLPauseLibrary::CanResumeNow(const UObject* WorldContextObject, const int32 Slot)
{
	const AHOLGameState* GameState = GetGameStateOf(WorldContextObject);
	const double Now = GameState ? GameState->GetServerWorldTimeSeconds() : 0.;
	return GameState && FHOLPauseRules::CanResume(GameState->GetNetMode(), MakeState(*GameState, Now), Slot, IsInMatch(*GameState, Slot), Now)
		== FHOLPauseRules::EAnswer::Allowed;
}
