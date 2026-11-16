// The availability rules of the technologies (T2, HOLTechnologyRules.as): each of the three conditions of GDD 11 alone,
// the chain I -> II, the order of the reasons. The link with the table's rows (Tools/Data/Technologies.csv) is T3's.

// A level I of tier 1 in a finished building, paid, the hero at level 3 (tier 1): nothing stops it.
void Test_TechnologyAvailable(FUnitTest& T)
{
	T.AssertEquals(HOLTechnologyRules::Available, HOLTechnologyCheck::Check(true, 1, 3, 1, 0, true, true, true, false, false), "all three conditions met");
	T.AssertTrue(HOLTechnologyCheck::Check(true, 1, 3, 1, 0, true, true, true, false, false) == HOLTechnologyRules::Available, "available");
}

// Condition 1: the tier, then the level when the row asks one (T-Q1: the stricter of the two holds).
void Test_TechnologyHero(FUnitTest& T)
{
	T.AssertEquals(HOLTechnologyRules::TierTooLow, HOLTechnologyCheck::Check(true, 0, 2, 1, 0, true, true, true, false, false), "tier 0 for a tier 1 technology");
	T.AssertEquals(HOLTechnologyRules::Available, HOLTechnologyCheck::Check(true, 1, 3, 1, 0, true, true, true, false, false), "tier 1 opens it");
	T.AssertEquals(HOLTechnologyRules::HeroLevelTooLow, HOLTechnologyCheck::Check(true, 1, 3, 1, 4, true, true, true, false, false), "tier open, level 3 < 4");
	T.AssertEquals(HOLTechnologyRules::Available, HOLTechnologyCheck::Check(true, 1, 4, 1, 4, true, true, true, false, false), "level 4 reached");
	T.AssertEquals(HOLTechnologyRules::TierTooLow, HOLTechnologyCheck::Check(true, 0, 0, 1, 0, true, true, true, false, false), "no hero: tier 0");
}

// Condition 2 and 3: the building and the cost.
void Test_TechnologyBuildingAndCost(FUnitTest& T)
{
	T.AssertEquals(HOLTechnologyRules::NoBuilding, HOLTechnologyCheck::Check(true, 1, 3, 1, 0, true, false, true, false, false), "no finished building");
	T.AssertEquals(HOLTechnologyRules::CannotPay, HOLTechnologyCheck::Check(true, 1, 3, 1, 0, true, true, false, false, false), "stock too low");
}

// A level II asks for its level I; a technology is researched once.
void Test_TechnologyChain(FUnitTest& T)
{
	T.AssertEquals(HOLTechnologyRules::RequiresMissing, HOLTechnologyCheck::Check(true, 2, 6, 2, 0, false, true, true, false, false), "level I not researched");
	T.AssertEquals(HOLTechnologyRules::Available, HOLTechnologyCheck::Check(true, 2, 6, 2, 0, true, true, true, false, false), "level I researched");
	T.AssertEquals(HOLTechnologyRules::AlreadyResearched, HOLTechnologyCheck::Check(true, 2, 6, 2, 0, true, true, true, true, false), "researched once");
}

// The order of the reasons: the first one is the one told (researched, tier, level, chain, building, cost).
void Test_TechnologyReasonOrder(FUnitTest& T)
{
	T.AssertEquals(HOLTechnologyRules::AlreadyResearched, HOLTechnologyCheck::Check(true, 0, 0, 2, 5, false, false, false, true, false), "researched first");
	T.AssertEquals(HOLTechnologyRules::TierTooLow, HOLTechnologyCheck::Check(true, 0, 0, 2, 5, false, false, false, false, false), "then the tier");
	T.AssertEquals(HOLTechnologyRules::HeroLevelTooLow, HOLTechnologyCheck::Check(true, 2, 3, 2, 5, false, false, false, false, false), "then the level");
	T.AssertEquals(HOLTechnologyRules::RequiresMissing, HOLTechnologyCheck::Check(true, 2, 6, 2, 5, false, false, false, false, false), "then the chain");
	T.AssertEquals(HOLTechnologyRules::NoBuilding, HOLTechnologyCheck::Check(true, 2, 6, 2, 5, true, false, false, false, false), "then the building");
	T.AssertEquals(HOLTechnologyRules::CannotPay, HOLTechnologyCheck::Check(true, 2, 6, 2, 5, true, true, false, false, false), "then the cost");
}

// A technology in a queue is refused (as the C++ rule: researched, queued, tier, level, chain, building, cost).
void Test_TechnologyQueued(FUnitTest& T)
{
	T.AssertEquals(HOLTechnologyRules::AlreadyQueued, HOLTechnologyCheck::Check(true, 1, 3, 1, 0, true, true, true, false, true), "queued");
	T.AssertEquals(HOLTechnologyRules::AlreadyResearched, HOLTechnologyCheck::Check(true, 1, 3, 1, 0, true, true, true, true, true), "researched wins");
	T.AssertEquals(HOLTechnologyRules::AlreadyQueued, HOLTechnologyCheck::Check(true, 0, 0, 2, 5, false, false, false, false, true), "queued before the tier");
	T.AssertEquals(HOLTechnologyRules::Available, HOLTechnologyCheck::Check(true, 1, 3, 1, 0, true, true, true, false, false), "not queued: available");
}

// T-Q11 = B (D601): a technology whose effect is not coded is refused first, whatever the rest (the C++ rule's order).
void Test_TechnologyNotImplemented(FUnitTest& T)
{
	T.AssertEquals(HOLTechnologyRules::NotImplemented, HOLTechnologyCheck::Check(false, 1, 3, 1, 0, true, true, true, false, false), "not implemented, everything ready");
	T.AssertEquals(HOLTechnologyRules::NotImplemented, HOLTechnologyCheck::Check(false, 0, 0, 2, 5, false, false, false, true, true), "before every other reason");
	T.AssertEquals(HOLTechnologyRules::Available, HOLTechnologyCheck::Check(true, 1, 3, 1, 0, true, true, true, false, false), "implemented: available");
	T.AssertEquals(HOLTechnologyRules::AlreadyQueued, HOLTechnologyCheck::Check(true, 1, 3, 1, 0, true, true, true, false, true), "implemented and queued: the queue rule");
}
// One vector of HeroesOfLiberty.Technologies.Rules (HOLTechnologyTableTests.cpp): the C++ rule through the library gives the expected
// reason (the one rule since MIG-CPP-Technology, D836 point 2: no script copy, the vectors were the proof of the move).
void TechnologyRulesVector(FUnitTest& T, const FString& Label, int Expected, bool bImplemented, int HeroTier, int HeroLevel, int MinTier, int MinHeroLevel,
	bool bRequiresDone, bool bBuildingFinished, bool bCanPay, bool bResearched, bool bQueued)
{
	T.AssertEquals(Expected, HOLTechnologyCheck::Check(bImplemented, HeroTier, HeroLevel, MinTier, MinHeroLevel, bRequiresDone, bBuildingFinished, bCanPay, bResearched, bQueued), "C++: " + Label);
}

// The availability part of HeroesOfLiberty.Technologies.Rules, vector for vector: Plough I (MinTier 1, MinHeroLevel 0) and Plough II (MinTier 2,
// requires Plough I) of Tools/Data/Technologies.csv, the "Ready" facts of the C++ test (hero tier 1, level 3, building finished, paid), the
// row made "Soon" (not implemented, T-Q11 = B, D601) and "Strict" (MinHeroLevel 5, T-Q1 = A). The rest of that C++ test (cost of a research
// as a queue item, bonus percents, Forge armours, the player's state, the "soon" log word) has no script twin and stays in C++.
void Test_TechnologyRulesVectors(FUnitTest& T)
{
	const int Plough1Tier = 1;
	const int Plough2Tier = 2;
	TechnologyRulesVector(T, "level I, tier 1, farm, paid: available", HOLTechnologyRules::Available, true, 1, 3, Plough1Tier, 0, true, true, true, false, false);
	TechnologyRulesVector(T, "no tier yet", HOLTechnologyRules::TierTooLow, true, 0, 3, Plough1Tier, 0, true, true, true, false, false);
	TechnologyRulesVector(T, "level II at tier 1", HOLTechnologyRules::TierTooLow, true, 1, 3, Plough2Tier, 0, true, true, true, false, false);
	TechnologyRulesVector(T, "level II without level I", HOLTechnologyRules::RequiresMissing, true, 2, 3, Plough2Tier, 0, false, true, true, false, false);
	TechnologyRulesVector(T, "level II at tier 2 with level I", HOLTechnologyRules::Available, true, 2, 3, Plough2Tier, 0, true, true, true, false, false);
	TechnologyRulesVector(T, "no finished building", HOLTechnologyRules::NoBuilding, true, 1, 3, Plough1Tier, 0, true, false, true, false, false);
	TechnologyRulesVector(T, "cannot pay", HOLTechnologyRules::CannotPay, true, 1, 3, Plough1Tier, 0, true, true, false, false, false);
	TechnologyRulesVector(T, "already queued", HOLTechnologyRules::AlreadyQueued, true, 1, 3, Plough1Tier, 0, true, true, true, false, true);
	TechnologyRulesVector(T, "researched wins over queued", HOLTechnologyRules::AlreadyResearched, true, 1, 3, Plough1Tier, 0, true, true, true, true, true);
	TechnologyRulesVector(T, "not implemented, everything else ready", HOLTechnologyRules::NotImplemented, false, 1, 3, Plough1Tier, 0, true, true, true, false, false);
	TechnologyRulesVector(T, "not implemented wins over the tier and the rest", HOLTechnologyRules::NotImplemented, false, 0, 3, Plough1Tier, 0, true, true, true, true, false);
	TechnologyRulesVector(T, "tier open, level 3 < 5", HOLTechnologyRules::HeroLevelTooLow, true, 1, 3, Plough1Tier, 5, true, true, true, false, false);
	TechnologyRulesVector(T, "level 5", HOLTechnologyRules::Available, true, 1, 5, Plough1Tier, 5, true, true, true, false, false);
	TechnologyRulesVector(T, "MinHeroLevel 0: the tier alone, whatever the level", HOLTechnologyRules::Available, true, 1, 0, Plough1Tier, 0, true, true, true, false, false);
}
