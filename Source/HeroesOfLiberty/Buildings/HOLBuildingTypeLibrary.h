// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Buildings/HOLBuildingTypes.h"
#include "HOLBuildingTypeLibrary.generated.h"

/**
 * The building type table (UHOLBuildingSettings::BuildingTypesTable, from Tools/Data/BuildingTypes.csv) for Angelscript (namespace
 * HOLBuildingType there): F7's garrison rule (Script/Garrison/HOLGarrisonRules.as) reads its places from it.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLBuildingTypeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** A copy of the row of this type; false (nothing logged, OutRow left as it was) if the table or the row is missing. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Buildings")
	static bool FindRow(FName TypeName, FHOLBuildingTypeRow& OutRow);
};
