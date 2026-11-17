// HOLMarket::NewBook, the book at rest the match starts from (MIG-Market, D836 point 4: UHOLMarketSubsystem::NewBook asks the script; the C++
// copy is gone). The bridge hands the script a settings struct whose price array may be short (a missing price counts as 0, as
// UHOLMarketSubsystem::FromBridge does), so NewBook must always answer one price per resource. RED until NewBook guards a short array
// (it indexes BasePrices[Resource] unchecked today; compiles and runs only after the build).

// The book at rest: one price per resource, each at its base (the figures the C++ test Market.Rules held: wood 80, stone 120).
void Test_MarketNewBookAtRest(FUnitTest& T)
{
	FHOLMarketSettings S = HOLMarket::DefaultSettings();
	FHOLMarketBook Book = HOLMarket::NewBook(S);
	T.AssertEquals(4, Book.Prices.Num(), "one price per resource");
	T.AssertEquals(S.BasePrices[MarketFood], Book.Prices[MarketFood], "food at its base");
	T.AssertEquals(80, Book.Prices[MarketWood], "fresh book: wood at 80");
	T.AssertEquals(120, Book.Prices[MarketStone], "fresh book: stone at 120");
	T.AssertEquals(S.BasePrices[MarketGold], Book.Prices[MarketGold], "gold at its (unused) base");
}

// A short price array (what a bridge struct may carry): the missing prices are 0, the book still has four entries.
void Test_MarketNewBookShortSettings(FUnitTest& T)
{
	FHOLMarketSettings S;
	S.BasePrices.Add(60);
	S.BasePrices.Add(90);
	FHOLMarketBook Book = HOLMarket::NewBook(S);
	T.AssertEquals(4, Book.Prices.Num(), "four prices from a two-price settings");
	T.AssertEquals(60, Book.Prices[0], "the first price kept");
	T.AssertEquals(90, Book.Prices[1], "the second price kept");
	T.AssertEquals(0, Book.Prices[2], "a missing price counts as 0");
	T.AssertEquals(0, Book.Prices[3], "a missing price counts as 0");
}
