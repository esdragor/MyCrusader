// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLUnitTierRulesLibrary.generated.h"

/**
 * The column MinTier of the unit type table for Angelscript (namespace HOLUnitTierRules there): the one truth of the tier of a unit (D836 point 6), read by
 * the script's tests against its own table. The rule itself is the script's alone (Script/Units/HOLUnitTierRules.as, namespace HOLUnitTier).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLUnitTierRulesLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** UHOLUnitTierSubsystem::GetMinTier: the column MinTier of the unit type table, -1 without a row. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UnitTier")
	static int32 GetUnitMinTier(FName UnitType);
};
