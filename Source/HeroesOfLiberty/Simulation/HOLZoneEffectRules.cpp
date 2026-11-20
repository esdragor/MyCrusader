// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLZoneEffectRules.h"

bool HOLZoneEffect::IsInZone(const int32 CenterXCm, const int32 CenterYCm, const int32 RadiusCm, const int32 PointXCm, const int32 PointYCm)
{
	// 64-bit squares: a 6 m radius and a map of a few hundred metres stay far from the limit, but the rule does not rely on it.
	const int64 Dx = static_cast<int64>(PointXCm) - CenterXCm;
	const int64 Dy = static_cast<int64>(PointYCm) - CenterYCm;
	const int64 Radius = RadiusCm;
	return Dx * Dx + Dy * Dy <= Radius * Radius;
}

bool HOLZoneEffect::ZoneActive(const int32 StartMs, const int32 DurationMs, const int32 NowMs)
{
	return NowMs >= StartMs && NowMs < StartMs + DurationMs;
}

int32 HOLZoneEffect::GetSpeedPercent(const bool bEnemyOfOwner, const bool bInZone, const bool bActive, const int32 SlowPercent)
{
	// The one place the slow is read: an enemy standing in an active zone walks at 100 - SlowPercent (never below 0, never above 100).
	return (bEnemyOfOwner && bInZone && bActive) ? 100 - FMath::Clamp(SlowPercent, 0, 100) : 100;
}