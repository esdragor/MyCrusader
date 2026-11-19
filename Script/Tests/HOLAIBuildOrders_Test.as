// Script port of HOLAIBuildOrdersTests.cpp (D853, CELEBORN-1119ca): the 3 tests of the AI slot's building and production orders.
//   AI.Build.Placement -> Test_AIBuildPlacement; AI.Build.Place -> Test_AIBuildPlace; AI.Build.Production -> Test_AIBuildProduction.
// Same values as the C++, same chain (UHOLAIOrderLibrary -> command queue -> UHOLConstructionSubsystem / UHOLProductionSubsystem at the step start; the
// step is UHOLSimulationSubsystem's OnStepStarted broadcast, no Mass). The world is HOLTestWorld's, the orders go through the HOLTestWorld wrappers
// (the script's own world context would not reach the test world). The C++ tests stay until the main session has seen these red then green.
//
// Not ported, and why (TestNotNull / "test world ready" of the C++ are checked as the script can):
//   - "the House row" (TestNotNull of UHOLBuildingSettings::FindType): the row is not exposed to the script; HOLTestWorld::PayBuildingCost returns false
//     without the row, and the test asserts that it paid ("the stock covered it").
//   - the C++ FWorld spawns the centres BEFORE InitVictory; here the game state (and its InitVictory) is spawned first. No rule depends on that order.

namespace AIBuildTest
{
	// SetSlotJson(Json, Slot, State, ?, Colour, Team, Name): State 0 Closed, 1 Human, 2 AI (as HOLDisconnect_Test.as).
	// The C++ FWorld: slot 0 Human, slot 1 AI, the others Closed.
	FString MakeJson()
	{
		FString Json = HOLMatchConfigRead::MakeDefaultJson();
		for (int Slot = 0; Slot < HOLMatchConfigRead::GetMaxSlots(); Slot++)
			Json = HOLMatchConfigRead::SetSlotJson(Json, Slot, 0, 0, Slot, 0, "");
		Json = HOLMatchConfigRead::SetSlotJson(Json, 0, 1, 0, 0, 0, "Human");
		Json = HOLMatchConfigRead::SetSlotJson(Json, 1, 2, 0, 1, 0, "");
		return Json;
	}

	// The C++ FWorld: a match's server, slot 0 human with a centre at (-20000, 0), slot 1 AI (a real PlayerState, the starting stock) with its centre
	// at (0, 0); the sources, the construction and the production subsystems start as at the match start. nullptr (and the out handles null) if a step fails.
	UHOLTestWorldHandle MakeWorld(AHOLBuilding&out OutCentre, AHOLBuilding&out OutEnemyCentre)
	{
		UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
		if (TestWorld == nullptr)
			return nullptr;
		AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 1, 1);
		HOLTestWorld::SpawnAIPlayerStates(TestWorld, MakeJson());
		OutCentre = HOLTestWorld::SpawnBuilding(TestWorld, n"MainCentre", 1, FVector(0.0, 0.0, 0.0), EHOLConstructionState::Finished);
		OutEnemyCentre = HOLTestWorld::SpawnBuilding(TestWorld, n"MainCentre", 0, FVector(-20000.0, 0.0, 0.0), EHOLConstructionState::Finished);
		bool bInit = HOLTestWorld::InitSourceSubsystem(TestWorld) && HOLTestWorld::InitConstructionSubsystem(TestWorld) && HOLTestWorld::InitProductionSubsystem(TestWorld);
		// IsReady of the C++: the AI's player state exists.
		bool bReady = State != nullptr && OutCentre != nullptr && OutEnemyCentre != nullptr && bInit && HOLTestWorld::CountPlayerStatesOfSlot(TestWorld, 1) == 1;
		if (!bReady)
		{
			HOLTestWorld::DestroyTestWorld(TestWorld);
			return nullptr;
		}
		return TestWorld;
	}

	float Dist2D(FVector A, FVector B)
	{
		float Dx = A.X - B.X;
		float Dy = A.Y - B.Y;
		return Math::Sqrt(Dx * Dx + Dy * Dy);
	}

	bool SameRes(FHOLResources A, FHOLResources B)
	{
		return A.Food == B.Food && A.Wood == B.Wood && A.Stone == B.Stone && A.Gold == B.Gold;
	}
}

// The placement query: the centre's own spot is refused (on a building), a spot in rings around it is found and passes the player's own check; the
// farm block is a mill and 8 farms, free, none on another.
void Test_AIBuildPlacement(FUnitTest& T)
{
	AHOLBuilding Centre;
	AHOLBuilding EnemyCentre;
	UHOLTestWorldHandle TestWorld = AIBuildTest::MakeWorld(Centre, EnemyCentre);
	T.AssertTrue(TestWorld != nullptr, "test world ready");
	if (TestWorld == nullptr)
		return;

	// The refusal's value comes from the game's enum (HOLTestWorld::GetPlacementRefusalBuilding), not from the test.
	T.AssertEquals(HOLTestWorld::GetPlacementRefusalBuilding(), HOLTestWorld::CheckAIPlacement(TestWorld, 1, n"House", FVector::ZeroVector), "a House on the centre: refused as on a building");
	T.AssertEquals(-1, HOLTestWorld::CheckAIPlacement(TestWorld, 1, n"MainCentre", FVector::ZeroVector), "a type nobody builds (the main centre): -1");

	FVector House;
	bool bHouseFound = HOLTestWorld::FindAIPlacement(TestWorld, 1, n"House", FVector::ZeroVector, 6, House);
	T.AssertTrue(bHouseFound, "a House spot is found around the centre");
	if (bHouseFound)
	{
		T.AssertEquals(0, HOLTestWorld::CheckAIPlacement(TestWorld, 1, n"House", House), "that spot passes the player's check");
		T.AssertTrue(AIBuildTest::Dist2D(House, FVector::ZeroVector) > 1.0, "and is beside the centre, not on it");
	}
	FVector Unused;
	T.AssertFalse(HOLTestWorld::FindAIPlacement(TestWorld, 1, n"House", FVector::ZeroVector, -1, Unused), "no ring (MaxRings -1): nothing found");

	FVector Mill;
	TArray<FVector> Farms;
	bool bBlockFound = HOLTestWorld::FindAIFarmBlock(TestWorld, 1, FVector::ZeroVector, 8, Mill, Farms);
	T.AssertTrue(bBlockFound, "a farm block is found");
	int NumFarms = Farms.Num();
	T.AssertEquals(8, NumFarms, "8 farms");
	if (bBlockFound && NumFarms == 8)
	{
		T.AssertEquals(0, HOLTestWorld::CheckAIPlacement(TestWorld, 1, n"Mill", Mill), "the mill passes the check");
		for (int Index = 0; Index < NumFarms; Index++)
			T.AssertEquals(0, HOLTestWorld::CheckAIPlacement(TestWorld, 1, n"Farm", Farms[Index]), "each farm passes the check");
		T.AssertFalse(HOLTestWorld::FarmBlockOverlaps(Mill, Farms), "none of the nine on another");
	}
	HOLTestWorld::DestroyTestWorld(TestWorld);
}

// A foundation placed by the AI through the queue: refused on a building before it enters the queue; at the next step it is owned by slot 1 and paid
// the type's cost, exactly; without the stock it is not placed (the human's refusal).
void Test_AIBuildPlace(FUnitTest& T)
{
	AHOLBuilding Centre;
	AHOLBuilding EnemyCentre;
	UHOLTestWorldHandle TestWorld = AIBuildTest::MakeWorld(Centre, EnemyCentre);
	T.AssertTrue(TestWorld != nullptr, "test world ready");
	if (TestWorld == nullptr)
		return;

	FVector Spot;
	bool bSpot = HOLTestWorld::FindAIPlacement(TestWorld, 1, n"House", FVector::ZeroVector, 6, Spot);
	T.AssertTrue(bSpot, "a spot");
	if (!bSpot)
	{
		HOLTestWorld::DestroyTestWorld(TestWorld);
		return;
	}
	T.AssertFalse(HOLTestWorld::IssueAIPlaceOrder(TestWorld, 1, n"House", FVector::ZeroVector), "on the centre: refused before the queue");
	T.AssertFalse(HOLTestWorld::IssueAIPlaceOrder(TestWorld, -1, n"House", Spot), "no slot: refused");

	FHOLResources Before = HOLTestWorld::GetSlotResources(TestWorld, 1);
	T.AssertTrue(HOLTestWorld::IssueAIPlaceOrder(TestWorld, 1, n"House", Spot), "on the free spot: enters the queue");
	T.AssertEquals(0, HOLTestWorld::CountBuildings(TestWorld, n"House", 1), "nothing before the step");
	HOLTestWorld::AdvanceSteps(TestWorld, 1);
	T.AssertEquals(1, HOLTestWorld::CountBuildings(TestWorld, n"House", 1), "next step: a House foundation of slot 1");
	// The expected stock is the game's: HOLConstruction::TryPay of the House row's cost on the stock before (HOLTestWorld::PayBuildingCost).
	FHOLResources Expected;
	T.AssertTrue(HOLTestWorld::PayBuildingCost(n"House", Before, Expected), "the stock covered it (and the House row exists)");
	T.AssertTrue(AIBuildTest::SameRes(HOLTestWorld::GetSlotResources(TestWorld, 1), Expected), "paid exactly the cost");

	// No stock: the order enters the queue (the AI cannot know better than the server), the foundation is not placed.
	FVector Other;
	T.AssertTrue(HOLTestWorld::FindAIPlacement(TestWorld, 1, n"House", FVector::ZeroVector, 6, Other), "a second spot");
	T.AssertTrue(AIBuildTest::Dist2D(Spot, Other) > 1.0, "the first is taken: the second differs");
	FHOLResources Nothing;
	HOLTestWorld::SetSlotResources(TestWorld, 1, Nothing);
	HOLTestWorld::IssueAIPlaceOrder(TestWorld, 1, n"House", Other);
	HOLTestWorld::AdvanceSteps(TestWorld, 1);
	T.AssertEquals(1, HOLTestWorld::CountBuildings(TestWorld, n"House", 1), "without the stock: still one House");
	HOLTestWorld::DestroyTestWorld(TestWorld);
}

// Production and rally of a finished building: queued (paid) and set at the next step; slot 1 cannot order slot 0's centre.
void Test_AIBuildProduction(FUnitTest& T)
{
	AHOLBuilding Centre;
	AHOLBuilding EnemyCentre;
	UHOLTestWorldHandle TestWorld = AIBuildTest::MakeWorld(Centre, EnemyCentre);
	T.AssertTrue(TestWorld != nullptr, "test world ready");
	if (TestWorld == nullptr)
		return;

	FHOLResources Before = HOLTestWorld::GetSlotResources(TestWorld, 1);
	T.AssertFalse(HOLTestWorld::IssueAIProduceOrder(TestWorld, 1, FVector(-20000.0, 0.0, 0.0), n"Peasant"), "slot 0's centre: not slot 1's building, refused");
	T.AssertFalse(HOLTestWorld::IssueAIProduceOrder(TestWorld, 1, FVector(9000.0, 9000.0, 0.0), n"Peasant"), "nothing at that point: refused");
	T.AssertTrue(HOLTestWorld::IssueAIProduceOrder(TestWorld, 1, FVector::ZeroVector, n"Peasant"), "a Peasant at its centre: enters the queue");
	T.AssertEquals(0, HOLTestWorld::GetProductionQueueCount(Centre), "nothing queued before the step");
	HOLTestWorld::AdvanceSteps(TestWorld, 1);
	T.AssertEquals(1, HOLTestWorld::GetProductionQueueCount(Centre), "next step: one Peasant in the centre's queue");
	FHOLResources After = HOLTestWorld::GetSlotResources(TestWorld, 1);
	T.AssertTrue(After.Food < Before.Food, "paid: the food went down");

	T.AssertFalse(HOLTestWorld::IssueAISetRallyOrder(TestWorld, 1, FVector(9000.0, 9000.0, 0.0), FVector(500.0, 500.0, 0.0)), "no building there: no rally");
	T.AssertTrue(HOLTestWorld::IssueAISetRallyOrder(TestWorld, 1, FVector::ZeroVector, FVector(3000.0, 1000.0, 0.0)), "the centre's rally point enters the queue");
	HOLTestWorld::AdvanceSteps(TestWorld, 1);
	T.AssertTrue(HOLTestWorld::HasBuildingRallyPoint(Centre), "next step: the rally point is set");
	T.AssertTrue(AIBuildTest::Dist2D(HOLTestWorld::GetBuildingRallyPoint(Centre), FVector(3000.0, 1000.0, 0.0)) < 1.0, "where it was asked");
	HOLTestWorld::DestroyTestWorld(TestWorld);
}
