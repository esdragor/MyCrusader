// The AI's cavalry, alone (HOLAICavalry.as, C1, D672 CV-Q4): the decision to build the Stable, and the Light Cavalry it queues there, at every
// level. NOT COMPILED, NOT RUN (written 2026-11-04 without a build): to run with `quick_check -Parts tests` once the build is allowed.

// Every level builds the Stable (D672): Easy, Normal, Hard, tier 1, an army, wood, an enemy centre, none yet.
void Test_AICavalryEveryLevelBuildsTheStable(FUnitTest& T)
{
	FHOLAICavalrySettings Rule;
	for (int Level = 0; Level <= 2; Level++)
	{
		T.AssertTrue(HOLAICavalry::WantsStable(Rule, Level, 1, false, true, 10, 400, 150), f"level {Level} builds the Stable");
	}
}

// The setting is the one place that decides which levels: raised to Normal, Easy stops; 3 = nobody.
void Test_AICavalryMinDifficultyIsTheOnlySwitch(FUnitTest& T)
{
	FHOLAICavalrySettings Rule;
	Rule.MinDifficulty = 1;
	T.AssertFalse(HOLAICavalry::WantsStable(Rule, 0, 1, false, true, 10, 400, 150), "Easy not when asked from Normal");
	T.AssertTrue(HOLAICavalry::WantsStable(Rule, 1, 1, false, true, 10, 400, 150), "Normal yes");
	Rule.MinDifficulty = 3;
	T.AssertFalse(HOLAICavalry::WantsStable(Rule, 2, 1, false, true, 10, 400, 150), "3 = nobody");
}

// Each condition holds it back alone: the tier, an existing Stable, no enemy centre, too few soldiers, too little wood.
void Test_AICavalryEachConditionHoldsTheStableBack(FUnitTest& T)
{
	FHOLAICavalrySettings Rule;
	T.AssertFalse(HOLAICavalry::WantsStable(Rule, 2, 0, false, true, 10, 400, 150), "tier 0: the Stable is not open");
	T.AssertFalse(HOLAICavalry::WantsStable(Rule, 2, 1, true, true, 10, 400, 150), "one already (finished or under way)");
	T.AssertFalse(HOLAICavalry::WantsStable(Rule, 2, 1, false, false, 10, 400, 150), "no enemy centre known");
	T.AssertFalse(HOLAICavalry::WantsStable(Rule, 2, 1, false, true, 5, 400, 150), "5 soldiers: the army comes first");
	T.AssertTrue(HOLAICavalry::WantsStable(Rule, 2, 1, false, true, 6, 400, 150), "6 soldiers: enough");
	T.AssertFalse(HOLAICavalry::WantsStable(Rule, 2, 1, false, true, 10, 149, 150), "149 wood for 150");
	T.AssertTrue(HOLAICavalry::WantsStable(Rule, 2, 2, false, true, 10, 150, 150), "tier 2 and the exact wood");
}

// The quantity is the only thing that changes with the level: 2, 4, 6 by default, and it grows with the level.
void Test_AICavalryQuantityByLevel(FUnitTest& T)
{
	FHOLAICavalryTrain Train;
	T.AssertEquals(2, HOLAICavalry::MaxFor(Train, 0), "Easy: 2");
	T.AssertEquals(4, HOLAICavalry::MaxFor(Train, 1), "Normal: 4");
	T.AssertEquals(6, HOLAICavalry::MaxFor(Train, 2), "Hard: 6");
	T.AssertEquals(6, HOLAICavalry::MaxFor(Train, 3), "any other number counts as Hard");
	T.AssertTrue(HOLAICavalry::MaxFor(Train, 0) < HOLAICavalry::MaxFor(Train, 1) && HOLAICavalry::MaxFor(Train, 1) < HOLAICavalry::MaxFor(Train, 2),
		"the quantity grows with the level");
}

// A finished Stable, an empty queue, an empty train: a Light Cavalry, at every level.
void Test_AICavalryQueuesALightCavalryAtEveryLevel(FUnitTest& T)
{
	FHOLAICavalrySettings Rule;
	FHOLAICavalryTrain Train;
	for (int Level = 0; Level <= 2; Level++)
	{
		const FHOLAICavalryChoice Choice = HOLAICavalry::ChooseCavalry(Rule, Train, Level, true, 0, 0, true, true);
		T.AssertTrue(Choice.bQueue, f"level {Level}: a unit is queued");
		T.AssertTrue(Choice.Type == n"LightCavalry", f"level {Level}: the Light Cavalry");
	}
}

// The train stops at the level's quantity (alive plus queued): Easy at 2, Normal at 4, Hard at 6.
void Test_AICavalryTrainStopsAtTheLevelsQuantity(FUnitTest& T)
{
	FHOLAICavalrySettings Rule;
	FHOLAICavalryTrain Train;
	T.AssertTrue(HOLAICavalry::ChooseCavalry(Rule, Train, 0, true, 0, 1, true, true).bQueue, "Easy, 1 of 2: another");
	T.AssertFalse(HOLAICavalry::ChooseCavalry(Rule, Train, 0, true, 0, 2, true, true).bQueue, "Easy, 2 of 2: no more");
	T.AssertTrue(HOLAICavalry::ChooseCavalry(Rule, Train, 1, true, 0, 3, true, true).bQueue, "Normal, 3 of 4: another");
	T.AssertFalse(HOLAICavalry::ChooseCavalry(Rule, Train, 1, true, 0, 4, true, true).bQueue, "Normal, 4 of 4: no more");
	T.AssertTrue(HOLAICavalry::ChooseCavalry(Rule, Train, 2, true, 0, 5, true, true).bQueue, "Hard, 5 of 6: another");
	T.AssertFalse(HOLAICavalry::ChooseCavalry(Rule, Train, 2, true, 0, 6, true, true).bQueue, "Hard, 6 of 6: no more");
}

// Held back by the Stable (not finished, queue busy), the purse and the population.
void Test_AICavalryHeldBack(FUnitTest& T)
{
	FHOLAICavalrySettings Rule;
	FHOLAICavalryTrain Train;
	T.AssertFalse(HOLAICavalry::ChooseCavalry(Rule, Train, 2, false, 0, 0, true, true).bQueue, "Stable not finished");
	T.AssertFalse(HOLAICavalry::ChooseCavalry(Rule, Train, 2, true, 1, 0, true, true).bQueue, "one is already queued");
	T.AssertFalse(HOLAICavalry::ChooseCavalry(Rule, Train, 2, true, 0, 0, false, true).bQueue, "cannot pay (80 food + 40 gold)");
	T.AssertFalse(HOLAICavalry::ChooseCavalry(Rule, Train, 2, true, 0, 0, true, false).bQueue, "no place left");
}

// The setting switches the queue off as it does the building: 3 = nobody, even with a Stable.
void Test_AICavalryNobodyQueuesWhenSwitchedOff(FUnitTest& T)
{
	FHOLAICavalrySettings Rule;
	Rule.MinDifficulty = 3;
	FHOLAICavalryTrain Train;
	T.AssertFalse(HOLAICavalry::ChooseCavalry(Rule, Train, 2, true, 0, 0, true, true).bQueue, "3 = nobody");
}

// The real chain, the type tables to the script: the cavalry's own figures are what the rules above pay for (needs UnitTypes.uasset regenerated
// from the CSV, which this session could not do: red until then, and that is the point of the test).
void Test_AICavalryPaysTheRealCost(FUnitTest& T)
{
	FHOLAITypeFigures Unit;
	T.AssertTrue(HOLAIOrder::ReadAIUnitFigures(n"LightCavalry", Unit), "the Light Cavalry is a type of the table");
	FHOLAITypeFigures Building;
	T.AssertTrue(HOLAIOrder::ReadAIBuildingFigures(n"Stable", Building), "the Stable is a type of the building table");
	T.AssertEquals(150, Building.CostWood, "the Stable costs 150 wood");
	T.AssertEquals(80, Unit.CostFood, "the Light Cavalry costs 80 food");
	T.AssertEquals(40, Unit.CostGold, "and 40 gold");
}
