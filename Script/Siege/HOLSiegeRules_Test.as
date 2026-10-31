// The siege engines' pure rules, alone (SiegeInProgress.md S1, GDD 6.1, D23, D77, D180): which engine opens at which tier, which are in
// the prototype, and why an engine cannot be queued now. The expectations come from GDD 6.1 and 11.1 (Ram tier 1, Mangonel tier 2,
// Trebuchet and Cannon tier 3; Ram and Mangonel only in the prototype), and from the rules' own functions, never from typed copies.

// GDD 6.1: the tier of each engine.
void Test_SiegeEngineTiers(FUnitTest& T)
{
	T.AssertEquals(1, HOLSiege::TierOf(EHOLSiegeEngine::Ram), "the Ram opens at tier 1");
	T.AssertEquals(2, HOLSiege::TierOf(EHOLSiegeEngine::Mangonel), "the Mangonel opens at tier 2");
	T.AssertEquals(3, HOLSiege::TierOf(EHOLSiegeEngine::Trebuchet), "the Trebuchet opens at tier 3");
	T.AssertEquals(3, HOLSiege::TierOf(EHOLSiegeEngine::Cannon), "the Cannon opens at tier 3 with the powder (D180)");
}

// D77: Ram and Mangonel are the prototype's siege; the other two wait.
void Test_SiegePrototypeSet(FUnitTest& T)
{
	T.AssertTrue(HOLSiege::InPrototype(EHOLSiegeEngine::Ram), "the Ram is in the prototype");
	T.AssertTrue(HOLSiege::InPrototype(EHOLSiegeEngine::Mangonel), "the Mangonel is in the prototype");
	T.AssertFalse(HOLSiege::InPrototype(EHOLSiegeEngine::Trebuchet), "the Trebuchet is not (D77)");
	T.AssertFalse(HOLSiege::InPrototype(EHOLSiegeEngine::Cannon), "the Cannon is not (D77)");
	T.AssertEquals(2, HOLSiege::PrototypeEngines().Num(), "two engines in the prototype");
}

// The tier of the engines never goes down along the list, and a prototype engine opens before tier 3.
void Test_SiegeTierOrder(FUnitTest& T)
{
	int Last = 0;
	for (int Index = 0; Index < HOLSiege::EngineCount; Index++)
	{
		EHOLSiegeEngine Engine = HOLSiege::EngineAt(Index);
		int Tier = HOLSiege::TierOf(Engine);
		T.AssertTrue(Tier >= Last, f"engine {Index}: tier {Tier} does not go below {Last}");
		T.AssertTrue(Tier >= 1, f"engine {Index}: the workshop is a tier 1 building, no engine opens at tier 0");
		if (HOLSiege::InPrototype(Engine))
			T.AssertTrue(Tier <= 2, f"engine {Index}: a prototype engine opens before tier 3");
		Last = Tier;
	}
}

// What a hero tier opens: nothing at tier 0, the Ram at 1, the Ram and the Mangonel from 2 (the prototype set only).
void Test_SiegeOpenedByTier(FUnitTest& T)
{
	T.AssertEquals(0, HOLSiege::OpenedEngines(0).Num(), "tier 0: no engine");
	TArray<EHOLSiegeEngine> One = HOLSiege::OpenedEngines(1);
	T.AssertEquals(1, One.Num(), "tier 1: one engine");
	T.AssertTrue(One.Num() == 1 && One[0] == EHOLSiegeEngine::Ram, "tier 1: the Ram");
	T.AssertEquals(2, HOLSiege::OpenedEngines(2).Num(), "tier 2: the Ram and the Mangonel");
	T.AssertEquals(2, HOLSiege::OpenedEngines(3).Num(), "tier 3: still the prototype's two");
}

// The reasons, one at a time: a tier too low, no finished workshop, an empty purse.
void Test_SiegeCheckOneReason(FUnitTest& T)
{
	T.AssertEquals(HOLSiege::Available, HOLSiege::Check(EHOLSiegeEngine::Ram, 1, true, true), "tier 1, workshop, paid: the Ram is available");
	T.AssertEquals(HOLSiege::TierTooLow, HOLSiege::Check(EHOLSiegeEngine::Ram, 0, true, true), "the Ram needs tier 1");
	T.AssertEquals(HOLSiege::TierTooLow, HOLSiege::Check(EHOLSiegeEngine::Mangonel, 1, true, true), "the Mangonel needs tier 2, tier 1 is not enough");
	T.AssertEquals(HOLSiege::Available, HOLSiege::Check(EHOLSiegeEngine::Mangonel, 2, true, true), "tier 2 opens the Mangonel");
	T.AssertEquals(HOLSiege::NoWorkshop, HOLSiege::Check(EHOLSiegeEngine::Ram, 1, false, true), "no finished siege workshop");
	T.AssertEquals(HOLSiege::CannotPay, HOLSiege::Check(EHOLSiegeEngine::Ram, 1, true, false), "the stock does not cover the cost");
}

// Outside the prototype first, then the order tier, workshop, cost: the first reason is the one told.
void Test_SiegeCheckReasonOrder(FUnitTest& T)
{
	T.AssertEquals(HOLSiege::NotInPrototype, HOLSiege::Check(EHOLSiegeEngine::Trebuchet, 3, true, true), "the Trebuchet is refused even at tier 3 (D77)");
	T.AssertEquals(HOLSiege::NotInPrototype, HOLSiege::Check(EHOLSiegeEngine::Cannon, 0, false, false), "outside the prototype comes first");
	T.AssertEquals(HOLSiege::TierTooLow, HOLSiege::Check(EHOLSiegeEngine::Mangonel, 0, false, false), "then the tier");
	T.AssertEquals(HOLSiege::NoWorkshop, HOLSiege::Check(EHOLSiegeEngine::Mangonel, 2, false, false), "then the workshop");
	T.AssertEquals(HOLSiege::CannotPay, HOLSiege::Check(EHOLSiegeEngine::Mangonel, 2, true, false), "then the cost");
}

// IsAvailable agrees with Check on every engine, every tier and every combination of the two booleans.
void Test_SiegeIsAvailableAgreesWithCheck(FUnitTest& T)
{
	for (int Index = 0; Index < HOLSiege::EngineCount; Index++)
	{
		EHOLSiegeEngine Engine = HOLSiege::EngineAt(Index);
		for (int Tier = 0; Tier <= 3; Tier++)
		{
			for (int Mask = 0; Mask < 4; Mask++)
			{
				bool bWorkshop = (Mask & 1) != 0;
				bool bPay = (Mask & 2) != 0;
				bool bAvailable = HOLSiege::IsAvailable(Engine, Tier, bWorkshop, bPay);
				T.AssertTrue(bAvailable == (HOLSiege::Check(Engine, Tier, bWorkshop, bPay) == HOLSiege::Available), f"engine {Index} tier {Tier} mask {Mask}: IsAvailable follows Check");
			}
		}
	}
}
