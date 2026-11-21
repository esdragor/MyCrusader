// D804 B ("attaque ailleurs"): the hits the player's buildings took that the HUD reads (UHOLScriptScreen::ObserveHitsReceived, the rule behind
// GetHitsReceivedNow). The first observation takes the baseline; a hit later than the latest seen counts once, with its place and time; the count
// then feeds HOLAlertRules::ShouldAlertAttackedElsewhere (one line per 30 s).
// Not compiled when written (GALADRIEL-1121sc): the stub counts nothing, so these tests are red on the first commit.

void Test_HitsReceivedCountedOnceWithPlaceAndTime(FUnitTest& T)
{
	FHOLHitsWatch Watch;
	TArray<FVector2D> Places;
	TArray<float> Times;
	Places.Add(FVector2D(100.0, 200.0));
	Times.Add(5.0);
	// Baseline: a hit that was there before the first read counts for nothing.
	UHOLScriptScreen::ObserveHitsReceived(Watch, Places, Times);
	int AtBaseline = Watch.HitsReceived;
	T.AssertEquals(0, AtBaseline, "the baseline counts nothing");

	// A second building is hit later, far away.
	Places.Add(FVector2D(3000.0, 400.0));
	Times.Add(9.0);
	UHOLScriptScreen::ObserveHitsReceived(Watch, Places, Times);
	int AfterOne = Watch.HitsReceived;
	T.AssertEquals(1, AfterOne, "one new hit: the count went up by one");
	T.AssertTrue(Watch.LastHitLocation.X > 1000.0, "the place is the second building's");
	T.AssertTrue(Math::Abs(Watch.LastHitSeconds - 9.0) < 0.001, "the time is the hit's own");

	// The same readings again: nothing more.
	UHOLScriptScreen::ObserveHitsReceived(Watch, Places, Times);
	int AfterAgain = Watch.HitsReceived;
	T.AssertEquals(1, AfterAgain, "a hit is counted once");

	// Two buildings hit between two reads: both count, the latest gives the place.
	Times[0] = 10.0;
	Times[1] = 12.0;
	Places[0] = FVector2D(-500.0, 50.0);
	UHOLScriptScreen::ObserveHitsReceived(Watch, Places, Times);
	int AfterTwo = Watch.HitsReceived;
	T.AssertEquals(3, AfterTwo, "two more hits in one read: the count is 3");
	T.AssertTrue(Watch.LastHitLocation.X > 1000.0, "the place is the newest hit's (time 12)");
}

// A thing never hit (negative time) counts for nothing; the count and the alert rule meet: a hit out of view gives the line.
void Test_HitsReceivedFeedsTheAttackedElsewhereAlert(FUnitTest& T)
{
	FHOLHitsWatch Watch;
	TArray<FVector2D> Places;
	TArray<float> Times;
	Places.Add(FVector2D(100.0, 200.0));
	Times.Add(-1.0);
	UHOLScriptScreen::ObserveHitsReceived(Watch, Places, Times);
	Times[0] = 20.0;
	UHOLScriptScreen::ObserveHitsReceived(Watch, Places, Times);
	int Hits = Watch.HitsReceived;
	T.AssertEquals(1, Hits, "a building never hit, then hit: one hit");

	FHOLAttackAlertWatch Alert;
	bool bLine = false;
	if (Hits > 0)
		bLine = HOLAlertRules::ShouldAlertAttackedElsewhere(Alert, false, Watch.LastHitSeconds, HOLAlertRules::AttackRepeatSeconds);
	T.AssertTrue(bLine, "the hit counted by the accessor's rule gives the attacked elsewhere line");
}
