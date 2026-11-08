// The AI's Siege Workshop, alone (HOLAISiege.as, S6): the decision to build it, and the engines it queues there.

// Hard, tier 1, an army, wood, an enemy centre, no Workshop yet: it builds.
void Test_AISiegeHardBuildsTheWorkshop(FUnitTest& T)
{
	FHOLAISiegeSettings Rule;
	T.AssertTrue(HOLAISiege::WantsWorkshop(Rule, 2, 1, false, true, 10, 400, 175), "Hard builds the Siege Workshop");
}

// Easy never does (the user: Hard yes, Easy no); Normal is not decided: it does not, by the provisional setting.
void Test_AISiegeEasyDoesNotBuildTheWorkshop(FUnitTest& T)
{
	FHOLAISiegeSettings Rule;
	T.AssertFalse(HOLAISiege::WantsWorkshop(Rule, 0, 1, false, true, 10, 400, 175), "Easy does not");
	T.AssertFalse(HOLAISiege::WantsWorkshop(Rule, 1, 1, false, true, 10, 400, 175), "Normal does not (provisional)");
}

// The setting is the one place that decides which difficulties: raised to Normal, Normal builds and Easy still does not.
void Test_AISiegeMinDifficultyIsTheOnlySwitch(FUnitTest& T)
{
	FHOLAISiegeSettings Rule;
	Rule.MinDifficulty = 1;
	T.AssertTrue(HOLAISiege::WantsWorkshop(Rule, 1, 1, false, true, 10, 400, 175), "Normal builds when asked");
	T.AssertFalse(HOLAISiege::WantsWorkshop(Rule, 0, 1, false, true, 10, 400, 175), "Easy still not");
	Rule.MinDifficulty = 3;
	T.AssertFalse(HOLAISiege::WantsWorkshop(Rule, 2, 1, false, true, 10, 400, 175), "3 = nobody");
}

// Each condition holds it back alone: the tier, an existing Workshop, no enemy centre, too few soldiers, too little wood.
void Test_AISiegeEachConditionHoldsItBack(FUnitTest& T)
{
	FHOLAISiegeSettings Rule;
	T.AssertFalse(HOLAISiege::WantsWorkshop(Rule, 2, 0, false, true, 10, 400, 175), "tier 0: the Workshop is not open");
	T.AssertFalse(HOLAISiege::WantsWorkshop(Rule, 2, 1, true, true, 10, 400, 175), "one already (finished or under way)");
	T.AssertFalse(HOLAISiege::WantsWorkshop(Rule, 2, 1, false, false, 10, 400, 175), "no enemy centre known");
	T.AssertFalse(HOLAISiege::WantsWorkshop(Rule, 2, 1, false, true, 7, 400, 175), "7 soldiers: the army comes first");
	T.AssertTrue(HOLAISiege::WantsWorkshop(Rule, 2, 1, false, true, 8, 400, 175), "8 soldiers: enough");
	T.AssertFalse(HOLAISiege::WantsWorkshop(Rule, 2, 1, false, true, 10, 174, 175), "174 wood for 175");
	T.AssertTrue(HOLAISiege::WantsWorkshop(Rule, 2, 2, false, true, 10, 175, 175), "tier 2 and the exact wood");
}

// A finished Workshop, an empty queue, tier 1: a Ram (the Mangonel opens at tier 2). Easy queues nothing.
void Test_AIEngineHardQueuesARamAtTier1(FUnitTest& T)
{
	FHOLAISiegeSettings Rule;
	FHOLAIEngineSettings Train;
	FHOLAIEngineChoice Choice = HOLAISiege::ChooseEngine(Rule, Train, 2, 1, true, 0, 0, 0, true, true);
	T.AssertTrue(Choice.bQueue, "a Ram");
	T.AssertTrue(Choice.Type == n"Ram", "the Ram at tier 1");
	T.AssertFalse(HOLAISiege::ChooseEngine(Rule, Train, 0, 1, true, 0, 0, 0, true, true).bQueue, "Easy: none");
}

// At tier 2 the Mangonel comes first, then the Rams of the train, then nothing more.
void Test_AIEngineTrainIsShortAndOrdered(FUnitTest& T)
{
	FHOLAISiegeSettings Rule;
	FHOLAIEngineSettings Train;
	T.AssertTrue(HOLAISiege::ChooseEngine(Rule, Train, 2, 2, true, 0, 0, 0, true, true).Type == n"Mangonel", "tier 2: the Mangonel first");
	T.AssertTrue(HOLAISiege::ChooseEngine(Rule, Train, 2, 2, true, 0, 0, 1, true, true).Type == n"Ram", "one Mangonel: a Ram");
	T.AssertTrue(HOLAISiege::ChooseEngine(Rule, Train, 2, 2, true, 0, 1, 1, true, true).Type == n"Ram", "one Ram: a second");
	T.AssertFalse(HOLAISiege::ChooseEngine(Rule, Train, 2, 2, true, 0, 2, 1, true, true).bQueue, "2 Rams and a Mangonel: the train is full");
}

// Held back by the Workshop (not finished, queue busy) and by the purse (a Mangonel too dear: the Ram goes instead).
void Test_AIEngineHeldBack(FUnitTest& T)
{
	FHOLAISiegeSettings Rule;
	FHOLAIEngineSettings Train;
	T.AssertFalse(HOLAISiege::ChooseEngine(Rule, Train, 2, 1, false, 0, 0, 0, true, true).bQueue, "Workshop not finished");
	T.AssertFalse(HOLAISiege::ChooseEngine(Rule, Train, 2, 1, true, 1, 0, 0, true, true).bQueue, "one is already queued");
	T.AssertTrue(HOLAISiege::ChooseEngine(Rule, Train, 2, 2, true, 0, 0, 0, true, false).Type == n"Ram", "cannot pay the Mangonel: a Ram");
	T.AssertFalse(HOLAISiege::ChooseEngine(Rule, Train, 2, 1, true, 0, 0, 0, false, false).bQueue, "cannot pay the Ram");
}

// The engines follow the same switch as the Workshop: Normal queues nothing by the provisional setting (even at tier 2 with a finished
// Workshop and the purse full), and raising MinDifficulty to Normal lets Normal queue and still not Easy.
void Test_AIEngineDifficultyIsTheSameSwitchAsTheWorkshop(FUnitTest& T)
{
	FHOLAISiegeSettings Rule;
	FHOLAIEngineSettings Train;
	T.AssertFalse(HOLAISiege::ChooseEngine(Rule, Train, 1, 2, true, 0, 0, 0, true, true).bQueue, "Normal: none (provisional)");
	Rule.MinDifficulty = 1;
	T.AssertTrue(HOLAISiege::ChooseEngine(Rule, Train, 1, 2, true, 0, 0, 0, true, true).Type == n"Mangonel", "Normal queues when asked");
	T.AssertFalse(HOLAISiege::ChooseEngine(Rule, Train, 0, 2, true, 0, 0, 0, true, true).bQueue, "Easy still not");
}

// The size of the train is a setting, not the rule: no Rams and no Mangonels = nothing; one more Ram allowed = a third Ram.
void Test_AIEngineTrainSizeIsASetting(FUnitTest& T)
{
	FHOLAISiegeSettings Rule;
	FHOLAIEngineSettings Train;
	Train.MaxRams = 0;
	Train.MaxMangonels = 0;
	T.AssertFalse(HOLAISiege::ChooseEngine(Rule, Train, 2, 2, true, 0, 0, 0, true, true).bQueue, "an empty train: nothing");
	Train.MaxRams = 3;
	Train.MaxMangonels = 1;
	T.AssertTrue(HOLAISiege::ChooseEngine(Rule, Train, 2, 2, true, 0, 2, 1, true, true).Type == n"Ram", "a third Ram when the train allows 3");
	T.AssertFalse(HOLAISiege::ChooseEngine(Rule, Train, 2, 2, true, 0, 3, 1, true, true).bQueue, "3 Rams and a Mangonel: full again");
}

// The Mangonel never comes before its tier, even with an empty train and the purse full; and the purse alone stops a train that is not full.
void Test_AIEngineNoMangonelBeforeTier2AndPurseStops(FUnitTest& T)
{
	FHOLAISiegeSettings Rule;
	FHOLAIEngineSettings Train;
	FHOLAIEngineChoice AtTier1 = HOLAISiege::ChooseEngine(Rule, Train, 2, 1, true, 0, 2, 0, true, true);
	T.AssertFalse(AtTier1.bQueue, "tier 1, 2 Rams: the Mangonel is not open, the Rams are full: nothing");
	FHOLAIEngineChoice Poor = HOLAISiege::ChooseEngine(Rule, Train, 2, 2, true, 0, 2, 0, true, false);
	T.AssertFalse(Poor.bQueue, "tier 2, Rams full, cannot pay the Mangonel: nothing");
}
