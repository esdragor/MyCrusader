// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLZoneEffectLibrary.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLZoneEffectRules.h"

namespace
{
	UHOLHeroSubsystem* FindHeroSubsystem(const UObject* WorldContextObject)
	{
		const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
		return World ? World->GetSubsystem<UHOLHeroSubsystem>() : nullptr;
	}
}

// STUB (commit a, red): the bodies below answer neutral so that the script tests that call the binding fail. The correction comes next.
bool UHOLZoneEffectLibrary::IsInZone(const int32 CenterXCm, const int32 CenterYCm, const int32 RadiusCm, const int32 PointXCm, const int32 PointYCm)
{
	return false;
}

bool UHOLZoneEffectLibrary::ZoneActive(const int32 StartMs, const int32 DurationMs, const int32 NowMs)
{
	return false;
}

int32 UHOLZoneEffectLibrary::GetSpeedPercent(const bool bEnemyOfOwner, const bool bInZone, const bool bActive, const int32 SlowPercent)
{
	return HOLZoneEffect::GetSpeedPercent(bEnemyOfOwner, bInZone, bActive, SlowPercent);
}

int32 UHOLZoneEffectLibrary::GetLightSealRadiusCm()
{
	return HOLZoneEffect::LightSealRadiusCm;
}

int32 UHOLZoneEffectLibrary::GetLightSealDurationMs()
{
	return HOLZoneEffect::LightSealDurationMs;
}

int32 UHOLZoneEffectLibrary::GetLightSealSlowPercent()
{
	return HOLZoneEffect::LightSealSlowPercent;
}

bool UHOLZoneEffectLibrary::StartLightSeal(const UObject* WorldContextObject, const int32 Slot, const FVector Center)
{
	UHOLHeroSubsystem* Heroes = FindHeroSubsystem(WorldContextObject);
	return Heroes && Heroes->StartLightSeal(Slot, Center);
}

int32 UHOLZoneEffectLibrary::GetActiveZoneCount(const UObject* WorldContextObject)
{
	const UHOLHeroSubsystem* Heroes = FindHeroSubsystem(WorldContextObject);
	if (!Heroes)
	{
		return 0;
	}
	int32 Count = 0;
	for (const HOLZoneEffect::FZone& Zone : Heroes->GetZones())
	{
		if (HOLZoneEffect::ZoneActive(Zone.StartMs, Zone.DurationMs, Heroes->GetZoneNowMs()))
		{
			++Count;
		}
	}
	return Count;
}
