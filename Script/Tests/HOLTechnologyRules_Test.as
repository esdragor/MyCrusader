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
// The script's rule (CheckImplemented) gives the simulation's reason (C++ HOLTechnology::Check, which the server applies) on every
// combination of the facts: AlreadyQueued, NotImplemented and the order of the reasons cannot drift apart unseen (T8).
void Test_TechnologyScriptAndSimulationRulesAgree(FUnitTest& T)
{
	int Cases = 0;
	int Differences = 0;
	for (int Implemented = 0; Implemented < 2; Implemented++)
	{
		for (int HeroTier = 0; HeroTier < 3; HeroTier++)
		{
			for (int HeroLevel = 0; HeroLevel < 7; HeroLevel += 3)
			{
				for (int MinTier = 0; MinTier < 3; MinTier++)
				{
					for (int MinLevel = 0; MinLevel < 7; MinLevel += 5)
					{
						for (int Flags = 0; Flags < 32; Flags++)
						{
							const bool bRequires = (Flags & 1) != 0;
							const bool bBuilding = (Flags & 2) != 0;
							const bool bPay = (Flags & 4) != 0;
							const bool bDone = (Flags & 8) != 0;
							const bool bQueued = (Flags & 16) != 0;
							const int Script = HOLTechnologyRules::CheckImplemented(Implemented == 1, HeroTier, HeroLevel, MinTier, MinLevel, bRequires, bBuilding, bPay, bDone, bQueued);
							const int Sim = HOLTechnologyCheck::Check(Implemented == 1, HeroTier, HeroLevel, MinTier, MinLevel, bRequires, bBuilding, bPay, bDone, bQueued);
							Cases++;
							if (Script != Sim)
								Differences++;
						}
					}
				}
			}
		}
	}
	T.AssertEquals(2 * 3 * 3 * 3 * 2 * 32, Cases, "every case compared");
	T.AssertEquals(0, Differences, "the script and the simulation agree on every case");
}

// One vector of HeroesOfLiberty.Technologies.Rules (HOLTechnologyTableTests.cpp): the C++ rule through the library and the script's rule
// both give the expected reason (the same vectors on both sides while two copies exist, MIG-J-Technology, D830/D835).
void TechnologyRulesVector(FUnitTest& T, const FString& Label, int Expected, bool bImplemented, int HeroTier, int HeroLevel, int MinTier, int MinHeroLevel,
	bool bRequiresDone, bool bBuildingFinished, bool bCanPay, bool bResearched, bool bQueued)
{
	T.AssertEquals(Expected, HOLTechnologyCheck::Check(bImplemented, HeroTier, HeroLevel, MinTier, MinHeroLevel, bRequiresDone, bBuildingFinished, bCanPay, bResearched, bQueued), "C++: " + Label);
	T.AssertEquals(Expected, HOLTechnologyRules::CheckImplemented(bImplemented, HeroTier, HeroLevel, MinTier, MinHeroLevel, bRequiresDone, bBuildingFinished, bCanPay, bResearched, bQueued), "script: " + Label);
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
