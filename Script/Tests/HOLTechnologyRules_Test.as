// The availability rules of the technologies (T2, HOLTechnologyRules.as): each of the three conditions of GDD 11 alone,
// the chain I -> II, the order of the reasons. The link with the table's rows (Tools/Data/Technologies.csv) is T3's.

// A level I of tier 1 in a finished building, paid, the hero at level 3 (tier 1): nothing stops it.
void Test_TechnologyAvailable(FUnitTest& T)
{
	T.AssertEquals(HOLTechnologyRules::Available, HOLTechnologyRules::Check(1, 3, 1, 0, true, true, true, false), "all three conditions met");
	T.AssertTrue(HOLTechnologyRules::IsAvailable(1, 3, 1, 0, true, true, true, false), "available");
}

// Condition 1: the tier, then the level when the row asks one (T-Q1: the stricter of the two holds).
void Test_TechnologyHero(FUnitTest& T)
{
	T.AssertEquals(HOLTechnologyRules::TierTooLow, HOLTechnologyRules::Check(0, 2, 1, 0, true, true, true, false), "tier 0 for a tier 1 technology");
	T.AssertEquals(HOLTechnologyRules::Available, HOLTechnologyRules::Check(1, 3, 1, 0, true, true, true, false), "tier 1 opens it");
	T.AssertEquals(HOLTechnologyRules::HeroLevelTooLow, HOLTechnologyRules::Check(1, 3, 1, 4, true, true, true, false), "tier open, level 3 < 4");
	T.AssertEquals(HOLTechnologyRules::Available, HOLTechnologyRules::Check(1, 4, 1, 4, true, true, true, false), "level 4 reached");
	T.AssertEquals(HOLTechnologyRules::TierTooLow, HOLTechnologyRules::Check(0, 0, 1, 0, true, true, true, false), "no hero: tier 0");
}

// Condition 2 and 3: the building and the cost.
void Test_TechnologyBuildingAndCost(FUnitTest& T)
{
	T.AssertEquals(HOLTechnologyRules::NoBuilding, HOLTechnologyRules::Check(1, 3, 1, 0, true, false, true, false), "no finished building");
	T.AssertEquals(HOLTechnologyRules::CannotPay, HOLTechnologyRules::Check(1, 3, 1, 0, true, true, false, false), "stock too low");
}

// A level II asks for its level I; a technology is researched once.
void Test_TechnologyChain(FUnitTest& T)
{
	T.AssertEquals(HOLTechnologyRules::RequiresMissing, HOLTechnologyRules::Check(2, 6, 2, 0, false, true, true, false), "level I not researched");
	T.AssertEquals(HOLTechnologyRules::Available, HOLTechnologyRules::Check(2, 6, 2, 0, true, true, true, false), "level I researched");
	T.AssertEquals(HOLTechnologyRules::AlreadyResearched, HOLTechnologyRules::Check(2, 6, 2, 0, true, true, true, true), "researched once");
}

// The order of the reasons: the first one is the one told (researched, tier, level, chain, building, cost).
void Test_TechnologyReasonOrder(FUnitTest& T)
{
	T.AssertEquals(HOLTechnologyRules::AlreadyResearched, HOLTechnologyRules::Check(0, 0, 2, 5, false, false, false, true), "researched first");
	T.AssertEquals(HOLTechnologyRules::TierTooLow, HOLTechnologyRules::Check(0, 0, 2, 5, false, false, false, false), "then the tier");
	T.AssertEquals(HOLTechnologyRules::HeroLevelTooLow, HOLTechnologyRules::Check(2, 3, 2, 5, false, false, false, false), "then the level");
	T.AssertEquals(HOLTechnologyRules::RequiresMissing, HOLTechnologyRules::Check(2, 6, 2, 5, false, false, false, false), "then the chain");
	T.AssertEquals(HOLTechnologyRules::NoBuilding, HOLTechnologyRules::Check(2, 6, 2, 5, true, false, false, false), "then the building");
	T.AssertEquals(HOLTechnologyRules::CannotPay, HOLTechnologyRules::Check(2, 6, 2, 5, true, true, false, false), "then the cost");
}

// A technology in a queue is refused (as the C++ rule: researched, queued, tier, level, chain, building, cost).
void Test_TechnologyQueued(FUnitTest& T)
{
	T.AssertEquals(HOLTechnologyRules::AlreadyQueued, HOLTechnologyRules::CheckQueued(1, 3, 1, 0, true, true, true, false, true), "queued");
	T.AssertEquals(HOLTechnologyRules::AlreadyResearched, HOLTechnologyRules::CheckQueued(1, 3, 1, 0, true, true, true, true, true), "researched wins");
	T.AssertEquals(HOLTechnologyRules::AlreadyQueued, HOLTechnologyRules::CheckQueued(0, 0, 2, 5, false, false, false, false, true), "queued before the tier");
	T.AssertEquals(HOLTechnologyRules::Available, HOLTechnologyRules::CheckQueued(1, 3, 1, 0, true, true, true, false, false), "not queued: available");
}

// T-Q11 = B (D601): a technology whose effect is not coded is refused first, whatever the rest (the C++ rule's order).
void Test_TechnologyNotImplemented(FUnitTest& T)
{
	T.AssertEquals(HOLTechnologyRules::NotImplemented, HOLTechnologyRules::CheckImplemented(false, 1, 3, 1, 0, true, true, true, false, false), "not implemented, everything ready");
	T.AssertEquals(HOLTechnologyRules::NotImplemented, HOLTechnologyRules::CheckImplemented(false, 0, 0, 2, 5, false, false, false, true, true), "before every other reason");
	T.AssertEquals(HOLTechnologyRules::Available, HOLTechnologyRules::CheckImplemented(true, 1, 3, 1, 0, true, true, true, false, false), "implemented: available");
	T.AssertEquals(HOLTechnologyRules::AlreadyQueued, HOLTechnologyRules::CheckImplemented(true, 1, 3, 1, 0, true, true, true, false, true), "implemented and queued: the queue rule");
}