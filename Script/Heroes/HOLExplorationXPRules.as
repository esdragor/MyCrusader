// The hero's exploration XP (GDD 9.4 "discovery of zones, of important places", D05, D06; ExplorationXPInProgress.md): pure functions of
// a settings struct and one side's state, tested alone in script (HOLExplorationXPRules_Test.as). The map's fog grid (4 m cells, FogInProgress.md)
// is cut into square tranches; the first explored cell of a tranche pays once, and a place (relay, neutral camp, altar) pays once when its cell
// is explored. D06: exploration by the units keeps paying while the hero is dead. Not wired yet: the server's fog will call OnCellExplored for
// each cell a side newly explores (C++, later). Nothing here knows about units, the fog or the engine.

// The numbers, one place (Retours\Q-exploration-xp.md: carrier and value not settled, three presets ready).
struct FHOLExplorationXPSettings
{
	// The side of a tranche, in fog cells (16 = 64 m).
	int CellsPerTranche = 16;

	// The XP a tranche pays, once, at its first explored cell (0 = the surface pays nothing).
	int XPPerTranche = 3;

	// The XP a place pays, once, when its cell is explored (0 = the places pay nothing).
	int XPPerPlace = 25;
}

// One side's memory of what already paid.
struct FHOLExplorationXPState
{
	// Fog cells per side of the map (110 to 220, D490).
	int GridCells = 0;
	// Tranches per side of the map (the last one may be partial).
	int TranchesPerSide = 0;
	// Per tranche (index Y * TranchesPerSide + X): it already paid or was explored at the start.
	TArray<bool> TrancheDone;
	// Per place (same order as the places' list): it already paid or was explored at the start.
	TArray<bool> PlaceDone;
}

// A place worth discovering (relay, neutral camp, altar): the fog cell it stands on.
struct FHOLExplorationPlace
{
	int CellX = 0;
	int CellY = 0;
}

namespace HOLExplorationXP
{
	// The presets (Q-exploration-xp.md): 0 = C both (default, recommended), 1 = A surface only, 2 = B places only.
	FHOLExplorationXPSettings Preset(int Index)
	{
		FHOLExplorationXPSettings Settings;
		Settings.CellsPerTranche = 16;
		if (Index == 1)
		{
			Settings.XPPerTranche = 6;
			Settings.XPPerPlace = 0;
		}
		else if (Index == 2)
		{
			Settings.XPPerTranche = 0;
			Settings.XPPerPlace = 40;
		}
		else
		{
			Settings.XPPerTranche = 3;
			Settings.XPPerPlace = 25;
		}
		return Settings;
	}

	// A side's fresh state for a GridCells x GridCells fog grid and NumPlaces places: nothing paid yet.
	FHOLExplorationXPState NewState(const FHOLExplorationXPSettings& Settings, int GridCells, int NumPlaces)
	{
		FHOLExplorationXPState State;
		int Side = Math::Max(1, Settings.CellsPerTranche);
		State.GridCells = Math::Max(0, GridCells);
		State.TranchesPerSide = (State.GridCells + Side - 1) / Side;
		for (int I = 0; I < State.TranchesPerSide * State.TranchesPerSide; I++)
			State.TrancheDone.Add(false);
		for (int I = 0; I < NumPlaces; I++)
			State.PlaceDone.Add(false);
		return State;
	}

	// Whether the cell is on the side's grid.
	bool InGrid(const FHOLExplorationXPState& State, int CellX, int CellY)
	{
		return CellX >= 0 && CellY >= 0 && CellX < State.GridCells && CellY < State.GridCells;
	}

	// The index of the cell's tranche in State.TrancheDone (the cell must be in the grid).
	int TrancheIndex(const FHOLExplorationXPSettings& Settings, const FHOLExplorationXPState& State, int CellX, int CellY)
	{
		int Side = Math::Max(1, Settings.CellsPerTranche);
		return (CellY / Side) * State.TranchesPerSide + (CellX / Side);
	}

	// Marks the cell's tranche and places done; the XP they pay, those not done yet.
	int MarkCell(const FHOLExplorationXPSettings& Settings, FHOLExplorationXPState& State, const TArray<FHOLExplorationPlace>& Places, int CellX, int CellY)
	{
		if (!InGrid(State, CellX, CellY))
			return 0;
		int XP = 0;
		int Tranche = TrancheIndex(Settings, State, CellX, CellY);
		if (!State.TrancheDone[Tranche])
		{
			State.TrancheDone[Tranche] = true;
			XP += Settings.XPPerTranche;
		}
		for (int I = 0; I < Places.Num() && I < State.PlaceDone.Num(); I++)
		{
			if (State.PlaceDone[I] || Places[I].CellX != CellX || Places[I].CellY != CellY)
				continue;
			State.PlaceDone[I] = true;
			XP += Settings.XPPerPlace;
		}
		return XP;
	}

	// A cell explored before the match starts (the bases' first sight, "Explored map" mode): it pays nothing, ever.
	void MarkExploredAtStart(FHOLExplorationXPState& State, const FHOLExplorationXPSettings& Settings, const TArray<FHOLExplorationPlace>& Places, int CellX, int CellY)
	{
		MarkCell(Settings, State, Places, CellX, CellY);
	}

	// The XP a side's hero gets when the side newly explores the cell (CellX, CellY): its tranche's and its places' first time, 0 otherwise.
	// bHeroDead is not read on purpose: D06, exploration keeps paying while the hero is recovering.
	int OnCellExplored(const FHOLExplorationXPSettings& Settings, FHOLExplorationXPState& State, const TArray<FHOLExplorationPlace>& Places, int CellX, int CellY, bool bHeroDead)
	{
		return MarkCell(Settings, State, Places, CellX, CellY);
	}
}
