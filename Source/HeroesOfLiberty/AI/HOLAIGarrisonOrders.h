// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLAIGarrisonOrders.generated.h"

class AHOLBuilding;

/**
 * S4-b (D839, a binding): the AI's garrison order, in script as HOLAIGarrisonOrder::IssueAIGarrisonOrder (the library's name without U and Library, as
 * HOLAIRageOrder). The rule that chooses the units and the tower is HOLAIGarrisonCall.as; the places, the owner and the kinds are checked by the
 * garrison subsystem as for a player's order.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLAIGarrisonOrderLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** The slot PlayerIndex's UnitIds go into Building: queued as a player's order is (UHOLCommandSubsystem::IssueGarrisonOrder). Server and solo only; false when nothing was queued. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool IssueAIGarrisonOrder(const UObject* WorldContextObject, int32 PlayerIndex, const TArray<int32>& UnitIds, AHOLBuilding* Building);
};
