// The storm's rules, alone (WorldEventsInProgress.md E1, GDD 12.3 D209, 12.2 bis D28): a zone, a warning then an active time, a slower walk
// and a shorter shot inside, and NO sight taken away. Settings read from the preset (never its numbers retyped in the expectations).

FHOLStorm StormAt(int X, int Y, int Radius, int Start)
{
	FHOLStorm Storm;
	Storm.X = X;
	Storm.Y = Y;
	Storm.Radius = Radius;
	Storm.StartSeconds = Start;
	return Storm;
}

// D28: the warning is seen by everybody 60 to 90 s ahead, for every event. Before the start nothing is announced.
void Test_StormWarningIsSixtyToNinety(FUnitTest& T)
{
	FHOLStormSettings S = HOLStorm::Preset(0);
	T.AssertTrue(S.WarningSeconds >= 60, "the warning lasts at least 60 s (D28)");
	T.AssertTrue(S.WarningSeconds <= 90, "the warning lasts at most 90 s (D28)");
	T.AssertTrue(S.DurationSeconds > 0, "the storm lasts");
	FHOLStorm Storm = StormAt(0, 0, 1000, 600);
	T.AssertEquals(HOLStorm::Dormant, HOLStorm::PhaseAt(S, Storm, 599), "one second before the start: nothing announced");
	T.AssertEquals(HOLStorm::Warning, HOLStorm::PhaseAt(S, Storm, 600), "at the start: the warning");
	T.AssertEquals(HOLStorm::Warning, HOLStorm::PhaseAt(S, Storm, 600 + S.WarningSeconds - 1), "the last second of the warning");
	T.AssertEquals(HOLStorm::Active, HOLStorm::PhaseAt(S, Storm, 600 + S.WarningSeconds), "then the storm");
	T.AssertEquals(HOLStorm::Active, HOLStorm::PhaseAt(S, Storm, 600 + S.WarningSeconds + S.DurationSeconds - 1), "its last second");
	T.AssertEquals(HOLStorm::Over, HOLStorm::PhaseAt(S, Storm, 600 + S.WarningSeconds + S.DurationSeconds), "then it is over");
}

// The effect: inside an active storm a walker is slower and a shooter's reach shorter; the warning and the end change nothing; outside nothing.
void Test_StormSlowsWalkAndShot(FUnitTest& T)
{
	FHOLStormSettings S = HOLStorm::Preset(0);
	T.AssertTrue(S.SpeedPercent < 100 && S.SpeedPercent > 0, "the storm slows without stopping");
	T.AssertTrue(S.RangePercent < 100 && S.RangePercent > 0, "the storm shortens the shot without removing it");
	TArray<FHOLStorm> Storms;
	Storms.Add(StormAt(1000, 2000, 500, 600));
	int Active = 600 + S.WarningSeconds + 5;
	T.AssertEquals(S.SpeedPercent, HOLStorm::SpeedPercentAt(S, Storms, Active, 1000, 2000), "walking in the storm");
	T.AssertEquals(S.SpeedPercent, HOLStorm::SpeedPercentAt(S, Storms, Active, 1500, 2000), "on the edge: in");
	T.AssertEquals(100, HOLStorm::SpeedPercentAt(S, Storms, Active, 1501, 2000), "one centimetre outside");
	T.AssertEquals(100, HOLStorm::SpeedPercentAt(S, Storms, 600 + 1, 1000, 2000), "during the warning nothing changes yet");
	T.AssertEquals(100, HOLStorm::SpeedPercentAt(S, Storms, 600 + S.WarningSeconds + S.DurationSeconds, 1000, 2000), "once over, nothing changes");
	T.AssertEquals(S.RangePercent, HOLStorm::RangePercentAt(S, Storms, Active, 1000, 2000), "the shooter's reach in the storm");
	T.AssertEquals(100, HOLStorm::RangePercentAt(S, Storms, Active, -3000, 0), "far from the storm: the whole reach");
	// A bow of 3000 cm shoots shorter, never to 0; a walker of 400 cm/s walks slower, never to 0.
	int Reach = HOLStorm::ScaledValue(3000, S.RangePercent);
	T.AssertTrue(Reach < 3000 && Reach > 0, "the scaled reach is shorter and not nil");
	int Speed = HOLStorm::ScaledValue(400, S.SpeedPercent);
	T.AssertTrue(Speed < 400 && Speed > 0, "the scaled speed is lower and not nil");
	T.AssertEquals(0, HOLStorm::ScaledValue(0, S.SpeedPercent), "a unit that does not walk stays still");
}

// D209: "aucune vision retirée": the sight a unit sees with is the same in and out of the storm, at every phase.
void Test_StormTakesNoSight(FUnitTest& T)
{
	FHOLStormSettings S = HOLStorm::Preset(0);
	TArray<FHOLStorm> Storms;
	Storms.Add(StormAt(0, 0, 800, 100));
	int Active = 100 + S.WarningSeconds + 1;
	T.AssertEquals(2500, HOLStorm::VisionAt(S, Storms, Active, 0, 0, 2500), "the sight inside an active storm");
	T.AssertEquals(2500, HOLStorm::VisionAt(S, Storms, 50, 0, 0, 2500), "before the storm");
	T.AssertEquals(2500, HOLStorm::VisionAt(S, Storms, Active, 5000, 0, 2500), "outside the storm");
}

// Two storms over one place: the strongest effect once, not a product (as the terrain's zones).
void Test_StormOverlapIsNotAProduct(FUnitTest& T)
{
	FHOLStormSettings S = HOLStorm::Preset(0);
	TArray<FHOLStorm> Storms;
	Storms.Add(StormAt(0, 0, 800, 100));
	Storms.Add(StormAt(300, 0, 800, 100));
	int Active = 100 + S.WarningSeconds + 1;
	T.AssertEquals(S.SpeedPercent, HOLStorm::SpeedPercentAt(S, Storms, Active, 200, 0), "in both storms: slowed once");
	T.AssertEquals(S.RangePercent, HOLStorm::RangePercentAt(S, Storms, Active, 200, 0), "in both storms: shortened once");
}

// The two presets differ (the user picks one) and both keep the D28 warning and a real effect.
void Test_StormPresetsDiffer(FUnitTest& T)
{
	FHOLStormSettings A = HOLStorm::Preset(0);
	FHOLStormSettings B = HOLStorm::Preset(1);
	T.AssertTrue(A.SpeedPercent != B.SpeedPercent || A.RangePercent != B.RangePercent, "the presets are two strengths");
	T.AssertTrue(B.WarningSeconds >= 60 && B.WarningSeconds <= 90, "the second preset keeps the 60 to 90 s warning");
	T.AssertTrue(B.SpeedPercent < 100 && B.RangePercent < 100, "the second preset still slows and shortens");
}
