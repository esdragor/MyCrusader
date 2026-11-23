// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLZoneEffectLibrary.h"
#include "Simulation/HOLHealProcessor.h"

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

bool UHOLZoneEffectLibrary::IsInZone(const int32 CenterXCm, const int32 CenterYCm, const int32 RadiusCm, const int32 PointXCm, const int32 PointYCm)
{
	return HOLZoneEffect::IsInZone(CenterXCm, CenterYCm, RadiusCm, PointXCm, PointYCm);
}

bool UHOLZoneEffectLibrary::ZoneActive(const int32 StartMs, const int32 DurationMs, const int32 NowMs)
{
	return HOLZoneEffect::ZoneActive(StartMs, DurationMs, NowMs);
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

bool UHOLZoneEffectLibrary::StartSacredLight(const UObject* WorldContextObject, const int32 Slot, const FVector Center)
{
	UHOLHeroSubsystem* Heroes = FindHeroSubsystem(WorldContextObject);
	return Heroes && Heroes->StartSacredLight(Slot, Center);
}

void UHOLZoneEffectLibrary::ConfigureHealHonor(const int32 HpPerPoint, const int32 Cap, const int32 KillGain, const int32 KillRadiusCm)
{
	UHOLHealProcessor::ConfigureHonor(HpPerPoint, Cap, KillGain, KillRadiusCm);
}

int32 UHOLZoneEffectLibrary::GetSacredLightRadiusCm()
{
	return HOLZoneEffect::SacredLightRadiusCm;
}

int32 UHOLZoneEffectLibrary::GetSacredLightDurationMs()
{
	return HOLZoneEffect::SacredLightDurationMs;
}

int32 UHOLZoneEffectLibrary::GetSacredLightHealPerSecond()
{
	return HOLZoneEffect::SacredLightHealPerSecond;
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
