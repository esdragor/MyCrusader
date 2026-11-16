// The server's market board (MIG-CPP-Market, D836 point 4), the script side of UHOLMarketSubsystem: the C++ part (the production subsystem) gives
// the numbers of the exchange and the book (bridge structures, converted here to the script rule's own) and the amount of a tribute; the rules
// are HOLMarketRules.as (HOLMarket::Recover) and HOLTributeRules.as (HOLTribute::Received), the script is the source. Low frequency only: the
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

	UFUNCTION(BlueprintOverride)
	int TributeReceived(int Sent, int TaxPercent)
	{
		return HOLTribute::Received(Sent, TaxPercent);
	}
}