// World half of the removed HOLUnitTierProductionTests.cpp (D853, CELEBORN-1119cb): the one former C++ test "HeroesOfLiberty.Production.UnitTier" on the REAL chain.
// (HOLUnitTierProduction_Test.as, already there, holds the 3 decision tests; this file is the world test, one test: Test_UnitTierProductionWorld.)
// G1 (GDD 11.1, D711): a unit opens at the tier of its column MinTier, by ONE rule. Same chain as the C++: the production order of a slot goes
// through the command queue (UHOLCommandSubsystem::IssueProductionOrder) and the production subsystem at the step start (UHOLSimulationSubsystem's
// OnStepStarted broadcast, no Mass); the stock and the buildings' queues are what is measured (no unit is born, the C++ did not read one either).
// Two AI slots of the Dawn each own a Barracks, an Archery range, a Stable, a Chapel and a Siege workshop; slot 0 at tier 0, slot 1 at tier 1
// (forced, hol.Cheat.Tier's own state), the same orders for both. The C++ test is removed (red seen then green on integ-19, CELEBORN-1119ch).
//
// Bindings added for it (UHOLTestWorldLibrary, D839: linking to expose): SetSlotTierOverride, GetSlotTier, IssueProductionOrder, GetUnitCostTotal.
// Differences: the game state comes from HOLTestWorld::SpawnGameState (default config, InitVictory), the AI player states from the 2-AI json
// (slots 0 and 1, Dawn, the rest closed); the buildings are spawned after the subsystems' start, as in the C++ FWorld.

namespace UnitTierWorldTest
{
	// SetSlotJson(Json, Slot, State, Faction, Colour, Team, Name): State 2 = AI, Faction 0 = DawnOrder (the C++ MakeConfig).
	FString MakeJson()
	{
		FString Json = HOLMatchConfigRead::MakeDefaultJson();
		for (int Slot = 0; Slot < HOLMatchConfigRead::GetMaxSlots(); Slot++)
		{
			int State = Slot <= 1 ? 2 : 0;
			Json = HOLMatchConfigRead::SetSlotJson(Json, Slot, State, 0, Slot, 0, "");
		}
		return Json;
	}

	int Total(FHOLResources Stock)
	{
		return Stock.Food + Stock.Wood + Stock.Stone + Stock.Gold;
	}

	// What the slot's five buildings hold in queue, as "B/R/S/C/W" (Barracks / range / stable / chapel / workshop).
	FString Queues(TArray<AHOLBuilding> Buildings, int Slot)
	{
		int Base = Slot * 5;
		int B = HOLTestWorld::GetProductionQueueCount(Buildings[Base + 0]);
		int R = HOLTestWorld::GetProductionQueueCount(Buildings[Base + 1]);
		int S = HOLTestWorld::GetProductionQueueCount(Buildings[Base + 2]);
		int C = HOLTestWorld::GetProductionQueueCount(Buildings[Base + 3]);
		int W = HOLTestWorld::GetProductionQueueCount(Buildings[Base + 4]);
		return f"{B}/{R}/{S}/{C}/{W}";
	}

	// Every unit of the roster ordered at its own building, for the slot (the C++ OrderAll).
	void OrderAll(UHOLTestWorldHandle TestWorld, TArray<AHOLBuilding> Buildings, int Slot)
	{
		int Base = Slot * 5;
		HOLTestWorld::IssueProductionOrder(TestWorld, Slot, Buildings[Base + 0], n"Spearman");
		HOLTestWorld::IssueProductionOrder(TestWorld, Slot, Buildings[Base + 0], n"ManAtArms");
		HOLTestWorld::IssueProductionOrder(TestWorld, Slot, Buildings[Base + 1], n"Archer");
		HOLTestWorld::IssueProductionOrder(TestWorld, Slot, Buildings[Base + 1], n"Crossbowman");
		HOLTestWorld::IssueProductionOrder(TestWorld, Slot, Buildings[Base + 2], n"LightCavalry");
		HOLTestWorld::IssueProductionOrder(TestWorld, Slot, Buildings[Base + 3], n"Monk");
		HOLTestWorld::IssueProductionOrder(TestWorld, Slot, Buildings[Base + 4], n"Ram");
		HOLTestWorld::IssueProductionOrder(TestWorld, Slot, Buildings[Base + 4], n"Mangonel");
	}

	int Cost(UHOLTestWorldHandle TestWorld, FName Type)
	{
		return HOLTestWorld::GetUnitCostTotal(TestWorld, Type);
	}

	// The price of the tier 1 roster minus the Mangonel (what slot 1 queues, and what slot 0 adds at tier 1 on top of its first lot).
	int Tier1Lot(UHOLTestWorldHandle TestWorld)
	{
		return Cost(TestWorld, n"Spearman") + Cost(TestWorld, n"ManAtArms") + Cost(TestWorld, n"Archer") + Cost(TestWorld, n"Crossbowman")
			+ Cost(TestWorld, n"LightCavalry") + Cost(TestWorld, n"Monk") + Cost(TestWorld, n"Ram");
	}
}

void Test_UnitTierProductionWorld(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "a test world is created");
	if (TestWorld == nullptr)
		return;
	AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 0, 2);
	HOLTestWorld::SpawnAIPlayerStates(TestWorld, UnitTierWorldTest::MakeJson());
	bool bInit = HOLTestWorld::InitSourceSubsystem(TestWorld) && HOLTestWorld::InitConstructionSubsystem(TestWorld) && HOLTestWorld::InitProductionSubsystem(TestWorld);

	// The five buildings of each slot at (Slot * -20000 + 4000, Y) as the C++ Put: Barracks 0, ArcheryRange 3000, Stable 6000, Chapel 9000, SiegeWorkshop 12000.
	TArray<AHOLBuilding> Buildings;
	for (int Slot = 0; Slot < 2; Slot++)
	{
		double X = Slot * -20000.0 + 4000.0;
		Buildings.Add(HOLTestWorld::SpawnBuilding(TestWorld, n"Barracks", Slot, FVector(X, 0.0, 0.0), EHOLConstructionState::Finished));
		Buildings.Add(HOLTestWorld::SpawnBuilding(TestWorld, n"ArcheryRange", Slot, FVector(X, 3000.0, 0.0), EHOLConstructionState::Finished));
		Buildings.Add(HOLTestWorld::SpawnBuilding(TestWorld, n"Stable", Slot, FVector(X, 6000.0, 0.0), EHOLConstructionState::Finished));
		Buildings.Add(HOLTestWorld::SpawnBuilding(TestWorld, n"Chapel", Slot, FVector(X, 9000.0, 0.0), EHOLConstructionState::Finished));
		Buildings.Add(HOLTestWorld::SpawnBuilding(TestWorld, n"SiegeWorkshop", Slot, FVector(X, 12000.0, 0.0), EHOLConstructionState::Finished));
	}
	bool bBuildings = true;
	for (int Index = 0; Index < Buildings.Num(); Index++)
		bBuildings = bBuildings && Buildings[Index] != nullptr;
	bool bReady = State != nullptr && bInit && bBuildings && HOLTestWorld::CountPlayerStatesOfSlot(TestWorld, 0) == 1 && HOLTestWorld::CountPlayerStatesOfSlot(TestWorld, 1) == 1;
	T.AssertTrue(bReady, "the test world stands");
	if (!bReady)
	{
		HOLTestWorld::DestroyTestWorld(TestWorld);
		return;
	}

	FHOLResources Rich = DisconnectTest::Res(5000, 5000, 5000, 5000);
	HOLTestWorld::SetSlotResources(TestWorld, 0, Rich);
	HOLTestWorld::SetSlotResources(TestWorld, 1, Rich);
	HOLTestWorld::SetSlotTierOverride(TestWorld, 0, 0);
	HOLTestWorld::SetSlotTierOverride(TestWorld, 1, 1);
	T.AssertEquals(0, HOLTestWorld::GetSlotTier(TestWorld, 0), "slot 0 is at tier 0");
	T.AssertEquals(1, HOLTestWorld::GetSlotTier(TestWorld, 1), "slot 1 is at tier 1");

	// 1. Tier 0, the same orders for both: the server refuses the Man-at-arms, the Crossbowman, the Light cavalry, the Monk, the Ram and the Mangonel
	// of slot 0, and queues what tier 0 opens (Spearman, Archer). Slot 1 (tier 1) queues all but the Mangonel (tier 2). The stock moves by exactly the
	// price of what was queued.
	UnitTierWorldTest::OrderAll(TestWorld, Buildings, 0);
	UnitTierWorldTest::OrderAll(TestWorld, Buildings, 1);
	HOLTestWorld::AdvanceSteps(TestWorld, 1);
	FString Queues0 = UnitTierWorldTest::Queues(Buildings, 0);
	FString Queues1 = UnitTierWorldTest::Queues(Buildings, 1);
	T.AssertTrue(Queues0 == "1/1/0/0/0", f"tier 0: Barracks / range / stable / chapel / workshop queues: the Spearman and the Archer only (got {Queues0})");
	T.AssertTrue(Queues1 == "2/2/1/1/1", f"tier 1: the Man-at-arms, the Crossbowman, the Light cavalry, the Monk and the Ram come out too, not the Mangonel (got {Queues1})");
	int Spent0 = 20000 - UnitTierWorldTest::Total(HOLTestWorld::GetSlotResources(TestWorld, 0));
	int Spent1 = 20000 - UnitTierWorldTest::Total(HOLTestWorld::GetSlotResources(TestWorld, 1));
	int PriceLow = UnitTierWorldTest::Cost(TestWorld, n"Spearman") + UnitTierWorldTest::Cost(TestWorld, n"Archer");
	T.AssertTrue(PriceLow > 0, "a Spearman and an Archer cost something (the unit rows are read)");
	T.AssertEquals(PriceLow, Spent0, "tier 0 paid the Spearman and the Archer only");
	T.AssertEquals(UnitTierWorldTest::Tier1Lot(TestWorld), Spent1, "tier 1 paid all its queued units");

	// 2. The hero reaches tier 1 (the forced tier lifted to 1): the same orders now pass for slot 0, the Mangonel still waits for tier 2.
	HOLTestWorld::SetSlotTierOverride(TestWorld, 0, 1);
	int Before = UnitTierWorldTest::Total(HOLTestWorld::GetSlotResources(TestWorld, 0));
	UnitTierWorldTest::OrderAll(TestWorld, Buildings, 0);
	HOLTestWorld::AdvanceSteps(TestWorld, 1);
	FString QueuesLater = UnitTierWorldTest::Queues(Buildings, 0);
	T.AssertTrue(QueuesLater == "3/3/1/1/1", f"tier 1 for slot 0: what tier 1 opens is queued on top of the first lot; the workshop holds the Ram only (got {QueuesLater})");
	int SpentLater = Before - UnitTierWorldTest::Total(HOLTestWorld::GetSlotResources(TestWorld, 0));
	T.AssertEquals(UnitTierWorldTest::Tier1Lot(TestWorld), SpentLater, "slot 0 paid the Man-at-arms, the Crossbowman, the Light cavalry, the Monk, the Ram and one more Spearman and Archer");

	// 3. Tier 2: the Mangonel is queued at the workshop.
	HOLTestWorld::SetSlotTierOverride(TestWorld, 0, 2);
	HOLTestWorld::IssueProductionOrder(TestWorld, 0, Buildings[4], n"Mangonel");
	HOLTestWorld::AdvanceSteps(TestWorld, 1);
	T.AssertEquals(2, HOLTestWorld::GetProductionQueueCount(Buildings[4]), "tier 2: the Mangonel is queued at the workshop");

	HOLTestWorld::DestroyTestWorld(TestWorld);
}
