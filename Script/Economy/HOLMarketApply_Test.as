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


// ---- The tribute applied to two stocks (D851 step 5, lot R1: the script is the only source of the debit and the credit). ----
// Values copied from UHOLProductionSubsystem::ApplyTribute as it behaves now (30 % tax, the sender loses Sent, the receiver gains
// Received, a refusal moves nothing). RED until HOLTribute::Settle exists in HOLTributeRules.as (compiles after the build).

FHOLResources MarketApplyStock(int Food, int Wood, int Stone, int Gold)
{
	FHOLResources Result;
	Result.Food = Food;
	Result.Wood = Wood;
	Result.Stone = Stone;
	Result.Gold = Gold;
	return Result;
}

// 100 wood given at 30 %: the giver pays all 100, the receiver gets 70, the other three goods of both stocks do not move.
void Test_TributeSettleDebitsAndCredits(FUnitTest& T)
{
	FHOLResources Giver = MarketApplyStock(10, 500, 20, 30);
	FHOLResources Taker = MarketApplyStock(1, 5, 2, 3);
	FHOLTributeSettled Done = HOLTribute::Settle(true, 0, 1, true, Giver, Taker, 1, 100, HOLTribute::DefaultTaxPercent);
	T.AssertTrue(Done.bOk, "accepted");
	T.AssertEquals(HOLTribute::Ok, Done.Reason, "no reason");
	T.AssertEquals(400, Done.Giver.Wood, "the giver pays 100");
	T.AssertEquals(75, Done.Taker.Wood, "the receiver gets 70 (5 + 70)");
	T.AssertEquals(70, Done.Received, "what arrived");
	T.AssertEquals(30, Done.Tax, "what the Market kept");
	T.AssertEquals(10, Done.Giver.Food, "giver food untouched");
	T.AssertEquals(30, Done.Giver.Gold, "giver gold untouched");
	T.AssertEquals(3, Done.Taker.Gold, "receiver gold untouched");
	T.AssertEquals(500, Giver.Wood, "the stock given is a copy: the input does not change");
}

// Gold is the fourth resource (index 3); a tribute of the whole stock empties it.
void Test_TributeSettleWholeStockOfGold(FUnitTest& T)
{
	FHOLTributeSettled Done = HOLTribute::Settle(true, 2, 0, true, MarketApplyStock(0, 0, 0, 250), MarketApplyStock(0, 0, 0, 0), 3, 250, HOLTribute::DefaultTaxPercent);
	T.AssertTrue(Done.bOk, "the whole stock is allowed");
	T.AssertEquals(0, Done.Giver.Gold, "emptied");
	T.AssertEquals(175, Done.Taker.Gold, "250 at 30 %: 175 arrive");
}

// A refusal moves nothing and says why: one short, no Market, oneself, nobody there, nothing sent, no such resource.
void Test_TributeSettleRefusals(FUnitTest& T)
{
	FHOLResources Giver = MarketApplyStock(0, 99, 0, 0);
	FHOLResources Taker = MarketApplyStock(7, 8, 9, 10);
	FHOLTributeSettled Short = HOLTribute::Settle(true, 0, 1, true, Giver, Taker, 1, 100, HOLTribute::DefaultTaxPercent);
	T.AssertFalse(Short.bOk, "99 cannot give 100");
	T.AssertEquals(HOLTribute::NotEnough, Short.Reason, "the stock");
	T.AssertEquals(99, Short.Giver.Wood, "the giver keeps his 99");
	T.AssertEquals(8, Short.Taker.Wood, "the receiver gets nothing");
	T.AssertEquals(0, Short.Received, "nothing arrived");
	T.AssertEquals(HOLTribute::NoMarket, HOLTribute::Settle(false, 0, 1, true, Giver, Taker, 1, 50, 30).Reason, "no Market");
	T.AssertEquals(HOLTribute::Self, HOLTribute::Settle(true, 1, 1, true, Giver, Taker, 1, 50, 30).Reason, "to oneself");
	T.AssertEquals(HOLTribute::NoRecipient, HOLTribute::Settle(true, 0, 5, false, Giver, Taker, 1, 50, 30).Reason, "nobody there");
	T.AssertEquals(HOLTribute::BadAmount, HOLTribute::Settle(true, 0, 1, true, Giver, Taker, 1, 0, 30).Reason, "nothing sent");
	T.AssertEquals(HOLTribute::BadResource, HOLTribute::Settle(true, 0, 1, true, Giver, Taker, 4, 50, 30).Reason, "no such resource");
	T.AssertFalse(HOLTribute::Settle(true, 0, 1, true, Giver, Taker, 4, 50, 30).bOk, "a refusal is not ok");
}

// ApplyTribute's own checks, in its order (D851 step 5 R2: the journal line follows the reason, so the order is part of the behavior): the amount and
// the resource and the market and the recipient come before the stock, the stock last. 99 for 100 with no Market is NoMarket, not NotEnough; a gift to
// oneself with nobody there is Self; conservation: what leaves the giver is what the receiver got plus the tax.
void Test_TributeSettleReasonOrderAndConservation(FUnitTest& T)
{
	FHOLResources Giver = MarketApplyStock(5, 99, 5, 5);
	FHOLResources Taker = MarketApplyStock(1, 1, 1, 1);
	T.AssertEquals(HOLTribute::NoMarket, HOLTribute::Settle(false, 0, 1, true, Giver, Taker, 1, 100, 30).Reason, "no Market before the stock");
	T.AssertEquals(HOLTribute::Self, HOLTribute::Settle(true, 0, 0, false, Giver, Taker, 1, 100, 30).Reason, "oneself before nobody there");
	T.AssertEquals(HOLTribute::BadAmount, HOLTribute::Settle(true, 0, 1, true, Giver, Taker, 9, -5, 30).Reason, "the amount before the resource");
	T.AssertEquals(HOLTribute::BadResource, HOLTribute::Settle(true, 0, 1, true, Giver, Taker, 9, 100, 30).Reason, "the resource before the stock");
	FHOLTributeSettled Done = HOLTribute::Settle(true, 0, 1, true, Giver, Taker, 1, 99, 30);
	T.AssertTrue(Done.bOk, "the whole 99 is allowed");
	T.AssertEquals(99, Giver.Wood - Done.Giver.Wood, "the giver lost 99");
	T.AssertEquals(Done.Received + Done.Tax, Done.Taker.Wood - Taker.Wood + Done.Tax, "the receiver got what arrived");
	T.AssertEquals(99, Done.Received + Done.Tax, "arrived plus tax is what was sent");
	T.AssertEquals(70, Done.Received, "99 at 30 %: 70 arrive");
}
