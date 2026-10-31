// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLRelayLibrary.generated.h"

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
};
