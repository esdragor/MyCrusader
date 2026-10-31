// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Buildings/HOLProductionRules.h"
#include "Technologies/HOLTechnologyTypes.h"
#include "HOLTechnologyRules.generated.h"

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

	/** True when a technology can be researched in the game as it is: its building is a type that exists (T-Q4 = A). */
	HEROESOFLIBERTY_API bool IsPlayable(const FHOLTechnologyRow& Row, TFunctionRef<bool(FName)> BuildingTypeExists);

	/** What a research takes as an item of the production queue: its cost, its time in steps (at least 1), no population place. */
	HEROESOFLIBERTY_API FHOLProductionCost MakeCost(const FHOLTechnologyRow& Row, float StepSeconds);

	/** True when a queue holds an item for this technology, started or not. */
	HEROESOFLIBERTY_API bool IsQueued(const TArray<FHOLProductionItem>& Queue, FName Technology);
}
