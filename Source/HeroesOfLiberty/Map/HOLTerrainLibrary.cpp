// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map/HOLTerrainLibrary.h"

#include "Map/HOLTerrainRules.h"

int32 UHOLTerrainLibrary::ServerSpeedPercent(const int32 MarshPercent, const int32 ZoneX, const int32 ZoneY, const int32 ZoneRadius, const int32 X, const int32 Y)
{
	FHOLTerrainZone Zone;
	Zone.Kind = HOLTerrainKind::Marsh;
	Zone.X = ZoneX;
	Zone.Y = ZoneY;
	Zone.Radius = ZoneRadius;
	return FHOLTerrainRules::GetSpeedPercentAt(MarshPercent, MakeArrayView(&Zone, 1), X, Y);
}

int32 UHOLTerrainLibrary::ServerScaledSpeed(const int32 Speed, const int32 Percent)
{
	return FHOLTerrainRules::GetScaledSpeed(Speed, Percent);
}

int32 UHOLTerrainLibrary::GetServerDefaultMarshPercent()
{
	return FHOLTerrainRules::DefaultMarshSpeedPercent;
}
