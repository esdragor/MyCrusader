// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLAIRageOrders.generated.h"

/**
 * L4 step 6: the AI's necrotic rage order, in script as HOLAIRageOrder::IssueAIRageOrder (the library's name without U and Library, as for
 * HOLAIOrder). A library of its own so that the rule file stays small; the rule that chooses the warriors is HOLAIRage.as.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLAIRageOrderLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * The slot PlayerIndex's Damned Warriors UnitIds cast their necrotic rage: queued as a player's order is (UHOLCommandSubsystem::IssueRageOrder), the
	 * simulation checks the owner, the type, the health and the cooldown. Server and solo only (refused on a client); false when nothing was queued.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool IssueAIRageOrder(const UObject* WorldContextObject, int32 PlayerIndex, const TArray<int32>& UnitIds);
};
