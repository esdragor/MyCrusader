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

// D833: the population full line lasts 15 s in the thread (the call: AddThreadLine(Line, HOLAlertRules::PopulationFullLineSeconds)).
// The line is visible, and gone before the next one can come (the repeat delay), so two lines never stack.
void Test_AlertPopulationFullLineDuration(FUnitTest& T)
{
	T.AssertTrue(HOLAlertRules::PopulationFullLineSeconds > 10.0, f"the line stays longer than a glance ({HOLAlertRules::PopulationFullLineSeconds} s)");
	T.AssertTrue(HOLAlertRules::PopulationFullLineSeconds < HOLAlertRules::PopulationRepeatSeconds, "the line is gone before the repeat delay lets the next one in");
	FHOLPopulationAlertWatch Watch;
	T.AssertTrue(HOLAlertRules::ShouldAlertPopulationFull(Watch, 10, 10, 0.0, HOLAlertRules::PopulationRepeatSeconds), "the line goes when the population becomes full");
	T.AssertFalse(HOLAlertRules::ShouldAlertPopulationFull(Watch, 10, 10, HOLAlertRules::PopulationFullLineSeconds, HOLAlertRules::PopulationRepeatSeconds), "still full when the line leaves: nothing new");
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

// D804 B: the first research done (nothing shown before) goes with its line, its own sound and a minimap ping; the effect is what MinorAlert
// computes from the kind and the two times.
void Test_AlertMinorFirstGoes(FUnitTest& T)
{
	FHOLMinorAlert Research = HOLAlertRules::MinorAlert(EHOLMinorAlertKind::ResearchDone, 100.0, -1000.0);
	T.AssertTrue(Research.bShow, "a research done with nothing shown before: the line goes");
	T.AssertTrue(Research.LineSeconds > 0.0, f"the line stays some time ({Research.LineSeconds} s)");
	T.AssertTrue(Research.bPingMinimap, "a research done pings the minimap");
	T.AssertTrue(Research.SoundKey != NAME_None, "a research done has a sound");
	FHOLMinorAlert Building = HOLAlertRules::MinorAlert(EHOLMinorAlertKind::BuildingDone, 100.0, -1000.0);
	T.AssertTrue(Building.bShow, "a building finished with nothing shown before: the line goes");
	T.AssertTrue(Building.bPingMinimap, "a building finished pings the minimap");
	T.AssertTrue(Building.SoundKey != NAME_None, "a building finished has a sound");
	T.AssertTrue(Building.SoundKey != Research.SoundKey, "each type has its own sound (GDD 15)");
}

// D804 B: once per 30 s per kind: 29 s after the last line nothing, 30 s after it goes again.
void Test_AlertMinorOncePer30s(FUnitTest& T)
{
	FHOLMinorAlert Soon = HOLAlertRules::MinorAlert(EHOLMinorAlertKind::BuildingDone, 129.0, 100.0);
	T.AssertFalse(Soon.bShow, "29 s after the last building line: quiet");
	T.AssertFalse(Soon.bPingMinimap, "a quiet alert pings nothing");
	T.AssertTrue(Soon.SoundKey == NAME_None, "a quiet alert plays nothing");
	FHOLMinorAlert Later = HOLAlertRules::MinorAlert(EHOLMinorAlertKind::BuildingDone, 130.0, 100.0);
	T.AssertTrue(Later.bShow, "30 s after the last building line: it goes again");
	// The delay is the same for the research kind and each kind keeps its own clock: the caller passes the kind's last time.
	FHOLMinorAlert OtherKind = HOLAlertRules::MinorAlert(EHOLMinorAlertKind::ResearchDone, 105.0, -1000.0);
	T.AssertTrue(OtherKind.bShow, "a research done 5 s after a building line goes: the kinds do not share the delay");
}