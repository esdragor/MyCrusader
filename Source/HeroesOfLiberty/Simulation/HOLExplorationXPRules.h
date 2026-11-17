// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * B2 (ExplorationXPInProgress.md, GDD 9.4, D05, D06): the hero's exploration XP, only in C++ (no script twin; D836)
 * for the server's fog (UHOLVisionSubsystem) to call. The fog grid (4 m cells) is cut into square tranches; the
 * first explored cell of a tranche pays once, a place pays once when its cell is explored, what is explored at the start pays nothing, ever.
 * The value returned is the final XP of the Exploration source: it is NOT passed again through HOLHeroXP::GetWeightPercent (the other
 * Exploration carriers, relays and neutral camps, add their XP as it is too; only Development goes through HOLHeroXP::GetXP).
 */
namespace HOLExplorationXP
{
	struct FSettings
	{
		/** The side of a tranche, in fog cells (16 = 64 m). */
		int32 CellsPerTranche = 16;
		/** The XP a tranche pays, once, at its first explored cell. */
		int32 XPPerTranche = 3;
		/** The XP a place pays, once, when its cell is explored. */
		int32 XPPerPlace = 25;
	};

	/** One camp's memory of what already paid. */
	struct FState
	{
		int32 GridCells = 0;
		int32 TranchesPerSide = 0;
		TArray<bool> TrancheDone;
		TArray<bool> PlaceDone;
	};

	/** Retours\Q-exploration-xp.md: 0 = C both (default), 1 = A surface only, 2 = B places only. */
	HEROESOFLIBERTY_API FSettings Preset(int32 Index);

	/** The console's hol.HeroXP.ExplorationPreset (0, 1 or 2; default 0 = C), read when the fog starts. */
	HEROESOFLIBERTY_API int32 GetPresetIndex();

	/** A camp's fresh state for a GridCells x GridCells fog grid and NumPlaces places: nothing paid yet. */
	HEROESOFLIBERTY_API FState NewState(const FSettings& Settings, int32 GridCells, int32 NumPlaces);

	/** A cell explored at the start (the first second's sight, the "Explored map" mode): its tranche and places marked, no XP. */
	HEROESOFLIBERTY_API void MarkExploredAtStart(FState& State, const FSettings& Settings, TConstArrayView<FIntPoint> Places, const FIntPoint& Cell);

	/**
	 * The XP the camp's hero gets when the camp newly explores Cell: its tranche's and places' first time, 0 otherwise and out of the grid.
	 * bHeroDead is not read on purpose: D06, exploration keeps paying while the hero is recovering.
	 */
	HEROESOFLIBERTY_API int32 OnCellExplored(const FSettings& Settings, FState& State, TConstArrayView<FIntPoint> Places, const FIntPoint& Cell, bool bHeroDead);
}
