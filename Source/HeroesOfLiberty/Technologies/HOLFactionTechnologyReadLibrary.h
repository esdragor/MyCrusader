// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLFactionTechnologyReadLibrary.generated.h"

/**
 * The faction filter of the technologies (L2: the `Faction` column of the technology table) read by Angelscript (namespace
 * HOLFactionTechnologyRead there), so that HOLFactionTechnologyTests.cpp can be ported to script (E3-06, D830, D839). Wrappers only:
 * HOLFactionRules::IsTechnologyOpenTo and UHOLTechnologySettings::GetBuildingTechnologies stay the one source of the rule.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLFactionTechnologyReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLFactionRules::IsTechnologyOpenTo: whether the faction (its key: Dawn, Legions...) researches the technology; a technology the table does not know is left open. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Technologies")
	static bool IsTechnologyOpenTo(FName Technology, FName FactionKey);

	/** UHOLTechnologySettings::GetBuildingTechnologies(Building, FactionKey, Out): what the building's research row offers that faction, in the table's order; a None key leaves all of them. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Technologies")
	static TArray<FName> GetBuildingTechnologies(FName Building, FName FactionKey);
};
