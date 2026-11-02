// V10: the volcano's lines on the HUD, from the views the library gives (pure). The alert's countdown, the lava, the altar held / contested / left, and
// nothing before the alert (a site only armed gives no warning, GDD 12.2).

FHOLVolcanoSiteView VbSite(int Phase)
{
	FHOLVolcanoSiteView Site;
	Site.Phase = Phase;
	Site.bAltarPresent = true;
	return Site;
}

void Test_VolcanoBannerSilentBeforeTheAlert(FUnitTest& T)
{
	TArray<FHOLVolcanoSiteView> Sites;
	FHOLVolcanoSiteView Armed = VbSite(1);
	Armed.bAltarLit = true;
	Sites.Add(Armed);
	const FHOLVolcanoBannerView View = HOLVolcanoBanner::Make(Sites);
	T.AssertFalse(View.bVisible, "an armed site with nobody at its altar: nothing to read");
	T.AssertEquals(View.Describe, "hidden", "line");
	T.AssertFalse(HOLVolcanoBanner::Make(TArray<FHOLVolcanoSiteView>()).bVisible, "no site, no banner");
	FHOLVolcanoSiteView Rest = VbSite(5);
	Sites[0] = Rest;
	T.AssertFalse(HOLVolcanoBanner::Make(Sites).bVisible, "a site at rest: nothing");
}

void Test_VolcanoBannerAlertCountdown(FUnitTest& T)
{
	TArray<FHOLVolcanoSiteView> Sites;
	FHOLVolcanoSiteView Warning = VbSite(2);
	Warning.SecondsToErupt = 75;
	Sites.Add(Warning);
	const FHOLVolcanoBannerView View = HOLVolcanoBanner::Make(Sites);
	T.AssertTrue(View.bVisible, "the alert is on the screen");
	T.AssertEquals(View.Describe, "volcano 1: alert 75", "line");
	T.AssertTrue(View.Text.Contains("1:15"), "the countdown is a clock (m:ss)");
	Sites[0].Phase = 3;
	T.AssertEquals(HOLVolcanoBanner::Make(Sites).Describe, "volcano 1: erupting", "line");
	Sites[0].Phase = 4;
	T.AssertEquals(HOLVolcanoBanner::Make(Sites).Describe, "volcano 1: cooling", "line");
}

void Test_VolcanoBannerAltar(FUnitTest& T)
{
	TArray<FHOLVolcanoSiteView> Sites;
	FHOLVolcanoSiteView Altar = VbSite(1);
	Altar.bAltarLit = true;
	Altar.AltarHold = 1;
	Altar.AltarPercent = 33;
	Altar.AltarSecondsLeft = 40;
	Sites.Add(Altar);
	FHOLVolcanoBannerView View = HOLVolcanoBanner::Make(Sites);
	T.AssertEquals(View.Describe, "volcano 1: altar held 33", "line");
	// The percentage is enough (user, 2026-11-02): no seconds after it. Only the alert's countdown keeps its m:ss.
	T.AssertEquals(View.Text, "Autel ancien : tenu 33 %", "the line says the share held, without a countdown in seconds");
	T.AssertFalse(View.Text.Contains(" s)"), "no seconds on the altar line");
	Sites[0].AltarHold = 2;
	T.AssertEquals(HOLVolcanoBanner::Make(Sites).Describe, "volcano 1: altar contested 33", "line");
	Sites[0].AltarHold = 0;
	T.AssertEquals(HOLVolcanoBanner::Make(Sites).Describe, "volcano 1: altar idle 33", "line");
	Sites[0].AltarPercent = 0;
	T.AssertFalse(HOLVolcanoBanner::Make(Sites).bVisible, "nobody at the altar and nothing counted: hidden");
	// Once the cycle is woken the altar is dark: only the alert is left to read.
	Sites[0].AltarHold = 1;
	Sites[0].AltarPercent = 50;
	Sites[0].bAltarLit = false;
	T.AssertFalse(HOLVolcanoBanner::Make(Sites).bVisible, "an altar no longer lit gives no line");
}

void Test_VolcanoBannerSeveralSites(FUnitTest& T)
{
	TArray<FHOLVolcanoSiteView> Sites;
	FHOLVolcanoSiteView Quiet = VbSite(5);
	Sites.Add(Quiet);
	FHOLVolcanoSiteView Warning = VbSite(2);
	Warning.SecondsToErupt = 9;
	Sites.Add(Warning);
	const FHOLVolcanoBannerView View = HOLVolcanoBanner::Make(Sites);
	T.AssertEquals(View.Describe, "volcano 2: alert 9", "line");
	T.AssertTrue(View.Text.Contains("Site 2"), "with several sites the line names the site");
}
