// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLTechnologyCheckLibrary.generated.h"

/**
 * The simulation's rule of the technologies (HOLTechnology::Check, C++, the one the server applies) for Angelscript (namespace
 * HOLTechnologyCheck there): only so that the script's own rule, Script/Technologies/HOLTechnologyRules.as, is checked against it
 * (Test_TechnologyScriptAndSimulationRulesAgree).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLTechnologyCheckLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * HOLTechnology::Check on a row with these facts, as the script's numbers (HOLTechnologyRules.as): 0 available, 1 researched,
	 * 2 tier too low, 3 hero level too low, 4 requires missing, 5 no building, 6 cannot pay, 7 already queued, 8 not implemented.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Technology")
	static int32 Check(bool bImplemented, int32 HeroTier, int32 HeroLevel, int32 MinTier, int32 MinHeroLevel, bool bRequiresDone,
		bool bBuildingFinished, bool bCanPay, bool bResearched, bool bQueued);
};