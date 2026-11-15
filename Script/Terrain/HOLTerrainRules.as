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
	// 1 road: speeds the walkers up (CQ6a = A); a marsh under it wins (the slowing zone first).
	const int Road = 1;
	// (2 was the forest's cover zone, removed by D796.)

	// The walker categories the forest tells apart (F4): on foot (never slowed by a forest), cavalry, siege engines.
	const int OnFoot = 0;
	const int Cavalry = 1;
	const int Siege = 2;

	// F4: the speed, in percent of normal, of a walker of the category in or out of a forest. What "in a forest" is (painted zone or trees
	// nearby) is the caller's (Q-forest-ford Q1); the movement step scales the speed by it (ScaledSpeed), as by the marsh's.
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

	// The presets: 0 the marsh slows to 60 % (the unit loses 2 cm in 5, clearly felt), 1 to 75 % (felt, a crossing is not a stop). Neither is
	// a decision: the user picks (Auto_Journey_Retours.md, C3-Q1).
	FHOLTerrainSettings Preset(int Index)
	{
		FHOLTerrainSettings Settings;
		Settings.bMarshStopsCharge = true;
		Settings.MarshSpeedPercent = (Index == 1) ? 75 : 60;
		return Settings;
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
