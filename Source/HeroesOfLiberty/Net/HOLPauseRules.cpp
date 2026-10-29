// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLPauseRules.h"

FHOLPauseRules::EAnswer FHOLPauseRules::CanPause(const ENetMode NetMode, const FState& State, const bool bRequesterInMatch)
{
	if (State.IsPaused())
	{
		return EAnswer::AlreadyPaused;
	}
	if (NetMode == NM_Standalone)
	{
		return EAnswer::Allowed;
	}
	if (!bRequesterInMatch)
	{
		return EAnswer::NotInMatch;
	}
	return State.PausesLeft > 0 ? EAnswer::Allowed : EAnswer::NoPauseLeft;
}

FHOLPauseRules::EAnswer FHOLPauseRules::CanResume(const ENetMode NetMode, const FState& State, const int32 RequesterSlot, const bool bRequesterInMatch,
	const double NowSeconds, const double TimerSeconds)
{
	if (!State.IsPaused())
	{
		return EAnswer::NotPaused;
	}
	if (NetMode == NM_Standalone)
	{
		return EAnswer::Allowed;
	}
	if (!bRequesterInMatch)
	{
		return EAnswer::NotInMatch;
	}
	if (RequesterSlot == State.PausedBySlot || TimerLeft(State, NowSeconds, TimerSeconds) <= 0.0)
	{
		return EAnswer::Allowed;
	}
	return EAnswer::TimerRunning;
}

void FHOLPauseRules::Pause(const ENetMode NetMode, FState& State, const int32 Slot, const double NowSeconds)
{
	if (NetMode != NM_Standalone)
	{
		State.PausesLeft = FMath::Max(State.PausesLeft - 1, 0);
	}
	State.PausedBySlot = Slot;
	State.PausedAtSeconds = NowSeconds;
}

void FHOLPauseRules::Resume(FState& State)
{
	State.PausedBySlot = INDEX_NONE;
}

double FHOLPauseRules::TimerLeft(const FState& State, const double NowSeconds, const double TimerSeconds)
{
	return State.IsPaused() ? FMath::Max(TimerSeconds - (NowSeconds - State.PausedAtSeconds), 0.0) : 0.0;
}

const TCHAR* FHOLPauseRules::ToString(const EAnswer Answer)
{
	switch (Answer)
	{
	case EAnswer::Allowed: return TEXT("allowed");
	case EAnswer::NotInMatch: return TEXT("not in the match");
	case EAnswer::NoPauseLeft: return TEXT("no pause left");
	case EAnswer::AlreadyPaused: return TEXT("already paused");
	case EAnswer::NotPaused: return TEXT("not paused");
	case EAnswer::TimerRunning: return TEXT("timer running");
	}
	return TEXT("?");
}
