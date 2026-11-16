// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HOLMarketBridgeTypes.generated.h"

/**
 * The Market's exchange numbers as the bridge UHOLMarketSubsystem hands them to the script (MIG-CPP-Market, D836 point 4). The minimal mirror of
 * HOLMarket::FSettings (Match/HOLMarketRules.h): the script rule (Script/Economy/HOLMarketRules.as) has its own FHOLMarketSettings, the board
 * script converts. Prices by resource index (0 food, 1 wood, 2 stone, 3 gold).
 */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLMarketBridgeSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroesOfLiberty|Market")
	TArray<int32> BasePrices;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroesOfLiberty|Market")
	int32 Step = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroesOfLiberty|Market")
	int32 FeePercent = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroesOfLiberty|Market")
	int32 MinPrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroesOfLiberty|Market")
	int32 MaxPrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroesOfLiberty|Market")
	int32 RecoverPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroesOfLiberty|Market")
	int32 RecoverEverySeconds = 1;
};

/** The common book of prices (mirror of HOLMarket::FBook): gold for one lot, by resource index. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLMarketBridgeBook
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroesOfLiberty|Market")
	TArray<int32> Prices;
};