// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLSiteView.h"

/**
 * One sacred site (S2; GDD 14.3, D429 to D431, D697 to D703): its owner (a slot), who is filling its count and the count of steps held
 * toward its capture. The simulation's copy of the rules written and tested in Script/Sites/HOLSiteRules.as (the simulation cannot call
 * the script): the script test Test_SiteScriptAndSimulationRulesAgree compares the two on every case.
 */
struct FHOLSiteState
{
	/** The owner's slot, INDEX_NONE for nobody. */
	int32 Owner = INDEX_NONE;
	/** The slot with the most takers among the adverse camp filling the count, INDEX_NONE while nobody fills it. */
	int32 Taker = INDEX_NONE;
	/** Steps held toward the capture. */
	int32 Held = 0;

	bool operator==(const FHOLSiteState& Other) const { return Owner == Other.Owner && Taker == Other.Taker && Held == Other.Held; }
};

/**
 * S10 (D431, D703): the victory count of the sacred sites, the simulation's copy of HOLSiteVictory::FHOLSiteVictoryState (Script/Sites/HOLSiteVictory.as).
 * Leader: the camp holding every site now (INDEX_NONE for none); Held: the steps it held them; Winner: the camp once the count is full (it stays).
 */
struct FHOLSiteVictoryState
{
	int32 Leader = INDEX_NONE;
	int32 Held = 0;
	int32 Winner = INDEX_NONE;
	/** The owners of the sites at the last step, to see a site change hands. */
	TArray<int32> LastOwners;
};

namespace HOLSite
{
	/** S10 (D699): seconds a camp must hold every site. PROVISIONAL (300, the dossier's value; the user has not confirmed it). */
	constexpr int32 DefaultVictorySeconds = 300;

	/**
	 * S10: the count after one step (the script's HOLSiteVictory::Step). OwnerBySite: the owner slot of each site (INDEX_NONE nobody's);
	 * ContestedBySite: nonzero when an adverse unit stands in the site's zone; TeamBySlot: the team of each slot; VictorySteps: the steps to
	 * hold. The camp holding every site is the team of the owners when they are all of one team. It restarts at 0 when the holding camp
	 * changes or when any site changed hands (even to a slot of the same camp); a contested site freezes it; nobody holding every site: 0.
	 * The winner once set stays.
	 */
	HEROESOFLIBERTY_API FHOLSiteVictoryState VictoryStep(const FHOLSiteVictoryState& State, const TArray<int32>& OwnerBySite,
		const TArray<int32>& ContestedBySite, const TArray<int32>& TeamBySlot, int32 VictorySteps);

	/** Values to tune in test (D345, the user chooses them), the same as Script/Sites/HOLSiteRules.as (the script test compares them). */
	constexpr int32 DefaultHoldSeconds = 60;
	constexpr int32 DefaultDecaySteps = 2;
	/** S6 (D699): the gold a held site pays its owner by the minute. PROVISIONAL default (small passive income), Retours\Q-sites-or-par-minute.md. */
	constexpr int32 DefaultGoldPerMinute = 30;

	/**
	 * S6 (D699): the whole gold to give the owner of Sites held sites for this step (0 most steps). Carry: the fractions kept from one step to
	 * the next (rate x sites, in gold x steps), so a minute of a site gives exactly its rate whatever the simulation rate (the pattern of
	 * HOLRelay::TerritoryXP). A negative count or rate is nothing. The same as HOLSiteVictory::GoldStep of the script.
	 */
	HEROESOFLIBERTY_API int32 GoldStep(int32& Carry, int32 Sites, int32 RatePerMinute, int32 StepsPerSecond);

	/** S5: the view of a site numbered Index at Location (zone Radius) in State: the share of the capture held (0 to 99) at HoldSteps. */
	HEROESOFLIBERTY_API FHOLSiteView MakeView(int32 Index, const FVector2D& Location, float Radius, const FHOLSiteState& State, int32 HoldSteps);

	/** Both count arrays set to Slots zeros. */
	HEROESOFLIBERTY_API void ResetCounts(TArray<int32>& Takers, TArray<int32>& Disturbers, int32 Slots);

	/**
	 * One living unit of Slot in the zone. bCanTake: a holy unit of its faction, or the hero. bIsPeasant: the worker, who counts for
	 * nothing. A slot out of the arrays is ignored.
	 */
	HEROESOFLIBERTY_API void AddUnit(TArray<int32>& Takers, TArray<int32>& Disturbers, int32 Slot, bool bCanTake, bool bIsPeasant);

	/**
	 * The site after one step. Takers / Disturbers: the units of each slot in the zone; TeamBySlot: the team of each slot (equal teams are
	 * one camp). One adverse camp alone with a taker: the count goes up (from zero if another camp had begun it, Q2 = B) and, full
	 * (HoldSteps), the site goes to the taker. A disturber of the owner's camp, or two adverse camps: frozen. An adverse camp alone with no
	 * taker: freezes a count another camp had begun, lets its own recede. Nobody adverse: it recedes by DecaySteps, to 0.
	 */
	HEROESOFLIBERTY_API FHOLSiteState Step(const FHOLSiteState& State, const TArray<int32>& Takers, const TArray<int32>& Disturbers,
		const TArray<int32>& TeamBySlot, int32 HoldSteps, int32 DecaySteps);
}
