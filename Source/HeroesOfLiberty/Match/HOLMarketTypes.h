// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "HOLMarketTypes.generated.h"

/**
 * The Market's numbers (M2, GDD 5.5, D70, D84, MK-Q1 = C, D582): a row of /Game/Match/MarketRates, imported from
 * Tools/Data/Market.csv by Tools/Editor/make_market.py, like the building and technology tables. One row, `Standard`, is the
 * rate of every player; another row (the Shadow Circle's black market, D69) can be added later with a lower tax. The exchange
 * rule itself is in script (Script/Economy/HOLMarketRules.as, FHOLMarketSettings): M4 hands it these numbers. Whole numbers
 * only, so that the server and the clients agree to the last gold piece.
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLMarketRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Gold for one lot of 100 food at rest. */
	UPROPERTY(EditAnywhere, Category = "Prices", meta = (ClampMin = "1"))
	int32 FoodPrice = 80;

	UPROPERTY(EditAnywhere, Category = "Prices", meta = (ClampMin = "1"))
	int32 WoodPrice = 80;

	UPROPERTY(EditAnywhere, Category = "Prices", meta = (ClampMin = "1"))
	int32 StonePrice = 120;

	/** Gold is the money: its price is kept for the book's four entries and is not traded. */
	UPROPERTY(EditAnywhere, Category = "Prices", meta = (ClampMin = "1"))
	int32 GoldPrice = 100;

	/** Tax in percent of the price: a sale pays the price minus it, a purchase costs the price plus it. */
	UPROPERTY(EditAnywhere, Category = "Rate", meta = (ClampMin = "0", ClampMax = "99"))
	int32 FeePercent = 30;

	/** How far one lot sold or bought moves its price (gold per lot). */
	UPROPERTY(EditAnywhere, Category = "Rate", meta = (ClampMin = "0"))
	int32 Step = 3;

	/** A price never leaves [MinPrice, MaxPrice]. */
	UPROPERTY(EditAnywhere, Category = "Rate", meta = (ClampMin = "1"))
	int32 MinPrice = 20;

	UPROPERTY(EditAnywhere, Category = "Rate", meta = (ClampMin = "1"))
	int32 MaxPrice = 300;

	/** Every RecoverEverySeconds seconds a price moves RecoverPoints toward its base. */
	UPROPERTY(EditAnywhere, Category = "Recovery", meta = (ClampMin = "0"))
	int32 RecoverPoints = 1;

	UPROPERTY(EditAnywhere, Category = "Recovery", meta = (ClampMin = "1", ForceUnits = "s"))
	int32 RecoverEverySeconds = 10;
};
