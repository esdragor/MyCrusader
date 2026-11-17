// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * C3 (MapsInProgress.md, GDD 5.4 D33, M-Q2 = A: only the visible effects): one round zone of the map, in whole centimetres
 * (the server and the clients agree to the centimetre). Kind: HOLTerrainKind.
 */
struct FHOLTerrainZone
{
	int32 Kind = 0;
	int32 X = 0;
	int32 Y = 0;
	int32 Radius = 0;
};

namespace HOLTerrainKind
{
	/** A marsh slows every walker (and, later, stops a charge, D345.3). */
	constexpr int32 Marsh = 0;
	/** A road speeds every walker up (C5, La Forêt profonde, CQ6a = A: 130 %); a marsh under it wins (the slowing zone first). */
	constexpr int32 Road = 1;
	/** D810 (Q-forest-ford Q1 = A): a forest the map paints; cavalry and siege walk slower in it (F4). 2 was the removed cover zone (D796), not reused. */
	constexpr int32 Forest = 3;
	/** D810 (Q-forest-ford Q1 = A): a ford the map paints; a unit standing in it takes more damage (F5, its defence reduced). */
	constexpr int32 Ford = 4;
}

/** D810: the walker categories the forest tells apart (F4). On foot are never slowed by a forest. */
namespace HOLTerrainWalker
{
	constexpr int32 OnFoot = 0;
	constexpr int32 Cavalry = 1;
	constexpr int32 Siege = 2;
}

/**
 * The speed rule of the terrain zones: pure functions of a list of zones and the marsh's strength. The same rule as
 * Script/Terrain/HOLTerrainRules.as (the map's tests read that one); a script test checks both agree (HOLTerrain::ServerSpeedPercent).
 */
struct HEROESOFLIBERTY_API FHOLTerrainRules
{
	/**
	 * A walker's speed in a marsh, in percent of its normal speed, until the user settles CQ2a (Auto_Journey_Retours.md): 60, the
	 * recommendation (preset 0 of the script). hol.Terrain.MarshSpeedPercent sets it on a server.
	 */
	static constexpr int32 DefaultMarshSpeedPercent = 60;

	/** Whether the point is in the zone (its edge counts in). */
	static bool Contains(const FHOLTerrainZone& Zone, int32 X, int32 Y);

	/** A walker's speed on a road, in percent of its normal speed (C5, CQ6a = A: 130). hol.Terrain.RoadSpeedPercent sets it on a server. */
	static constexpr int32 DefaultRoadSpeedPercent = 130;

	/**
	 * The speed, in percent of normal, of a walker standing at (X, Y): the slowest of the slowing zones that hold the point; else the
	 * road's percent if a road zone holds it; 100 outside all.
	 */
	static int32 GetSpeedPercentAt(int32 MarshSpeedPercent, TConstArrayView<FHOLTerrainZone> Zones, int32 X, int32 Y, int32 RoadSpeedPercent = DefaultRoadSpeedPercent);

	/** A speed (cm/s) scaled by a percent, never below 1 cm/s for a walker that walks. */
	static int32 GetScaledSpeed(int32 Speed, int32 Percent);

	/** D810 F4 (Q-forest-ford Q2 = A): the speed of a cavalry walker in a forest, in percent of normal. */
	static constexpr int32 DefaultForestCavalrySpeedPercent = 80;
	/** D810 F4: the speed of a siege engine in a forest, in percent. */
	static constexpr int32 DefaultForestSiegeSpeedPercent = 70;
	/** D810 F5: the damage a unit standing on a ford takes, in percent of the hit. */
	static constexpr int32 DefaultFordDamageTakenPercent = 120;

	/** Whether the point is in at least one zone of the kind (HOLTerrainKind; the zone's edge counts in). */
	static bool IsInZoneOfKind(TConstArrayView<FHOLTerrainZone> Zones, int32 Kind, int32 X, int32 Y);

	/** D810 F4: the speed, in percent of normal, of a walker of the category (HOLTerrainWalker) in or out of a forest; on foot and outside a forest 100. */
	static int32 GetForestSpeedPercent(int32 Category, bool bInForest, int32 CavalryPercent = DefaultForestCavalrySpeedPercent, int32 SiegePercent = DefaultForestSiegeSpeedPercent);

	/**
	 * D810: the speed percent of a walker of the category at (X, Y), all zones together: GetSpeedPercentAt (marsh, road), then, when a
	 * forest zone holds the point, the slowest of it and the forest's percent for the category (a forest slows like a marsh: slowest wins).
	 */
	static int32 GetWalkerSpeedPercentAt(int32 MarshSpeedPercent, TConstArrayView<FHOLTerrainZone> Zones, int32 Category, int32 X, int32 Y, int32 RoadSpeedPercent = DefaultRoadSpeedPercent);

	/** D810 F5: the percent of a hit a unit takes, on a ford or not (100 off a ford); composes with the other damage-taken percents. */
	static int32 GetFordDamageTakenPercent(bool bOnFord, int32 FordPercent = DefaultFordDamageTakenPercent);

	/** D810 F5: the damage a unit takes from a hit of Damage, on a ford or not; never below 0. */
	static int32 GetFordDamageTaken(int32 Damage, bool bOnFord, int32 FordPercent = DefaultFordDamageTakenPercent);

	/** D810 (GDD 5.4, D33, D345.3): whether a charge is stopped where the unit stands: a marsh holds the point, when the setting says so. */
	static bool StopsCharge(bool bMarshStopsCharge, TConstArrayView<FHOLTerrainZone> Zones, int32 X, int32 Y);
};
