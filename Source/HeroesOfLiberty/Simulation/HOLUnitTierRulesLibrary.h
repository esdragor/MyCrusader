// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLUnitTierRulesLibrary.generated.h"

/**
 * The simulation's unit tier rule (HOLUnitTier, C++) for Angelscript (namespace HOLUnitTierRules there): only so that the script's own rule,
 * Script/Units/HOLUnitTierRules.as (namespace HOLUnitTier), is checked against it (Test_UnitTierScriptAndSimulationRulesAgree).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLUnitTierRulesLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLUnitTier::Check on the MinTier of the type's row: 0 available, 1 not listed, 2 tier too low, 3 no building, 4 cannot pay. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UnitTier")
	static int32 CheckUnit(FName UnitType, int32 Tier, bool bBuildingFinished, bool bCanPay);

	/** HOLUnitTier::GetMinTier: the column MinTier of the unit type table, -1 without a row. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UnitTier")
	static int32 GetUnitMinTier(FName UnitType);
};
