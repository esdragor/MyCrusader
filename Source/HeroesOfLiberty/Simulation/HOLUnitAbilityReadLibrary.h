// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLUnitAbilityReadLibrary.generated.h"

/**
 * The simulation's necrotic rage (HOLUnitAbility, Simulation/HOLUnitAbilityRules.h, C++, read by Mass) for Angelscript (namespace
 * HOLUnitAbilityRead there): D836 (2) keeps the rule in C++ and the script reads its constants here, so that its own copy can go
 * (socle step 1 of the migration, D830). Wrappers only: no rule here.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLUnitAbilityReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLUnitAbility::RageUnitType: the unit type that casts the rage. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UnitAbility")
	static FName GetRageUnitType();

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UnitAbility")
	static int32 GetRageDurationMs();

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UnitAbility")
	static int32 GetRageCooldownMs();

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UnitAbility")
	static int32 GetRageHpCostPerHit();

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UnitAbility")
	static int32 GetRageMinHp();

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UnitAbility")
	static int32 GetRageStopHp();

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UnitAbility")
	static int32 GetRageDamagePercent();

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UnitAbility")
	static int32 GetRageRatePercent();

	/** HOLUnitAbility::CanCastRage: alive, at MinHp or more, cool-down over. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UnitAbility")
	static bool CanCastRage(bool bAlive, float Health, int32 CooldownMs);

	/** HOLUnitAbility::GetRageHitCost: the hit points a blow under the rage costs, never under StopHp. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UnitAbility")
	static float GetRageHitCost(float Health);

	/** HOLUnitAbility::GetRageIntervalMs: the attack interval under the rage. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UnitAbility")
	static int32 GetRageIntervalMs(int32 IntervalMs);
};
