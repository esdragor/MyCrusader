// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLMapRulesReadLibrary.generated.h"

/** One map of the match table (FHOLMapInfo, a plain C++ struct there): the key is an FString here (a TCHAR* there). */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLMapInfoValue
{
	GENERATED_BODY()

	/** Asset name under /Game/Maps and key of Tools/Data/Maps.csv. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Map")
	FString Key;

	/** The translated name. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Map")
	FText DisplayName;

	/** The Budget column: standard, centre or dense. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Map")
	bool bBudget = false;

	/** The Budget centre: the gold is one cluster in the middle of the map. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Map")
	bool bGoldAtCentre = false;

	/** The Budget dense: twice the wood. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Map")
	bool bDenseWood = false;

	/** The sacred sites the map's file lays. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Map")
	int32 NumSites = 2;
};

/**
 * The pure rules of the match maps (FHOLMapRules, Map/HOLMapRules.h, C++, same module as this library) for Angelscript (namespace
 * HOLMapRulesRead there): D836 keeps the rule in C++ and the script reads it here, so that the search part of the test of the rule
 * (HOLMapRulesTests.cpp) can be ported to script (socle step 1, D830, D839). Wrappers only: no rule here. A file is given by its
 * path as text ("/Game/Maps/Plain.Plain": the FSoftObjectPath of the rule, whose script exposure is not verified); a nullable pointer
 * of the rule is a bool plus an out value.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLMapRulesReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** FHOLMapRules::GetMaps: the maps, in the order of Tools/Data/Maps.csv. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Map")
	static TArray<FHOLMapInfoValue> GetMaps();

	/** FHOLMapRules::FindByKey: false (Out untouched) for an unknown key; the key's case is ignored. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Map")
	static bool FindByKey(const FString& Key, FHOLMapInfoValue& Out);

	/** FHOLMapRules::GetAssetPath: the file of a map as the configuration carries it, as text (/Game/Maps/Plain.Plain). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Map")
	static FString GetAssetPath(const FString& Key);

	/** FHOLMapRules::FindByAsset: the map of a file path (/Game/Maps/Plain.Plain); false (Out untouched) for an unknown, sized or test map. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Map")
	static bool FindByAsset(const FString& AssetPath, FHOLMapInfoValue& Out);

	/** FHOLMapRules::GetDisplayName: the translated name of a map file, or its asset name for one the table does not know. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Map")
	static FText GetDisplayName(const FString& AssetPath);
};
