// The market's exchange rule (MarketInProgress.md M1, GDD 5.5, D70, D84): pure functions of a settings struct and a book of
// prices, tested alone in script (HOLMarketRules_Test.as). Whole numbers only, so that the server and the clients agree
// to the last gold piece. Resources by index as everywhere in script: 0 food, 1 wood, 2 stone, 3 gold (gold is the
// money: it has no price). The book is the game's single common price list (D84, one for every player, allies and
// enemies); the match keeps it (M4), this file only says how a trade moves it.

// The numbers of the exchange, one place (MK-Q1: the values are not settled, two presets are ready).
struct FHOLMarketSettings
{
	// Gold for one lot of each resource at rest (index = resource; the gold entry is unused).
	TArray<int> BasePrices;

	// How far one lot sold or bought moves its price (gold per lot).
	int Step = 0;

	// The tax, in percent of the price: a sale pays the price minus it, a purchase costs the price plus it.
	int FeePercent = 0;

	// The price never leaves [MinPrice, MaxPrice].
	int MinPrice = 0;
	int MaxPrice = 0;

	// The way back to the base: every RecoverEverySeconds seconds a price moves RecoverPoints toward its base. The match
	// counts the intervals (M4) and hands them to HOLMarket::Recover.
	int RecoverPoints = 0;
	int RecoverEverySeconds = 0;
}

// The common prices: gold for one lot, by resource.
struct FHOLMarketBook
{
	TArray<int> Prices;
}

// What one trade did, or why it did not.
struct FHOLMarketTrade
{
	// True when the trade is done.
	bool bOk = false;

	// Why not: "lots", "resource", "stock" (not enough to sell) or "gold" (not enough to buy); empty when done.
	FString Reason;

	// The gold that moved: earned by a sale, paid by a purchase; 0 when refused.
	int Gold = 0;

	// The goods that moved: leaving the stock in a sale, arriving in a purchase; 0 when refused.
	int Units = 0;

	// The book after the trade (the book given, unchanged, when refused).
	FHOLMarketBook Book;
}

namespace HOLMarket
{
	// Goods in one lot: trades are made of whole lots.
	const int Lot = 100;

	// The goods that can be traded: food, wood, stone (index 0 to 2); gold is what they trade against.
	const int TradableResources = 3;

	// The values of MK-Q1: option 0 = A (AoE2 style: tax 30 %, 3 points a lot, 1 point back every 10 s), 1 = B (gentler:
	// tax 20 %, 2 points a lot, 1 point back every 5 s). Anything else: A.
	FHOLMarketSettings Preset(int Option)
	{
		FHOLMarketSettings Settings;
		for (int Resource = 0; Resource < 4; Resource++)
		{
			Settings.BasePrices.Add(100);
		}
		Settings.MinPrice = 20;
		Settings.MaxPrice = 300;
		Settings.RecoverPoints = 1;
		if (Option == 1)
		{
			Settings.FeePercent = 20;
			Settings.Step = 2;
			Settings.RecoverEverySeconds = 5;
		}
		else
		{
			Settings.FeePercent = 30;
			Settings.Step = 3;
			Settings.RecoverEverySeconds = 10;
		}
		return Settings;
	}

	// A book at rest: every price at its base.
	FHOLMarketBook NewBook(const FHOLMarketSettings& Settings)
	{
		FHOLMarketBook Book;
		for (int Resource = 0; Resource < 4; Resource++)
		{
			Book.Prices.Add(Settings.BasePrices[Resource]);
		}
		return Book;
	}

	// What a lot at this price pays when sold: the price minus the tax, rounded down.
	int SaleGold(int Price, int FeePercent)
	{
		return Price * (100 - FeePercent) / 100;
	}

	// What a lot at this price costs when bought: the price plus the tax, rounded up.
	int PurchaseGold(int Price, int FeePercent)
	{
		return (Price * (100 + FeePercent) + 99) / 100;
	}

	int ClampPrice(const FHOLMarketSettings& Settings, int Price)
	{
		return Math::Clamp(Price, Settings.MinPrice, Settings.MaxPrice);
	}

	// A refused trade: nothing moves, the book is the one given.
	FHOLMarketTrade Refuse(const FHOLMarketBook& Book, FString Reason)
	{
		FHOLMarketTrade Trade;
		Trade.Reason = Reason;
		Trade.Book = Book;
		return Trade;
	}

	// The refusal that comes before any price is read: lots not positive, or a resource that is not tradable. Empty if none.
	FString CheckRequest(int Resource, int Lots)
	{
		if (Lots <= 0)
		{
			return "lots";
		}
		if (Resource < 0 || Resource >= TradableResources)
		{
			return "resource";
		}
		return "";
	}

	// Sells Lots lots of Resource out of a stock of Stock, at the tax FeePercent (the settings' own for a usual player; a
	// lower one for the black market, D69). Each lot is paid at the price the one before left, and lowers it by a step.
	FHOLMarketTrade Sell(const FHOLMarketSettings& Settings, const FHOLMarketBook& Book, int Resource, int Lots, int Stock,
		int FeePercent)
	{
		FString Bad = CheckRequest(Resource, Lots);
		if (!Bad.IsEmpty())
		{
			return Refuse(Book, Bad);
		}
		if (Stock < Lots * Lot)
		{
			return Refuse(Book, "stock");
		}

		FHOLMarketTrade Trade;
		Trade.Book = Book;
		for (int Index = 0; Index < Lots; Index++)
		{
			int Price = Trade.Book.Prices[Resource];
			Trade.Gold += SaleGold(Price, FeePercent);
			Trade.Book.Prices[Resource] = ClampPrice(Settings, Price - Settings.Step);
		}
		Trade.Units = Lots * Lot;
		Trade.bOk = true;
		return Trade;
	}

	// Buys Lots lots of Resource with GoldStock gold, at the tax FeePercent. Each lot costs the price the one before left,
	// and raises it by a step.
	FHOLMarketTrade Buy(const FHOLMarketSettings& Settings, const FHOLMarketBook& Book, int Resource, int Lots, int GoldStock,
		int FeePercent)
	{
		FString Bad = CheckRequest(Resource, Lots);
		if (!Bad.IsEmpty())
		{
			return Refuse(Book, Bad);
		}

		FHOLMarketTrade Trade;
		Trade.Book = Book;
		for (int Index = 0; Index < Lots; Index++)
		{
			int Price = Trade.Book.Prices[Resource];
			Trade.Gold += PurchaseGold(Price, FeePercent);
			Trade.Book.Prices[Resource] = ClampPrice(Settings, Price + Settings.Step);
		}
		if (Trade.Gold > GoldStock)
		{
			return Refuse(Book, "gold");
		}
		Trade.Units = Lots * Lot;
		Trade.bOk = true;
		return Trade;
	}

	// The way back to rest: after Intervals intervals (RecoverEverySeconds seconds each) every price has moved
	// RecoverPoints per interval toward its base, never past it.
	FHOLMarketBook Recover(const FHOLMarketSettings& Settings, const FHOLMarketBook& Book, int Intervals)
	{
		FHOLMarketBook Later = Book;
		int Move = Math::Max(0, Intervals) * Settings.RecoverPoints;
		for (int Resource = 0; Resource < Later.Prices.Num() && Resource < Settings.BasePrices.Num(); Resource++)
		{
			int Base = Settings.BasePrices[Resource];
			int Price = Later.Prices[Resource];
			if (Price < Base)
			{
				Later.Prices[Resource] = Math::Min(Base, Price + Move);
			}
			else if (Price > Base)
			{
				Later.Prices[Resource] = Math::Max(Base, Price - Move);
			}
		}
		return Later;
	}
}
