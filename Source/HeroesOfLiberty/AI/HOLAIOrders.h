// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLAIOrders.generated.h"

/**
 * An AI slot's orders for Blueprint and Angelscript (IA0.2, IA0.4's brain): the same queue and the same limits as a
 * human's (HOLPlayerOrders), without a controller. Server and solo only (the authority): refused on a client. Unit
 * identifiers as the summary gives them (FHOLAISummary::IdlePeasants: int32, script has no uint32).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLAIOrderLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * PlayerIndex's units gather the source SourceIndex (FHOLAISourceNear::SourceIndex), applied at the next step; its
	 * soldiers among them walk to it. Whether the order entered the queue.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool IssueAIGatherOrder(const UObject* WorldContextObject, int32 PlayerIndex, const TArray<int32>& UnitIds, int32 SourceIndex, bool bQueue = false);

	/** PlayerIndex's units go to Target, applied at the next step. Whether the order entered the queue. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool IssueAIMoveOrder(const UObject* WorldContextObject, int32 PlayerIndex, const TArray<int32>& UnitIds, FVector Target, bool bQueue = false);
};
