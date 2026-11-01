// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map/HOLTerrainRules.h"

bool FHOLTerrainRules::Contains(const FHOLTerrainZone& Zone, const int32 X, const int32 Y)
{
	const int64 DX = static_cast<int64>(X) - Zone.X;
	const int64 DY = static_cast<int64>(Y) - Zone.Y;
	const int64 R = Zone.Radius;
	return DX * DX + DY * DY <= R * R;
}

int32 FHOLTerrainRules::GetSpeedPercentAt(const int32 MarshSpeedPercent, const TConstArrayView<FHOLTerrainZone> Zones, const int32 X, const int32 Y, const int32 RoadSpeedPercent)
{
	int32 Percent = 100;
	bool bOnRoad = false;
	for (const FHOLTerrainZone& Zone : Zones)
	{
		if (Zone.Kind == HOLTerrainKind::Marsh && MarshSpeedPercent < Percent && Contains(Zone, X, Y))
		{
			Percent = MarshSpeedPercent;
		}
		else if (Zone.Kind == HOLTerrainKind::Road && !bOnRoad && Contains(Zone, X, Y))
		{
			bOnRoad = true;
		}
	}
	return Percent == 100 && bOnRoad ? RoadSpeedPercent : Percent;
}

int32 FHOLTerrainRules::GetScaledSpeed(const int32 Speed, const int32 Percent)
{
	if (Speed <= 0)
	{
		return 0;
	}
	return FMath::Max(Speed * Percent / 100, 1);
}
