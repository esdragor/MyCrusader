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
	T.AssertTrue(HOLAlertRules::PopulationFullLineSeconds <= HOLAlertRules::PopulationRepeatSeconds, "the line is gone when the repeat delay lets the next one in");
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

// D833: the population full line STAYS 15 s in the thread (a display duration, not a repeat delay); the repeat delay stays the 30 s of Q-alerts-minor (default A).
// Both come from the HUD's own constants (HOLAlertRules), the repeat through the real rule.
void Test_AlertPopulationFullLineStays15s(FUnitTest& T)
{
	const float Delay = HOLAlertRules::PopulationRepeatSeconds;
	T.AssertEquals(15.0, HOLAlertRules::PopulationFullLineSeconds, "the line stays 15 s in the thread (D833)");
	T.AssertEquals(30.0, Delay, "the repeat delay between two population full lines stays 30 s (Q-alerts-minor, default A)");
	FHOLPopulationAlertWatch Watch;
	T.AssertTrue(HOLAlertRules::ShouldAlertPopulationFull(Watch, 10, 10, 0.0, Delay), "full: the first line goes");
	// The line is gone at 15 s; a place freed and taken again 16 s after the first line is still inside the 30 s delay: quiet, 31 s after it a new line goes.
	T.AssertFalse(HOLAlertRules::ShouldAlertPopulationFull(Watch, 9, 10, 5.0, Delay), "a place freed");
	T.AssertFalse(HOLAlertRules::ShouldAlertPopulationFull(Watch, 10, 10, 16.0, Delay), "full again 16 s after the first line (the line is gone): quiet, the delay is 30 s");
	T.AssertFalse(HOLAlertRules::ShouldAlertPopulationFull(Watch, 9, 10, 20.0, Delay), "a place freed again");
	T.AssertTrue(HOLAlertRules::ShouldAlertPopulationFull(Watch, 10, 10, 31.0, Delay), "full again 31 s after the first line: a new line");
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
// D804 B, the part in script: the counters the C++ accessor gives become alerts. The first reading is the baseline; a building finished is a
// building alert with its own place in time; a research is the other kind and has its own clock; a second event within 30 s says nothing.
void Test_AlertMinorStepFromCounters(FUnitTest& T)
{
	FHOLMinorWatch Watch;
	FHOLMinorStep Baseline = HOLAlertRules::MinorStep(Watch, 3, 2, 100.0);
	T.AssertFalse(Baseline.Building.bShow, "the first reading is the baseline: nothing for the 3 buildings already there");
	T.AssertFalse(Baseline.Research.bShow, "the first reading is the baseline: nothing for the 2 researches already there");
	FHOLMinorStep Built = HOLAlertRules::MinorStep(Watch, 4, 2, 101.0);
	T.AssertTrue(Built.Building.bShow, "a fourth building finished: the building line goes");
	T.AssertTrue(Built.Building.bPingMinimap, "a finished building pings the minimap");
	T.AssertFalse(Built.Research.bShow, "no research done: no research line");
	FHOLMinorStep Again = HOLAlertRules::MinorStep(Watch, 5, 2, 110.0);
	T.AssertFalse(Again.Building.bShow, "another building 9 s after the line: quiet (once per 30 s)");
	FHOLMinorStep Researched = HOLAlertRules::MinorStep(Watch, 5, 3, 112.0);
	T.AssertTrue(Researched.Research.bShow, "a research done 11 s after a building line: the research clock is its own, the line goes");
	T.AssertFalse(Researched.Building.bShow, "no new building: no building line");
	FHOLMinorStep Later = HOLAlertRules::MinorStep(Watch, 6, 3, 131.0);
	T.AssertTrue(Later.Building.bShow, "a building 30 s after the last building line: it goes again");
}

// An event dropped inside the 30 s is not kept for later: the same counter read again says nothing.
void Test_AlertMinorStepNoReplay(FUnitTest& T)
{
	FHOLMinorWatch Watch;
	HOLAlertRules::MinorStep(Watch, 0, 0, 10.0);
	FHOLMinorStep First = HOLAlertRules::MinorStep(Watch, 1, 0, 20.0);
	T.AssertTrue(First.Building.bShow, "the first building after the baseline: the line goes");
	HOLAlertRules::MinorStep(Watch, 2, 0, 25.0);
	FHOLMinorStep Same = HOLAlertRules::MinorStep(Watch, 2, 0, 60.0);
	T.AssertFalse(Same.Building.bShow, "the counter did not move since the last reading: no line, the dropped event is not replayed");
}