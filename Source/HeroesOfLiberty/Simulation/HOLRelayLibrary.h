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
 * The simulation's relay constants (HOLRelay, C++) for Angelscript (namespace HOLRelayRules there), checked against the script's own
 * (Script/Relays/HOLRelayRules.as, namespace HOLRelay) by Test_RelayConstantsAgreeWithTheSimulation, and the AI's reads of the relays. The rules
 * themselves (Step, TerritoryXP) are script-only (bascule Relay).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLRelayRulesLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** The provisional values of the simulation's rules (HOLRelay), so that the script's are checked against them. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Relay")
	static int32 GetHoldSeconds();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Relay")
	static int32 GetDecaySteps();

	/**
	 * R6: the relays of the match as the AI reads them (the simulation's, server or solo; empty on a client or without relays), in the order
	 * they were laid.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Relay", meta = (WorldContext = "WorldContextObject"))
	static void ReadAIRelays(const UObject* WorldContextObject, TArray<FHOLAIRelay>& OutRelays);

	/**
	 * R6, RQ3 = B (D589): what the AI does with the relays. 0 = A, it ignores them; 1 = B, a detachment by level (default, the
	 * recommendation); 2 = C, the whole army chains the relays before attacking. The console variable hol.AI.RelayMode.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Relay")
	static int32 GetAIRelayMode();
};
