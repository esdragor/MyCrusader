// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Buildings/HOLProductionRules.h"
#include "Technologies/HOLTechnologyTypes.h"
#include "HOLTechnologyRules.generated.h"

enum class EHOLAttackType : uint8;

/**
 * The technologies a player has researched (T3, GDD 11): the names of the rows of the technology table, in the order they
 * were finished. Set by the server alone (the production at the end of a research), replicated to its player only (like the
 * stock, D203: nobody sees another player's technologies). What is being researched is not kept here: it is an item of the
 * production queue of the building (T-Q6 = A, D583: one queue of 5 for units and technologies, the same code and interface).
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLTechnologyState
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FName> Done;

	bool Has(const FName Technology) const { return Done.Contains(Technology); }

	/** True when it was added; a technology is researched once. */
	bool Add(FName Technology);

	bool operator==(const FHOLTechnologyState& Other) const = default;
};

/**
 * Pure rules of the technologies (T3, T4, GDD 11, D27, D180, T-Q1 to T-Q6 = A, D583), applied by the server when a research
 * is ordered (HOLProduction's queue is the research queue) and read by the grid (T4) and the AI (T7). The same order of
 * reasons as Script/Technologies/HOLTechnologyRules.as, which the interface and the AI read in script (D483).
 */
namespace HOLTechnology
{
	/** What stops a technology, the first reason in this order is the one given to the player; None when nothing does. */
	enum class EBlock : uint8
	{
		None,
		/** No code applies its effect yet (the table's Implemented is false): greyed with "Soon" (T-Q11 = B, D601), before every other reason. */
		NotImplemented,
		/** Researched already (a technology is researched once). */
		AlreadyResearched,
		/** Already in a production queue of its owner (one research of it at a time). */
		AlreadyQueued,
		/** The hero has not opened the tier yet. */
		TierTooLow,
		/** The tier is open but the row asks a higher hero level (MinHeroLevel, T-Q1: 0 in the table, the tier only). */
		HeroLevelTooLow,
		/** The technology it follows (level I of a line) is not researched yet. */
		RequiresMissing,
		/** No finished building of the type that researches it. */
		NoBuilding,
		/** The stock does not cover the cost. */
		CannotPay,
	};

	/** What the caller knows of the player and the building when a research is ordered. */
	struct FFacts
	{
		/** The tier the hero's level has opened (HOLHeroXP::GetTier), 0 without a hero. */
		int32 HeroTier = 0;
		int32 HeroLevel = 0;
		/** The row asks for no other technology, or that one is researched. */
		bool bRequiresDone = true;
		/** The building is of the type the row names, finished and the player's. */
		bool bBuildingFinished = false;
		bool bCanPay = false;
		bool bResearched = false;
		bool bQueued = false;
	};

	HEROESOFLIBERTY_API EBlock Check(const FHOLTechnologyRow& Row, const FFacts& Facts);

	/** The words of a block for the logs ("tier too low", "requires missing"...), the same as the server's. */
	HEROESOFLIBERTY_API const TCHAR* DescribeBlock(EBlock Block);

	/** True when a technology can be researched in the game as it is: its building is a type that exists (T-Q4 = A) and some code applies its effect (Implemented, T-Q11 = B). */
	HEROESOFLIBERTY_API bool IsPlayable(const FHOLTechnologyRow& Row, TFunctionRef<bool(FName)> BuildingTypeExists);

	/** What a research takes as an item of the production queue: its cost, its time in steps (at least 1), no population place. */
	HEROESOFLIBERTY_API FHOLProductionCost MakeCost(const FHOLTechnologyRow& Row, float StepSeconds);

	/**
	 * T5: the percent a statistic gains from the technologies Done. The Percent of a row is the TOTAL of its line (T-Q2: level II
	 * = level I and more), so each technology adds its Percent minus the one of the technology it follows when that one is done:
	 * level I and II together give level II's percent, not the sum. Lines of different technologies on one statistic add up.
	 */
	HEROESOFLIBERTY_API int32 GetBonusPercent(const TArray<FName>& Done, FName Stat, TFunctionRef<const FHOLTechnologyRow*(FName)> Find);

	/** The statistics the Wheelbarrow changes (T5, GDD 11.1: speed and capacity of the workers). */
	inline const FName WorkerCarryStat(TEXT("WorkerCarry"));
	inline const FName WorkerSpeedStat(TEXT("WorkerSpeed"));

	/** T6: the statistics the Forge's attack technologies change (MeleeAttack, RangedAttack of the table): a hit's base attack, by the attack's type. */
	inline const FName MeleeAttackStat(TEXT("MeleeAttack"));
	inline const FName RangedAttackStat(TEXT("RangedAttack"));

	/** T6: the statistic that raises the hits of this attack type (Melee, Ranged), NAME_None for the others (siege, hero: no technology yet). */
	HEROESOFLIBERTY_API FName GetAttackStat(EHOLAttackType AttackType);

	/** Q-chapelle-moine, Vow of Poverty (GDD 11.1): the statistic that lowers the cost of this unit type (MonkCostDiscount for the Monk), NAME_None for the others. */
	HEROESOFLIBERTY_API FName GetCostDiscountStat(FName UnitType);

	/** A unit's resource cost lowered by Percent (0-90): each resource Cost * (100 - Percent) / 100, rounded to the nearest whole unit (a free resource stays free). */
	HEROESOFLIBERTY_API FHOLResources GetDiscountedCost(const FHOLResources& Cost, int32 Percent);

	/** A whole quantity (a carrying capacity) raised by Percent: Base + Base * Percent / 100, rounded to the nearest whole unit, never below Base. */
	HEROESOFLIBERTY_API int32 ScaleByPercent(int32 Base, int32 Percent);

	/** The statistic a gathering technology changes for a resource (GatherFood...). */
	HEROESOFLIBERTY_API FName GetGatherStat(EHOLResource Resource);

	/** True when a queue holds an item for this technology, started or not. */
	HEROESOFLIBERTY_API bool IsQueued(const TArray<FHOLProductionItem>& Queue, FName Technology);
}
