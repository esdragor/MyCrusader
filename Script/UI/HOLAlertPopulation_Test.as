// D202 minor alert "population full" (GLORFINDEL-1121sb, pure rule HOLAlertRules::PopulationFullStep, no C++). The stub never shows: the tests are red
// on the first commit, then green with the rule. The alert is read from the rule's own return value (the line, its seconds, its sound, no minimap ping).

// The first reading only takes the baseline (a game that starts full says nothing); then the population BECOMES full: the line goes, with the minor
// line's time, the kind's own sound and no minimap ping.
void Test_AlertPopulationFullRisingEdge(FUnitTest& T)
{
	FHOLPopulationWatch Watch;
	FHOLMinorAlert AtBaseline = HOLAlertRules::PopulationFullStep(Watch, 10, 10, 0.0);
	T.AssertFalse(AtBaseline.bShow, "the first reading is the baseline: nothing, even full");

	FHOLMinorAlert Below = HOLAlertRules::PopulationFullStep(Watch, 9, 10, 5.0);
	T.AssertFalse(Below.bShow, "9 of 10 is not full");

	FHOLMinorAlert Full = HOLAlertRules::PopulationFullStep(Watch, 10, 10, 10.0);
	T.AssertTrue(Full.bShow, "10 of 10 after 9 of 10: the line goes");
	T.AssertFalse(Full.bPingMinimap, "population full has no place: no minimap ping");
	T.AssertTrue(Full.SoundKey == n"MinorPopulationFull", "the kind's own sound key");
	float Seconds = Full.LineSeconds;
	T.AssertTrue(Seconds > 0.0, "the line stays in the thread for some seconds");
}

// Staying full says nothing more; a cap of 0 is never full; a population that falls and fills again inside 30 s is silent (one line per 30 s), after 30 s it speaks again.
void Test_AlertPopulationFullEdgeAndRepeat(FUnitTest& T)
{
	FHOLPopulationWatch Watch;
	HOLAlertRules::PopulationFullStep(Watch, 5, 10, 0.0);
	FHOLMinorAlert First = HOLAlertRules::PopulationFullStep(Watch, 10, 10, 10.0);
	T.AssertTrue(First.bShow, "first time full: the line goes");

	FHOLMinorAlert Staying = HOLAlertRules::PopulationFullStep(Watch, 10, 10, 20.0);
	T.AssertFalse(Staying.bShow, "still full: no second line");

	HOLAlertRules::PopulationFullStep(Watch, 8, 10, 25.0);
	FHOLMinorAlert TooSoon = HOLAlertRules::PopulationFullStep(Watch, 10, 10, 30.0);
	T.AssertFalse(TooSoon.bShow, "full again 20 s after the last line: silent (once per 30 s)");

	HOLAlertRules::PopulationFullStep(Watch, 8, 10, 35.0);
	FHOLMinorAlert Later = HOLAlertRules::PopulationFullStep(Watch, 10, 10, 45.0);
	T.AssertTrue(Later.bShow, "full again 35 s after the last line: the line goes");

	FHOLPopulationWatch NoHouse;
	HOLAlertRules::PopulationFullStep(NoHouse, 0, 5, 0.0);
	FHOLMinorAlert ZeroCap = HOLAlertRules::PopulationFullStep(NoHouse, 0, 0, 40.0);
	T.AssertFalse(ZeroCap.bShow, "a cap of 0 is not full");
}
