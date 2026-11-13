// Sacred sites, the match option, pure rules (SacredSitesInProgress.md S7, D359, D699, D702; HOLSiteOption.as). The values are the rule's
// constants, read from it (a test does not retype a value): the margin of Nomad is provisional (Retours/Q-sites-nomade-distance.md).

TArray<int> OptionList(int A, int B = -1, int C = -1)
{
	TArray<int> List;
	List.Add(A);
	if (B >= 0)
		List.Add(B);
	if (C >= 0)
		List.Add(C);
	return List;
}

// The box is a plain boolean, unticked when the configuration does not carry it (D699).
void Test_SiteOptionUntickedByDefault(FUnitTest& T)
{
	T.AssertTrue(!HOLSiteOption::DefaultSacredSites, "an old configuration without the field: no sacred sites");
}

// Without the box, the sites of the map play no part (the relays alone); with it, they do, if the map lays any.
void Test_SiteOptionOffMeansNoActiveSite(FUnitTest& T)
{
	T.AssertTrue(!HOLSiteOption::SitesActive(false, 2), "box unticked, a map with 2 sites: no site active");
	T.AssertTrue(HOLSiteOption::SitesActive(true, 2), "box ticked, 2 sites: the sites are active");
	T.AssertTrue(HOLSiteOption::SitesActive(true, 3), "box ticked, 3 sites: active");
	T.AssertTrue(!HOLSiteOption::SitesActive(true, 0), "box ticked on a map without site: nothing active");
}

// The box is greyed on a map that lays no site.
void Test_SiteOptionBoxGreyedWithoutSites(FUnitTest& T)
{
	T.AssertTrue(!HOLSiteOption::BoxEnabled(0), "a map without site: the box is greyed");
	T.AssertTrue(HOLSiteOption::BoxEnabled(2), "a map with 2 sites: the box can be ticked");
}

// Nomad (D702): no starting group in the zone of a site, and none closer than the zone plus the margin; every site counts.
void Test_SiteOptionNomadGroupKeepsAwayFromSites(FUnitTest& T)
{
	const int Radius = 1200;
	const int Least = HOLSiteOption::NomadMinDistanceCm(Radius);
	T.AssertEquals(Radius + HOLSiteOption::NomadMarginCm, Least, "the least distance is the zone plus the margin");
	T.AssertTrue(Least > Radius, "outside the zone, whatever the margin");
	T.AssertTrue(!HOLSiteOption::NomadGroupClear(true, OptionList(0), OptionList(Radius)), "a group on the centre of a site: refused");
	T.AssertTrue(!HOLSiteOption::NomadGroupClear(true, OptionList(Radius - 1), OptionList(Radius)), "a group inside the zone: refused");
	T.AssertTrue(!HOLSiteOption::NomadGroupClear(true, OptionList(Least - 1), OptionList(Radius)), "one centimetre short of the least distance: refused");
	T.AssertTrue(HOLSiteOption::NomadGroupClear(true, OptionList(Least), OptionList(Radius)), "at the least distance: accepted");
	T.AssertTrue(!HOLSiteOption::NomadGroupClear(true, OptionList(Least + 5000, Least - 1), OptionList(Radius, Radius)), "far from the first site, too close to the second: refused");
	T.AssertTrue(HOLSiteOption::NomadGroupClear(true, OptionList(Least + 5000, Least), OptionList(Radius, Radius)), "far enough from both: accepted");
	const int Wide = 2000;
	T.AssertTrue(!HOLSiteOption::NomadGroupClear(true, OptionList(Least), OptionList(Wide)), "a wider site keeps the group further");
	T.AssertTrue(HOLSiteOption::NomadGroupClear(true, OptionList(HOLSiteOption::NomadMinDistanceCm(Wide)), OptionList(Wide)), "its own least distance: accepted");
}

// Nomad without the sites active (box unticked or map without site): the sites never refuse a group.
void Test_SiteOptionNomadWithoutSitesRefusesNothing(FUnitTest& T)
{
	T.AssertTrue(HOLSiteOption::NomadGroupClear(false, OptionList(0), OptionList(1200)), "sites inactive: a group on a site's centre is accepted");
	TArray<int> None;
	T.AssertTrue(HOLSiteOption::NomadGroupClear(true, None, None), "no site at all: accepted");
}
