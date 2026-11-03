// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Map/HOLFogMode.h"
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

/** F7.3 (b): how a viewer's machine draws a building: as it is, as its camp last saw it (grey), or not at all (never seen). */
enum class EHOLBuildingDraw : uint8
{
	Live,
	Memory,
	Hidden,
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

	/**
	 * D357, D376: the camp whose sight a player's own machine shows (the client's side of GetReceivedCamps): its own camp; a slot
	 * defeated alone (free for all, Diplomacy) or no camp at all INDEX_NONE, no fog for it, everything in sight; defeated in a
	 * team, the team's. Whatever shows every camp here is what GetReceivedCamps sends every camp to.
	 */
	HEROESOFLIBERTY_API int32 GetViewerCamp(int32 Camp, bool bDefeated, bool bTeamCamp);

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

	/**
	 * F7.3 (b): a building of another camp (bOwnCamp false: the viewer's camp is its owner's or its ally's) out of the viewer's
	 * sight is drawn from the memory, not as it is now.
	 */
	HEROESOFLIBERTY_API bool BuildingDrawsFromMemory(bool bOwnCamp, bool bInSight);

	/** F7.3 (b): Live when it is the viewer's camp's or in sight; else Memory when its camp remembers it (bRemembered), else Hidden. */
	HEROESOFLIBERTY_API EHOLBuildingDraw GetBuildingDraw(bool bOwnCamp, bool bInSight, bool bRemembered);

	/**
	 * F8 (D522): the fog a match starts with: the console's hol.Fog.Mode (CVarValue 0 to 2) when set, else the configuration's (the menu's,
	 * the lobby's); -1 follows the configuration.
	 */
	HEROESOFLIBERTY_API EHOLFogMode ResolveFogMode(int32 CVarValue, EHOLFogMode ConfigMode);

	/**
	 * F7.3 (c): a source used up (a tree cut) stays drawn on the viewer's machine while its camp remembers it standing (bRemembered)
	 * and its place is out of sight (bInSight false): the cut is learned by looking at the place again (D382).
	 */
	HEROESOFLIBERTY_API bool SourceStaysDrawn(bool bRemembered, bool bInSight);

	/**
	 * F7.3 (b), a client: the actor of a remembered building is drawn (a ghost actor made here, the server no longer sends the
	 * real one out of the camp's sight) only while its place is out of sight and no real building stands there.
	 */
	HEROESOFLIBERTY_API bool GhostActorWanted(bool bInSight, bool bRealPresent);

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

	/**
	 * F6e.3 (D520, R7): the cells Camp explored as run lengths, for the server to send to a player who comes back: alternating
	 * runs over the grid's cells in index order, the first of unexplored cells (0 if cell 0 is explored), summing to Grid.Num().
	 */
	TArray<uint32> GetExploredRuns(int32 Camp) const;
	/** Marks explored (never takes back) the cells the runs say for Camp; false, nothing changed, when they do not sum to Grid.Num(). */
	bool AddExploredRuns(int32 Camp, TConstArrayView<uint32> Runs);

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

/**
 * F7 / F8 (FogInProgress.md): the fog's picture of one camp, what the post process (UHOLFogRenderSubsystem) and the minimap
 * (Galadriel) both read: one byte a cell of the grid, in the grid's order (index = Y * Side + X: a row of the picture is one Y, so
 * its lower corner is the grid's), 0 unexplored, ExploredByte explored, 255 in sight. A picture without fog (Camp INDEX_NONE:
 * mode Disabled, a spectator) is everything in sight.
 */
namespace HOLFogPicture
{
	constexpr uint8 UnexploredByte = 0;
	constexpr uint8 ExploredByte = 115;
	constexpr uint8 VisibleByte = 255;

	HEROESOFLIBERTY_API uint8 ToByte(EHOLCellVision Vision);
	/** The state of a byte, which may have been filtered (blurred): in sight from 200, explored from 60, else unexplored. */
	HEROESOFLIBERTY_API EHOLCellVision FromByte(uint8 Byte);
	/** Fills Bytes (Grid.Num() of them) from Map for Camp, counts the cells in sight and the cells ever seen. */
	HEROESOFLIBERTY_API void Fill(const FHOLVisionMap& Map, int32 Camp, TArray<uint8>& Bytes, int32& NumVisible, int32& NumExplored);
	/** The state of the cell under a point of the ground (X, Y in cm) read from the bytes; a point out of the grid: unexplored. */
	HEROESOFLIBERTY_API EHOLCellVision Read(const FHOLVisionGrid& Grid, TConstArrayView<uint8> Bytes, const FVector2D& Point);
	/** The bytes interpolated bilinearly between the cells' centres, 0 (unexplored) to 1 (in sight): a soft edge for a picture finer than the grid (the minimap). Outside the grid or without bytes: 0. */
	HEROESOFLIBERTY_API float ReadSmooth(const FHOLVisionGrid& Grid, TConstArrayView<uint8> Bytes, const FVector2D& Point);
	/** OutBytes = Bytes smoothed by a 1-2-1 by 1-2-1 kernel (edge cells repeat themselves): a field with no step, for ReadSmooth to blend when the picture is finer than the cells (the minimap). A uniform field stays itself. */
	HEROESOFLIBERTY_API void Blur(const FHOLVisionGrid& Grid, TConstArrayView<uint8> Bytes, TArray<uint8>& OutBytes);
	/** The point's place in the picture, (0, 0) at the grid's lower corner (-X, -Y) and (1, 1) at the upper one; out of [0, 1] outside the grid. */
	HEROESOFLIBERTY_API FVector2D ToUV(const FHOLVisionGrid& Grid, const FVector2D& Point);
}
