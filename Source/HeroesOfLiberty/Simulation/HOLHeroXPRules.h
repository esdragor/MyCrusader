// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct FHOLResources;

/** Where a hero's XP comes from (J8b-Q1, D05): each source has its weight (J8b-Q2). Exploration has no carrier yet. */
enum class EHOLXPSource : uint8
{
	/** A unit (or a hero) the hero killed himself. */
	Combat,
	/** An enemy killed by his troops near him (CommandRadius). */
	Command,
	/** One of his player's buildings finished. */
	Development,
	/** Exploration and territory (relays, secondary centres, fog: later). */
	Exploration,
};

/**
 * Pure rules of the hero's XP and levels (J8b.1; J8b-Q1 to Q3, Q5): what a kill or a building is worth, the levels 1 to 6
 * (D04, prototype), the stats a level gives. Integers where the outcome is compared (XP, levels), no engine state: tested
 * on their own (HeroesOfLiberty.HeroXP.*). Values indicative, to be tuned in test ([Q04]).
 */
namespace HOLHeroXP
{
	/** Prototype levels (D04): 1 to 6. */
	constexpr int32 MaxLevel = 6;

	/** Command (J8b-Q1): an enemy killed by his troops within 3 tiles (12 m) of the living hero. */
	inline constexpr float CommandRadius = 1200.f;

	/** XP for a hero killed (J8b-Q5), before the levels' ratio and the source's weight. */
	constexpr int32 HeroKillXP = 300;

	/** The source's weight in percent (J8b-Q2: military > economy > exploration): 100, 75, 50, 25. */
	HEROESOFLIBERTY_API int32 GetWeightPercent(EHOLXPSource Source);

	/** What something is worth before the weight: its cost, the four resources added up. */
	HEROESOFLIBERTY_API int32 GetValue(const FHOLResources& Cost);

	/** XP of a kill or a building worth Value through Source (rounded down). */
	HEROESOFLIBERTY_API int32 GetXP(int32 Value, EHOLXPSource Source);

	/** XP of a hero of VictimLevel killed by the side of a hero of KillerLevel (D05 catch-up), through Source (rounded down). */
	HEROESOFLIBERTY_API int32 GetHeroKillXP(int32 VictimLevel, int32 KillerLevel, EHOLXPSource Source);

	/** D06: development and exploration still count while the hero is dead; combat and command stop. */
	HEROESOFLIBERTY_API bool CountsWhileDead(EHOLXPSource Source);

	/**
	 * The source of an enemy's death for a player's hero, or none: killed by the hero (Combat), or by one of his troops
	 * within CommandRadius of him (Command); nothing while he is dead (D06) or farther.
	 */
	HEROESOFLIBERTY_API TOptional<EHOLXPSource> GetKillSource(bool bKilledByHero, bool bHeroAlive, float DistanceToHero);

	/** J8b.2: what the simulation knows of a death, for the killer's side hero (GetKillXP). */
	struct FKillFacts
	{
		/** The victim's cost (GetValue), for a unit that is not a hero. */
		int32 VictimValue = 0;
		bool bVictimHero = false;
		/** The victim hero's level, and the killer side's hero's (D05 catch-up). */
		int32 VictimLevel = 1;
		int32 HeroLevel = 1;
		bool bKilledByHero = false;
		bool bHeroAlive = false;
		/** From the victim to the hero, in cm. */
		float DistanceToHero = MAX_flt;
	};

	/**
	 * J8b.2: the XP a death gives the killer side's hero and its source (GetKillSource): a unit its cost x the weight, a
	 * hero GetHeroKillXP; 0 and no source when it gives none (hero dead, troops too far).
	 */
	HEROESOFLIBERTY_API int32 GetKillXP(const FKillFacts& Facts, TOptional<EHOLXPSource>& OutSource);

	/** Total XP to reach Level (J8b-Q2): 0, 200, 600, 1 300, 2 200, 3 300; levels outside 1..MaxLevel are clamped. */
	HEROESOFLIBERTY_API int32 GetLevelXP(int32 Level);

	/** The level of a total XP: the highest one reached, MaxLevel at most. */
	HEROESOFLIBERTY_API int32 GetLevel(int32 XP);

	/** Stats at a level in percent of level 1's (J8b-Q3): +8 % a level, 140 at level 6. */
	HEROESOFLIBERTY_API int32 GetStatPercent(int32 Level);

	/** A level 1 stat (max HP, attack) at Level, unrounded (health and damage are floats). */
	HEROESOFLIBERTY_API float GetLevelStat(float Level1Stat, int32 Level);

	/** Health after a level up from OldLevel to NewLevel (J8b-Q3): the max HP gained is added to the current health. */
	HEROESOFLIBERTY_API float GetHealthAfterLevelUp(float Health, float Level1MaxHealth, int32 OldLevel, int32 NewLevel);
}
