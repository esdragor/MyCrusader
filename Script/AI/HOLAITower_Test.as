// The AI's Watch Tower, alone (HOLAITower.as, point 7, FARAMIR-1124sg): how many towers a level wants and when the next one is built.
// The values asserted come from the rule (MaxFor, WantsTower), not from a constant copied here.

// The quantity grows with the level: Easy none, Normal one, Hard two; any other number counts as Hard.
void Test_AITowerQuantityByLevel(FUnitTest& T)
{
	FHOLAITowerSettings Rule;
	const int Easy = HOLAITower::MaxFor(Rule, 0);
	const int Normal = HOLAITower::MaxFor(Rule, 1);
	const int Hard = HOLAITower::MaxFor(Rule, 2);
	T.AssertEquals(0, Easy, "Easy: no tower");
	T.AssertEquals(1, Normal, "Normal: one");
	T.AssertEquals(2, Hard, "Hard: two");
	T.AssertEquals(Hard, HOLAITower::MaxFor(Rule, 3), "any other number counts as Hard");
	T.AssertTrue(Easy < Normal && Normal < Hard, "the quantity grows with the level");
}

// Normal and Hard build their first tower; Easy never does (its quantity is 0).
void Test_AITowerFirstTowerByLevel(FUnitTest& T)
{
	FHOLAITowerSettings Rule;
	T.AssertFalse(HOLAITower::WantsTower(Rule, 0, 0, 0, true, 10, 400, 100), "Easy: none wanted");
	T.AssertTrue(HOLAITower::WantsTower(Rule, 1, 0, 0, true, 10, 400, 100), "Normal builds its first");
	T.AssertTrue(HOLAITower::WantsTower(Rule, 2, 0, 0, true, 10, 400, 100), "Hard builds its first");
}

// It stops at the level's quantity (finished plus under way): Normal at 1, Hard at 2.
void Test_AITowerStopsAtTheLevelsQuantity(FUnitTest& T)
{
	FHOLAITowerSettings Rule;
	T.AssertFalse(HOLAITower::WantsTower(Rule, 1, 0, 1, true, 10, 400, 100), "Normal, 1 of 1: no more");
	T.AssertTrue(HOLAITower::WantsTower(Rule, 2, 0, 1, true, 10, 400, 100), "Hard, 1 of 2: another");
	T.AssertFalse(HOLAITower::WantsTower(Rule, 2, 0, 2, true, 10, 400, 100), "Hard, 2 of 2: no more");
}

// Each condition holds it back alone: the tier, no enemy centre, too few soldiers, too little wood; the exact wood and soldiers pass.
void Test_AITowerEachConditionHoldsItBack(FUnitTest& T)
{
	FHOLAITowerSettings Rule;
	Rule.BuildTier = 1;
	T.AssertFalse(HOLAITower::WantsTower(Rule, 2, 0, 0, true, 10, 400, 100), "tier 0 when the rule asks tier 1");
	T.AssertTrue(HOLAITower::WantsTower(Rule, 2, 1, 0, true, 10, 400, 100), "tier 1: open");
	FHOLAITowerSettings Plain;
	T.AssertFalse(HOLAITower::WantsTower(Plain, 2, 0, 0, false, 10, 400, 100), "no enemy centre known");
	T.AssertFalse(HOLAITower::WantsTower(Plain, 2, 0, 0, true, Plain.MinSoldiers - 1, 400, 100), "one soldier short: the army comes first");
	T.AssertTrue(HOLAITower::WantsTower(Plain, 2, 0, 0, true, Plain.MinSoldiers, 400, 100), "the exact soldiers");
	T.AssertFalse(HOLAITower::WantsTower(Plain, 2, 0, 0, true, 10, 99, 100), "99 wood for 100");
	T.AssertTrue(HOLAITower::WantsTower(Plain, 2, 0, 0, true, 10, 100, 100), "the exact wood");
}
