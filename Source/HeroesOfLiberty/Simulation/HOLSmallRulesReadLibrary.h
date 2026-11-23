// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLSmallRulesReadLibrary.generated.h"

/**
 * Read libraries of two small pure rules (MIG-C-PetitesRegles, D830, D836, D839), envelopes only (no rule here), so that the tests of
 * HOLHeroXPTests.cpp and HOLMonkTests.cpp can be ported to script. The rules stay in C++ (HOLHeroXPRules.h, HOLHealRules.h).
 * A source of XP is its number (EHOLXPSource: 0 Combat, 1 Command, 2 Development, 3 Exploration, 4 Duel).
 */

/** HOLHeroXP:: for Angelscript (namespace HOLHeroXPRead there). */
UCLASS()
class HEROESOFLIBERTY_API UHOLHeroXPReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLHeroXP::GetWeightPercent. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroXP")
	static int32 GetWeightPercent(int32 Source);

	/** HOLHeroXP::GetValue of the cost made of these four resources. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroXP")
	static int32 GetValue(int32 Food, int32 Wood, int32 Stone, int32 Gold);

	/** HOLHeroXP::GetXP. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroXP")
	static int32 GetXP(int32 Value, int32 Source);

	/** HOLHeroXP::GetHeroKillXP. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroXP")
	static int32 GetHeroKillXP(int32 VictimLevel, int32 KillerLevel, int32 Source);

	/** HOLHeroXP::CountsWhileDead. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroXP")
	static bool CountsWhileDead(int32 Source);

	/** HOLHeroXP::GetKillSource: false (OutSource untouched) when the death gives none, else true with the source's number. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroXP")
	static bool GetKillSource(bool bKilledByHero, bool bHeroAlive, float DistanceToHero, int32& OutSource);

	/** HOLHeroXP::GetLevelXP. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroXP")
	static int32 GetLevelXP(int32 Level);

	/** HOLHeroXP::GetLevel. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroXP")
	static int32 GetLevel(int32 XP);

	/** HOLHeroXP::GetTier. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroXP")
	static int32 GetTier(int32 Level);

	/** HOLHeroXP::GetStatPercent. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroXP")
	static int32 GetStatPercent(int32 Level);

	/** HOLHeroXP::GetLevelStat. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroXP")
	static float GetLevelStat(float Level1Stat, int32 Level);

	/** HOLHeroXP::GetHealthAfterLevelUp. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroXP")
	static float GetHealthAfterLevelUp(float Health, float Level1MaxHealth, int32 OldLevel, int32 NewLevel);

	/** HOLForumRules::GetLives(Tier, Falls) (used by the Tiers test of the hero's XP). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HeroXP")
	static int32 GetForumLives(int32 Tier, int32 Falls);
};

/**
 * HOLHeal:: (HOLHealRules.h) and the Monk's discount (HOLTechnology::GetCostDiscountStat, GetDiscountedCost) for Angelscript (namespace
 * HOLHealRead there). The candidates of ChooseTarget are parallel arrays read by index (ids, X and Y in cm, health, maximum health, camp,
 * unhealable as 0 or 1); the allied owners and the candidate's owner are not told (0 and 255: the camp only, the standard modes).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLHealReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLHeal::IsHealable of one candidate (owner not told). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Heal")
	static bool IsHealable(int32 HealerId, int32 HealerCamp, int32 UnitId, float X, float Y, float Health, float MaxHealth, int32 Camp, bool bUnhealable);

	/** HOLHeal::ChooseTarget on the candidates read by index from the parallel arrays: an index of them, -1 (INDEX_NONE) if none. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Heal")
	static int32 ChooseTarget(int32 HealerId, int32 HealerCamp, float HealerX, float HealerY, float Range, int32 OrderedId, int32 KeptId,
		const TArray<int32>& Ids, const TArray<float>& Xs, const TArray<float>& Ys, const TArray<float>& Healths, const TArray<float>& MaxHealths,
		const TArray<int32>& Camps, const TArray<int32>& Unhealables);

	/** HOLHeal::GetOrderWalk as its number (EOrderWalk: 0 Keep, 1 Walk, 2 Stop). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Heal")
	static int32 GetOrderWalk(float Distance, float Range, bool bWalking, float GoalDrift);

	/** HOLHeal::OrderStopMargin (cm). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Heal")
	static float GetOrderStopMargin();

	/** HOLHeal::OrderRepathDistance (cm). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Heal")
	static float GetOrderRepathDistance();

	/** HOLHeal::GetHealedHealth. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Heal")
	static float GetHealedHealth(float Health, float MaxHealth, float HealRate, int32 StepMs);

	/** HOLTechnology::GetCostDiscountStat: the statistic's name as text, empty (NAME_None) for a unit type with none. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Heal")
	static FString GetCostDiscountStat(const FString& UnitType);

	/** HOLTechnology::GetDiscountedCost of the cost made of these four resources: the four lowered resources in the outs. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Heal")
	static void GetDiscountedCost(int32 Food, int32 Wood, int32 Stone, int32 Gold, int32 Percent, int32& OutFood, int32& OutWood, int32& OutStone, int32& OutGold);
};
