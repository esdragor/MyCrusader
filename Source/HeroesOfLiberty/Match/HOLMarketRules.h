// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLMarketTypes.h"
#include "Match/HOLResources.h"

/**
 * The Market's exchange rule where the simulation runs (M4a, GDD 5.5, D70, D84, D582): the server prices every trade here.
 * The same rule as Script/Economy/HOLMarketRules.as (which the AI and the interface use to quote), whole numbers only so
 * that the server and the clients agree to the last gold piece; both are tested on the same figures (the Market.csv ones).
 * Resources by index: 0 food, 1 wood, 2 stone, 3 gold (gold is the money, it has no tradable price).
 */
/** D372 (tributes, GDD 5.5): what a gift reaches the receiver as, the Market keeping the tax. The script's rule (Script/Economy/HOLTributeRules.as) has the same figures. */
namespace HOLTribute
{
	/** About 30 % (D372, indicative, to be tuned in play). */
	constexpr int32 DefaultTaxPercent = 30;

	/** What reaches the receiver of Sent at TaxPercent: the tax rounded down (a gift of 1 is not eaten), bounded to 0..100 %. */
	HEROESOFLIBERTY_API int32 Received(int32 Sent, int32 TaxPercent);
}

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
	/** Buys Lots lots of 100 GOLD of Resource (the interface: "100 gold for X goods"): each lot gives 100 * 100 / (the lot's price plus the tax) goods, rounded down, at the price the one before left, and raises it by a step. Units = the goods, Gold = 100 per lot. */
	HEROESOFLIBERTY_API FTrade BuyWithGold(const FSettings& Settings, const FBook& Book, int32 Resource, int32 Lots, int32 GoldStock, int32 FeePercent);
	/**
	 * One trade on a player's stock (M4b): the order's end of the chain. A sale takes Lots lots of Resource from Stock and gives the gold;
	 * a purchase takes Lots lots of 100 GOLD and gives the goods (BuyWithGold); OutStock is Stock changed (Stock itself when refused). The server calls it for a
	 * player who owns a finished Market (that check, and the book kept for the match, are the caller's).
	 */
	HEROESOFLIBERTY_API FTrade Apply(const FSettings& Settings, const FBook& Book, const FHOLResources& Stock, EHOLResource Resource, int32 Lots, bool bBuy, int32 FeePercent, FHOLResources& OutStock);

	/** The Standard row of /Game/Match/MarketRates (make_market.py), as settings; false (logged) if the table or the row is missing. */
	HEROESOFLIBERTY_API bool LoadStandardSettings(FSettings& OutSettings);

	/** After Intervals intervals every price has moved RecoverPoints per interval toward its base, never past it. */
	HEROESOFLIBERTY_API FBook Recover(const FSettings& Settings, const FBook& Book, int32 Intervals);
}