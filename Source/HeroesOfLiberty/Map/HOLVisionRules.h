// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/SparseArray.h"

/**
 * F1 (FogInProgress.md, D381-D383, D518-D526): the fog of war's rules, pure: no world, no Mass. The solo game and the
 * server call them on the simulated units, a client on what it receives (D525): one rule, two callers.
 */

/** What a camp knows of a cell (D381, D382). */
enum class EHOLCellVision : uint8
{
	/** Never seen: black (D381). */
	Unexplored,
	/** Seen once, out of sight now: grey, its last look remembered (D382). */
	Explored,
	/** In sight of an observer of the camp. */
	Visible,
};

/**
 * The fog's grid (D344): square cells of CellSize (one tile, 4 m), Side a side, centred on the world's origin as the map
 * (AHOLMapBounds). A point out of the grid has no cell.
 */
struct HEROESOFLIBERTY_API FHOLVisionGrid
{
	int32 Side = 0;
	double CellSize = 400.;

	/** Side x Side cells of CellSize centred on the origin. */
	static FHOLVisionGrid MakeCentered(int32 InSide, double InCellSize);

	int32 Num() const { return Side * Side; }
	bool IsInside(const FIntPoint& Cell) const { return Cell.X >= 0 && Cell.Y >= 0 && Cell.X < Side && Cell.Y < Side; }
	int32 ToIndex(const FIntPoint& Cell) const { return Cell.Y * Side + Cell.X; }
	FIntPoint ToCell(int32 Index) const { return FIntPoint(Index % Side, Index / Side); }

	/** The cell holding a point of the ground (X, Y); outside the grid: a cell IsInside refuses. */
	FIntPoint ToCell(const FVector2D& Point) const;

	/** The centre of a cell, world space. */
	FVector2D GetCenter(const FIntPoint& Cell) const;
};

/** The values of the rules (provisional, D523: to tune in test). */
struct HEROESOFLIBERTY_API FHOLVisionTuning
{
	/**
	 * D383: a unit standing higher sees further, a moderate bonus: HeightBonusPerCm cm of sight for each cm above the
	 * ground around it, at most MaxHeightBonus (provisional: 1 for 1, two tiles at most). The ground is flat today (0).
	 */
	double HeightBonusPerCm = 1.;
	double MaxHeightBonus = 800.;

	/**
	 * F2 (D383): standing trees (a source type that BlocksSight) that make a cell forest (provisional: the woods are planted
	 * 2.2 m apart, 3 or 4 trees a cell; the grove's 7 m apart, one at most: not a forest).
	 */
	int32 ForestTrees = 2;
};

/** F3.2 (D523): what a building is, for its sight. */
enum class EHOLBuildingSight : uint8
{
	/** Finished (a main forum standing): its type's sight. */
	Standing,
	/** A site (a foundation, under construction, a forum rising again): half of it. */
	Site,
	/** A main forum in ruin: none; what it saw stays explored. */
	Rubble,
};

/**
 * F4 (D382, D520): what a camp remembers of a building or a source it saw, as at its last look: shown in the fog (grey),
 * sometimes already gone. Never a unit.
 */
struct FHOLGhost
{
	/** HOLVisionRules::MakeGhostKey: a building's or a source's, never both. */
	uint64 Key = 0;
	FName TypeName;
	/** The building's owner slot; FHOLSource::NoOwner (0xFF) for a source. */
	uint8 Owner = 0xFF;
	FVector2D Position = FVector2D::ZeroVector;
	/** A building: standing, a site, in ruin. */
	EHOLBuildingSight State = EHOLBuildingSight::Standing;
	/** A source: what it held. */
	int32 Remaining = 0;

	bool operator==(const FHOLGhost& Other) const
	{
		return Key == Other.Key && TypeName == Other.TypeName && Owner == Other.Owner && Position == Other.Position && State == Other.State
			&& Remaining == Other.Remaining;
	}
};

namespace HOLVisionRules
{
	/** F6a: every camp (a spectator's sight, D376). */
	inline constexpr uint32 AllCamps = MAX_uint32;

	/**
	 * F6a (D518, D376): the camps (a bit per camp of FHOLVisionMap) whose sight a connection receives: a player its own
	 * camp; a spectator (Camp INDEX_NONE) every camp; a slot defeated its former team's (bTeamCamp: its slot had a team)
	 * or, alone, every camp.
	 */
	HEROESOFLIBERTY_API uint32 GetReceivedCamps(int32 Camp, bool bDefeated, bool bTeamCamp);

	/** F6a: a connection receiving ReceivedCamps gets an object seen by ObjectCamps (a bit per camp). */
	inline bool Receives(const uint32 ReceivedCamps, const uint32 ObjectCamps) { return (ReceivedCamps & ObjectCamps) != 0; }

	/** F4: a ghost's key, a building's (its actor's unique identifier) or a source's (its index), apart. */
	inline uint64 MakeGhostKey(const bool bSource, const uint32 Id) { return (static_cast<uint64>(bSource ? 1 : 0) << 32) | Id; }

	/**
	 * F4 (D382): one step of a camp's memory. Present: the buildings and sources standing now, as they are. Each one in
	 * sight (IsVisible at its position) is remembered as it is; out of sight its ghost keeps its last look (a fire, a ruin,
	 * a tree cut: not learned). A ghost whose thing is gone is forgotten only once its place is in sight again (D382: a
	 * destruction is learned by going back). Returns true when the memory changed.
	 */
	HEROESOFLIBERTY_API bool UpdateGhosts(TMap<uint64, FHOLGhost>& Ghosts, TConstArrayView<FHOLGhost> Present, TFunctionRef<bool(const FVector2D&)> IsVisible);

	/** F3.2 (D523): a building's sight from its type's SightRadius: whole standing, half a site, none in ruin. */
	HEROESOFLIBERTY_API double GetBuildingSight(double SightRadius, EHOLBuildingSight State);

	/** D383: an observer's sight (cm): its type's SightRadius plus the height bonus of Height cm above the ground (never below 0). */
	HEROESOFLIBERTY_API double GetSightRadius(double BaseRadius, double Height, const FHOLVisionTuning& Tuning);

	/**
	 * The cells of the disc of an observer standing in Cell with a sight of Radius cm: every cell whose centre lies within
	 * Radius of the centre of Cell, inside the grid, in row order. No blocker.
	 */
	HEROESOFLIBERTY_API TArray<FIntPoint> GetDisc(const FHOLVisionGrid& Grid, const FIntPoint& Cell, double Radius);

	/**
	 * D383 (D33): whether an observer in From sees To through the forest. The line from From to To (Bresenham, from the
	 * observer: the same cells on the server and the client) is cut by a forest cell strictly between them: the edge of a
	 * forest is seen, not what stands behind it. An observer in a forest sees out of it: the forest cells next to it on
	 * the line, up to the first open cell, do not cut (the woodcutter is not blind). Blocked: one per cell of the grid.
	 */
	HEROESOFLIBERTY_API bool IsInLineOfSight(const FHOLVisionGrid& Grid, TConstArrayView<bool> Blocked, const FIntPoint& From, const FIntPoint& To);

	/** The disc of GetDisc kept to the cells in line of sight (IsInLineOfSight). */
	HEROESOFLIBERTY_API TArray<FIntPoint> GetSeenCells(const FHOLVisionGrid& Grid, TConstArrayView<bool> Blocked, const FIntPoint& Cell, double Radius);
}

/**
 * The fog of every camp (a camp: a team, or a slot alone, HOLVictoryRules::GetCamp): a count per cell and camp of the
 * observers seeing it, and whether the camp ever saw it (D521: an observer costs nothing while it stays in its cell; a
 * move to another cell takes its seen cells off and lays its new ones). The cells an observer counts are kept with it, so
 * it takes off exactly what it laid even if the forest changed meanwhile (a tree cut: SetBlocked lays again the observers
 * around). A team's sight is the union of its members' (D371): their observers count in the same camp.
 */
class HEROESOFLIBERTY_API FHOLVisionMap
{
public:
	void Init(const FHOLVisionGrid& InGrid, int32 InNumCamps, const FHOLVisionTuning& InTuning = FHOLVisionTuning());

	const FHOLVisionGrid& GetGrid() const { return Grid; }
	int32 GetNumCamps() const { return NumCamps; }

	/**
	 * F2 (D383): a cell becomes forest, or open again (its last tree cut). The observers whose disc reaches it count their
	 * cells again. Returns false when nothing changed.
	 */
	bool SetBlocked(const FIntPoint& Cell, bool bBlocked);
	bool IsBlocked(const FIntPoint& Cell) const;

	/**
	 * F2: a tree (a source that BlocksSight) stands at Position, or is gone (used up): its cell counts it; reaching
	 * ForestTrees it turns forest, falling below it opens again (SetBlocked). Out of the grid: nothing.
	 */
	void AddBlocker(const FVector2D& Position);
	void RemoveBlocker(const FVector2D& Position);
	int32 GetBlockers(const FIntPoint& Cell) const;

	/** A new observer of Camp at Position (world, X, Y) seeing Radius cm (GetSightRadius). Returns its handle. */
	int32 AddObserver(int32 Camp, const FVector2D& Position, double Radius);

	/** The observer moved: nothing to do while it stays in its cell (D521); returns true when it changed cell (counted again). */
	bool MoveObserver(int32 Handle, const FVector2D& Position);

	/** Its sight changed (a site finished, the height): counted again. */
	void SetObserverRadius(int32 Handle, double Radius);

	/** The observer is gone (dead, destroyed): its cells counted off; they stay explored. */
	void RemoveObserver(int32 Handle);

	int32 GetNumObservers() const { return Observers.Num(); }

	/** D389 (« Carte explorée »): every cell of Camp explored from the start; in sight still only around its observers. */
	void ExploreAll(int32 Camp);

	/** Observers of Camp seeing Cell (0 out of the grid). */
	int32 GetCount(int32 Camp, const FIntPoint& Cell) const;

	EHOLCellVision GetVision(int32 Camp, const FIntPoint& Cell) const;
	bool IsVisible(int32 Camp, const FVector2D& Position) const;

	/** For the reports (hol.Fog.Report): cells of Camp in sight, cells ever seen. */
	int32 CountVisible(int32 Camp) const;
	int32 CountExplored(int32 Camp) const;

private:
	struct FObserver
	{
		int32 Camp = 0;
		FIntPoint Cell = FIntPoint::ZeroValue;
		double Radius = 0.;
		/** The cells it counts (indices of the grid). */
		TArray<int32> Seen;
	};

	void Lay(FObserver& Observer);
	void TakeOff(FObserver& Observer);

	FHOLVisionGrid Grid;
	FHOLVisionTuning Tuning;
	int32 NumCamps = 0;
	/** Per cell: standing trees (AddBlocker). */
	TArray<uint16> Trees;
	/** Per cell. */
	TArray<bool> Blocked;
	/** Per camp, then per cell: Counts[Camp * Grid.Num() + Index]. */
	TArray<uint16> Counts;
	TArray<bool> Explored;
	TSparseArray<FObserver> Observers;
};
