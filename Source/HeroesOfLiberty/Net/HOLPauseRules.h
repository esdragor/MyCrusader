// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineBaseTypes.h"

/**
 * The limited pause (D513): 3 pauses per match in all, shared by every player; the match freezes and everyone sees who
 * paused and a 60 s timer; before its end only the one who paused resumes, after it any player; only the players still
 * in the match (neither defeated nor spectators) may pause or resume; the solo game pauses freely (no limit, no timer).
 * The server decides; the engine's console "pause" stays refused (FHOLCheatRules::IsPauseAllowed).
 */
struct HEROESOFLIBERTY_API FHOLPauseRules
{
	static constexpr int32 DefaultPausesPerMatch = 3;
	static constexpr double DefaultTimerSeconds = 60.0;

	struct FState
	{
		int32 PausesLeft = DefaultPausesPerMatch;
		/** INDEX_NONE when the match runs. */
		int32 PausedBySlot = INDEX_NONE;
		double PausedAtSeconds = 0.0;

		bool IsPaused() const { return PausedBySlot != INDEX_NONE; }
	};

	enum class EAnswer : uint8
	{
		Allowed,
		NotInMatch,
		NoPauseLeft,
		AlreadyPaused,
		NotPaused,
		TimerRunning,
	};

	static EAnswer CanPause(ENetMode NetMode, const FState& State, bool bRequesterInMatch);
	static EAnswer CanResume(ENetMode NetMode, const FState& State, int32 RequesterSlot, bool bRequesterInMatch, double NowSeconds,
		double TimerSeconds = DefaultTimerSeconds);

	/** Applies an allowed pause: one fewer left (not in solo), who and when. */
	static void Pause(ENetMode NetMode, FState& State, int32 Slot, double NowSeconds);
	static void Resume(FState& State);

	/** Seconds left on the timer, 0 once over (and when not paused). */
	static double TimerLeft(const FState& State, double NowSeconds, double TimerSeconds = DefaultTimerSeconds);

	static const TCHAR* ToString(EAnswer Answer);
};
