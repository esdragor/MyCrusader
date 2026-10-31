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

	/** The speed, in percent of normal, of a walker standing at (X, Y): the slowest of the zones that hold the point, 100 outside all. */
	static int32 GetSpeedPercentAt(int32 MarshSpeedPercent, TConstArrayView<FHOLTerrainZone> Zones, int32 X, int32 Y);

	/** A speed (cm/s) scaled by a percent, never below 1 cm/s for a walker that walks. */
	static int32 GetScaledSpeed(int32 Speed, int32 Percent);
};
