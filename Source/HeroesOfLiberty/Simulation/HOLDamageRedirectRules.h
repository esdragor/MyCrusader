// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * NOT COMPILED (SAURON-1120f, wired by SAURON-1120pc). Aube A2 (AubeInProgress.md), D19 *Defi du Chevalier* passive: part of the damage an allied unit
 * takes near a Virtuous Knight is redirected to the Knight. THE single source of the rule (D839): the combat processor (Mass, per entity and per step)
 * reads this namespace (UHOLCombatProcessor::GetDamageRedirect / ApplyDamageRedirect, at the melee blow, the arrow that lands and the splash), there is
 * no script twin. Pure: no world, no fragment, no random.
 *
 * Decided: heroes are neither protected nor redirected, and the Knight does not redirect its own damage (GDD 1121-1128); D864 = several Knights in
 * range: the Nearest mode (DefaultCumulMode); D865 = the percent (30) and the radius (800 cm) are the unit table's columns DamageRedirectPercent /
 * DamageRedirectRadius (HOLUnitTypes.h, row VirtuousKnight). The percent, the radius and the mode stay PARAMETERS of Redirect (the processor passes
 * the table's numbers): the CappedSum mode is kept for the test and as the alternative of D864, not used by the game.
 */
namespace HOLDamageRedirect
{
	/** What several Knights in range of the same ally do together (Q-aube-a2-cumul.md). */
	enum class ECumulMode : uint8
	{
		/** Only the nearest Knight in range redirects its percent (ties: the lowest Id). Recommended default. */
		Nearest,
		/** Every Knight in range adds its percent; the sum is capped at CapPercent and shared equally between them. */
		CappedSum,
	};

	/** Placeholder default behind one line (not a game value): see Q-aube-a2-cumul.md. */
	constexpr ECumulMode DefaultCumulMode = ECumulMode::Nearest;

	/** A Knight candidate, positions in centimetres (as the zone rules). */
	struct FKnight
	{
		int32 Id = 0;
		int32 XCm = 0;
		int32 YCm = 0;
		bool bAlive = true;
	};

	/** The part of one blow that goes to one Knight. */
	struct FShare
	{
		int32 KnightId = 0;
		float Damage = 0.f;
	};

	struct FResult
	{
		/** What the struck ally still takes. Kept + the sum of the shares = the damage of the blow (conservation). */
		float Kept = 0.f;
		TArray<FShare, TInlineAllocator<4>> Shares;
	};

	/**
	 * Who takes which part of a blow of Damage dealt to an ally at (AllyXCm, AllyYCm) with id AllyId.
	 * - a hero ally (bAllyIsHero) or a non-positive Damage: nothing is redirected;
	 * - a Knight redirects only if alive, not the struck unit itself, and the ally is inside RadiusCm (edge included);
	 * - PercentPerKnight is clamped to 0..100 (100 = the active version, everything); CapPercent (CappedSum only) is clamped to 0..100.
	 */
	HEROESOFLIBERTY_API FResult Redirect(float Damage, bool bAllyIsHero, int32 AllyId, int32 AllyXCm, int32 AllyYCm, TConstArrayView<FKnight> Knights,
		int32 RadiusCm, int32 PercentPerKnight, ECumulMode Mode, int32 CapPercent);
}
