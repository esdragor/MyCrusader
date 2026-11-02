// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Pure rules of the siege engines (SiegeInProgress.md S1b, GDD 6.1, D23, D77, D180): which engine opens at which tier, which are
 * in the prototype, and why an engine cannot be queued now. The C++ copy of Script/Siege/HOLSiegeRules.as (the simulation reads
 * C++, the interface and the AI read the script, D483): the two are tested with the same cases, and this one against the real
 * technology table (Tests/HOLSiegeTests.cpp). Integers and booleans only, no world access. The stats of an engine (health, damage,
 * cost) are not here: they belong to the tables (S2).
 */
namespace HOLSiege
{
	/** The engines of GDD 6.1, in the order of their tier (the Cracheur de bile and the Bombarde are the Cannon's role, D59). */
	enum class EEngine : uint8
	{
		Ram,
		Mangonel,
		Trebuchet,
		Cannon,
	};

	/** What stops an engine, the first reason in this order is the one given to the player; None when nothing does. */
	enum class EBlock : uint8
	{
		None,
		/** Not one of the prototype's two engines (D77). */
		NotInPrototype,
		/** The hero has not opened the engine's tier yet. */
		TierTooLow,
		/** No finished siege workshop of the player's. */
		NoWorkshop,
		/** The stock does not cover the cost. */
		CannotPay,
	};

	/** The tier an engine opens at: Ram 1, Mangonel 2, Trebuchet 3, Cannon 3 (GDD 6.1; the workshop is itself a tier 1 building). */
	HEROESOFLIBERTY_API int32 GetTier(EEngine Engine);

	/** True for the prototype's siege: the Ram and the Mangonel (D77). */
	HEROESOFLIBERTY_API bool InPrototype(EEngine Engine);

	/** Why an engine cannot be queued now, or None. HeroTier: the tier the hero's level has opened (HOLHeroXP::GetTier). */
	HEROESOFLIBERTY_API EBlock Check(EEngine Engine, int32 HeroTier, bool bWorkshopFinished, bool bCanPay);

	/** The engine a unit type is, by the name of its row (Ram, Mangonel, Trebuchet, Cannon); false for any other type (S3). */
	HEROESOFLIBERTY_API bool FindEngine(FName TypeName, EEngine& OutEngine);

	/** S5: the technology statistic that raises this type's health (Ram: RamHealth), NAME_None for any other type. */
	HEROESOFLIBERTY_API FName GetHealthStat(FName TypeName);

	/** S5: the technology statistic that raises this type's damage (Mangonel: MangonelDamage), NAME_None for any other type. */
	HEROESOFLIBERTY_API FName GetDamageStat(FName TypeName);

	/** The words of a block in the log of the production ("tier too low"), the same family as HOLTechnology::DescribeBlock (S3). */
	HEROESOFLIBERTY_API const TCHAR* DescribeBlock(EBlock Block);
}
