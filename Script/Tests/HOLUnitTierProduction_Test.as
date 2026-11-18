// Script half of HeroesOfLiberty.Production.UnitTier (HOLUnitTierProductionTests.cpp, SAURON-1117f, D830/D839): the DECISION of the production orders by
// tier, with the unit rule the server calls (HOLUnitTier::Check, the column MinTier). The same lot of eight orders (every unit of the roster at its own
// building, the buildings finished, the stock rich) is played at tier 0, 1 and 2 and the queue each building would hold ("B/R/S/C/W": Barracks, Archery
// range, Stable, Chapel, Siege workshop) is counted, as the C++ test measures it on the real chain.
// NOT ported (the world, the subsystems and the buildings are not exposed to script, see Docs\Rapports\SAURON-1117f.md): the real queues of
// AHOLBuilding, the stock paid by the server (Cost sums), the refusal by the command queue; the C++ test stays.

// The building each unit of the roster is ordered at (OrderAll of the C++ test): 0 Barracks, 1 range, 2 stable, 3 chapel, 4 workshop.
int UnitTierBuildingOf(const FString& Type)
{
	if (Type == "Spearman" || Type == "ManAtArms")
		return 0;
	if (Type == "Archer" || Type == "Crossbowman")
		return 1;
	if (Type == "LightCavalry")
		return 2;
	if (Type == "Monk")
		return 3;
	return 4; // Ram, Mangonel
}

TArray<FString> UnitTierRoster()
{
	TArray<FString> Types;
	Types.Add("Spearman");
	Types.Add("ManAtArms");
	Types.Add("Archer");
	Types.Add("Crossbowman");
	Types.Add("LightCavalry");
	Types.Add("Monk");
	Types.Add("Ram");
	Types.Add("Mangonel");
	return Types;
}

// The queue counts one lot of orders adds for a slot at this tier: the orders the rule lets through.
FString UnitTierQueuesAt(int Tier)
{
	TArray<int> Counts;
	for (int Index = 0; Index < 5; Index++)
	{
		Counts.Add(0);
	}
	TArray<FString> Roster = UnitTierRoster();
	for (int Index = 0; Index < Roster.Num(); Index++)
	{
		if (HOLUnitTier::Check(Roster[Index], Tier, true, true) == HOLUnitTier::Available)
		{
			Counts[UnitTierBuildingOf(Roster[Index])] += 1;
		}
	}
	return "" + Counts[0] + "/" + Counts[1] + "/" + Counts[2] + "/" + Counts[3] + "/" + Counts[4];
}

// 1. Tier 0: the Spearman and the Archer only. Tier 1: all but the Mangonel (tier 2).
void Test_UnitTierProduction_LotAtTier0And1(FUnitTest& T)
{
	T.AssertEquals("1/1/0/0/0", UnitTierQueuesAt(0), "tier 0: Barracks / range / stable / chapel / workshop queues: the Spearman and the Archer only");
	T.AssertEquals("2/2/1/1/1", UnitTierQueuesAt(1), "tier 1: the Man-at-arms, the Crossbowman, the Light cavalry, the Monk and the Ram come out too (not the Mangonel)");
}

// 2. A slot lifted from tier 0 to 1 and sending the same orders again: the first lot plus the second lot (3/3/1/1/1 of the C++ test).
void Test_UnitTierProduction_SecondLotAfterTierLift(FUnitTest& T)
{
	TArray<FString> Roster = UnitTierRoster();
	TArray<int> Counts;
	for (int Index = 0; Index < 5; Index++)
	{
		Counts.Add(0);
	}
	for (int Tier = 0; Tier <= 1; Tier++)
	{
		for (int Index = 0; Index < Roster.Num(); Index++)
		{
			if (HOLUnitTier::Check(Roster[Index], Tier, true, true) == HOLUnitTier::Available)
			{
				Counts[UnitTierBuildingOf(Roster[Index])] += 1;
			}
		}
	}
	T.AssertEquals("3/3/1/1/1", "" + Counts[0] + "/" + Counts[1] + "/" + Counts[2] + "/" + Counts[3] + "/" + Counts[4], "tier 1 for slot 0: queued on top of the first lot; the workshop holds the Ram only");
}

// 3. Tier 2: the Mangonel is queued at the workshop (the Ram and the Mangonel: 2 orders pass at the workshop).
void Test_UnitTierProduction_MangonelAtTier2(FUnitTest& T)
{
	T.AssertEquals(HOLUnitTier::TierTooLow, HOLUnitTier::Check("Mangonel", 1, true, true), "tier 1: the Mangonel waits for tier 2");
	T.AssertEquals(HOLUnitTier::Available, HOLUnitTier::Check("Mangonel", 2, true, true), "tier 2: the Mangonel is queued");
	T.AssertEquals("2/2/1/1/2", UnitTierQueuesAt(2), "tier 2: the workshop holds the Ram and the Mangonel");
}
