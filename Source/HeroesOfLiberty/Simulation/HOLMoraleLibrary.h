// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLMoraleLibrary.generated.h"

/**
 * The morale effects of a unit as the script sees them (PLAN-MORAL-1120 step 2). The rule is ONE source: HOLMorale (Simulation/HOLMoraleRules.h);
 * these two structs only carry its state across the script bridge (the script has no fixed array: Effects holds at most HOLMorale::MaxEffects
 * entries when passed to the rule, the extra ones are ignored). No rule here (D839).
 */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLMoraleEffectData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Morale")
	int32 Id = -1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Morale")
	int32 Dealt = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Morale")
	int32 Taken = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Morale")
	int32 Rate = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Morale")
	int32 DurationMs = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Morale")
	bool bAura = false;
};

USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLMoraleStateData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Morale")
	TArray<FHOLMoraleEffectData> Effects;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Morale")
	int32 ImmunityMs = 0;
};

/**
 * Bridge of the C++ morale rule (HOLMorale) to Angelscript: wrappers only, each one converts the script state to the rule's fixed state, calls the
 * rule and converts back. The script file Script/Morale/HOLMoraleRules.as wraps these under the old names (namespace HOLMorale). The names carry
 * "Native" because the script namespace of this class is also HOLMorale (the name of the wrapper functions of the script).
 * A script cannot pass an int32& to a UFUNCTION, so the state is UPARAM(ref) (a struct) and the codes are returned.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLMoraleLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Morale")
	static int32 NativeAttenuationCap();

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Morale")
	static int32 NativeBonusCap(int32 Lever);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Morale")
	static int32 NativeMalusCap(int32 Lever);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Morale")
	static FHOLMoraleEffectData NativeMakeEffect(int32 Id, int32 Dealt, int32 Taken, int32 Rate, int32 DurationMs, bool bAura);

	/** The effect of a row of the morale table (a Gives row gives the levers of the row it names); a bad id gives Id -1. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Morale")
	static FHOLMoraleEffectData NativeMakeEffectFromRow(int32 RowId);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Morale")
	static int32 NativeLeverValue(const FHOLMoraleEffectData& Effect, int32 Lever);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Morale")
	static bool NativeIsMalus(const FHOLMoraleEffectData& Effect);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Morale")
	static int32 NativeScale(int32 Value, int32 FactorPercent);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Morale")
	static FHOLMoraleEffectData NativeWithIntensity(const FHOLMoraleEffectData& Effect, int32 FactorPercent);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Morale")
	static bool NativeHasIntensity(const FHOLMoraleEffectData& Effect);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Morale")
	static FHOLMoraleEffectData NativeAttenuated(const FHOLMoraleEffectData& Effect, int32 AttenuationPercent);

	/** Natures and Percents go in pairs; an unpaired nature is ignored. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Morale")
	static int32 NativeCombineAttenuation(const TArray<int32>& Natures, const TArray<int32>& Percents);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Morale")
	static bool NativeHas(const FHOLMoraleStateData& State, int32 Id);

	/** HOLMorale::Apply: returns one of the Apply* codes (0 nothing, 1 posed, 2 refreshed, 3 blocked by immunity, 4 no room). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Morale")
	static int32 NativeApply(UPARAM(ref) FHOLMoraleStateData& State, const FHOLMoraleEffectData& Effect, int32 AttenuationPercent);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Morale")
	static int32 NativeTotal(const FHOLMoraleStateData& State, int32 Lever);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Morale")
	static int32 NativeMultiplier(const FHOLMoraleStateData& State, int32 Lever);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Morale")
	static void NativeTick(UPARAM(ref) FHOLMoraleStateData& State, int32 ElapsedMs);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Morale")
	static void NativeClearAuras(UPARAM(ref) FHOLMoraleStateData& State);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Morale")
	static int32 NativeRemoveMalus(UPARAM(ref) FHOLMoraleStateData& State);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Morale")
	static void NativeGrantImmunity(UPARAM(ref) FHOLMoraleStateData& State, int32 DurationMs);

	/**
	 * PLAN-MORAL-1120 step 6 (S2-b): a morale order of the duel (HOLDuelLedger::PendingMorale). Lays the table row EffectId (Triumph, Demoralised,
	 * Hesitation) for DurationMs on every unit of camp OrderSlot within RadiusCm of the point, at the next step of the morale processor (one source
	 * of the rule: the processor, the same path as the cry; never a Peasant, D705). Wrapper only (D839): the order is queued, the processor lays it.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Morale")
	static void ApplyEffectAround(const UObject* WorldContextObject, int32 EffectId, int32 CenterXCm, int32 CenterYCm, int32 OrderSlot, int32 DurationMs, int32 RadiusCm);

	/**
	 * PLAN-MORAL-1120 step 8 (S2-c, D404): Lumiere sacree's brick. Takes off the MALUS of the units of camp OrderSlot within RadiusCm of the point
	 * and makes them immune to every malus for a few seconds (HOLMorale::SacredLightImmunityMs); the bonus stay. Wrapper only (D839): the order is
	 * queued, the processor lays it in its next step, the same path as ApplyEffectAround (the rule is HOLMorale::ClearMalusAndProtect).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Morale")
	static void ClearMalusAround(const UObject* WorldContextObject, int32 CenterXCm, int32 CenterYCm, int32 RadiusCm, int32 OrderSlot);
};
