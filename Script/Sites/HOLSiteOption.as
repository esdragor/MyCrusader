// Sacred sites, the match option, pure rules (SacredSitesInProgress.md S7; GDD 14.3 Sacred sites, D359, D699, D702): the box "Sacred sites"
// of the lobby is a plain boolean (D699, FHOLMatchConfig::bSacredSites to come in the C++ of S7). Without it, no site of the map is active (the
// relays alone); the box is greyed when the map lays no site; an old configuration without the field reads false. Combinations (D702):
// Regicide, Catastrophe and Explored map combine as they are; Nomad needs a rule: no starting group in the zone of a site, at a minimal
// distance. Integers only, no world access: the distances and radii are the caller's input. Tested in HOLSiteOption_Test.as.
// Provisional (Retours/Q-sites-nomade-distance.md): the margin beyond the zone of a site (NomadMarginCm), one line below.
namespace HOLSiteOption
{
	// The box when the configuration does not carry it (an old JSON, an old save): unticked.
	const bool DefaultSacredSites = true;

	// Provisional, Q-sites-nomade-distance option B (recommended, not chosen yet): beyond the zone of a site, the sight of a Peasant
	// (UnitTypes.csv Peasant SightRadius 3200 cm), so that no starting group sees a site at the start (D701: a site is visible once explored).
	// Option A = 0 (just outside the zone), option C = HOLNomad::MinEnemyDistanceCm (4000 cm).
	const int NomadMarginCm = 3200;

	// The box can be ticked: the map lays at least one site.
	bool BoxEnabled(int MapSiteCount)
	{
		return true;
	}

	// The sites of the map take part in the match: the box ticked and the map lays at least one.
	bool SitesActive(bool bSacredSites, int MapSiteCount)
	{
		return true;
	}

	// The least distance between a Nomad starting group and the centre of a site of zone RadiusCm.
	int NomadMinDistanceCm(int RadiusCm)
	{
		return 0;
	}

	// A Nomad starting group may stand here: without active sites always; with them, at least NomadMinDistanceCm from the centre of every
	// site (DistanceBySiteCm and RadiusBySiteCm site by site, the same order).
	bool NomadGroupClear(bool bSitesActive, const TArray<int>& DistanceBySiteCm, const TArray<int>& RadiusBySiteCm)
	{
		return true;
	}
}
