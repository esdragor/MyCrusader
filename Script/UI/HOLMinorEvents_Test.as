// D202 / D804 B: the minor events the HUD reads (UHOLScriptScreen::ObserveMinorEvents, the rule behind GetMinorEventsNow): the counter goes up by one
// when a finished building of the player appears, and by the researches gained; the first observation only takes the baseline.
// Not compiled when written (GLORFINDEL-1120i): the stub counts nothing, so both tests are red on the first commit.

// A finished House arrives after the baseline (a build that ends, or one that reaches a client replicated finished): the count is 1 and the place is its own.
void Test_MinorEventsBuildingFinished(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "test world ready");
	if (TestWorld == nullptr)
		return;
	AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 1, 1);
	AHOLBuilding Centre = HOLTestWorld::SpawnBuilding(TestWorld, n"MainCentre", 1, FVector(0.0, 0.0, 0.0), EHOLConstructionState::Finished);
	T.AssertTrue(State != nullptr && Centre != nullptr, "game state and centre spawned");

	FHOLMinorEventsWatch Watch;
	TArray<AHOLBuilding> Finished;
	Finished.Add(Centre);
	// Baseline: the centre already there counts for nothing.
	UHOLScriptScreen::ObserveMinorEvents(Watch, Finished, 0);
	int AtBaseline = Watch.BuildingsFinished;
	T.AssertEquals(0, AtBaseline, "the baseline counts nothing");

	AHOLBuilding House = HOLTestWorld::SpawnBuilding(TestWorld, n"House", 1, FVector(2000.0, 500.0, 0.0), EHOLConstructionState::Finished);
	T.AssertTrue(House != nullptr, "house spawned");
	Finished.Add(House);
	UHOLScriptScreen::ObserveMinorEvents(Watch, Finished, 0);
	int AfterHouse = Watch.BuildingsFinished;
	T.AssertEquals(1, AfterHouse, "one finished building appeared: the count went up by one");
	T.AssertTrue(Watch.LastBuildingLocation.X > 1000.0, "the last place is the house's, computed from the building");

	// The same buildings again: nothing more.
	UHOLScriptScreen::ObserveMinorEvents(Watch, Finished, 0);
	int AfterAgain = Watch.BuildingsFinished;
	T.AssertEquals(1, AfterAgain, "a building is counted once");
	HOLTestWorld::DestroyTestWorld(TestWorld);
}

// The researches: the first count is the baseline, a later one counts the difference.
void Test_MinorEventsResearchDone(FUnitTest& T)
{
	FHOLMinorEventsWatch Watch;
	TArray<AHOLBuilding> None;
	UHOLScriptScreen::ObserveMinorEvents(Watch, None, 2);
	int AtBaseline = Watch.ResearchesDone;
	T.AssertEquals(0, AtBaseline, "the researches already done at the baseline count for nothing");
	UHOLScriptScreen::ObserveMinorEvents(Watch, None, 3);
	int AfterOne = Watch.ResearchesDone;
	T.AssertEquals(1, AfterOne, "one research more: the count went up by one");
}
