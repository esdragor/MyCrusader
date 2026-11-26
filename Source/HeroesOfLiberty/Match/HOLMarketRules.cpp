// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLMarketRules.h"

#include "Engine/DataTable.h"

namespace HOLMarket
{
	FSettings SettingsFromRow(const FHOLMarketRow& Row)
	{
		FSettings Settings;
		Settings.BasePrices[0] = Row.FoodPrice;
		Settings.BasePrices[1] = Row.WoodPrice;
		Settings.BasePrices[2] = Row.StonePrice;
		Settings.BasePrices[3] = Row.GoldPrice;
		Settings.Step = Row.Step;
		Settings.FeePercent = Row.FeePercent;
		Settings.MinPrice = Row.MinPrice;
		Settings.MaxPrice = Row.MaxPrice;
		Settings.RecoverPoints = Row.RecoverPoints;
		Settings.RecoverEverySeconds = FMath::Max(1, Row.RecoverEverySeconds);
		return Settings;
	}

	bool LoadStandardSettings(FSettings& OutSettings)
	{
		const UDataTable* Table = LoadObject<UDataTable>(nullptr, TEXT("/Game/Match/MarketRates.MarketRates"));
		const FHOLMarketRow* Row = Table ? Table->FindRow<FHOLMarketRow>(TEXT("Standard"), TEXT("market rates"), false) : nullptr;
		if (!Row)
		{
			UE_LOG(LogTemp, Warning, TEXT("Market: /Game/Match/MarketRates or its Standard row is missing (run Tools/Editor/make_market.py)"));
			return false;
		}
		OutSettings = SettingsFromRow(*Row);
		return true;
	}
}
