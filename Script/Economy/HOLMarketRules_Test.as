// The market's exchange rule, alone (MarketInProgress.md M1, GDD 5.5, D70, D84): books and stocks built from the preset
// (never numbers typed in the expectations when the preset says them), HOLMarket::Sell / Buy / Recover asked.
// Resources by index as everywhere in script: 0 food, 1 wood, 2 stone, 3 gold.

const int MarketFood = 0;
const int MarketWood = 1;
const int MarketStone = 2;
const int MarketGold = 3;

// The default preset (C of MK-Q1, D582).
FHOLMarketSettings MarketSettings()
{
	return HOLMarket::DefaultSettings();
}

// Every price of the book at its base; the preset's own numbers are read, not retyped.
void Test_MarketFreshBookAtBase(FUnitTest& T)
{
	FHOLMarketSettings S = MarketSettings();
	FHOLMarketBook Book = HOLMarket::NewBook(S);
	T.AssertEquals(4, Book.Prices.Num(), "one price per resource");
	for (int Resource = 0; Resource < 4; Resource++)
	{
		T.AssertEquals(S.BasePrices[Resource], Book.Prices[Resource], f"resource {Resource}: price at base");
	}
	T.AssertTrue(S.FeePercent > 0, "the preset has a tax");
	T.AssertTrue(S.Step > 0, "the preset moves the price");
}

// One lot of wood sold at the base: the gold is the base minus the tax, the price falls by one step.
void Test_MarketSellOneLot(FUnitTest& T)
{
	FHOLMarketSettings S = MarketSettings();
	FHOLMarketBook Book = HOLMarket::NewBook(S);
	int Base = Book.Prices[MarketWood];
	FHOLMarketTrade Trade = HOLMarket::Sell(S, Book, MarketWood, 1, 500, S.FeePercent);
	T.AssertTrue(Trade.bOk, "sale accepted");
	T.AssertTrue(Trade.Reason.IsEmpty(), "no refusal reason");
	T.AssertEquals(100, Trade.Units, "one lot of wood leaves the stock");
	T.AssertEquals(Base * (100 - S.FeePercent) / 100, Trade.Gold, "gold = price minus the tax");
	T.AssertEquals(Base - S.Step, Trade.Book.Prices[MarketWood], "the price falls by one step");
	T.AssertEquals(Base, Trade.Book.Prices[MarketFood], "food untouched");
	T.AssertEquals(Base, Book.Prices[MarketWood], "the book given is not changed");
}

// One lot bought: the cost is the price plus the tax, the price rises by one step.
void Test_MarketBuyOneLot(FUnitTest& T)
{
	FHOLMarketSettings S = MarketSettings();
	FHOLMarketBook Book = HOLMarket::NewBook(S);
	int Base = Book.Prices[MarketStone];
	FHOLMarketTrade Trade = HOLMarket::Buy(S, Book, MarketStone, 1, 1000, S.FeePercent);
	T.AssertTrue(Trade.bOk, "purchase accepted");
	T.AssertEquals(100, Trade.Units, "one lot of stone arrives");
	T.AssertEquals((Base * (100 + S.FeePercent) + 99) / 100, Trade.Gold, "gold paid = price plus the tax, rounded up");
	T.AssertEquals(Base + S.Step, Trade.Book.Prices[MarketStone], "the price rises by one step");
}

// The interface's purchase (user's correction 2026-11-01): 100 gold buys X goods, lot by lot; X is read from the preset, not typed.
void Test_MarketBuyWithGold(FUnitTest& T)
{
	FHOLMarketSettings S = MarketSettings();
	FHOLMarketBook Book = HOLMarket::NewBook(S);
	int Price = Book.Prices[MarketWood];
	FHOLMarketTrade One = HOLMarket::BuyWithGold(S, Book, MarketWood, 1, 1000, S.FeePercent);
	T.AssertTrue(One.bOk, "purchase accepted");
	T.AssertEquals(100, One.Gold, "a lot spends 100 gold");
	T.AssertEquals(HOLMarket::Lot * HOLMarket::Lot / HOLMarket::PurchaseGold(Price, S.FeePercent), One.Units, "goods = 100 gold over the price of 100 goods");
	T.AssertEquals(96, One.Units, "wood at 80, tax 30 %: 96 wood for 100 gold");
	T.AssertEquals(Price + S.Step, One.Book.Prices[MarketWood], "the price rises by a step");
	FHOLMarketTrade Two = HOLMarket::BuyWithGold(S, Book, MarketWood, 2, 1000, S.FeePercent);
	T.AssertEquals(200, Two.Gold, "two lots: 200 gold");
	T.AssertEquals(96 + 92, Two.Units, "the second lot at the price the first left (83)");
	T.AssertEquals(64, HOLMarket::BuyWithGold(S, Book, MarketStone, 1, 1000, S.FeePercent).Units, "stone at 120: 64");
	T.AssertEquals("gold", HOLMarket::BuyWithGold(S, Book, MarketWood, 1, 99, S.FeePercent).Reason, "99 gold: refused");
	T.AssertEquals("resource", HOLMarket::BuyWithGold(S, Book, MarketGold, 1, 1000, S.FeePercent).Reason, "gold is not bought");
	T.AssertEquals("lots", HOLMarket::BuyWithGold(S, Book, MarketWood, 0, 1000, S.FeePercent).Reason, "no lots");
}

// Several lots: each one is paid at the price the one before left (the price moves lot by lot).
void Test_MarketSellSeveralLotsLotByLot(FUnitTest& T)
{
	FHOLMarketSettings S = MarketSettings();
	FHOLMarketBook Book = HOLMarket::NewBook(S);
	int Price = Book.Prices[MarketFood];
	int Expected = 0;
	for (int Lot = 0; Lot < 3; Lot++)
	{
		Expected += Price * (100 - S.FeePercent) / 100;
		Price -= S.Step;
	}
	FHOLMarketTrade Trade = HOLMarket::Sell(S, Book, MarketFood, 3, 1000, S.FeePercent);
	T.AssertTrue(Trade.bOk, "sale accepted");
	T.AssertEquals(300, Trade.Units, "three lots leave the stock");
	T.AssertEquals(Expected, Trade.Gold, "each lot at the price left by the one before");
	T.AssertEquals(Price, Trade.Book.Prices[MarketFood], "the price after three lots");
}

// The common price: a second seller (the same book handed on) gets less than the first.
void Test_MarketSharedBookSecondSellerGetsLess(FUnitTest& T)
{
	FHOLMarketSettings S = MarketSettings();
	FHOLMarketBook Book = HOLMarket::NewBook(S);
	FHOLMarketTrade First = HOLMarket::Sell(S, Book, MarketWood, 2, 1000, S.FeePercent);
	FHOLMarketTrade Second = HOLMarket::Sell(S, First.Book, MarketWood, 2, 1000, S.FeePercent);
	T.AssertTrue(First.bOk && Second.bOk, "both sales accepted");
	T.AssertTrue(Second.Gold < First.Gold, "the second seller, on the price the first left, earns less");
	// A buyer then pays less than at the base (the price is low), and wood is cheaper for the adversary too.
	FHOLMarketTrade Buyer = HOLMarket::Buy(S, Second.Book, MarketWood, 1, 1000, S.FeePercent);
	T.AssertTrue(Buyer.Gold < (Book.Prices[MarketWood] * (100 + S.FeePercent) + 99) / 100, "buying on a sold-off price costs less");
}

// The tax makes a round trip lose: selling a lot then buying it back costs more than it paid.
void Test_MarketRoundTripLoses(FUnitTest& T)
{
	FHOLMarketSettings S = MarketSettings();
	FHOLMarketBook Book = HOLMarket::NewBook(S);
	FHOLMarketTrade Sold = HOLMarket::Sell(S, Book, MarketWood, 1, 100, S.FeePercent);
	FHOLMarketTrade Bought = HOLMarket::Buy(S, Sold.Book, MarketWood, 1, 1000, S.FeePercent);
	T.AssertTrue(Sold.bOk && Bought.bOk, "both accepted");
	T.AssertTrue(Bought.Gold > Sold.Gold, "buying back costs more than the sale paid");
	T.AssertEquals(Book.Prices[MarketWood], Bought.Book.Prices[MarketWood], "the price is back where it was");
}

// The refusals: nothing changes, the reason says why.
void Test_MarketRefusals(FUnitTest& T)
{
	FHOLMarketSettings S = MarketSettings();
	FHOLMarketBook Book = HOLMarket::NewBook(S);

	FHOLMarketTrade Zero = HOLMarket::Sell(S, Book, MarketWood, 0, 500, S.FeePercent);
	T.AssertFalse(Zero.bOk, "zero lots refused");
	T.AssertEquals("lots", Zero.Reason, "zero lots: reason");

	FHOLMarketTrade GoldSale = HOLMarket::Sell(S, Book, MarketGold, 1, 500, S.FeePercent);
	T.AssertFalse(GoldSale.bOk, "gold is not sold for gold");
	T.AssertEquals("resource", GoldSale.Reason, "gold sale: reason");

	FHOLMarketTrade GoldBuy = HOLMarket::Buy(S, Book, MarketGold, 1, 500, S.FeePercent);
	T.AssertFalse(GoldBuy.bOk, "gold is not bought with gold");
	T.AssertEquals("resource", GoldBuy.Reason, "gold purchase: reason");

	FHOLMarketTrade Short = HOLMarket::Sell(S, Book, MarketWood, 2, 150, S.FeePercent);
	T.AssertFalse(Short.bOk, "two lots from 150 refused");
	T.AssertEquals("stock", Short.Reason, "not enough stock: reason");

	FHOLMarketTrade Poor = HOLMarket::Buy(S, Book, MarketStone, 1, 10, S.FeePercent);
	T.AssertFalse(Poor.bOk, "a lot with 10 gold refused");
	T.AssertEquals("gold", Poor.Reason, "not enough gold: reason");

	T.AssertEquals(0, Poor.Gold, "a refusal moves no gold");
	T.AssertEquals(0, Poor.Units, "a refusal moves no goods");
	T.AssertEquals(Book.Prices[MarketStone], Poor.Book.Prices[MarketStone], "a refusal leaves the prices");
}

// The bounds: the price never leaves [Min, Max]; a sale at the floor and a purchase at the cap still go through.
void Test_MarketBounds(FUnitTest& T)
{
	FHOLMarketSettings S = MarketSettings();
	FHOLMarketBook Low = HOLMarket::NewBook(S);
	Low.Prices[MarketWood] = S.MinPrice + 1;
	FHOLMarketTrade Down = HOLMarket::Sell(S, Low, MarketWood, 3, 1000, S.FeePercent);
	T.AssertTrue(Down.bOk, "sale near the floor accepted");
	T.AssertEquals(S.MinPrice, Down.Book.Prices[MarketWood], "the price stops at the floor");

	FHOLMarketBook High = HOLMarket::NewBook(S);
	High.Prices[MarketWood] = S.MaxPrice - 1;
	FHOLMarketTrade Up = HOLMarket::Buy(S, High, MarketWood, 3, 100000, S.FeePercent);
	T.AssertTrue(Up.bOk, "purchase near the cap accepted");
	T.AssertEquals(S.MaxPrice, Up.Book.Prices[MarketWood], "the price stops at the cap");
}

// The rate is a parameter: a better one (the black market, D69) pays more on sale and charges less on purchase.
void Test_MarketBetterRate(FUnitTest& T)
{
	FHOLMarketSettings S = MarketSettings();
	FHOLMarketBook Book = HOLMarket::NewBook(S);
	int Better = S.FeePercent / 2;
	FHOLMarketTrade Normal = HOLMarket::Sell(S, Book, MarketFood, 1, 100, S.FeePercent);
	FHOLMarketTrade Black = HOLMarket::Sell(S, Book, MarketFood, 1, 100, Better);
	T.AssertTrue(Black.Gold > Normal.Gold, "a better rate pays more");
	FHOLMarketTrade NormalBuy = HOLMarket::Buy(S, Book, MarketFood, 1, 1000, S.FeePercent);
	FHOLMarketTrade BlackBuy = HOLMarket::Buy(S, Book, MarketFood, 1, 1000, Better);
	T.AssertTrue(BlackBuy.Gold < NormalBuy.Gold, "a better rate charges less");
	FHOLMarketTrade Free = HOLMarket::Sell(S, Book, MarketFood, 1, 100, 0);
	T.AssertEquals(Book.Prices[MarketFood], Free.Gold, "no tax: the whole price");
}

// The way back: every price moves toward its base by RecoverPoints per interval, never past it.
void Test_MarketRecoverTowardBase(FUnitTest& T)
{
	FHOLMarketSettings S = MarketSettings();
	FHOLMarketBook Book = HOLMarket::NewBook(S);
	Book.Prices[MarketFood] = Book.Prices[MarketFood] - 10;
	Book.Prices[MarketWood] = Book.Prices[MarketWood] + 10;

	FHOLMarketBook Later = HOLMarket::Recover(S, Book, 2);
	T.AssertEquals(Book.Prices[MarketFood] + 2 * S.RecoverPoints, Later.Prices[MarketFood], "a low price rises");
	T.AssertEquals(Book.Prices[MarketWood] - 2 * S.RecoverPoints, Later.Prices[MarketWood], "a high price falls");
	T.AssertEquals(Book.Prices[MarketStone], Later.Prices[MarketStone], "a price at base stays");

	FHOLMarketBook Long = HOLMarket::Recover(S, Book, 1000);
	T.AssertEquals(S.BasePrices[MarketFood], Long.Prices[MarketFood], "a long wait: back to base, not past it (low)");
	T.AssertEquals(S.BasePrices[MarketWood], Long.Prices[MarketWood], "a long wait: back to base, not past it (high)");
}

// The two presets of MK-Q1 are both whole: B is gentler than A on the tax and on the step.
void Test_MarketPresets(FUnitTest& T)
{
	FHOLMarketSettings A = HOLMarket::Preset(0);
	FHOLMarketSettings B = HOLMarket::Preset(1);
	T.AssertTrue(B.FeePercent < A.FeePercent, "B taxes less than A");
	T.AssertTrue(B.Step < A.Step, "B moves the price less than A");
	T.AssertTrue(A.MinPrice < A.MaxPrice && B.MinPrice < B.MaxPrice, "bounds in order");
	for (int Resource = 0; Resource < 4; Resource++)
	{
		T.AssertTrue(A.BasePrices[Resource] >= A.MinPrice && A.BasePrices[Resource] <= A.MaxPrice, f"A: base {Resource} within the bounds");
		T.AssertTrue(B.BasePrices[Resource] >= B.MinPrice && B.BasePrices[Resource] <= B.MaxPrice, f"B: base {Resource} within the bounds");
	}
}

// MK-Q1 = C (D582): base prices 80 food, 80 wood, 120 stone, 100 gold; the rest as A (tax 30 %, 3 points a lot, 1 point
// back every 10 s, bounds 20 to 300). The numbers are the user's answer, so they are typed here on purpose.
void Test_MarketPresetCNumbers(FUnitTest& T)
{
	FHOLMarketSettings C = HOLMarket::Preset(2);
	T.AssertEquals(4, C.BasePrices.Num(), "C: one base price per resource");
	T.AssertEquals(80, C.BasePrices[MarketFood], "C: food base");
	T.AssertEquals(80, C.BasePrices[MarketWood], "C: wood base");
	T.AssertEquals(120, C.BasePrices[MarketStone], "C: stone base");
	T.AssertEquals(100, C.BasePrices[MarketGold], "C: gold base");
	T.AssertEquals(30, C.FeePercent, "C: tax 30 %");
	T.AssertEquals(3, C.Step, "C: 3 points a lot");
	T.AssertEquals(1, C.RecoverPoints, "C: 1 point back");
	T.AssertEquals(10, C.RecoverEverySeconds, "C: every 10 s");
	T.AssertEquals(20, C.MinPrice, "C: floor 20");
	T.AssertEquals(300, C.MaxPrice, "C: cap 300");
}

// C is the default: an option nobody names (and the named default) gives C; A and B stay reachable by 0 and 1.
void Test_MarketDefaultIsC(FUnitTest& T)
{
	FHOLMarketSettings C = HOLMarket::Preset(2);
	FHOLMarketSettings Unnamed = HOLMarket::Preset(-1);
	FHOLMarketSettings Far = HOLMarket::Preset(99);
	T.AssertEquals(80, Unnamed.BasePrices[MarketFood], "unnamed option: food at 80");
	T.AssertEquals(120, Far.BasePrices[MarketStone], "far option: stone at 120");
	for (int Resource = 0; Resource < 4; Resource++)
	{
		T.AssertEquals(C.BasePrices[Resource], Unnamed.BasePrices[Resource], f"unnamed option: base {Resource} as C");
		T.AssertEquals(C.BasePrices[Resource], Far.BasePrices[Resource], f"far option: base {Resource} as C");
	}
	T.AssertEquals(100, HOLMarket::Preset(0).BasePrices[MarketFood], "A is still option 0 (food 100)");
	T.AssertEquals(100, HOLMarket::Preset(1).BasePrices[MarketStone], "B is still option 1 (stone 100)");
}

// A round trip with C, by the figures: 1 lot of wood sold at 80 pays 56 (80 minus 30 %), the price falls to 77, buying a lot
// back costs 101 (77 plus 30 %, rounded up) and the price returns to 80; stone, 1 lot sold at 120 pays 84, bought back at 117 costs 153.
void Test_MarketRoundTripC(FUnitTest& T)
{
	FHOLMarketSettings C = HOLMarket::Preset(2);
	FHOLMarketBook Book = HOLMarket::NewBook(C);
	T.AssertEquals(80, Book.Prices[MarketWood], "C: fresh book, wood at 80");
	FHOLMarketTrade Sold = HOLMarket::Sell(C, Book, MarketWood, 1, 100, C.FeePercent);
	T.AssertEquals(56, Sold.Gold, "C: a lot of wood at 80 pays 56");
	T.AssertEquals(77, Sold.Book.Prices[MarketWood], "C: wood falls to 77");
	FHOLMarketTrade Bought = HOLMarket::Buy(C, Sold.Book, MarketWood, 1, 1000, C.FeePercent);
	T.AssertEquals(101, Bought.Gold, "C: a lot of wood at 77 costs 101");
	T.AssertEquals(80, Bought.Book.Prices[MarketWood], "C: wood back at 80");

	FHOLMarketTrade StoneSold = HOLMarket::Sell(C, Book, MarketStone, 1, 100, C.FeePercent);
	T.AssertEquals(84, StoneSold.Gold, "C: a lot of stone at 120 pays 84");
	FHOLMarketTrade StoneBought = HOLMarket::Buy(C, StoneSold.Book, MarketStone, 1, 1000, C.FeePercent);
	T.AssertEquals(153, StoneBought.Gold, "C: a lot of stone at 117 costs 153");
}

// Recovery with C: wood sold down to 71 comes back one point per interval, to 80 after 9 intervals and no further.
void Test_MarketRecoverC(FUnitTest& T)
{
	FHOLMarketSettings C = HOLMarket::Preset(2);
	FHOLMarketBook Book = HOLMarket::NewBook(C);
	FHOLMarketTrade Sold = HOLMarket::Sell(C, Book, MarketWood, 3, 300, C.FeePercent);
	T.AssertEquals(71, Sold.Book.Prices[MarketWood], "C: three lots sold, wood at 71");
	T.AssertEquals(75, HOLMarket::Recover(C, Sold.Book, 4).Prices[MarketWood], "C: 4 intervals, wood at 75");
	T.AssertEquals(80, HOLMarket::Recover(C, Sold.Book, 9).Prices[MarketWood], "C: 9 intervals, wood back at 80");
	T.AssertEquals(80, HOLMarket::Recover(C, Sold.Book, 50).Prices[MarketWood], "C: no further than 80");
}

// MIG-J-Market (D830, D835): the vectors of the C++ twin's pure part, HeroesOfLiberty.Market.Rules (HOLMarketRulesTests.cpp), name for
// name, so the script and the C++ are held to the same figures while both copies exist. The C++ reads its settings from Tools/Data/Market.csv
// (row Standard): the "settings:" lines below hold the script's default preset to the figures that C++ test reads from the table.
void Test_MarketRulesVectors(FUnitTest& T)
{
	FHOLMarketSettings S = MarketSettings();
	T.AssertEquals(80, S.BasePrices[MarketFood], "settings: food base");
	T.AssertEquals(120, S.BasePrices[MarketStone], "settings: stone base");
	T.AssertEquals(100, S.BasePrices[MarketGold], "settings: gold base");
	T.AssertEquals(30, S.FeePercent, "settings: tax");
	T.AssertEquals(3, S.Step, "settings: step");
	T.AssertEquals(20, S.MinPrice, "settings: floor");
	T.AssertEquals(300, S.MaxPrice, "settings: cap");

	FHOLMarketBook Book = HOLMarket::NewBook(S);
	T.AssertEquals(80, Book.Prices[MarketWood], "fresh book: wood at 80");
	T.AssertEquals(120, Book.Prices[MarketStone], "fresh book: stone at 120");

	// The round trip: wood sold at 80 pays 56 and falls to 77; bought back at 77 costs 101 and returns to 80.
	FHOLMarketTrade Sold = HOLMarket::Sell(S, Book, MarketWood, 1, 100, S.FeePercent);
	T.AssertTrue(Sold.bOk, "a lot of wood sold");
	T.AssertEquals(56, Sold.Gold, "it pays 56");
	T.AssertEquals(100, Sold.Units, "100 wood leave the stock");
	T.AssertEquals(77, Sold.Book.Prices[MarketWood], "wood falls to 77");
	T.AssertEquals(80, Book.Prices[MarketWood], "the book given is not changed");
	FHOLMarketTrade Bought = HOLMarket::Buy(S, Sold.Book, MarketWood, 1, 1000, S.FeePercent);
	T.AssertTrue(Bought.bOk, "a lot of wood bought");
	T.AssertEquals(101, Bought.Gold, "it costs 101");
	T.AssertEquals(80, Bought.Book.Prices[MarketWood], "wood returns to 80");

	// Stone: 120 sold pays 84, bought back at 117 costs 153.
	FHOLMarketTrade StoneSold = HOLMarket::Sell(S, Book, MarketStone, 1, 100, S.FeePercent);
	T.AssertEquals(84, StoneSold.Gold, "a lot of stone pays 84");
	T.AssertEquals(153, HOLMarket::Buy(S, StoneSold.Book, MarketStone, 1, 1000, S.FeePercent).Gold, "a lot of stone bought back costs 153");

	// Lot by lot: three lots of wood at 80, 77, 74.
	FHOLMarketTrade Three = HOLMarket::Sell(S, Book, MarketWood, 3, 300, S.FeePercent);
	T.AssertEquals(56 + 53 + 51, Three.Gold, "three lots, each at the price left by the one before");
	T.AssertEquals(71, Three.Book.Prices[MarketWood], "three lots: wood at 71");

	// The refusals change nothing.
	T.AssertEquals("lots", HOLMarket::Sell(S, Book, MarketWood, 0, 500, S.FeePercent).Reason, "zero lots");
	T.AssertEquals("resource", HOLMarket::Sell(S, Book, MarketGold, 1, 500, S.FeePercent).Reason, "gold is not traded");
	T.AssertEquals("stock", HOLMarket::Sell(S, Book, MarketWood, 2, 150, S.FeePercent).Reason, "two lots from 150");
	FHOLMarketTrade Poor = HOLMarket::Buy(S, Book, MarketStone, 1, 10, S.FeePercent);
	T.AssertEquals("gold", Poor.Reason, "a lot with 10 gold");
	T.AssertTrue(!Poor.bOk && Poor.Gold == 0 && Poor.Units == 0 && Poor.Book.Prices[MarketStone] == 120, "a refusal moves nothing");

	// A purchase by lots of 100 gold, each lot at the price the one before left.
	FHOLMarketTrade Gold1 = HOLMarket::BuyWithGold(S, Book, MarketWood, 1, 1000, S.FeePercent);
	T.AssertTrue(Gold1.bOk, "100 gold spent on wood");
	T.AssertEquals(100, Gold1.Gold, "it spends 100 gold");
	T.AssertEquals(96, Gold1.Units, "it brings 96 wood (80 plus 30 % = 104 per 100 wood)");
	T.AssertEquals(83, Gold1.Book.Prices[MarketWood], "wood rises to 83");
	FHOLMarketTrade Gold2 = HOLMarket::BuyWithGold(S, Book, MarketWood, 2, 1000, S.FeePercent);
	T.AssertEquals(200, Gold2.Gold, "two lots of gold: 200 gold");
	T.AssertEquals(96 + 92, Gold2.Units, "two lots of gold: 96 + 92 wood (83 plus 30 % = 108)");
	T.AssertEquals(86, Gold2.Book.Prices[MarketWood], "two lots of gold: wood at 86");
	T.AssertEquals(64, HOLMarket::BuyWithGold(S, Book, MarketStone, 1, 1000, S.FeePercent).Units, "100 gold on stone brings 64");
	T.AssertEquals("gold", HOLMarket::BuyWithGold(S, Book, MarketWood, 1, 99, S.FeePercent).Reason, "with 99 gold: refused (gold)");
	T.AssertEquals("resource", HOLMarket::BuyWithGold(S, Book, MarketGold, 1, 1000, S.FeePercent).Reason, "gold is not bought with gold");

	// The bounds, and the way back.
	FHOLMarketBook Low = Book;
	Low.Prices[MarketWood] = S.MinPrice + 1;
	T.AssertEquals(S.MinPrice, HOLMarket::Sell(S, Low, MarketWood, 3, 1000, S.FeePercent).Book.Prices[MarketWood], "the price stops at the floor");
	FHOLMarketBook High = Book;
	High.Prices[MarketWood] = S.MaxPrice - 1;
	T.AssertEquals(S.MaxPrice, HOLMarket::Buy(S, High, MarketWood, 3, 100000, S.FeePercent).Book.Prices[MarketWood], "the price stops at the cap");
	T.AssertEquals(75, HOLMarket::Recover(S, Three.Book, 4).Prices[MarketWood], "4 intervals: wood 71 to 75");
	T.AssertEquals(80, HOLMarket::Recover(S, Three.Book, 9).Prices[MarketWood], "9 intervals: back to 80");
	T.AssertEquals(80, HOLMarket::Recover(S, Three.Book, 50).Prices[MarketWood], "no further than the base");
	T.AssertEquals(S.MaxPrice - 3, HOLMarket::Recover(S, High, 2).Prices[MarketWood], "a price above the base comes down");
}

// MIG-J-Market: the pure head of HeroesOfLiberty.Market.Tribute (HOLMarketRulesTests.cpp), name for name; its world part (order queue,
// production step, stocks) stays in C++.
void Test_MarketTributeVectors(FUnitTest& T)
{
	T.AssertEquals(30, HOLTribute::DefaultTaxPercent, "the tax is 30 % by default");
	T.AssertEquals(70, HOLTribute::Received(100, 30), "100 sent: 70 arrive");
	T.AssertEquals(1, HOLTribute::Received(1, 30), "1 sent: the tax rounds down, the gift is not eaten");
	T.AssertEquals(0, HOLTribute::Received(100, 250), "a tax over 100 % is bounded");
	T.AssertEquals(0, HOLTribute::Received(0, 30), "nothing sent: nothing arrives");
}

// E3-14 (D830, D835): port of HeroesOfLiberty.Market.Table (HOLMarketTableTests.cpp), same assertions, on the real table Tools/Data/Market.csv
// read through the exposed type FHOLMarketRow. The C++ test imports the file with UDataTable::CreateTableFromCSVString into a transient table
// (RowStruct = FHOLMarketRow); script sees neither that import nor UDataTable creation, so the CSV is read here (FFileHelper + header columns
// by name, one int per column): the C++ "import problems" check has no script twin. The numbers are the user's answer MK-Q1 = C (D582), typed on purpose.
int MarketCsvInt(const FString& Text, bool& bOk)
{
	FString Clean = Text.TrimStartAndEnd();
	bOk = !Clean.IsEmpty();
	int Value = 0;
	for (int Index = 0; Index < Clean.Len(); Index++)
	{
		int Digit = Clean[Index] - 48;
		if (Digit < 0 || Digit > 9)
		{
			bOk = false;
			break;
		}
		Value = Value * 10 + Digit;
	}
	return Value;
}

// Sets the field the column names; false for a column the type does not have.
bool MarketRowSetColumn(FHOLMarketRow& Row, const FString& Column, int Value)
{
	if (Column == "FoodPrice") { Row.FoodPrice = Value; }
	else if (Column == "WoodPrice") { Row.WoodPrice = Value; }
	else if (Column == "StonePrice") { Row.StonePrice = Value; }
	else if (Column == "GoldPrice") { Row.GoldPrice = Value; }
	else if (Column == "FeePercent") { Row.FeePercent = Value; }
	else if (Column == "Step") { Row.Step = Value; }
	else if (Column == "MinPrice") { Row.MinPrice = Value; }
	else if (Column == "MaxPrice") { Row.MaxPrice = Value; }
	else if (Column == "RecoverPoints") { Row.RecoverPoints = Value; }
	else if (Column == "RecoverEverySeconds") { Row.RecoverEverySeconds = Value; }
	else { return false; }
	return true;
}

void Test_MarketTableRows(FUnitTest& T)
{
	FString Path = FPaths::ProjectDir() + "Tools/Data/Market.csv";
	FString Text;
	T.AssertTrue(FFileHelper::LoadFileToString(Text, Path), f"{Path} is there");

	TArray<FString> Lines;
	Text.ParseIntoArray(Lines, "\n");
	T.AssertTrue(Lines.Num() >= 2, "a header and at least one row");

	TArray<FString> Columns;
	Lines[0].TrimStartAndEnd().ParseIntoArray(Columns, ",", false);
	T.AssertEquals(11, Columns.Num(), "Name + the 10 columns of FHOLMarketRow");

	TArray<FString> Names;
	TArray<FHOLMarketRow> Rows;
	for (int Line = 1; Line < Lines.Num(); Line++)
	{
		TArray<FString> Cells;
		Lines[Line].TrimStartAndEnd().ParseIntoArray(Cells, ",", false);
		if (Cells.Num() != Columns.Num())
		{
			T.AssertTrue(false, f"Market.csv line {Line}: {Cells.Num()} cells for {Columns.Num()} columns");
			continue;
		}
		FHOLMarketRow Row;
		for (int Column = 1; Column < Columns.Num(); Column++)
		{
			bool bOk = false;
			int Value = MarketCsvInt(Cells[Column], bOk);
			T.AssertTrue(bOk, f"Market.csv line {Line}: {Columns[Column]} is a whole number");
			T.AssertTrue(MarketRowSetColumn(Row, Columns[Column].TrimStartAndEnd(), Value), f"Market.csv: column {Columns[Column]} is a field of FHOLMarketRow");
		}
		Names.Add(Cells[0].TrimStartAndEnd());
		Rows.Add(Row);
	}

	int Standard = -1;
	for (int Index = 0; Index < Names.Num(); Index++)
	{
		if (Names[Index] == "Standard")
		{
			Standard = Index;
		}
	}
	T.AssertTrue(Standard >= 0, "the Standard row is in the table");
	if (Standard >= 0)
	{
		// 1. The numbers of MK-Q1 = C (D582).
		FHOLMarketRow Row = Rows[Standard];
		T.AssertEquals(80, Row.FoodPrice, "food base price");
		T.AssertEquals(80, Row.WoodPrice, "wood base price");
		T.AssertEquals(120, Row.StonePrice, "stone base price");
		T.AssertEquals(100, Row.GoldPrice, "gold base price");
		T.AssertEquals(30, Row.FeePercent, "tax in percent");
		T.AssertEquals(3, Row.Step, "points a lot");
		T.AssertEquals(1, Row.RecoverPoints, "points back");
		T.AssertEquals(10, Row.RecoverEverySeconds, "seconds between two returns");
		T.AssertEquals(20, Row.MinPrice, "floor");
		T.AssertEquals(300, Row.MaxPrice, "cap");
	}

	// 2. Coherence, whatever the numbers become: bounds in order, every base price inside them, a real tax and step.
	for (int Index = 0; Index < Rows.Num(); Index++)
	{
		FHOLMarketRow Each = Rows[Index];
		FString Name = Names[Index];
		T.AssertTrue(Each.MinPrice < Each.MaxPrice, f"{Name}: floor below cap");
		TArray<int> Bases;
		Bases.Add(Each.FoodPrice);
		Bases.Add(Each.WoodPrice);
		Bases.Add(Each.StonePrice);
		Bases.Add(Each.GoldPrice);
		for (int Base : Bases)
		{
			T.AssertTrue(Base >= Each.MinPrice && Base <= Each.MaxPrice, f"{Name}: base price {Base} within the bounds");
		}
		T.AssertTrue(Each.FeePercent >= 0 && Each.FeePercent < 100, f"{Name}: tax between 0 and 99 percent");
		T.AssertTrue(Each.Step > 0, f"{Name}: the price moves with a lot");
		T.AssertTrue(Each.RecoverPoints > 0 && Each.RecoverEverySeconds > 0, f"{Name}: the price comes back");
	}
}