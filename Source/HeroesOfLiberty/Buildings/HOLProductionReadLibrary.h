// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Match/HOLResources.h"
#include "HOLProductionReadLibrary.generated.h"

/** A production queue for the script (L17-12, GLORFINDEL-1117g): the items of a TArray<FHOLProductionItem> as two parallel arrays (type, start step; -1 = waiting behind the head). */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLProductionQueueState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Production")
	TArray<FName> Types;

	UPROPERTY(BlueprintReadWrite, Category = "Production")
	TArray<int64> StartSteps;
};

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

	/** HOLProduction::TryEnqueue with one cost for every type (CostResources, BuildSteps, Population). Returns EHOLProductionRefusal as an int (0 None, 1 NotProducible, 2 QueueFull, 3 NotEnoughResources). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Production")
	static int32 TryEnqueue(UPARAM(ref) FHOLProductionQueueState& Queue, FName TypeName, const FHOLResources& CostResources, int32 BuildSteps, int32 Population, int32 MaxQueue, UPARAM(ref) FHOLResources& Stock);

	/** HOLProduction::Cancel (the cost given back to Stock). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Production")
	static bool Cancel(UPARAM(ref) FHOLProductionQueueState& Queue, int32 Index, const FHOLResources& CostResources, int32 BuildSteps, int32 Population, UPARAM(ref) FHOLResources& Stock);

	/** HOLProduction::Advance with one cost for every type; the types that came out are appended to OutDone. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Production")
	static void Advance(UPARAM(ref) FHOLProductionQueueState& Queue, int64 Step, const FHOLResources& CostResources, int32 BuildSteps, int32 Population, UPARAM(ref) int32& FreePopulation, UPARAM(ref) TArray<FName>& OutDone);
};