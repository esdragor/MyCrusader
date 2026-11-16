// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLProductionReadLibrary.generated.h"

/**
 * The pure rules of the production of units (HOLProduction, Buildings/HOLProductionRules.h, C++, applied by the server) for Angelscript
 * (namespace HOLProductionRead there): D836 keeps the rules in C++ and the script reads them here, so that their test (HOLProductionTests.cpp)
 * can be ported to script (socle step 1, S1g, D830, D839). Wrappers only: no rule here. Only the functions with flat arguments are here:
 * TryEnqueue, Cancel, Advance, GetStartedPopulation need FHOLProductionItem (a plain USTRUCT(), not exposed), FHOLProductionCost
 * (a plain struct) and, for Advance, a TFunctionRef: not exposed.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLProductionReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLProduction::GetPopulationCap. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Production")
	static int32 GetPopulationCap(int32 Provided);

	/** HOLProduction::GetProgress of an item that started at StartStep (-1: not started). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Production")
	static float GetProgress(int64 StartStep, double Step, int32 BuildSteps);

	/** HOLProduction::IsWaitingForRoom of an item that started at StartStep (-1: not started). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Production")
	static bool IsWaitingForRoom(int64 StartStep, double Step, int32 BuildSteps);
};