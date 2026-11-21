// The display line of a power box (HOLPowerDisplay.as, GALADRIEL-1121sh): text key + arguments from the cost, the state and the cool-down,
// independent of the editor's language (patterns passed as literals). The stable ASCII line (HOLPowerBook::Describe) stays what the tests compare.

FHOLPowerRow PowerDisplayFixture()
{
	FHOLPowerRow Row;
	Row.Key = "fixtureA";
	Row.Cost = 40;
	Row.CooldownMs = 8000;
	Row.MinTier = 2;
	Row.bEffectCoded = true;
	return Row;
}

// The state comes from the real rule (StateOf), the display from the state: each refusal has its own key and its argument.
void Test_PowerDisplayKeyAndArgumentFollowTheState(FUnitTest& T)
{
	const FHOLPowerRow Row = PowerDisplayFixture();
	// Rich, cooled, tier met: ready.
	FHOLPowerDisplay Ready = HOLPowerDisplay::Make(Row, HOLPowerBook::StateOf(Row, 100, 0, 2), 0);
	T.AssertEquals("PowerReady", Ready.TextKey, "ready key");
	T.AssertTrue(Ready.Cost == 40, "ready: the cost is the row's");
	T.AssertEquals("fixtureA", Ready.Name, "ready: the name is the row's key (the text table translates it)");
	// 4500 ms left: 5 s (rounded up, like Describe).
	FHOLPowerDisplay Cooling = HOLPowerDisplay::Make(Row, HOLPowerBook::StateOf(Row, 100, 4500, 2), 4500);
	T.AssertEquals("PowerCooling", Cooling.TextKey, "cooling key");
	T.AssertTrue(Cooling.Extra == 5, "cooling: 4500 ms shows 5 s");
	// Too poor.
	FHOLPowerDisplay Short = HOLPowerDisplay::Make(Row, HOLPowerBook::StateOf(Row, 10, 0, 2), 0);
	T.AssertEquals("PowerShort", Short.TextKey, "short key");
	T.AssertTrue(Short.Cost == 40, "short: the cost still shown");
	// Tier 1 for a power that wants tier 2.
	FHOLPowerDisplay Tier = HOLPowerDisplay::Make(Row, HOLPowerBook::StateOf(Row, 100, 0, 1), 0);
	T.AssertEquals("PowerTier", Tier.TextKey, "tier key");
	T.AssertTrue(Tier.Extra == 2, "tier: the tier required is shown");
	// No coded effect.
	FHOLPowerRow Uncoded = PowerDisplayFixture();
	Uncoded.bEffectCoded = false;
	FHOLPowerDisplay Soon = HOLPowerDisplay::Make(Uncoded, HOLPowerBook::StateOf(Uncoded, 100, 0, 2), 0);
	T.AssertEquals("PowerSoon", Soon.TextKey, "soon key");
}

// The composition replaces {0} (name), {1} (cost), {2} (extra) in a pattern given as a literal: no table, no language.
void Test_PowerDisplayComposesAPattern(FUnitTest& T)
{
	const FHOLPowerRow Row = PowerDisplayFixture();
	FHOLPowerDisplay Cooling = HOLPowerDisplay::Make(Row, EHOLPowerState::Cooling, 4500);
	const FString Line = HOLPowerDisplay::Compose("{0} / {1} / {2}", Cooling);
	T.AssertEquals("fixtureA / 40 / 5", Line, "name / cost / seconds");
}

// The display never changes the stable line: it stays ASCII, "<key> cost <n> <state>".
void Test_PowerDisplayLeavesTheStableLineAlone(FUnitTest& T)
{
	const FHOLPowerRow Row = PowerDisplayFixture();
	const FString Stable = HOLPowerBook::Describe(Row, EHOLPowerState::Ready, 0);
	T.AssertEquals("fixtureA cost 40 ready", Stable, "Describe unchanged");
}
