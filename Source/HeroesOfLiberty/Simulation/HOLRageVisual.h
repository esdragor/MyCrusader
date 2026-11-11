// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UWorld;

/**
 * L4 (Q-legions-warrior-rage-visual): how the Damned Warrior's necrotic rage is SEEN, behind the cvar hol.Rage.Visual. The rage had no
 * drawing at all (only the health bar drops faster); the user chooses among the options with the scenario legions_warrior_compare.
 * 0 = D nothing (the default: today's behaviour), 1 = A body tint, 2 = B ring on the ground, 3 = C state on the grid's rage cell (seconds left
 * while it runs, greyed with the recharge seconds after), 4 = A + C. The rules are pure (a mode and the rage's milliseconds in, what to draw
 * out); the state is read from the simulation's own FHOLCombatFragment (the host's: a client has no combat fragment, so it draws none).
 */
namespace HOLRageVisual
{
	constexpr int32 ModeNone = 0;
	constexpr int32 ModeTint = 1;
	constexpr int32 ModeRing = 2;
	constexpr int32 ModeIcon = 3;
	constexpr int32 ModeTintIcon = 4;

	/** The cvar hol.Rage.Visual (0 when it is not registered). */
	HEROESOFLIBERTY_API int32 GetMode();

	inline bool HasTint(const int32 Mode) { return Mode == ModeTint || Mode == ModeTintIcon; }
	inline bool HasRing(const int32 Mode) { return Mode == ModeRing; }

	/** Option B: radius (cm) and width of the ring on the ground under a warrior whose rage runs (a proposal, to be judged on the screen). */
	constexpr double RingRadius = 170.0;
	constexpr double RingWidth = 22.0;

	/** Option B: a warrior gets its ring while its rage runs (RageMs > 0) and the mode draws one. */
	inline bool ShouldDrawRing(const int32 Mode, const int32 RageMs)
	{
		return HasRing(Mode) && RageMs > 0;
	}
	inline bool HasIcon(const int32 Mode) { return Mode == ModeIcon || Mode == ModeTintIcon; }

	/** The share of the dark necrotic red mixed into the owner's colour while the rage runs (a proposal, to be judged on the screen). */
	constexpr float TintShare = 0.6f;

	/** The colour a unit is drawn in: its owner's colour, darkened towards a dark necrotic red while its rage runs (RageMs > 0) and the mode tints. */
	inline FVector3f GetDrawColor(const int32 Mode, const int32 RageMs, const FVector3f& Base)
	{
		return HasTint(Mode) && RageMs > 0 ? FMath::Lerp(Base, FVector3f(0.25f, 0.02f, 0.02f), TintShare) : Base;
	}

	/** The rage's state of a unit, read from its combat fragment: false for a unit without one (a client, a dead id). */
	HEROESOFLIBERTY_API bool GetRageState(const UWorld* World, uint32 UnitId, int32& OutRageMs, int32& OutCooldownMs);
}
