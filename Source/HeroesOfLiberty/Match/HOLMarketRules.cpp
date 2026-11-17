// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLMarketRules.h"

#include "Engine/DataTable.h"

namespace HOLMarket
{
	namespace
	{
		/** What a lot at this price pays when sold: the price minus the tax, rounded down. */
		int32 SaleGold(const int32 Price, const int32 FeePercent)
		{
			return Price * (100 - FeePercent) / 100;
		}

		/** What a lot at this price costs when bought: the price plus the tax, rounded up. */
		int32 PurchaseGold(const int32 Price, const int32 FeePercent)
		{
			return (Price * (100 + FeePercent) + 99) / 100;
		}

		FTrade Refuse(const FBook& Book, const TCHAR* Reason)
		{
			FTrade Trade;
			Trade.Reason = FName(Reason);
			Trade.Book = Book;
			return Trade;
		}

		/** The refusal that comes before any price is read, or none. */
		const TCHAR* CheckRequest(const int32 Resource, const int32 Lots)
		{
			if (Lots <= 0)
			{
				return TEXT("lots");
			}
			if (Resource < 0 || Resource >= TradableResources)
			{
				return TEXT("resource");
			}
			return nullptr;
		}
	}

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

	FTrade Sell(const FSettings& Settings, const FBook& Book, const int32 Resource, const int32 Lots, const int32 Stock, const int32 FeePercent)
	{
		if (const TCHAR* Bad = CheckRequest(Resource, Lots))
		{
			return Refuse(Book, Bad);
		}
		if (Stock < Lots * Lot)
		{
			return Refuse(Book, TEXT("stock"));
		}
		FTrade Trade;
		Trade.Book = Book;
		for (int32 Index = 0; Index < Lots; ++Index)
		{
			const int32 Price = Trade.Book.Prices[Resource];
			Trade.Gold += SaleGold(Price, FeePercent);
			Trade.Book.Prices[Resource] = FMath::Clamp(Price - Settings.Step, Settings.MinPrice, Settings.MaxPrice);
		}
		Trade.Units = Lots * Lot;
		Trade.bOk = true;
		return Trade;
	}

	FTrade BuyWithGold(const FSettings& Settings, const FBook& Book, const int32 Resource, const int32 Lots, const int32 GoldStock, const int32 FeePercent)
	{
		if (const TCHAR* Bad = CheckRequest(Resource, Lots))
		{
			return Refuse(Book, Bad);
		}
		if (GoldStock < Lots * Lot)
		{
			return Refuse(Book, TEXT("gold"));
		}
		FTrade Trade;
		Trade.Book = Book;
		for (int32 Index = 0; Index < Lots; ++Index)
		{
			const int32 Price = Trade.Book.Prices[Resource];
			Trade.Units += Lot * Lot / PurchaseGold(Price, FeePercent);
			Trade.Book.Prices[Resource] = FMath::Clamp(Price + Settings.Step, Settings.MinPrice, Settings.MaxPrice);
		}
		Trade.Gold = Lots * Lot;
		Trade.bOk = true;
		return Trade;
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
