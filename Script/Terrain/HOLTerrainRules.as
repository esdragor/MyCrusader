// The terrain rules that live only in script until their wiring (D810): the forest (F4), the ford (F5) and the charge stopped in a marsh
// (GDD 5.4, D33), pure functions of a settings struct, tested alone in script (HOLTerrainRules_Test.as). Whole numbers only (percent).
// D836 (2026-11-16): the walkers' speed rule of the marsh and the road is the C++'s alone (FHOLTerrainRules, called by the Mass step for every
// walker at every step; read from script through UHOLTerrainLibrary, HOLTerrain::Server*); its script copy and agreement tests are removed.
// Nothing here knows about units, orders or the engine.

// The numbers of the script-only rules, one place.
struct FHOLTerrainSettings
{
	// Whether a charge (heavy cavalry, D345.3) is stopped in a marsh (GDD 5.4). Read by the charge when it exists.
	bool bMarshStopsCharge = true;

	// D796 (2026-11-14): no cover in a forest any more (the pure rule of B4, its settings and console variables removed); a forest
	// blocks walkers by its trees (ForestFordInProgress.md), nothing here.

	// F4 (GDD 8.1, D33): the speed of a cavalry walker in a forest, in percent of its normal speed (Q-forest-ford Q2 = A by default: 80).
	int ForestCavalrySpeedPercent = 80;

	// F4: the speed of a siege engine in a forest, in percent (Q-forest-ford Q2 = A: 70). Units on foot keep their speed.
	int ForestSiegeSpeedPercent = 70;

	// F5 (GDD 8.1, D33): the damage a unit standing on a ford takes, in percent of the hit (its defence reduced; Q2 = A: 120).
	int FordDamageTakenPercent = 120;
}

// One round zone of the map: its centre and radius in centimetres, and its kind.
struct FHOLTerrainZone
{
	int Kind = 0;
	int X = 0;
	int Y = 0;
	int Radius = 0;
}

namespace HOLTerrain
{
	// The kinds of zone the maps can lay (by index as everywhere in script: 0 marsh).
	const int Marsh = 0;
	// 1 road: speeds the walkers up (CQ6a = A); the rule is the C++'s (D836).
	const int Road = 1;
	// (2 was the forest's cover zone, removed by D796.)

	// The walker categories the forest tells apart (F4): on foot (never slowed by a forest), cavalry, siege engines.
	const int OnFoot = 0;
	const int Cavalry = 1;
	const int Siege = 2;

	// F4: the speed, in percent of normal, of a walker of the category in or out of a forest. What "in a forest" is (painted zone or trees
	// nearby) is the caller's (Q-forest-ford Q1); the movement step scales the speed by it as by the marsh's (the C++ scaling, D836).
	int ForestSpeedPercent(const FHOLTerrainSettings& Settings, int Category, bool bInForest)
	{
		if (!bInForest)
		{
			return 100;
		}
		if (Category == Cavalry)
		{
			return Settings.ForestCavalrySpeedPercent;
		}
		if (Category == Siege)
		{
			return Settings.ForestSiegeSpeedPercent;
		}
		return 100;
	}

	// F5: the damage a unit takes from a hit, on a ford or not (its defence reduced on a ford). Never below 0.
	int FordDamageTaken(const FHOLTerrainSettings& Settings, int Damage, bool bOnFord)
	{
		if (Damage <= 0)
		{
			return 0;
		}
		return bOnFord ? Damage * Settings.FordDamageTakenPercent / 100 : Damage;
	}

	// Whether the point (X, Y) is in the zone (its edge counts in). Kept for StopsCharge only (D836).
	bool Contains(const FHOLTerrainZone& Zone, int X, int Y)
	{
		int64 DX = X - Zone.X;
		int64 DY = Y - Zone.Y;
		int64 R = Zone.Radius;
		return DX * DX + DY * DY <= R * R;
	}

	// Whether a charge is stopped where the unit stands (a marsh, when the settings say so).
	bool StopsCharge(const FHOLTerrainSettings& Settings, const TArray<FHOLTerrainZone>& Zones, int X, int Y)
	{
		if (!Settings.bMarshStopsCharge)
		{
			return false;
		}
		for (int Index = 0; Index < Zones.Num(); Index++)
		{
			if (Zones[Index].Kind == Marsh && Contains(Zones[Index], X, Y))
			{
				return true;
			}
		}
		return false;
	}
}
