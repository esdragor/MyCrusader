// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * L4 step 5 (LegionsInProgress.md, LQ-3 / D678): the Damned Warrior's necrotic rage in the combat simulation. The numbers are those of
 * Tools/Data/UnitAbilities.csv (NecroticRage) and Tools/Data/Effects.csv (NecroticRage), answer Q2 = A of the 2026-11-09 session; the
 * Angelscript rules (Script/Units/HOLUnitAbilityRules.as) read the same CSV. The combat processor cannot call the script, so these
 * constants are a copy: change both places. The morale cap (Q-legions-rage-cap.md) does not touch the rage here: the combat has no
 * morale call, so the +40 % is applied in full until the user decides.
 */
namespace HOLUnitAbility
{
	/** The unit type that casts the rage. */
	inline const FName RageUnitType(TEXT("DamnedWarrior"));

	constexpr int32 RageDurationMs = 10000;
	constexpr int32 RageCooldownMs = 40000;
	constexpr int32 RageHpCostPerHit = 3;
	constexpr int32 RageMinHp = 30;
	constexpr int32 RageStopHp = 20;
	constexpr int32 RageDamagePercent = 40;
	constexpr int32 RageRatePercent = 25;

	/** Can the rage be cast now: alive, at MinHp or more, cool-down over (counted from the start of the last cast). */
	inline bool CanCastRage(const bool bAlive, const float Health, const int32 CooldownMs)
	{
		return bAlive && Health >= static_cast<float>(RageMinHp) && CooldownMs <= 0;
	}

	/** The hit points a blow dealt under the rage costs: the cost, never taking the unit under StopHp. */
	inline float GetRageHitCost(const float Health)
	{
		const float Room = Health - static_cast<float>(RageStopHp);
		return Room <= 0.f ? 0.f : FMath::Min(Room, static_cast<float>(RageHpCostPerHit));
	}

	/** The attack interval under the rage: 25 % more blows in the same time. */
	inline int32 GetRageIntervalMs(const int32 IntervalMs)
	{
		return FMath::Max(1, IntervalMs * 100 / (100 + RageRatePercent));
	}
}
