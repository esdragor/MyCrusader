// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLMarketTypes.h"

/**
 * The Market's exchange rule where the simulation runs (M4a, GDD 5.5, D70, D84, D582): the server prices every trade here.
 * The same rule as Script/Economy/HOLMarketRules.as (which the AI and the interface use to quote), whole numbers only so
 * that the server and the clients agree to the last gold piece; both are tested on the same figures (the Market.csv ones).
 * Resources by index: 0 food, 1 wood, 2 stone, 3 gold (gold is the money, it has no tradable price).
 */
namespace HOLMarket
{
	/** Goods in one lot: trades are made of whole lots. */
	constexpr int32 Lot = 100;
	/** Food, wood, stone (index 0 to 2) are traded against gold. */
	constexpr int32 TradableResources = 3;
	/** Entries of a book: the four resources. */
	constexpr int32 BookSize = 4;

	/** The numbers of the exchange (a row of Market.csv). */
	struct HEROESOFLIBERTY_API FSettings
	{
		int32 BasePrices[BookSize] = { 100, 100, 100, 100 };
		int32 Step = 0;
		int32 FeePercent = 0;
		int32 MinPrice = 0;
		int32 MaxPrice = 0;
		int32 RecoverPoints = 0;
		int32 RecoverEverySeconds = 1;
	};

	/** The common prices: gold for one lot, by resource (one per match, for every player, D84). */
	struct HEROESOFLIBERTY_API FBook
	{
		int32 Prices[BookSize] = { 100, 100, 100, 100 };
	};

	/** What one trade did, or why it did not ("lots", "resource", "stock", "gold"; empty when done). */
	struct HEROESOFLIBERTY_API FTrade
	{
		bool bOk = false;
		FName Reason;
		/** Gold earned by a sale, paid by a purchase; 0 when refused. */
		int32 Gold = 0;
		/** Goods that left the stock (sale) or arrived (purchase); 0 when refused. */
		int32 Units = 0;
		/** The book after the trade (the one given, unchanged, when refused). */
		FBook Book;
	};

	HEROESOFLIBERTY_API FSettings SettingsFromRow(const FHOLMarketRow& Row);
	HEROESOFLIBERTY_API FBook NewBook(const FSettings& Settings);
	/** Sells Lots lots of Resource out of Stock, at the tax FeePercent (the black market's is lower, D69); each lot at the price the one before left. */
	HEROESOFLIBERTY_API FTrade Sell(const FSettings& Settings, const FBook& Book, int32 Resource, int32 Lots, int32 Stock, int32 FeePercent);
	/** Buys Lots lots of Resource with GoldStock gold, at the tax FeePercent. */
	HEROESOFLIBERTY_API FTrade Buy(const FSettings& Settings, const FBook& Book, int32 Resource, int32 Lots, int32 GoldStock, int32 FeePercent);
	/** After Intervals intervals every price has moved RecoverPoints per interval toward its base, never past it. */
	HEROESOFLIBERTY_API FBook Recover(const FSettings& Settings, const FBook& Book, int32 Intervals);
}