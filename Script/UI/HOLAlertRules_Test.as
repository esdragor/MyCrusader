// The minor alerts (D202, HOLAlertRules.as): the population full line goes once when the population becomes full, not again while it stays
// full, nor sooner than the repeat delay after a dip; again once the delay has passed.

void Test_AlertPopulationFullOnReaching(FUnitTest& T)
{
	FHOLPopulationAlertWatch Watch;
	T.AssertFalse(HOLAlertRules::ShouldAlertPopulationFull(Watch, 8, 10, 0.0, 30.0), "8 / 10: room left, no line");
	T.AssertTrue(HOLAlertRules::ShouldAlertPopulationFull(Watch, 10, 10, 5.0, 30.0), "10 / 10: the population has become full, the line goes");
	T.AssertEquals(HOLAlertRules::PopulationFullText().ToString(), "Population pleine", "the line's text, the queue note's key");
	T.AssertFalse(HOLAlertRules::ShouldAlertPopulationFull(Watch, 10, 10, 50.0, 30.0), "still full later: no second line");
	T.AssertFalse(HOLAlertRules::ShouldAlertPopulationFull(Watch, 0, 0, 51.0, 30.0), "no place at all (no building yet): not full, no line");
}

void Test_AlertPopulationFullRepeat(FUnitTest& T)
{
	FHOLPopulationAlertWatch Watch;
	T.AssertTrue(HOLAlertRules::ShouldAlertPopulationFull(Watch, 10, 10, 0.0, 30.0), "full at once: the line goes");
	// A unit dies, another is queued: 9 then 10 within the delay, no new line.
	T.AssertFalse(HOLAlertRules::ShouldAlertPopulationFull(Watch, 9, 10, 4.0, 30.0), "a place freed: no line");
	T.AssertFalse(HOLAlertRules::ShouldAlertPopulationFull(Watch, 10, 10, 6.0, 30.0), "full again 6 s after the line: kept quiet");
	// A house raises the cap; full again once the delay has passed: a new line.
	T.AssertFalse(HOLAlertRules::ShouldAlertPopulationFull(Watch, 10, 20, 10.0, 30.0), "a house built: room again");
	T.AssertTrue(HOLAlertRules::ShouldAlertPopulationFull(Watch, 20, 20, 40.0, 30.0), "full again 40 s after the line: a new line");
	// The delay is the caller's: a shorter one lets the next line come sooner.
	FHOLPopulationAlertWatch Quick;
	T.AssertTrue(HOLAlertRules::ShouldAlertPopulationFull(Quick, 10, 10, 0.0, 2.0), "first line");
	T.AssertFalse(HOLAlertRules::ShouldAlertPopulationFull(Quick, 9, 10, 1.0, 2.0), "dip");
	T.AssertTrue(HOLAlertRules::ShouldAlertPopulationFull(Quick, 10, 10, 3.0, 2.0), "full again 3 s later with a 2 s delay: a new line");
	T.AssertTrue(HOLAlertRules::PopulationRepeatSeconds >= 2.0, "the default delay (Q-alerts-minor) is a real delay");
}

void Test_AlertAttackedElsewhereOncePer30s(FUnitTest& T)
{
	FHOLAttackAlertWatch Watch;
	T.AssertTrue(HOLAlertRules::ShouldAlertAttackedElsewhere(Watch, false, 100.0, HOLAlertRules::AttackRepeatSeconds), "a hit out of view: the line goes");
	T.AssertFalse(HOLAlertRules::ShouldAlertAttackedElsewhere(Watch, false, 110.0, HOLAlertRules::AttackRepeatSeconds), "another hit 10 s later: quiet");
	T.AssertFalse(HOLAlertRules::ShouldAlertAttackedElsewhere(Watch, false, 129.0, HOLAlertRules::AttackRepeatSeconds), "29 s after the line: still quiet");
	T.AssertTrue(HOLAlertRules::ShouldAlertAttackedElsewhere(Watch, false, 130.0, HOLAlertRules::AttackRepeatSeconds), "30 s after the line: the line goes again");
}

void Test_AlertAttackedInViewQuiet(FUnitTest& T)
{
	FHOLAttackAlertWatch Watch;
	T.AssertFalse(HOLAlertRules::ShouldAlertAttackedElsewhere(Watch, true, 100.0, 30.0), "a hit in view: the player sees it, no line");
	T.AssertTrue(HOLAlertRules::ShouldAlertAttackedElsewhere(Watch, false, 101.0, 30.0), "a hit out of view right after: the line goes (the hit in view did not count)");
}