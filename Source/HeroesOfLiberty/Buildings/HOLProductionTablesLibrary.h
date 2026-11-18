// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Match/HOLResources.h"
#include "HOLProductionTablesLibrary.generated.h"

/** The production-related values of one row of the unit type table, flat for the script (L17-12, GLORFINDEL-1117g). Enums as ints (EHOLArmorCategory, EHOLAttackType). */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLUnitTypeSummary
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Production")
	FHOLResources Cost;

	UPROPERTY(BlueprintReadWrite, Category = "Production")
	float BuildTime = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Production")
	int32 Population = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Production")
	int32 MaxHealth = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Production")
	int32 Attack = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Production")
	int32 MeleeArmor = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Production")
	int32 RangedArmor = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Production")
	int32 HeroArmor = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Production")
	float AttackInterval = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Production")
	float AttackRange = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Production")
	float AggressionRadius = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Production")
	float Speed = 0.f;

	/** EHOLAttackType as an int (0 Melee, 1 Ranged, 2 Siege, 3 Hero). */
	UPROPERTY(BlueprintReadWrite, Category = "Production")
	int32 AttackType = 0;

	/** EHOLArmorCategory values (0 LightInfantry, 1 HeavyInfantry, 2 Ranged, 3 Cavalry, 4 Siege, 5 Peasant, 6 GunpowderInfantry, 7 Heavy). */
	UPROPERTY(BlueprintReadWrite, Category = "Production")
	TArray<int32> ArmorCategories;

	/** Bonuses as two parallel arrays: the target category (EHOLArmorCategory as int) and the damage added. */
	UPROPERTY(BlueprintReadWrite, Category = "Production")
	TArray<int32> BonusCategories;

	UPROPERTY(BlueprintReadWrite, Category = "Production")
	TArray<int32> BonusDamages;
};

/** The production-related values of one row of the building type table, flat for the script. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLBuildingProductionSummary
{
	GENERATED_BODY()

	/** EHOLBuildingCategory as an int (0 Economic, 1 Military, ...: see HOLBuildingTypes.h). */
	UPROPERTY(BlueprintReadWrite, Category = "Production")
	int32 Category = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Production")
	TArray<FName> Produces;

	UPROPERTY(BlueprintReadWrite, Category = "Production")
	int32 PopulationProvided = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Production")
	int32 MaxQueue = 0;

	/** Number of resources the building accepts as a deposit (0: not a deposit). */
	UPROPERTY(BlueprintReadWrite, Category = "Production")
	int32 AcceptsCount = 0;
};

/**
 * The tables behind the production rules (the unit type table, the building type table, the match settings) for Angelscript (namespace
 * HOLProductionTables there), so that HeroesOfLiberty.Production.Tables (HOLProductionTests.cpp) can be ported to script (L17-12,
 * GLORFINDEL-1117g, D830, D839). Wrappers only: no rule here.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLProductionTablesLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** A flat copy of the row of this unit type; false (OutRow left as it was) if the table or the row is missing. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Production")
	static bool FindUnitType(FName TypeName, FHOLUnitTypeSummary& OutRow);

	/** A flat copy of the production values of this building type; false (OutRow left as it was) if the row is missing. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Production")
	static bool FindBuildingType(FName TypeName, FHOLBuildingProductionSummary& OutRow);

	/** UHOLMatchSettings::StartingResources. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Production")
	static FHOLResources GetStartingResources();
};
