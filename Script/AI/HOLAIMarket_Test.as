// The AI's use of the Market, alone (HOLAIMarket.as, MK-Q3): stocks and wishes as arrays, the book from the preset.

const int AIMarketFood = 0;
const int AIMarketWood = 1;
const int AIMarketStone = 2;
const int AIMarketGold = 3;

TArray<int> AIMarketStock(int Food, int Wood, int Stone, int Gold)
{
	TArray<int> Stock;
	Stock.Add(Food);
	Stock.Add(Wood);
	Stock.Add(Stone);
	Stock.Add(Gold);
	return Stock;
}

// Stone is missing and gold is there: it buys stone, the lots it needs (a lot of 100 for a missing 80).
void Test_AIMarketBuysWhatIsMissingWithGold(FUnitTest& T)
{
	FHOLMarketSettings M = HOLMarket::DefaultSettings();
	FHOLMarketBook Book = HOLMarket::NewBook(M);
	FHOLAIMarketSettings Rule;
	FHOLAIMarketChoice Choice = HOLAIMarket::Choose(M, Book, Rule, AIMarketStock(400, 400, 20, 500), AIMarketStock(0, 0, 100, 0), true, 2);
	T.AssertTrue(Choice.bTrade, "a trade");
	T.AssertTrue(Choice.bBuy, "a purchase");
	T.AssertEquals(AIMarketStone, Choice.Resource, "stone");
	T.AssertEquals(2, Choice.Lots, "a lot of 100 gold buys 64 stone at 120 (156 per 100): two lots (126 stone) cover the 80 missing");
}

// Gold is short for the lots wanted: it buys as many as the gold pays for.
void Test_AIMarketBuysOnlyWhatGoldPays(FUnitTest& T)
{
	FHOLMarketSettings M = HOLMarket::DefaultSettings();
	FHOLMarketBook Book = HOLMarket::NewBook(M);
	FHOLAIMarketSettings Rule;
	// 400 stone missing = more than 5 lots of 100 gold (64 stone each, MaxLots 5 at most): 330 gold pays three lots of 100.
	FHOLAIMarketChoice Choice = HOLAIMarket::Choose(M, Book, Rule, AIMarketStock(0, 0, 0, 330), AIMarketStock(0, 0, 400, 0), true, 2);
	T.AssertTrue(Choice.bTrade && Choice.bBuy, "a purchase");
	T.AssertEquals(3, Choice.Lots, "three lots of 100 gold");
}

// No gold to buy even one lot: it sells the surplus instead (wood over 600, keeping its reserve).
void Test_AIMarketSellsSurplusWhenPoor(FUnitTest& T)
{
	FHOLMarketSettings M = HOLMarket::DefaultSettings();
	FHOLMarketBook Book = HOLMarket::NewBook(M);
	FHOLAIMarketSettings Rule;
	FHOLAIMarketChoice Choice = HOLAIMarket::Choose(M, Book, Rule, AIMarketStock(0, 900, 0, 10), AIMarketStock(0, 0, 200, 0), true, 2);
	T.AssertTrue(Choice.bTrade, "a trade");
	T.AssertFalse(Choice.bBuy, "a sale");
	T.AssertEquals(AIMarketWood, Choice.Resource, "wood");
	T.AssertEquals(5, Choice.Lots, "900 - 300 kept = 600: capped at 5 lots");
}

// A surplus under SellAbove is not sold; the resource that is missing is never the one sold.
void Test_AIMarketKeepsReserveAndNeverSellsWhatIsMissing(FUnitTest& T)
{
	FHOLMarketSettings M = HOLMarket::DefaultSettings();
	FHOLMarketBook Book = HOLMarket::NewBook(M);
	FHOLAIMarketSettings Rule;
	FHOLAIMarketChoice Low = HOLAIMarket::Choose(M, Book, Rule, AIMarketStock(0, 550, 0, 0), AIMarketStock(0, 0, 200, 0), true, 2);
	T.AssertFalse(Low.bTrade, "550 is not above 600: nothing sold");
	FHOLAIMarketChoice Missing = HOLAIMarket::Choose(M, Book, Rule, AIMarketStock(0, 900, 0, 0), AIMarketStock(0, 1500, 0, 0), true, 2);
	T.AssertFalse(Missing.bTrade, "wood is what is missing: it does not sell it for itself");
}

// Nothing missing, no Market, a difficulty that does not use it: no trade.
void Test_AIMarketNoTradeWhenNotNeeded(FUnitTest& T)
{
	FHOLMarketSettings M = HOLMarket::DefaultSettings();
	FHOLMarketBook Book = HOLMarket::NewBook(M);
	FHOLAIMarketSettings Rule;
	TArray<int> Rich = AIMarketStock(900, 900, 900, 900);
	T.AssertFalse(HOLAIMarket::Choose(M, Book, Rule, Rich, AIMarketStock(100, 100, 100, 100), true, 2).bTrade, "nothing missing");
	T.AssertFalse(HOLAIMarket::Choose(M, Book, Rule, AIMarketStock(0, 0, 0, 900), AIMarketStock(0, 0, 100, 0), false, 2).bTrade, "no Market built");
	Rule.MinDifficulty = 2;
	T.AssertFalse(HOLAIMarket::Choose(M, Book, Rule, AIMarketStock(0, 0, 0, 900), AIMarketStock(0, 0, 100, 0), true, 1).bTrade, "normal does not use it when the rule starts at hard (MK-Q3 C)");
	T.AssertTrue(HOLAIMarket::Choose(M, Book, Rule, AIMarketStock(0, 0, 0, 900), AIMarketStock(0, 0, 100, 0), true, 2).bTrade, "hard does");
	Rule.MinDifficulty = 3;
	T.AssertFalse(HOLAIMarket::Choose(M, Book, Rule, AIMarketStock(0, 0, 0, 900), AIMarketStock(0, 0, 100, 0), true, 2).bTrade, "MK-Q3 B: nobody uses it");
}

// MK-Q3 = C (the user's answer, 2026-10-31): the rule as it stands (no value touched) is used by Hard only; Easy and Normal leave the Market alone.
void Test_AIMarketDefaultRuleIsHardOnly(FUnitTest& T)
{
	FHOLMarketSettings M = HOLMarket::DefaultSettings();
	FHOLMarketBook Book = HOLMarket::NewBook(M);
	FHOLAIMarketSettings Rule;
	TArray<int> Stock = AIMarketStock(0, 0, 0, 900);
	TArray<int> Wanted = AIMarketStock(0, 0, 100, 0);
	T.AssertFalse(HOLAIMarket::Choose(M, Book, Rule, Stock, Wanted, true, 0).bTrade, "easy does not use the Market");
	T.AssertFalse(HOLAIMarket::Choose(M, Book, Rule, Stock, Wanted, true, 1).bTrade, "normal does not use the Market");
	T.AssertTrue(HOLAIMarket::Choose(M, Book, Rule, Stock, Wanted, true, 2).bTrade, "hard does");
}

// Gold is what is missing: it sells the surplus for it.
void Test_AIMarketSellsForGold(FUnitTest& T)
{
	FHOLMarketSettings M = HOLMarket::DefaultSettings();
	FHOLMarketBook Book = HOLMarket::NewBook(M);
	FHOLAIMarketSettings Rule;
	FHOLAIMarketChoice Choice = HOLAIMarket::Choose(M, Book, Rule, AIMarketStock(0, 0, 800, 0), AIMarketStock(0, 0, 0, 300), true, 2);
	T.AssertTrue(Choice.bTrade && !Choice.bBuy, "a sale");
	T.AssertEquals(AIMarketStone, Choice.Resource, "stone, the surplus");
}
// MK-Q3 = C (D605): the Market is BUILT by Hard only, at tier II (age 1), when a need has been missing for more than a minute, and only one.
void Test_AIMarketBuildOnlyAfterAMinuteOfLack(FUnitTest& T)
{
	FHOLAIMarketSettings Rule;
	FHOLAIMarketBuildMemory Memory;
	TArray<int> Stock = AIMarketStock(0, 0, 20, 500);
	TArray<int> Wish = AIMarketStock(0, 0, 100, 0);
	T.AssertFalse(HOLAIMarket::WantsMarket(Rule, Memory, Stock, Wish, 100.f, 2, 1, false), "the lack starts now: not yet");
	T.AssertFalse(HOLAIMarket::WantsMarket(Rule, Memory, Stock, Wish, 159.f, 2, 1, false), "59 s of lack: not yet");
	T.AssertFalse(HOLAIMarket::WantsMarket(Rule, Memory, Stock, Wish, 160.f, 2, 1, false), "exactly a minute is not MORE than a minute");
	T.AssertTrue(HOLAIMarket::WantsMarket(Rule, Memory, Stock, Wish, 161.f, 2, 1, false), "more than a minute of lack: build the Market");
}

void Test_AIMarketBuildNeverForEasyAndNormal(FUnitTest& T)
{
	FHOLAIMarketSettings Rule;
	FHOLAIMarketBuildMemory Easy;
	FHOLAIMarketBuildMemory Normal;
	FHOLAIMarketBuildMemory Hard;
	TArray<int> Stock = AIMarketStock(0, 0, 20, 500);
	TArray<int> Wish = AIMarketStock(0, 0, 100, 0);
	HOLAIMarket::WantsMarket(Rule, Easy, Stock, Wish, 0.f, 0, 1, false);
	HOLAIMarket::WantsMarket(Rule, Normal, Stock, Wish, 0.f, 1, 1, false);
	HOLAIMarket::WantsMarket(Rule, Hard, Stock, Wish, 0.f, 2, 1, false);
	T.AssertFalse(HOLAIMarket::WantsMarket(Rule, Easy, Stock, Wish, 600.f, 0, 1, false), "Easy never builds the Market, whatever the lack");
	T.AssertFalse(HOLAIMarket::WantsMarket(Rule, Normal, Stock, Wish, 600.f, 1, 1, false), "Normal never builds the Market");
	T.AssertTrue(HOLAIMarket::WantsMarket(Rule, Hard, Stock, Wish, 600.f, 2, 1, false), "Hard does");
}

void Test_AIMarketBuildWaitsForTierAndBuildsOnlyOne(FUnitTest& T)
{
	FHOLAIMarketSettings Rule;
	FHOLAIMarketBuildMemory Memory;
	TArray<int> Stock = AIMarketStock(0, 0, 20, 500);
	TArray<int> Wish = AIMarketStock(0, 0, 100, 0);
	HOLAIMarket::WantsMarket(Rule, Memory, Stock, Wish, 0.f, 2, 0, false);
	T.AssertFalse(HOLAIMarket::WantsMarket(Rule, Memory, Stock, Wish, 600.f, 2, 0, false), "tier 0: the age 1 Market is not open yet");
	T.AssertTrue(HOLAIMarket::WantsMarket(Rule, Memory, Stock, Wish, 601.f, 2, 1, false), "tier 1 reached after a long lack: build at once");
	T.AssertFalse(HOLAIMarket::WantsMarket(Rule, Memory, Stock, Wish, 700.f, 2, 1, true), "one Market (finished or under way) is enough");
}

void Test_AIMarketBuildLackMustBeUnbroken(FUnitTest& T)
{
	FHOLAIMarketSettings Rule;
	FHOLAIMarketBuildMemory Memory;
	TArray<int> Missing = AIMarketStock(0, 0, 20, 500);
	TArray<int> Wish = AIMarketStock(0, 0, 100, 0);
	TArray<int> Enough = AIMarketStock(0, 0, 150, 500);
	HOLAIMarket::WantsMarket(Rule, Memory, Missing, Wish, 0.f, 2, 1, false);
	HOLAIMarket::WantsMarket(Rule, Memory, Enough, Wish, 50.f, 2, 1, false);
	T.AssertFalse(HOLAIMarket::WantsMarket(Rule, Memory, Missing, Wish, 100.f, 2, 1, false), "the need was met at 50 s: the minute starts again at 100 s");
	T.AssertFalse(HOLAIMarket::WantsMarket(Rule, Memory, Missing, Wish, 160.f, 2, 1, false), "60 s since 100 s: not more than a minute");
	T.AssertTrue(HOLAIMarket::WantsMarket(Rule, Memory, Missing, Wish, 161.f, 2, 1, false), "more than a minute since the lack came back");
}
