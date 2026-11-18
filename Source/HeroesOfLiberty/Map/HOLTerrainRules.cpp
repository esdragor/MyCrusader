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

bool FHOLTerrainRules::IsInZoneOfKind(const TConstArrayView<FHOLTerrainZone> Zones, const int32 Kind, const int32 X, const int32 Y)
{
	for (const FHOLTerrainZone& Zone : Zones)
	{
		if (Zone.Kind == Kind && Contains(Zone, X, Y))
		{
			return true;
		}
	}
	return false;
}

int32 FHOLTerrainRules::GetForestSpeedPercent(const int32 Category, const bool bInForest, const int32 CavalryPercent, const int32 SiegePercent)
{
	if (!bInForest)
	{
		return 100;
	}
	if (Category == HOLTerrainWalker::Cavalry)
	{
		return CavalryPercent;
	}
	if (Category == HOLTerrainWalker::Siege)
	{
		return SiegePercent;
	}
	return 100;
}

int32 FHOLTerrainRules::GetWalkerSpeedPercentAt(const int32 MarshSpeedPercent, const TConstArrayView<FHOLTerrainZone> Zones, const int32 Category, const int32 X, const int32 Y, const int32 RoadSpeedPercent)
{
	const int32 ZonePercent = GetSpeedPercentAt(MarshSpeedPercent, Zones, X, Y, RoadSpeedPercent);
	const int32 ForestPercent = GetForestSpeedPercent(Category, IsInZoneOfKind(Zones, HOLTerrainKind::Forest, X, Y));
	return FMath::Min(ZonePercent, ForestPercent);
}

int32 FHOLTerrainRules::GetFordDamageTakenPercent(const bool bOnFord, const int32 FordPercent)
{
	return bOnFord ? FordPercent : 100;
}

int32 FHOLTerrainRules::GetFordDamageTaken(const int32 Damage, const bool bOnFord, const int32 FordPercent)
{
	return Damage <= 0 ? 0 : Damage * GetFordDamageTakenPercent(bOnFord, FordPercent) / 100;
}

bool FHOLTerrainRules::StopsCharge(const bool bMarshStopsCharge, const TConstArrayView<FHOLTerrainZone> Zones, const int32 X, const int32 Y)
{
	return bMarshStopsCharge && IsInZoneOfKind(Zones, HOLTerrainKind::Marsh, X, Y);
}