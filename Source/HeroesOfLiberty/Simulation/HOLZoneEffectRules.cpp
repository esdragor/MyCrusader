// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLZoneEffectRules.h"

// STUB (commit 1, red test): neutral answers, the zone does nothing yet.
bool HOLZoneEffect::IsInZone(const int32 CenterXCm, const int32 CenterYCm, const int32 RadiusCm, const int32 PointXCm, const int32 PointYCm)
{
	return false;
}

bool HOLZoneEffect::ZoneActive(const int32 StartMs, const int32 DurationMs, const int32 NowMs)
{
	return false;
}

int32 HOLZoneEffect::GetSpeedPercent(const bool bEnemyOfOwner, const bool bInZone, const bool bActive, const int32 SlowPercent)
{
	return 100;
}
