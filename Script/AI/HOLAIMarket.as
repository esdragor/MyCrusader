// What the AI does with the Market (MarketInProgress.md M6, GDD 5.5, question MK-Q3): a pure choice over its stock, what it
// wants and the common book, tested alone in script (HOLAIMarket_Test.as). One trade at a time: the brain asks again at its
// next decision. Resources by index as everywhere in script: 0 food, 1 wood, 2 stone, 3 gold. The rule's numbers are in one
// struct so that the answer to MK-Q3 (when the AI uses the Market, how much it keeps) changes a value, not the rule.

// The numbers of the rule.
struct FHOLAIMarketSettings
{
	// Never sells below this (the reserve kept), nor a resource that is not above SellAbove.
	int Reserve = 300;
	int SellAbove = 600;

	// Lots in one trade at most.
	int MaxLots = 5;

	// MK-Q3 C: the lowest difficulty (0 easy, 1 normal, 2 hard) that uses the Market; 0 = every difficulty (A), 3 = none (B).
	int MinDifficulty = 0;
}

// What the AI wants to do.
struct FHOLAIMarketChoice
{
	bool bTrade = false;
	bool bBuy = false;
	int Resource = 0;
	int Lots = 0;
	FString Reason;
}

namespace HOLAIMarket
{
	FHOLAIMarketChoice None(FString Reason)
	{
		FHOLAIMarketChoice Choice;
		Choice.Reason = Reason;
		return Choice;
	}

	// The one trade the AI wants now, or none. Stock and Wanted are by resource (what it holds, what it wants to hold over its
	// next window: the brain fills them from its needs, HOLAINeeds.as). The resource most missing is bought with gold (as many
	// lots as the gold pays for, MaxLots at most); with too little gold, or when gold itself is missing, the biggest surplus
	// (above SellAbove, keeping the reserve and what it wants of it) is sold; the resource that is missing is never the one sold.
	FHOLAIMarketChoice Choose(const FHOLMarketSettings& Market, const FHOLMarketBook& Book, const FHOLAIMarketSettings& Rule,
		const TArray<int>& Stock, const TArray<int>& Wanted, bool bMarketReady, int Difficulty)
	{
		if (!bMarketReady)
		{
			return None("no market");
		}
		if (Difficulty < Rule.MinDifficulty)
		{
			return None("difficulty");
		}

		// What is missing most.
		int Missing = 0;
		int Needed = -1;
		for (int Resource = 0; Resource < 4; Resource++)
		{
			int Lack = Wanted[Resource] - Stock[Resource];
			if (Lack > Missing)
			{
				Missing = Lack;
				Needed = Resource;
			}
		}
		if (Needed < 0)
		{
			return None("nothing missing");
		}

		// Buy it with the gold there is.
		if (Needed < HOLMarket::TradableResources)
		{
			int Lots = Math::Min(Rule.MaxLots, (Missing + HOLMarket::Lot - 1) / HOLMarket::Lot);
			while (Lots >= 1)
			{
				FHOLMarketTrade Trade = HOLMarket::Buy(Market, Book, Needed, Lots, Stock[3], Market.FeePercent);
				if (Trade.bOk)
				{
					FHOLAIMarketChoice Choice;
					Choice.bTrade = true;
					Choice.bBuy = true;
					Choice.Resource = Needed;
					Choice.Lots = Lots;
					Choice.Reason = "buy what is missing";
					return Choice;
				}
				Lots--;
			}
		}

		// Else sell the biggest surplus.
		int Seller = -1;
		int Surplus = 0;
		for (int Resource = 0; Resource < HOLMarket::TradableResources; Resource++)
		{
			if (Resource == Needed || Stock[Resource] <= Rule.SellAbove)
			{
				continue;
			}
			int Keep = Math::Max(Wanted[Resource], Rule.Reserve);
			int Extra = Stock[Resource] - Keep;
			if (Extra >= HOLMarket::Lot && Extra > Surplus)
			{
				Surplus = Extra;
				Seller = Resource;
			}
		}
		if (Seller < 0)
		{
			return None("no surplus");
		}
		FHOLAIMarketChoice Sale;
		Sale.bTrade = true;
		Sale.bBuy = false;
		Sale.Resource = Seller;
		Sale.Lots = Math::Min(Rule.MaxLots, Surplus / HOLMarket::Lot);
		Sale.Reason = "sell the surplus";
		return Sale;
	}
}