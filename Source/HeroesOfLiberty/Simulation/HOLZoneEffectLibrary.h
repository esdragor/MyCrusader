// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLZoneEffectLibrary.generated.h"

/**
 * The zone rules (HOLZoneEffect, Simulation/HOLZoneEffectRules.h, C++, applied by the server) for Angelscript (namespace HOLZoneEffect there):
 * the script reads the ONE source (D839: its former copy HOLGroundAbility::IsInZone / ZoneActive is retired) and puts the Seal of Light
 * down through StartLightSeal (the hero subsystem keeps the zone). Wrappers only: no rule here.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLZoneEffectLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLZoneEffect::IsInZone (centimetres, the edge counts as inside). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Zone")
	static bool IsInZone(int32 CenterXCm, int32 CenterYCm, int32 RadiusCm, int32 PointXCm, int32 PointYCm);

	/** HOLZoneEffect::ZoneActive (milliseconds, from StartMs, over at StartMs + DurationMs). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Zone")
	static bool ZoneActive(int32 StartMs, int32 DurationMs, int32 NowMs);

	/** HOLZoneEffect::GetSpeedPercent. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Zone")
	static int32 GetSpeedPercent(bool bEnemyOfOwner, bool bInZone, bool bActive, int32 SlowPercent);

	/** HOLZoneEffect::LightSealRadiusCm (D859). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Zone")
	static int32 GetLightSealRadiusCm();

	/** HOLZoneEffect::LightSealDurationMs (D859). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Zone")
	static int32 GetLightSealDurationMs();

	/** HOLZoneEffect::LightSealSlowPercent (D859). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Zone")
	static int32 GetLightSealSlowPercent();

	/**
	 * UHOLHeroSubsystem::StartLightSeal: the slot puts a Seal of Light at Center (server or solo only). The range (800 cm) and the cool-down
	 * (40 s) are checked by the caller from the UnitAbilities line. False: no world, no subsystem, on a client or out of range of the slots.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Zone", meta = (WorldContext = "WorldContextObject"))
	static bool StartLightSeal(const UObject* WorldContextObject, int32 Slot, FVector Center);

	/** How many zones of the hero subsystem are active now (UHOLHeroSubsystem::GetZones, ZoneActive at GetZoneNowMs); 0 without a world. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Zone", meta = (WorldContext = "WorldContextObject"))
	static int32 GetActiveZoneCount(const UObject* WorldContextObject);
};
