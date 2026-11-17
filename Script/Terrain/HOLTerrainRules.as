// The terrain rules of the forest (F4), the ford (F5) and the charge stopped in a marsh (GDD 5.4, 8.1, D33; D810), as the script reads them.
// D836 (2026-11-17): the rule has one source, the C++'s (FHOLTerrainRules, called by the Mass step for every walker at every step); this
// file holds NO second copy: its numbers and its functions are wrappers of UHOLTerrainReadLibrary (HOLTerrainRead:: here) and of the older
// UHOLTerrainLibrary (HOLTerrain::Server*). Pure functions of a settings struct, tested alone in script (HOLTerrainRules_Test.as).
// Nothing here knows about units, orders or the engine. Whole numbers only (percent).

// The knobs of the rules, one place: each starts at the C++'s default (never a number retyped here), a test or a caller may change one.
struct FHOLTerrainSettings
{
	// Whether a charge (heavy cavalry, D345.3) is stopped in a marsh (GDD 5.4). Read by the charge when it exists.
	bool bMarshStopsCharge = true;

	// F4 (GDD 8.1, D33; D844: 100, the forest slows nobody): the speed of a cavalry walker in a forest, in percent of its normal speed.
	int ForestCavalrySpeedPercent = HOLTerrainRead::GetDefaultForestCavalrySpeedPercent();

	// F4 (D844: 100): the speed of a siege engine in a forest, in percent. Units on foot keep their speed.
	int ForestSiegeSpeedPercent = HOLTerrainRead::GetDefaultForestSiegeSpeedPercent();

	// F5 (GDD 8.1, D33): the damage a unit standing on a ford takes, in percent of the hit (its defence reduced).
	int FordDamageTakenPercent = HOLTerrainRead::GetDefaultFordDamageTakenPercent();
}

// One round zone of the map: its centre and radius in centimetres, and its kind (HOLTerrain::Marsh, Road, Forest, Ford).
struct FHOLTerrainZone
{
	int Kind = 0;
	int X = 0;
	int Y = 0;
	int Radius = 0;
}

namespace HOLTerrain
{
	// The kinds of zone the maps lay and the walker categories, all read from the C++ (HOLTerrainKind, HOLTerrainWalker).
	const int Marsh = HOLTerrainRead::GetMarshKind();
	const int Road = HOLTerrainRead::GetRoadKind();
	const int Forest = HOLTerrainRead::GetForestKind();
	const int Ford = HOLTerrainRead::GetFordKind();
	const int OnFoot = HOLTerrainRead::GetWalkerOnFoot();
	const int Cavalry = HOLTerrainRead::GetWalkerCavalry();
	const int Siege = HOLTerrainRead::GetWalkerSiege();

	// The zones as the library takes them: a flat list of quadruples (Kind, X, Y, Radius). Packing only, no rule.
	TArray<int> ZoneQuadruples(const TArray<FHOLTerrainZone>& Zones)
	{
		TArray<int> Quadruples;
		for (int Index = 0; Index < Zones.Num(); Index++)
		{
			Quadruples.Add(Zones[Index].Kind);
			Quadruples.Add(Zones[Index].X);
			Quadruples.Add(Zones[Index].Y);
			Quadruples.Add(Zones[Index].Radius);
		}
		return Quadruples;
	}

	// F4: the speed, in percent of normal, of a walker of the category in or out of a forest (the C++ rule).
	int ForestSpeedPercent(const FHOLTerrainSettings& Settings, int Category, bool bInForest)
	{
		return HOLTerrainRead::GetForestSpeedPercent(Category, bInForest, Settings.ForestCavalrySpeedPercent, Settings.ForestSiegeSpeedPercent);
	}

	// D810: the speed percent of a walker of the category at (X, Y), all zones together (marsh, road, forest; the C++ rule).
	int WalkerSpeedPercentAt(const TArray<FHOLTerrainZone>& Zones, int Category, int X, int Y)
	{
		return HOLTerrainRead::GetWalkerSpeedPercentAt(HOLTerrainRead::GetDefaultMarshSpeedPercent(), ZoneQuadruples(Zones), Category, X, Y, HOLTerrainRead::GetDefaultRoadSpeedPercent());
	}

	// Whether the point is in at least one zone of the kind (the C++ rule; the zone's edge counts in).
	bool IsInZoneOfKind(const TArray<FHOLTerrainZone>& Zones, int Kind, int X, int Y)
	{
		return HOLTerrainRead::IsInZoneOfKind(ZoneQuadruples(Zones), Kind, X, Y);
	}

	// F5: the damage a unit takes from a hit, on a ford or not (the C++ rule). Never below 0.
	int FordDamageTaken(const FHOLTerrainSettings& Settings, int Damage, bool bOnFord)
	{
		return HOLTerrainRead::GetFordDamageTaken(Damage, bOnFord, Settings.FordDamageTakenPercent);
	}

	// Whether a charge is stopped where the unit stands (a marsh, when the settings say so; the C++ rule).
	bool StopsCharge(const FHOLTerrainSettings& Settings, const TArray<FHOLTerrainZone>& Zones, int X, int Y)
	{
		return HOLTerrainRead::StopsCharge(Settings.bMarshStopsCharge, ZoneQuadruples(Zones), X, Y);
	}
}
