// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLSiteView.h"
#include "Simulation/HOLSiteBoardTypes.h"

// FHOLSiteState and FHOLSiteVictoryState (the simulation's copy of the script's rule states) are USTRUCTs of the script bridge since MIG-CPP-Site.

namespace HOLSite
{
	/** S10 (D699): seconds a camp must hold every site. 600, chosen by the user (D747, 2026-11-08). */
	constexpr int32 DefaultVictorySeconds = 600;

	/** Values to tune in test (D345, the user chooses them), the same as Script/Sites/HOLSiteRules.as (the script test compares them). */
	constexpr int32 DefaultHoldSeconds = 60;
	constexpr int32 DefaultDecaySteps = 2;
	/** S6 (D699): the gold a held site pays its owner by the minute. PROVISIONAL default (small passive income), Retours\Q-sites-or-par-minute.md. */
	constexpr int32 DefaultGoldPerMinute = 30;


	/** S5: the view of a site numbered Index at Location (zone Radius) in State: the share of the capture held (0 to 99) at HoldSteps. */
	HEROESOFLIBERTY_API FHOLSiteView MakeView(int32 Index, const FVector2D& Location, float Radius, const FHOLSiteState& State, int32 HoldSteps);

	/** Both count arrays set to Slots zeros. */
	HEROESOFLIBERTY_API void ResetCounts(TArray<int32>& Takers, TArray<int32>& Disturbers, int32 Slots);

	/**
	 * One living unit of Slot in the zone. bCanTake: a holy unit of its faction, or the hero. bIsPeasant: the worker, who counts for
	 * nothing. A slot out of the arrays is ignored.
	 */
	HEROESOFLIBERTY_API void AddUnit(TArray<int32>& Takers, TArray<int32>& Disturbers, int32 Slot, bool bCanTake, bool bIsPeasant);
}
