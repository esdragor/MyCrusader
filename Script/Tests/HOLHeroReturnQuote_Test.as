// The hero's return quoted (J8a.5 loop 1b, HOLHeroReturnQuote.as): an order sent at step Step is taken at Step + 1 and
// priced there. Death at 1 000, level 1: ready at 1 760, price 0 from 2 960. The real chain: HeroesOfLiberty.Hero.ReturnQuote.

// Alive: known without a step; no price, not buyable.
void Test_HeroReturnQuoteAlive(FUnitTest& T)
{
	const FHOLHeroReturnQuote Quote = HOLHeroReturnQuote::Compute(500, EHOLHeroState::Alive, -1, 1, 500, true, false);
	T.AssertTrue(Quote.State == EHOLHeroReturnQuoteState::Alive, "alive");
	T.AssertEquals(-1, Quote.Price, "no price");
	T.AssertEquals(-1, Quote.StepsToReady, "no delay");
	T.AssertFalse(Quote.bCanBuy, "not buyable");
	const FHOLHeroReturnQuote NoStep = HOLHeroReturnQuote::Compute(-1, EHOLHeroState::Alive, -1, 1, 500, true, false);
	T.AssertTrue(NoStep.State == EHOLHeroReturnQuoteState::Alive, "alive even before the server step is known");
}

// Waiting: the steps and seconds left from the step that takes the order, no price.
void Test_HeroReturnQuoteWaiting(FUnitTest& T)
{
	const FHOLHeroReturnQuote AtDeath = HOLHeroReturnQuote::Compute(1000, EHOLHeroState::Dead, 1000, 1, 500, true, false);
	T.AssertTrue(AtDeath.State == EHOLHeroReturnQuoteState::Waiting, "waiting at death");
	T.AssertEquals(1001, AtDeath.TakeStep, "taken at the next step");
	T.AssertEquals(759, AtDeath.StepsToReady, "759 steps left from the step that takes it");
	T.AssertEquals(38, AtDeath.SecondsToReady, "37.95 s rounded up: 38");
	T.AssertEquals(-1, AtDeath.Price, "no price");
	T.AssertEquals(500, AtDeath.Gold, "the gold given");
	T.AssertTrue(AtDeath.bHasPlace, "a place");
	T.AssertFalse(AtDeath.bCanBuy, "not buyable");
	const FHOLHeroReturnQuote Last = HOLHeroReturnQuote::Compute(1758, EHOLHeroState::Dead, 1000, 1, 500, true, false);
	T.AssertTrue(Last.State == EHOLHeroReturnQuoteState::Waiting, "still waiting two steps before");
	T.AssertEquals(1, Last.StepsToReady, "one step left");
	T.AssertEquals(1, Last.SecondsToReady, "0.05 s rounded up: 1");
}

// Ready: one step before the ready step an order is taken at it (150, as UHOLHeroReturnRule.Buy); the price is the next
// step's (598 after: 76, as the real order of HeroesOfLiberty.Hero.ReturnOrderPrice); 0 at the end, still buyable.
void Test_HeroReturnQuoteReady(FUnitTest& T)
{
	const FHOLHeroReturnQuote First = HOLHeroReturnQuote::Compute(1759, EHOLHeroState::Dead, 1000, 1, 500, true, false);
	T.AssertTrue(First.State == EHOLHeroReturnQuoteState::Ready, "ready one step before (still Dead in the sheet)");
	T.AssertEquals(0, First.StepsToReady, "0 step left");
	T.AssertEquals(0, First.SecondsToReady, "0 s left");
	T.AssertEquals(150, First.Price, "150");
	T.AssertTrue(First.bCanBuy, "buyable");
	const FHOLHeroReturnQuote Later = HOLHeroReturnQuote::Compute(1760 + 598, EHOLHeroState::ReadyToReturn, 1000, 1, 500, true, false);
	T.AssertEquals(HOLHeroReturn::PriceAt(1760 + 599, 1000, 1), Later.Price, "the price at the next step");
	T.AssertEquals(76, Later.Price, "76");
	const FHOLHeroReturnQuote End = HOLHeroReturnQuote::Compute(2959, EHOLHeroState::ReadyToReturn, 1000, 1, 0, true, false);
	T.AssertEquals(0, End.Price, "0 at the end");
	T.AssertTrue(End.bCanBuy, "buyable at 0 with 0 gold");
}

// Not buyable: gold short, gold unknown (no stock, another player's), no place, the slot defeated; the price still said.
void Test_HeroReturnQuoteNotBuyable(FUnitTest& T)
{
	const FHOLHeroReturnQuote Short = HOLHeroReturnQuote::Compute(1759, EHOLHeroState::ReadyToReturn, 1000, 1, 149, true, false);
	T.AssertEquals(150, Short.Price, "150 quoted");
	T.AssertFalse(Short.bCanBuy, "149 gold for 150: not buyable");
	T.AssertTrue(HOLHeroReturnQuote::Compute(1759, EHOLHeroState::ReadyToReturn, 1000, 1, 150, true, false).bCanBuy, "150 for 150: buyable");
	T.AssertFalse(HOLHeroReturnQuote::Compute(1759, EHOLHeroState::ReadyToReturn, 1000, 1, -1, true, false).bCanBuy, "gold unknown: not buyable");
	const FHOLHeroReturnQuote NoPlace = HOLHeroReturnQuote::Compute(1759, EHOLHeroState::ReadyToReturn, 1000, 1, 500, false, false);
	T.AssertFalse(NoPlace.bHasPlace, "no place");
	T.AssertFalse(NoPlace.bCanBuy, "no place: not buyable");
	T.AssertFalse(HOLHeroReturnQuote::Compute(1759, EHOLHeroState::ReadyToReturn, 1000, 1, 500, true, true).bCanBuy, "defeated: not buyable");
}

// Unknown, never a price: a dead hero whose death step is not known here (another player's, on a client), no server step
// yet (the reception clock before its first sample), no hero.
void Test_HeroReturnQuoteUnknown(FUnitTest& T)
{
	const FHOLHeroReturnQuote Other = HOLHeroReturnQuote::Compute(2000, EHOLHeroState::ReadyToReturn, -1, 1, -1, true, false);
	T.AssertTrue(Other.State == EHOLHeroReturnQuoteState::Unknown, "death step unknown: unknown");
	T.AssertEquals(-1, Other.Price, "... no price");
	T.AssertEquals(-1, Other.StepsToReady, "... no delay");
	T.AssertFalse(Other.bCanBuy, "... not buyable");
	const FHOLHeroReturnQuote NoClock = HOLHeroReturnQuote::Compute(-1, EHOLHeroState::Dead, 1000, 1, 500, true, false);
	T.AssertTrue(NoClock.State == EHOLHeroReturnQuoteState::Unknown, "no server step: unknown");
	T.AssertEquals(-1, NoClock.Price, "... no price");
	T.AssertEquals(-1, NoClock.TakeStep, "... no take step");
	const FHOLHeroReturnQuote NoHero = HOLHeroReturnQuote::Compute(2000, EHOLHeroState::None, -1, 0, 500, true, false);
	T.AssertTrue(NoHero.State == EHOLHeroReturnQuoteState::Unknown, "no hero: unknown");
}
