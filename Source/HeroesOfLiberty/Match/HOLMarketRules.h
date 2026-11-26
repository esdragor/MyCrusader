// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLMarketTypes.h"
#include "Match/HOLResources.h"

/**
 * The Market's numbers on the C++ side (M4a, GDD 5.5, D70, D84, D582): the settings, the book and the trade's result, as the bridge
 * UHOLMarketSubsystem carries them. The exchange rule itself is Script/Economy/HOLMarketRules.as alone (D836 point 4), whole numbers only
 * so that the server and the clients agree to the last gold piece.
 * Resources by index: 0 food, 1 wood, 2 stone, 3 gold (gold is the money, it has no tradable price).
 */
/** D372 (tributes, GDD 5.5): the Market keeps a tax on a gift. What a gift reaches the receiver as is the script's rule (Script/Economy/HOLTributeRules.as), reached through UHOLMarketSubsystem::SettleTribute; only the default tax figure stays here. */
namespace HOLTribute
{
	/** About 30 % (D372, indicative, to be tuned in play). */
	constexpr int32 DefaultTaxPercent = 30;
	/** The script's reason number for "stock too short" (HOLTribute::NotEnough in HOLTributeRules.as, held by the Market.ServerBoard test): ApplyTribute tells it from the other refusals in the journal. */
	constexpr int32 NotEnoughReason = 4;
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
	// Sell, BuyWithGold, Buy and Apply are the script's only (Script/Economy/HOLMarketRules.as, D836 point 4): the server's trade through
	// UHOLMarketSubsystem::Apply, the grid's quote through UHOLMarketSubsystem::Quote (CLAUDE-1125s removed the last C++ copy).

	/** The Standard row of /Game/Match/MarketRates (make_market.py), as settings; false (logged) if the table or the row is missing. */
	HEROESOFLIBERTY_API bool LoadStandardSettings(FSettings& OutSettings);
}