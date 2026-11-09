// S10: the sacred sites' victory count on the HUD (banner "Victoire sacrée", announcements at 60 s and 10 s), from the numbers the game state
// replicates (HOLSite::GetVictoryCount: leader, seconds left, frozen, winner). Pure, like HOLVolcanoBanner.

void Test_SiteBannerSilentWithoutLeader(FUnitTest& T)
{
	const FHOLSiteBannerView View = HOLSiteBanner::Make(-1, -1, false, -1, "Alice");
	T.AssertFalse(View.bVisible, "nobody holds every site: nothing to read");
	T.AssertEquals(View.Describe, "hidden", "line");
}

void Test_SiteBannerCountdownIsAClock(FUnitTest& T)
{
	const FHOLSiteBannerView View = HOLSiteBanner::Make(0, 125, false, -1, "Alice");
	T.AssertTrue(View.bVisible, "a camp holds every site: the banner is on the screen");
	T.AssertEquals(View.Describe, "sacred leader 0 left 125", "line");
	T.AssertTrue(View.Text.Contains("2:05"), "the countdown is a clock (m:ss)");
	T.AssertTrue(View.Text.Contains("Alice"), "the camp in the lead is named");
}

void Test_SiteBannerFrozenAndWon(FUnitTest& T)
{
	const FHOLSiteBannerView Frozen = HOLSiteBanner::Make(1, 90, true, -1, "Bob");
	T.AssertEquals(Frozen.Describe, "sacred leader 1 left 90 frozen", "line");
	T.AssertFalse(Frozen.Text == HOLSiteBanner::Make(1, 90, false, -1, "Bob").Text, "a frozen count says so");
	const FHOLSiteBannerView Won = HOLSiteBanner::Make(1, 0, false, 1, "Bob");
	T.AssertTrue(Won.bVisible, "the victory stays on the screen");
	T.AssertEquals(Won.Describe, "sacred won 1", "line");
	T.AssertFalse(Won.Text.Contains("0:00"), "won: no countdown any more");
}

void Test_SiteBannerAnnouncesAt60And10(FUnitTest& T)
{
	T.AssertEquals(HOLSiteBanner::Announce(61, 60), 60, "crossing 60 s");
	T.AssertEquals(HOLSiteBanner::Announce(70, 50), 60, "a jump over 60 s is still announced once");
	T.AssertEquals(HOLSiteBanner::Announce(11, 10), 10, "crossing 10 s");
	T.AssertEquals(HOLSiteBanner::Announce(30, 29), 0, "an ordinary second says nothing");
	T.AssertEquals(HOLSiteBanner::Announce(60, 59), 0, "60 s is announced once, not again");
	T.AssertEquals(HOLSiteBanner::Announce(-1, 50), 0, "a count that just appeared (no previous number) says nothing");
	T.AssertEquals(HOLSiteBanner::Announce(5, 0), 0, "the win is the banner's, not an announcement");
	T.AssertEquals(HOLSiteBanner::Announce(600, 599), 0, "the start of the count says nothing");
}
