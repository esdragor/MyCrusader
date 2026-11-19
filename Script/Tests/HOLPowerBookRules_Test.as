// The state of a faction's power book button (A6a, HOLPowerBookRules.as): the refusal order, "ready" only when everything is met,
// the described line. Fixture rows only: no real power, cost or cool-down (all of them are tuned in play).

FHOLPowerRow PowerBookFixture(bool bCoded)
{
	FHOLPowerRow Row;
	Row.Key = "fixtureA";
	Row.Cost = 40;
	Row.CooldownMs = 8000;
	Row.MinTier = 1;
	Row.bEffectCoded = bCoded;
	return Row;
}

// Several refusals at once: Soon beats the tier, the tier beats the cool-down, the cool-down beats the stock.
void Test_PowerBookStateOrder(FUnitTest& T)
{
	// No coded effect, and too early, cooling, too poor: Soon.
	EHOLPowerState Soon = HOLPowerBook::StateOf(PowerBookFixture(false), 0, 5000, 0);
	T.AssertTrue(Soon == EHOLPowerState::Soon, "soon before the tier, the cool-down and the stock");
	// Soon whatever the rest: even a rich, cooled, high-tier faction.
	EHOLPowerState SoonRich = HOLPowerBook::StateOf(PowerBookFixture(false), 1000, 0, 3);
	T.AssertTrue(SoonRich == EHOLPowerState::Soon, "soon even when everything else is met");
	// Coded, tier too low, cooling, too poor: the tier.
	EHOLPowerState Tier = HOLPowerBook::StateOf(PowerBookFixture(true), 0, 5000, 0);
	T.AssertTrue(Tier == EHOLPowerState::TierLocked, "the tier before the cool-down and the stock");
	// Coded, tier met, cooling, too poor: the cool-down.
	EHOLPowerState Cool = HOLPowerBook::StateOf(PowerBookFixture(true), 0, 5000, 1);
	T.AssertTrue(Cool == EHOLPowerState::Cooling, "the cool-down before the stock");
	// Coded, tier met, cooled, too poor: the stock.
	EHOLPowerState Short = HOLPowerBook::StateOf(PowerBookFixture(true), 0, 0, 1);
	T.AssertTrue(Short == EHOLPowerState::ShortOfResource, "short of the resource");
}

// Ready only when the effect is coded, the tier reached, the cool-down over and the stock at least the cost (the edge included).
void Test_PowerBookReadyOnlyWhenAllMet(FUnitTest& T)
{
	EHOLPowerState Edge = HOLPowerBook::StateOf(PowerBookFixture(true), 40, 0, 1);
	T.AssertTrue(Edge == EHOLPowerState::Ready, "stock = cost, tier = required, cool-down 0: ready");
	EHOLPowerState Above = HOLPowerBook::StateOf(PowerBookFixture(true), 500, 0, 3);
	T.AssertTrue(Above == EHOLPowerState::Ready, "a higher tier and a bigger stock: ready");
	EHOLPowerState OneShort = HOLPowerBook::StateOf(PowerBookFixture(true), 39, 0, 1);
	T.AssertTrue(OneShort == EHOLPowerState::ShortOfResource, "one short of the cost: not ready");
	EHOLPowerState OneMs = HOLPowerBook::StateOf(PowerBookFixture(true), 40, 1, 1);
	T.AssertTrue(OneMs == EHOLPowerState::Cooling, "1 ms of cool-down left: not ready");
	EHOLPowerState LowTier = HOLPowerBook::StateOf(PowerBookFixture(true), 40, 0, 0);
	T.AssertTrue(LowTier == EHOLPowerState::TierLocked, "one tier short: not ready");
	EHOLPowerState NoEffect = HOLPowerBook::StateOf(PowerBookFixture(false), 40, 0, 1);
	T.AssertTrue(NoEffect != EHOLPowerState::Ready, "no coded effect: not ready");
	// A free power with an empty stock is ready.
	FHOLPowerRow Free = PowerBookFixture(true);
	Free.Cost = 0;
	EHOLPowerState FreeState = HOLPowerBook::StateOf(Free, 0, 0, 1);
	T.AssertTrue(FreeState == EHOLPowerState::Ready, "cost 0, stock 0: ready");
}

// The line of the logs: key, cost, then the state (the cool-down in whole seconds, rounded up).
void Test_PowerBookDescribeLine(FUnitTest& T)
{
	FHOLPowerRow Row = PowerBookFixture(true);
	FString Ready = HOLPowerBook::Describe(Row, EHOLPowerState::Ready, 0);
	T.AssertEquals("fixtureA cost 40 ready", Ready, "ready");
	FString Cooling = HOLPowerBook::Describe(Row, EHOLPowerState::Cooling, 3000);
	T.AssertEquals("fixtureA cost 40 cooling 3 s", Cooling, "cooling, whole seconds");
	FString Rounded = HOLPowerBook::Describe(Row, EHOLPowerState::Cooling, 2001);
	T.AssertEquals("fixtureA cost 40 cooling 3 s", Rounded, "cooling, rounded up");
	FString Short = HOLPowerBook::Describe(Row, EHOLPowerState::ShortOfResource, 0);
	T.AssertEquals("fixtureA cost 40 short", Short, "short");
	FString Tier = HOLPowerBook::Describe(Row, EHOLPowerState::TierLocked, 0);
	T.AssertEquals("fixtureA cost 40 tier 1", Tier, "tier locked shows the required tier");
	FString Soon = HOLPowerBook::Describe(Row, EHOLPowerState::Soon, 0);
	T.AssertEquals("fixtureA cost 40 soon", Soon, "soon");
}