// The server's market board (MIG-CPP-Market, D836 point 4), the script side of UHOLMarketSubsystem: the C++ part (the production subsystem) gives
// the numbers of the exchange and the book (bridge structures, converted here to the script rule's own) and the settlement of a tribute; the rules
// are HOLMarketRules.as (HOLMarket::Recover) and HOLTributeRules.as (HOLTribute::Settle, which applies HOLTribute::Received), the script is the source. Low frequency only: the
// return of the book to rest and the tribute. Server and solo only. Tested by HeroesOfLiberty.Market.ServerBoard.
class UHOLMarketBoardScript : UHOLMarketSubsystem
{
	UFUNCTION(BlueprintOverride)
	FHOLMarketBridgeBook RecoverPrices(FHOLMarketBridgeSettings Settings, FHOLMarketBridgeBook Book, int Intervals)
	{
		FHOLMarketSettings Rule;
		Rule.BasePrices = Settings.BasePrices;
		Rule.Step = Settings.Step;
		Rule.FeePercent = Settings.FeePercent;
		Rule.MinPrice = Settings.MinPrice;
		Rule.MaxPrice = Settings.MaxPrice;
		Rule.RecoverPoints = Settings.RecoverPoints;
		Rule.RecoverEverySeconds = Settings.RecoverEverySeconds;
		FHOLMarketBook RuleBook;
		RuleBook.Prices = Book.Prices;
		FHOLMarketBook After = HOLMarket::Recover(Rule, RuleBook, Intervals);
		FHOLMarketBridgeBook Out;
		Out.Prices = After.Prices;
		return Out;
	}

	// One trade (a Trade order, the Market click's quote): HOLMarket::Apply of the script; Stock is what is given up.
	UFUNCTION(BlueprintOverride)
	FHOLMarketBridgeTrade TradeOnce(FHOLMarketBridgeSettings Settings, FHOLMarketBridgeBook Book, int Resource, int Lots, bool bBuy, int Stock)
	{
		FHOLMarketSettings Rule;
		Rule.BasePrices = Settings.BasePrices;
		Rule.Step = Settings.Step;
		Rule.FeePercent = Settings.FeePercent;
		Rule.MinPrice = Settings.MinPrice;
		Rule.MaxPrice = Settings.MaxPrice;
		Rule.RecoverPoints = Settings.RecoverPoints;
		Rule.RecoverEverySeconds = Settings.RecoverEverySeconds;
		FHOLMarketBook RuleBook;
		RuleBook.Prices = Book.Prices;
		FHOLMarketTrade Done = HOLMarket::Apply(Rule, RuleBook, Resource, Lots, bBuy, Stock, Settings.FeePercent);
		FHOLMarketBridgeTrade Out;
		Out.bOk = Done.bOk;
		Out.Reason = Done.Reason;
		Out.Gold = Done.Gold;
		Out.Units = Done.Units;
		Out.Book.Prices = Done.Book.Prices;
		return Out;
	}

	// The book at rest a match starts from (HOLGameState::ResetMarketBook, once per match): HOLMarket::NewBook of the script.
	UFUNCTION(BlueprintOverride)
	FHOLMarketBridgeBook FreshBook(FHOLMarketBridgeSettings Settings)
	{
		FHOLMarketSettings Rule;
		Rule.BasePrices = Settings.BasePrices;
		FHOLMarketBook After = HOLMarket::NewBook(Rule);
		FHOLMarketBridgeBook Out;
		Out.Prices = After.Prices;
		return Out;
	}

	// One tribute on the two stocks (D851 step 5 R1): HOLTribute::Settle of the script; the C++ caller applies the stocks it returns.
	UFUNCTION(BlueprintOverride)
	FHOLTributeBridgeResult TributeSettle(bool bSenderHasMarket, int Sender, int Receiver, bool bReceiverInMatch, FHOLResources GiverStock, FHOLResources TakerStock, int Resource, int Sent, int TaxPercent)
	{
		FHOLTributeSettled Done = HOLTribute::Settle(bSenderHasMarket, Sender, Receiver, bReceiverInMatch, GiverStock, TakerStock, Resource, Sent, TaxPercent);
		FHOLTributeBridgeResult Out;
		Out.bOk = Done.bOk;
		Out.Reason = Done.Reason;
		Out.Giver = Done.Giver;
		Out.Taker = Done.Taker;
		Out.Received = Done.Received;
		return Out;
	}
}