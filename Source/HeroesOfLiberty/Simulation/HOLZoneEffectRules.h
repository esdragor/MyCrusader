// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Pure rules of the faction powers that put a zone on the ground (D859: the Seal of Light of the Dawn). A zone: a disc on the ground,
 * put down at a time, lasting a while, that slows the ENEMY units standing in it. Centimetres and milliseconds are integers (the
 * server runs them, the outcome is compared); no engine state: tested on their own (HeroesOfLiberty.Zone.*) and through the
 * combat processor (HeroesOfLiberty.Zone.Slow). ONE source (D839): the script reads these through UHOLZoneEffectLibrary (HOLZoneEffect:: in script); its former copy (HOLGroundAbilityRules.as) is retired. The cast range (800 cm) and the cool-down (40 s) are the
 * UnitAbilities table's, not here.
 */
namespace HOLZoneEffect
{
	/** D859, Seal of Light: the radius of the zone (6 m), how long it stays (10 s), and how much it slows an enemy (-40 %, so x0.60). */
	constexpr int32 LightSealRadiusCm = 600;
	constexpr int32 LightSealDurationMs = 10000;
	constexpr int32 LightSealSlowPercent = 40;

	/** D689, Sacred Light (S1-a): a zone that HEALS the allies standing in it, continuously, for about 5 s. Radius and rate are provisional defaults (Retours/Q-sacred-light-values.md); the cost (25 Honour) and the cool-down (20 s) are the script's (S1-b). */
	constexpr int32 SacredLightRadiusCm = 600;
	constexpr int32 SacredLightDurationMs = 5000;
	constexpr int32 SacredLightHealPerSecond = 30;

	/** One zone on the ground, put down by OwnerSlot at StartMs. */
	struct FZone
	{
		int32 OwnerSlot = -1;
		int32 CenterXCm = 0;
		int32 CenterYCm = 0;
		int32 RadiusCm = 0;
		int32 StartMs = 0;
		int32 DurationMs = 0;
		int32 SlowPercent = 0;
		/** Health an ALLY of the owner in the zone gets each second (0: not a healing zone; Sacred Light). */
		int32 HealPerSecond = 0;
	};

	/** Is the point inside the disc (the edge counts). */
	HEROESOFLIBERTY_API bool IsInZone(int32 CenterXCm, int32 CenterYCm, int32 RadiusCm, int32 PointXCm, int32 PointYCm);

	/** Is a zone put down at StartMs, lasting DurationMs, active at NowMs (from StartMs, over at StartMs + DurationMs). */
	HEROESOFLIBERTY_API bool ZoneActive(int32 StartMs, int32 DurationMs, int32 NowMs);

	/** The share of its walk, in percent, of a unit: 100 - SlowPercent when an enemy of the zone's owner stands in an active zone, else 100. */
	HEROESOFLIBERTY_API int32 GetSpeedPercent(bool bEnemyOfOwner, bool bInZone, bool bActive, int32 SlowPercent);
}
