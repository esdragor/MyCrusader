// The terrain zones' speed rule (MapsInProgress.md C3, GDD 5.4 D33, M-Q2 = A: only the visible effects): pure functions of a list of
// zones and a settings struct, tested alone in script (HOLTerrainRules_Test.as). Whole numbers only (percent), so that the server and the
// clients agree to the centimetre. The map says where the zones are (El Dorado's marsh, C3); the walkers' step asks SpeedPercentAt where a
// unit stands and scales its speed by it. Nothing here knows about units, orders or the engine.

// The numbers of the zones, one place (C3-Q1: the strength of the marsh is not settled, two presets are ready).
struct FHOLTerrainSettings
{
	// A walker's speed in a marsh, in percent of its normal speed (GDD 5.4: "slows everybody").
	int MarshSpeedPercent = 100;

	// Whether a charge (heavy cavalry, D345.3) is stopped in a marsh (GDD 5.4). Read by the charge when it exists.
	bool bMarshStopsCharge = true;

	// A walker's speed on a road, in percent of its normal speed (C5, La Foret profonde, CQ6a = A: 130). Read as the marsh's.
	int RoadSpeedPercent = 130;

	// The ranged damage a target in a forest takes, in percent (B4, GDD 8.1 D33 "cover in a forest"; melee never changes). 100 = no cover
	// (a bare settings struct); the presets and hol.Terrain.ForestRangedPercent give the question's default (Q-forest-cover Q2 = A: 75).
	int ForestRangedDamagePercent = 100;

	// What "in a forest" means (Q-forest-cover Q1): false = inside a painted forest zone (A, default), true = near trees (B: at least
	// ForestTreeCount trees within ForestTreeRadius cm of the unit). hol.Terrain.ForestByTrees picks it.
	bool bForestByTrees = false;
	int ForestTreeRadius = 600;
	int ForestTreeCount = 3;
}

// One tree of the map, by its trunk position in centimetres (only what the forest rule needs).
struct FHOLTerrainTree
{
	int X = 0;
	int Y = 0;
}

// The forest's settings on a running game (B4): read by SettingsFromCVars, one line to change each (Q-forest-cover).
const FConsoleVariable CVar_TerrainForestRangedPercent("hol.Terrain.ForestRangedPercent", 75);
const FConsoleVariable CVar_TerrainForestByTrees("hol.Terrain.ForestByTrees", 0);

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
	// 1 road: speeds the walkers up (CQ6a = A); a marsh under it wins (the slowing zone first).
	const int Road = 1;
	// 2 forest (B4, Q-forest-cover Q1 = A): no effect on speed here, cover against ranged damage (DamagePercentAt).
	const int Forest = 2;

	// The presets: 0 the marsh slows to 60 % (the unit loses 2 cm in 5, clearly felt), 1 to 75 % (felt, a crossing is not a stop). Neither is
	// a decision: the user picks (Auto_Journey_Retours.md, C3-Q1). Both give the forest's cover its default (Q-forest-cover Q2 = A: 75 %).
	FHOLTerrainSettings Preset(int Index)
	{
		FHOLTerrainSettings Settings;
		Settings.bMarshStopsCharge = true;
		Settings.MarshSpeedPercent = (Index == 1) ? 75 : 60;
		Settings.ForestRangedDamagePercent = 75;
		return Settings;
	}

	// The settings of a running game: preset 0 with the forest read from the console variables (hol.Terrain.ForestRangedPercent,
	// hol.Terrain.ForestByTrees), so that the two answers of Q-forest-cover are one line away.
	FHOLTerrainSettings SettingsFromCVars()
	{
		FHOLTerrainSettings Settings = Preset(0);
		Settings.ForestRangedDamagePercent = CVar_TerrainForestRangedPercent.GetInt();
		Settings.bForestByTrees = CVar_TerrainForestByTrees.GetInt() != 0;
		return Settings;
	}

	// Whether a unit standing at (X, Y) is in a forest: inside a forest zone (A), or with enough trees near it (B), as the settings say.
	// RED STUB (B4 first red test): always false.
	bool InForest(const FHOLTerrainSettings& Settings, const TArray<FHOLTerrainZone>& Zones, const TArray<FHOLTerrainTree>& Trees, int X, int Y)
	{
		return false;
	}

	// The damage percent a target standing at (X, Y) takes from a hit: the forest's cover for a ranged hit, 100 for a melee hit or
	// outside a forest. Only the target's place counts (the shooter may stand anywhere).
	// RED STUB (B4 first red test): always 100.
	int DamagePercentAt(const FHOLTerrainSettings& Settings, const TArray<FHOLTerrainZone>& Zones, const TArray<FHOLTerrainTree>& Trees,
		bool bRanged, int X, int Y)
	{
		return 100;
	}

	// A hit's damage scaled by a percent, never below 1 for a hit that hurts (a covered arrow still scratches).
	int ScaledDamage(int Damage, int Percent)
	{
		if (Damage <= 0)
		{
			return 0;
		}
		int Scaled = Damage * Percent / 100;
		return Scaled < 1 ? 1 : Scaled;
	}

	// Whether the point (X, Y) is in the zone (its edge counts in: a unit on the border is slowed).
	bool Contains(const FHOLTerrainZone& Zone, int X, int Y)
	{
		int64 DX = X - Zone.X;
		int64 DY = Y - Zone.Y;
		int64 R = Zone.Radius;
		return DX * DX + DY * DY <= R * R;
	}

	// The speed, in percent of normal, of a walker standing at (X, Y): the slowest of the zones that hold the point, 100 outside all.
	int SpeedPercentAt(const FHOLTerrainSettings& Settings, const TArray<FHOLTerrainZone>& Zones, int X, int Y)
	{
		int Percent = 100;
		bool bOnRoad = false;
		for (int Index = 0; Index < Zones.Num(); Index++)
		{
			if (!Contains(Zones[Index], X, Y))
			{
				continue;
			}
			if (Zones[Index].Kind == Marsh && Settings.MarshSpeedPercent < Percent)
			{
				Percent = Settings.MarshSpeedPercent;
			}
			else if (Zones[Index].Kind == Road)
			{
				bOnRoad = true;
			}
		}
		return (Percent == 100 && bOnRoad) ? Settings.RoadSpeedPercent : Percent;
	}

	// A speed (cm/s) scaled by a percent, never below 1 cm/s for a walker that walks.
	int ScaledSpeed(int Speed, int Percent)
	{
		if (Speed <= 0)
		{
			return 0;
		}
		int Scaled = Speed * Percent / 100;
		return Scaled < 1 ? 1 : Scaled;
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
