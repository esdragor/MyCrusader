// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
class UHOLUnitSubsystem;
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

	/** HOLUnitAbility::GetRageCooldownEndMs: when the cool-down of a rage cast at StartMs ends (StartMs + the cool-down). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UnitAbility")
	static int32 GetRageCooldownEndMs(int32 StartMs);

	/** HOLUnitAbility::GetRageEffectEndMs: when the effect of a rage cast at StartMs ends by time (StartMs + the duration). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UnitAbility")
	static int32 GetRageEffectEndMs(int32 StartMs);

	/** HOLUnitAbility::IsRageEffectEnded: over by time, or because the unit is at (or under) the stop floor. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UnitAbility")
	static bool IsRageEffectEnded(int32 StartMs, int32 NowMs, float Health);

	/** SAURON-1125b (Dawn A7): the simulation step index of the units' world, for the script rule of the Knight's challenge (0 without a simulation). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UnitAbility")
	static int64 GetSimulationStep(const UHOLUnitSubsystem* Units);

	/** SAURON-1125b: one simulation step in whole milliseconds (the script divides a duration by it; 0 without a simulation). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UnitAbility")
	static int32 GetSimulationStepMs(const UHOLUnitSubsystem* Units);

	/** SAURON-1125b: UHOLUnitSubsystem::CastKnightChallenge, the liaison of the script rule HOLKnightChallenge (no rule here). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UnitAbility")
	static bool CastKnightChallenge(UHOLUnitSubsystem* Units, int32 UnitId, int32 RequiredOwner, int64 EndStep, int32 RadiusCm, int32 CooldownMs);

	/** D923 (FARAMIR-1126m3): the unit subsystem of the actor's world (nullptr without one), for the script RPC that casts the Knight's challenge (HOLKnightChallenge::CastChallenge takes it). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UnitAbility")
	static UHOLUnitSubsystem* GetUnitSubsystemOf(const AActor* WorldActor);
};
