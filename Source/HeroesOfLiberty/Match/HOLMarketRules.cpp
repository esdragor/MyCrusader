// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLMarketRules.h"

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

	FBook NewBook(const FSettings& Settings)
	{
		FBook Book;
		for (int32 Resource = 0; Resource < BookSize; ++Resource)
		{
			Book.Prices[Resource] = Settings.BasePrices[Resource];
		}
		return Book;
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

	FTrade Buy(const FSettings& Settings, const FBook& Book, const int32 Resource, const int32 Lots, const int32 GoldStock, const int32 FeePercent)
	{
		if (const TCHAR* Bad = CheckRequest(Resource, Lots))
		{
			return Refuse(Book, Bad);
		}
		FTrade Trade;
		Trade.Book = Book;
		for (int32 Index = 0; Index < Lots; ++Index)
		{
			const int32 Price = Trade.Book.Prices[Resource];
			Trade.Gold += PurchaseGold(Price, FeePercent);
			Trade.Book.Prices[Resource] = FMath::Clamp(Price + Settings.Step, Settings.MinPrice, Settings.MaxPrice);
		}
		if (Trade.Gold > GoldStock)
		{
			return Refuse(Book, TEXT("gold"));
		}
		Trade.Units = Lots * Lot;
		Trade.bOk = true;
		return Trade;
	}

	FBook Recover(const FSettings& Settings, const FBook& Book, const int32 Intervals)
	{
		FBook Later = Book;
		const int32 Move = FMath::Max(0, Intervals) * Settings.RecoverPoints;
		for (int32 Resource = 0; Resource < BookSize; ++Resource)
		{
			const int32 Base = Settings.BasePrices[Resource];
			const int32 Price = Later.Prices[Resource];
			if (Price < Base)
			{
				Later.Prices[Resource] = FMath::Min(Base, Price + Move);
			}
			else if (Price > Base)
			{
				Later.Prices[Resource] = FMath::Max(Base, Price - Move);
			}
		}
		return Later;
	}
}