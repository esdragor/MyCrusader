// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map/HOLTerrainReadLibrary.h"

#include "Map/HOLTerrainRules.h"

namespace
{
	/** The flat quadruples (Kind, X, Y, Radius) as the zones of the rule; an incomplete last quadruple is dropped. */
	TArray<FHOLTerrainZone> ToZones(const TArray<int32>& Quadruples)
	{
		TArray<FHOLTerrainZone> Zones;
		for (int32 Index = 0; Index + 3 < Quadruples.Num(); Index += 4)
		{
			FHOLTerrainZone Zone;
			Zone.Kind = Quadruples[Index];
			Zone.X = Quadruples[Index + 1];
			Zone.Y = Quadruples[Index + 2];
			Zone.Radius = Quadruples[Index + 3];
			Zones.Add(Zone);
		}
		return Zones;
	}
}

int32 UHOLTerrainReadLibrary::GetMarshKind() { return HOLTerrainKind::Marsh; }
int32 UHOLTerrainReadLibrary::GetRoadKind() { return HOLTerrainKind::Road; }
int32 UHOLTerrainReadLibrary::GetForestKind() { return HOLTerrainKind::Forest; }
int32 UHOLTerrainReadLibrary::GetFordKind() { return HOLTerrainKind::Ford; }
int32 UHOLTerrainReadLibrary::GetWalkerOnFoot() { return HOLTerrainWalker::OnFoot; }
int32 UHOLTerrainReadLibrary::GetWalkerCavalry() { return HOLTerrainWalker::Cavalry; }
int32 UHOLTerrainReadLibrary::GetWalkerSiege() { return HOLTerrainWalker::Siege; }
int32 UHOLTerrainReadLibrary::GetDefaultMarshSpeedPercent() { return FHOLTerrainRules::DefaultMarshSpeedPercent; }
int32 UHOLTerrainReadLibrary::GetDefaultRoadSpeedPercent() { return FHOLTerrainRules::DefaultRoadSpeedPercent; }
int32 UHOLTerrainReadLibrary::GetDefaultForestCavalrySpeedPercent() { return FHOLTerrainRules::DefaultForestCavalrySpeedPercent; }
int32 UHOLTerrainReadLibrary::GetDefaultForestSiegeSpeedPercent() { return FHOLTerrainRules::DefaultForestSiegeSpeedPercent; }
int32 UHOLTerrainReadLibrary::GetDefaultFordDamageTakenPercent() { return FHOLTerrainRules::DefaultFordDamageTakenPercent; }

int32 UHOLTerrainReadLibrary::GetForestSpeedPercent(const int32 Category, const bool bInForest, const int32 CavalryPercent, const int32 SiegePercent)
{
	return FHOLTerrainRules::GetForestSpeedPercent(Category, bInForest, CavalryPercent, SiegePercent);
}

int32 UHOLTerrainReadLibrary::GetWalkerSpeedPercentAt(const int32 MarshPercent, const TArray<int32>& ZoneQuadruples, const int32 Category, const int32 X, const int32 Y, const int32 RoadPercent)
{
	return FHOLTerrainRules::GetWalkerSpeedPercentAt(MarshPercent, ToZones(ZoneQuadruples), Category, X, Y, RoadPercent);
}

bool UHOLTerrainReadLibrary::IsInZoneOfKind(const TArray<int32>& ZoneQuadruples, const int32 Kind, const int32 X, const int32 Y)
{
	return FHOLTerrainRules::IsInZoneOfKind(ToZones(ZoneQuadruples), Kind, X, Y);
}

int32 UHOLTerrainReadLibrary::GetFordDamageTakenPercent(const bool bOnFord, const int32 FordPercent)
{
	return FHOLTerrainRules::GetFordDamageTakenPercent(bOnFord, FordPercent);
}

int32 UHOLTerrainReadLibrary::GetFordDamageTaken(const int32 Damage, const bool bOnFord, const int32 FordPercent)
{
	return FHOLTerrainRules::GetFordDamageTaken(Damage, bOnFord, FordPercent);
}

bool UHOLTerrainReadLibrary::StopsCharge(const bool bMarshStopsCharge, const TArray<int32>& ZoneQuadruples, const int32 X, const int32 Y)
{
	return FHOLTerrainRules::StopsCharge(bMarshStopsCharge, ToZones(ZoneQuadruples), X, Y);
}
