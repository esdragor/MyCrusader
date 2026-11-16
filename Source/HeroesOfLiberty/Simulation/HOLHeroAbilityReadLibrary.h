// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Simulation/HOLHeroAbilityRules.h"
#include "HOLHeroAbilityReadLibrary.generated.h"

/**
 * The hero's aura and abilities rules that STAY in C++ (HOLHeroAbility, Simulation/HOLHeroAbilityRules.h, run by the server: Mass) read by Angelscript
 * (namespace HOLHeroAbilityRead there): D836 keeps the rule in C++, so that its tests (HOLHeroAbilityTests.cpp: Unlock, Cast, Tick, Damage) can be
 * ported to script (socle step 1f, D830, D839). Wrappers only: no rule here. EHOLHeroAbility is already a BlueprintType UENUM.
 * Tick takes its remaining time by value and returns the new one (a script cannot pass an int32& to a UFUNCTION as the rule does).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLHeroAbilityReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroAbility")
	static int32 GetUnlockLevel(EHOLHeroAbility Ability);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroAbility")
	static bool IsUnlocked(EHOLHeroAbility Ability, int32 Level);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroAbility")
	static int32 GetCooldownMs(EHOLHeroAbility Ability);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroAbility")
	static bool CanCast(EHOLHeroAbility Ability, int32 Level, bool bHeroAlive, int32 CooldownMs);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroAbility")
	static bool CanPlantBanner(float Distance);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroAbility")
	static bool IsWithinAura(float Distance);

	/** HOLHeroAbility::Tick: RemainingMs counted down by StepMs, 0 at least. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroAbility")
	static int32 Tick(int32 RemainingMs, int32 StepMs);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroAbility")
	static int32 GetDamageTakenPercent(bool bInAura, bool bCloseRanks);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroAbility")
	static int32 GetCloseRanksSpeedPercent(bool bUnderCry, bool bDawnStep, int32 BasePercent);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroAbility")
	static float GetReducedDamage(float Damage, int32 DamageTakenPercent);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroAbility")
	static int32 GetHealRatePercent(bool bInAura, bool bTalent);

	/** The values the tests read (constants of the rule, read here so that the script keeps no copy). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroAbility")
	static int32 GetCloseRanksDurationMs();

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroAbility")
	static int32 GetBannerDurationMs();

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroAbility")
	static int32 GetMoraleReductionCapPercent();
};