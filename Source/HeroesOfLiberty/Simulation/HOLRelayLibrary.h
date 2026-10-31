// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/Vector.h"
#include "HOLRelayLibrary.generated.h"

/** A relay as the AI reads it (R6): where, how wide, who holds it and who is filling it (slots, INDEX_NONE for nobody). */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLAIRelay
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "Relay")
	FVector Location = FVector::ZeroVector;

	/** The zone's radius, cm. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "Relay")
	float Radius = 1200.f;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "Relay")
	int32 Owner = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "Relay")
	int32 Taker = INDEX_NONE;
};
/**
 * The simulation's relay rules (HOLRelay::Step, C++) for Angelscript (namespace HOLRelayRules there): only so that the script's own rules,
 * Script/Relays/HOLRelayRules.as (namespace HOLRelay), are checked against them (Test_RelayScriptAndSimulationRulesAgree).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLRelayRulesLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLRelay::Step on one relay: its state in, its state out. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Relay")
	static void StepRelay(int32 Owner, int32 Taker, int32 Held, const TArray<int32>& UnitsBySlot, const TArray<int32>& TeamBySlot, int32 HoldSteps,
		int32 DecaySteps, int32& OutOwner, int32& OutTaker, int32& OutHeld);

	/** The provisional values of the simulation's rules (HOLRelay), so that the script's are checked against them. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Relay")
	static int32 GetHoldSeconds();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Relay")
	static int32 GetDecaySteps();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Relay")
	static int32 GetRelayXPPerMinute();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Relay")
	static int32 GetCentreXPPerMinute();

	/** HOLRelay::TerritoryXP: the carry in and out, the whole XP given this step. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Relay")
	static int32 TerritoryXPStep(int32 Carry, int32 Relays, int32 Centres, int32 StepsPerSecond, int32& OutCarry);

	/**
	 * R6: the relays of the match as the AI reads them (the simulation's, server or solo; empty on a client or without relays), in the order
	 * they were laid.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Relay", meta = (WorldContext = "WorldContextObject"))
	static void ReadAIRelays(const UObject* WorldContextObject, TArray<FHOLAIRelay>& OutRelays);

	/**
	 * R6, RQ3 PROVISIONAL (not decided): what the AI does with the relays. 0 = A, it ignores them; 1 = B, a detachment by level (default, the
	 * recommendation); 2 = C, the whole army chains the relays before attacking. The console variable hol.AI.RelayMode.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Relay")
	static int32 GetAIRelayMode();
};
