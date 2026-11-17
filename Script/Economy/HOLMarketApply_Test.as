// HOLMarket::Apply, the order's end of the chain (MIG-C++-FINAL, D836 point 4: the script is the only source of the trade rule).
// One trade on a stock: a sale gives Sell, a purchase gives BuyWithGold; the stock asked is the one given up (the goods for
// a sale, the gold for a purchase). RED until HOLMarket::Apply exists in HOLMarketRules.as (compiles after the build).

// A sale goes through Sell: same gold, same units, same book as Sell asked directly.
void Test_MarketApplySellIsSell(FUnitTest& T)
{
	FHOLMarketSettings S = HOLMarket::DefaultSettings();
	FHOLMarketBook Book = HOLMarket::NewBook(S);
	FHOLMarketTrade Direct = HOLMarket::Sell(S, Book, 1, 2, 500, S.FeePercent);
	FHOLMarketTrade Applied = HOLMarket::Apply(S, Book, 1, 2, false, 500, S.FeePercent);
	T.AssertTrue(Applied.bOk, "sale accepted");
	T.AssertEquals(Direct.Gold, Applied.Gold, "the gold of Sell");
	T.AssertEquals(Direct.Units, Applied.Units, "the goods of Sell");
	T.AssertEquals(Direct.Book.Prices[1], Applied.Book.Prices[1], "the price Sell leaves");
}

// A purchase goes through BuyWithGold: the gold given up is 100 a lot, the goods are what it buys.
void Test_MarketApplyBuyIsBuyWithGold(FUnitTest& T)
{
	FHOLMarketSettings S = HOLMarket::DefaultSettings();
	FHOLMarketBook Book = HOLMarket::NewBook(S);
	FHOLMarketTrade Direct = HOLMarket::BuyWithGold(S, Book, 0, 3, 1000, S.FeePercent);
	FHOLMarketTrade Applied = HOLMarket::Apply(S, Book, 0, 3, true, 1000, S.FeePercent);
	T.AssertTrue(Applied.bOk, "purchase accepted");
	T.AssertEquals(300, Applied.Gold, "100 gold a lot");
	T.AssertEquals(Direct.Units, Applied.Units, "the goods of BuyWithGold");
	T.AssertEquals(Direct.Book.Prices[0], Applied.Book.Prices[0], "the price BuyWithGold leaves");
}

// Refusals keep the book and say why: not enough goods to sell, not enough gold to buy.
void Test_MarketApplyRefusals(FUnitTest& T)
{
	FHOLMarketSettings S = HOLMarket::DefaultSettings();
	FHOLMarketBook Book = HOLMarket::NewBook(S);
	FHOLMarketTrade Sale = HOLMarket::Apply(S, Book, 2, 1, false, 50, S.FeePercent);
	T.AssertFalse(Sale.bOk, "50 stone cannot pay a lot of 100");
	T.AssertEquals("stock", Sale.Reason, "refused for the stock");
	T.AssertEquals(Book.Prices[2], Sale.Book.Prices[2], "the book is unchanged");
	FHOLMarketTrade Purchase = HOLMarket::Apply(S, Book, 2, 1, true, 99, S.FeePercent);
	T.AssertFalse(Purchase.bOk, "99 gold cannot buy a lot of 100 gold");
	T.AssertEquals("gold", Purchase.Reason, "refused for the gold");
}
