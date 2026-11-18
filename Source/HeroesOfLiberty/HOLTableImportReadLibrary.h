// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLTableImportReadLibrary.generated.h"

/**
 * The editor's CSV import (UDataTable::CreateTableFromCSVString on a file of Tools/Data) for Angelscript (namespace HOLTableImportRead there):
 * the script test of a table reads the rows the engine's import builds, not its own parser (D830, D839; GLORFINDEL-1117j: the last
 * assertion without a script twin of Market.Table, Market.Rules, Siege.Technologies, Technologies). Wrappers only: no rule here.
 * A table is a key ("Market", "Technology", "BuildingType": the row struct), a row is its name, a field is its text as the reflection exports
 * it (ExportText: numbers as written, a name as itself, a list as "(A,B)", a bool as "True" / "False").
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLTableImportReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Tools/Data/File (a plain file name, ".csv") imported into a transient table of the row struct Key. False (the problems in OutProblems,
	 * the others emptied) if the key or file is unknown or the import reports a problem. True: OutRowNames in the file's order,
	 * OutColumns the fields of the row struct (declaration order), OutValues row by row (OutRowNames.Num() * OutColumns.Num() texts).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Data")
	static bool ImportTable(const FString& Key, const FString& File, TArray<FString>& OutRowNames, TArray<FString>& OutColumns, TArray<FString>& OutValues, TArray<FString>& OutProblems);

	/**
	 * HOLMarket::SettingsFromRow of the row RowName of Tools/Data/Market.csv, imported as above: false if the import fails or the row is missing.
	 * OutBasePrices: food, wood, stone, gold (index = resource).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Data")
	static bool MarketSettingsFromRow(const FString& RowName, TArray<int32>& OutBasePrices, int32& OutStep, int32& OutFeePercent, int32& OutMinPrice, int32& OutMaxPrice, int32& OutRecoverPoints, int32& OutRecoverEverySeconds);
};
