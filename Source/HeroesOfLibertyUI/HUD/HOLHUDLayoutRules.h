// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Math/Box2D.h"

/**
 * Pure layout rules of the HUD's panels (the "panel hiding another" check of Auto_Journey.md, lesson 12b): whether two panels' boxes cover
 * each other, in the same units (Slate units, the HUD's scale). The overlap line of hol.HUD.Report reads it and preflight.ps1 fails on it.
 */
namespace HOLHUDLayout
{
	/** Whether the two boxes cover each other by more than Tolerance (units) on both axes: touching edges and a hairline do not count. */
	inline bool Overlaps(const FBox2D& A, const FBox2D& B, const float Tolerance = 1.f)
	{
		const double Across = FMath::Min(A.Max.X, B.Max.X) - FMath::Max(A.Min.X, B.Min.X);
		const double Down = FMath::Min(A.Max.Y, B.Max.Y) - FMath::Max(A.Min.Y, B.Min.Y);
		return Across > Tolerance && Down > Tolerance;
	}

	/**
	 * Q-heros-triches-600 = A (2026-11-02): under this window height (px) the cheats' panel (top left, grows downwards) and the hero's block
	 * (bottom left) cover each other. The cheats' panel is a development tool; nothing moves for the player.
	 */
	constexpr float LowWindowHeight = 700.f;

	/** A window lower than LowWindowHeight; a height not known yet (0) is not low. */
	inline bool IsLowWindow(const float HeightPx)
	{
		return HeightPx > 0.f && HeightPx < LowWindowHeight;
	}

	/** Whether the cheats' panel starts open (the kept choice), which a low window does not: it starts folded, one click opens it. */
	inline bool CheatsStartOpen(const bool bSavedOpen, const float HeightPx)
	{
		return bSavedOpen && !IsLowWindow(HeightPx);
	}

	/**
	 * Called each tick: true on the tick the cheats' panel must fold, i.e. it is open and the hero's block has just opened (or the window has just
	 * become low) in a low window. Edge-triggered, so that a click opening the panel afterwards (the developer's choice) is not undone.
	 * bWasLow and bWasShown are the previous tick's, updated here.
	 */
	inline bool ShouldFoldCheats(const float HeightPx, const bool bHeroBlockShown, const bool bCheatsOpen, bool& bWasLow, bool& bWasShown)
	{
		const bool bLow = IsLowWindow(HeightPx);
		const bool bConflict = bLow && bHeroBlockShown;
		const bool bWasConflict = bWasLow && bWasShown;
		bWasLow = bLow;
		bWasShown = bHeroBlockShown;
		return bCheatsOpen && bConflict && !bWasConflict;
	}
}
