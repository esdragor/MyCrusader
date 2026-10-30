// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HOLHeroAbilityRules.generated.h"

/**
 * The Paladin-Commander's kit at the prototype (D42, D52): a passive aura, two abilities cast by the player. BlueprintType:
 * the script's hero block casts them (J8c.3, UHOLHeroLibrary). The index of the private sheet's CooldownEndSteps.
 */
UENUM(BlueprintType)
enum class EHOLHeroAbility : uint8
{
	/** Aura de l'Aube (J8c-Q1): passive, the hero's units near him take less damage. */
	Aura,
	/** Serrez les rangs (J8c-Q2): a cry, the units near him take less damage for a while. */
	CloseRanks,
	/** Bannière de l'Aube (J8c-Q3): planted on a point, it gives the aura there for a while. */
	Banner,
};

/**
 * Pure rules of the hero's aura and abilities (J8c.1; J8c-Q1 to Q3): who is covered, how much damage a covered unit
 * takes, when an ability can be cast, its cool-down and duration on the fixed step. Morale effects (D74): an effect does
 * not stack with itself, different effects add up under one cap. Percents and milliseconds are integers (the server runs
 * them, the outcome is compared); no engine state: tested on their own (HeroesOfLiberty.HeroAbility.*). Values
 * indicative, to be tuned in test.
 */
namespace HOLHeroAbility
{
	/** Reach of the aura, of the cry and of the banner's aura (J8c-Q1 to Q3): 3 tiles (12 m), as the command (J8b-Q1). */
	inline constexpr float AuraRadius = 1200.f;

	/** Farthest point from the hero a banner can be planted on (J8c-Q3): 4 tiles (16 m). */
	inline constexpr float BannerPlantRange = 1600.f;

	/** Damage taken less in the aura of the hero or of a banner (J8c-Q1), in percent. */
	constexpr int32 AuraReductionPercent = 15;

	/** Damage taken less under the cry (J8c-Q2), in percent. */
	constexpr int32 CloseRanksReductionPercent = 25;

	/** Cap of the morale effects' damage reduction, all added up (D74, J8c-Q2), in percent. */
	constexpr int32 MoraleReductionCapPercent = 50;

	/** How long the cry covers the units it reached (J8c-Q2). */
	constexpr int32 CloseRanksDurationMs = 15000;

	/** How long a banner stands (J8c-Q3). */
	constexpr int32 BannerDurationMs = 45000;

	/** The hero's level an ability opens at (D52: aura and cry at 1, banner at 4). */
	HEROESOFLIBERTY_API int32 GetUnlockLevel(EHOLHeroAbility Ability);

	/** Whether a hero of Level has Ability. */
	HEROESOFLIBERTY_API bool IsUnlocked(EHOLHeroAbility Ability, int32 Level);

	/** The cool-down an ability starts when cast (J8c-Q2: cry 60 s, J8c-Q3: banner 120 s); 0 for the passive aura. */
	HEROESOFLIBERTY_API int32 GetCooldownMs(EHOLHeroAbility Ability);

	/**
	 * Whether the player can cast Ability now: the hero alive (§ 9.8), the ability open at his level, an active one (not the
	 * aura) and its cool-down over.
	 */
	HEROESOFLIBERTY_API bool CanCast(EHOLHeroAbility Ability, int32 Level, bool bHeroAlive, int32 CooldownMs);

	/** Whether a banner can be planted Distance cm from the hero (BannerPlantRange, edge included). */
	HEROESOFLIBERTY_API bool CanPlantBanner(float Distance);

	/** Whether a unit Distance cm from an aura's source (hero, banner) or from the crying hero is covered (edge included). */
	HEROESOFLIBERTY_API bool IsWithinAura(float Distance);

	/** Counts a cool-down or an effect's time down by one step, to 0 at least. */
	HEROESOFLIBERTY_API void Tick(int32& RemainingMs, int32 StepMs);

	/**
	 * The share of a hit a unit takes, in percent (100: all of it). bInAura: within the aura of its living hero or of one of
	 * its player's banners (one aura counts, however many cover it); bCloseRanks: under the cry. The reductions add up,
	 * MoraleReductionCapPercent at most.
	 */
	HEROESOFLIBERTY_API int32 GetDamageTakenPercent(bool bInAura, bool bCloseRanks);

	/** A hit's damage after the morale effects (DamageTakenPercent of it), unrounded: health is a float. */
	HEROESOFLIBERTY_API float GetReducedDamage(float Damage, int32 DamageTakenPercent);
}
