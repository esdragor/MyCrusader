// Lot MIG-C-Vision (CELEBORN-1117m, D830, D836, D839): the script port of the 7 pure tests of HOLVisionTests.cpp (Grid, Disc, Forest, Height,
// Counts, TreeCut, SightRadius), run against the read library of the fog's rules (UHOLVisionRulesReadLibrary, namespace HOLVisionRulesRead,
// and UHOLVisionMapHolder for the FHOLVisionMap). Same vectors as the C++ tests; the C++ tests stay until the red then the green are seen.
// NOT COMPILED nor run when written (machine saturated): the main session builds and runs it. A grid is Side and CellSize (20 x 400 cm here);
// a cell's vision is 0 Unexplored, 1 Explored, 2 Visible.

const int HOLVisionSide = 20;
const double HOLVisionCell = 400.0;
const int HOLVisionUnexplored = 0;
const int HOLVisionExplored = 1;
const int HOLVisionVisible = 2;

// One bool per cell, forest on the columns From to To (every row).
void HOLVisionAddWall(TArray<bool>& Blocked, int Side, int From, int To)
{
	if (Blocked.Num() != Side * Side)
	{
		Blocked.SetNum(Side * Side);
	}
	for (int Y = 0; Y < Side; ++Y)
	{
		for (int X = From; X <= To; ++X)
		{
			Blocked[Y * Side + X] = true;
		}
	}
}

bool HOLVisionSees(const TArray<bool>& Blocked, int FromX, int FromY, int ToX, int ToY)
{
	return HOLVisionRulesRead::SeenCellsContains(HOLVisionSide, HOLVisionCell, Blocked, FromX, FromY, 3200.0, ToX, ToY);
}

int HOLVisionCellOfX(const FVector2D& Point)
{
	int X = -1;
	int Y = -1;
	HOLVisionRulesRead::ToCell(HOLVisionSide, HOLVisionCell, Point, X, Y);
	return X;
}

int HOLVisionCellOfY(const FVector2D& Point)
{
	int X = -1;
	int Y = -1;
	HOLVisionRulesRead::ToCell(HOLVisionSide, HOLVisionCell, Point, X, Y);
	return Y;
}

bool HOLVisionInsideCellOf(const FVector2D& Point)
{
	return HOLVisionRulesRead::IsInsideGrid(HOLVisionSide, HOLVisionCell, HOLVisionCellOfX(Point), HOLVisionCellOfY(Point));
}

void Test_VisionGrid(FUnitTest& T)
{
	T.AssertEquals(20, HOLVisionSide, "side");
	T.AssertEquals(400, HOLVisionRulesRead::GetGridNum(HOLVisionSide, HOLVisionCell), "cells");
	T.AssertTrue(HOLVisionCellOfX(FVector2D(0.0, 0.0)) == 10 && HOLVisionCellOfY(FVector2D(0.0, 0.0)) == 10, "the origin in the cell right of the middle");
	T.AssertTrue(HOLVisionCellOfX(FVector2D(-4000.0, -4000.0)) == 0 && HOLVisionCellOfY(FVector2D(-4000.0, -4000.0)) == 0, "the corner");
	T.AssertTrue(HOLVisionCellOfX(FVector2D(3999.0, -1.0)) == 19 && HOLVisionCellOfY(FVector2D(3999.0, -1.0)) == 9, "just in the last cell");
	T.AssertFalse(HOLVisionInsideCellOf(FVector2D(4000.0, 0.0)), "the far edge out of the grid");
	T.AssertFalse(HOLVisionInsideCellOf(FVector2D(-4001.0, 0.0)), "below the corner out of the grid");
	const FVector2D First = HOLVisionRulesRead::GetCellCenter(HOLVisionSide, HOLVisionCell, 0, 0);
	T.AssertTrue(First.X == -3800.0 && First.Y == -3800.0, "centre of the first cell");
	const FVector2D Centre = HOLVisionRulesRead::GetCellCenter(HOLVisionSide, HOLVisionCell, 7, 13);
	T.AssertTrue(HOLVisionCellOfX(Centre) == 7 && HOLVisionCellOfY(Centre) == 13, "a centre comes back to its cell");
}

void Test_VisionDisc(FUnitTest& T)
{
	T.AssertEquals(13, HOLVisionRulesRead::GetDiscNum(HOLVisionSide, HOLVisionCell, 10, 10, 800.0), "a sight of 2 cells: 13 cells");
	TArray<int> OffsetX;
	TArray<int> OffsetY;
	OffsetX.Add(0); OffsetY.Add(0);
	OffsetX.Add(2); OffsetY.Add(0);
	OffsetX.Add(-2); OffsetY.Add(0);
	OffsetX.Add(0); OffsetY.Add(2);
	OffsetX.Add(0); OffsetY.Add(-2);
	OffsetX.Add(1); OffsetY.Add(1);
	OffsetX.Add(-1); OffsetY.Add(-1);
	for (int Index = 0; Index < OffsetX.Num(); ++Index)
	{
		T.AssertTrue(HOLVisionRulesRead::DiscContains(HOLVisionSide, HOLVisionCell, 10, 10, 800.0, 10 + OffsetX[Index], 10 + OffsetY[Index]),
			"cell " + OffsetX[Index] + "," + OffsetY[Index] + " in the disc");
	}
	T.AssertFalse(HOLVisionRulesRead::DiscContains(HOLVisionSide, HOLVisionCell, 10, 10, 800.0, 12, 12), "the corner 2,2 out of the disc");
	T.AssertFalse(HOLVisionRulesRead::DiscContains(HOLVisionSide, HOLVisionCell, 10, 10, 800.0, 13, 10), "3 cells out of the disc");

	T.AssertEquals(6, HOLVisionRulesRead::GetDiscNum(HOLVisionSide, HOLVisionCell, 0, 0, 800.0), "in the grid's corner: the quarter (6 cells)");
	T.AssertEquals(197, HOLVisionRulesRead::GetDiscNum(40, 400.0, 20, 20, 3200.0), "D523: a sight of 8 cells, 197 cells");
	T.AssertEquals(1, HOLVisionRulesRead::GetDiscNum(HOLVisionSide, HOLVisionCell, 4, 4, 0.0), "no sight: its own cell");

	TArray<bool> Open;
	T.AssertEquals(HOLVisionRulesRead::GetDiscNum(HOLVisionSide, HOLVisionCell, 10, 10, 1600.0),
		HOLVisionRulesRead::GetSeenCellsNum(HOLVisionSide, HOLVisionCell, Open, 10, 10, 1600.0), "no forest: the seen cells are the disc");
}

void Test_VisionForest(FUnitTest& T)
{
	TArray<bool> Wall;
	HOLVisionAddWall(Wall, HOLVisionSide, 8, 8);
	T.AssertTrue(HOLVisionSees(Wall, 5, 10, 7, 10), "before the wall: seen");
	T.AssertTrue(HOLVisionSees(Wall, 5, 10, 8, 10), "the edge of the forest: seen");
	T.AssertTrue(HOLVisionSees(Wall, 5, 10, 8, 12), "the edge seen aslant");
	T.AssertFalse(HOLVisionSees(Wall, 5, 10, 9, 10), "behind the wall: hidden");
	T.AssertFalse(HOLVisionSees(Wall, 5, 10, 12, 10), "far behind the wall: hidden");
	T.AssertFalse(HOLVisionSees(Wall, 5, 10, 10, 12), "behind the wall aslant: hidden");
	T.AssertTrue(HOLVisionSees(Wall, 5, 10, 1, 10), "on the open side: seen");

	// An observer in a forest of 3 columns sees out of it on both sides, then the next forest stops it as any other.
	TArray<bool> Woods;
	HOLVisionAddWall(Woods, HOLVisionSide, 8, 10);
	HOLVisionAddWall(Woods, HOLVisionSide, 14, 15);
	T.AssertTrue(HOLVisionRulesRead::IsInLineOfSight(HOLVisionSide, HOLVisionCell, Woods, 9, 10, 9, 10), "in the forest: its own cell seen");
	T.AssertTrue(HOLVisionSees(Woods, 9, 10, 10, 10), "in the forest: its neighbour seen");
	T.AssertTrue(HOLVisionSees(Woods, 9, 10, 12, 10), "in the forest: out of it on the right");
	T.AssertTrue(HOLVisionSees(Woods, 9, 10, 6, 10), "in the forest: out of it on the left");
	T.AssertTrue(HOLVisionSees(Woods, 9, 10, 14, 10), "the next forest's edge: seen");
	T.AssertFalse(HOLVisionSees(Woods, 9, 10, 15, 10), "inside the next forest: hidden");
	T.AssertFalse(HOLVisionSees(Woods, 9, 10, 16, 10), "behind the next forest: hidden");
	T.AssertFalse(HOLVisionSees(Woods, 9, 10, 18, 10), "out of its sight: hidden");
}

void Test_VisionHeight(FUnitTest& T)
{
	const double Max = HOLVisionRulesRead::GetMaxHeightBonus();
	T.AssertTrue(HOLVisionRulesRead::GetSightRadius(3200.0, 0.0) == 3200.0, "on flat ground: its sight");
	T.AssertTrue(HOLVisionRulesRead::GetSightRadius(3200.0, 300.0) > 3200.0, "higher: further");
	T.AssertTrue(HOLVisionRulesRead::GetSightRadius(3200.0, 100000.0) == 3200.0 + Max, "moderate: capped");
	T.AssertTrue(HOLVisionRulesRead::GetSightRadius(3200.0, -500.0) == 3200.0, "lower: no malus");
	T.AssertTrue(HOLVisionRulesRead::GetDiscNum(60, 400.0, 30, 30, HOLVisionRulesRead::GetSightRadius(3200.0, 400.0))
		> HOLVisionRulesRead::GetDiscNum(60, 400.0, 30, 30, 3200.0), "a hill made by the test: more cells seen");
}

int HOLVisionUnion(int Side, double Cell, int AX, int AY, int BX, int BY, double Radius)
{
	// The cells of both discs, once each: |A| + |B| - |A and B|, the intersection counted cell by cell over the grid.
	int Both = 0;
	for (int Y = 0; Y < Side; ++Y)
	{
		for (int X = 0; X < Side; ++X)
		{
			if (HOLVisionRulesRead::DiscContains(Side, Cell, AX, AY, Radius, X, Y) || HOLVisionRulesRead::DiscContains(Side, Cell, BX, BY, Radius, X, Y))
			{
				++Both;
			}
		}
	}
	return Both;
}

void Test_VisionCounts(FUnitTest& T)
{
	UHOLVisionMapHolder Map = UHOLVisionMapHolder::Create(HOLVisionSide, HOLVisionCell, 2);
	T.AssertEquals(HOLVisionUnexplored, Map.GetVision(0, 10, 10), "at first: black");

	const FVector2D Start = Map.GetCellCenter(10, 10);
	const int Scout = Map.AddObserver(0, Start, 800.0);
	T.AssertEquals(1, Map.GetCount(0, 10, 10), "its cell seen once");
	T.AssertEquals(HOLVisionVisible, Map.GetVision(0, 12, 10), "in sight");
	T.AssertEquals(HOLVisionRulesRead::GetDiscNum(HOLVisionSide, HOLVisionCell, 10, 10, 800.0), Map.CountVisible(0), "its disc in sight");
	T.AssertEquals(0, Map.CountVisible(1), "the other camp sees nothing");
	T.AssertFalse(Map.IsVisible(1, Start), "the other camp: hidden");
	T.AssertTrue(Map.IsVisible(0, Start), "its camp: seen");

	T.AssertFalse(Map.MoveObserver(Scout, Start + FVector2D(150.0, -150.0)), "a step in its cell: nothing counted again");
	T.AssertTrue(Map.MoveObserver(Scout, Map.GetCellCenter(14, 10)), "to another cell: counted again");
	T.AssertEquals(HOLVisionExplored, Map.GetVision(0, 8, 10), "left behind: explored");
	T.AssertEquals(0, Map.GetCount(0, 8, 10), "left behind: no observer");
	T.AssertEquals(HOLVisionVisible, Map.GetVision(0, 16, 10), "ahead: in sight");
	const int BothDiscs = HOLVisionUnion(HOLVisionSide, HOLVisionCell, 10, 10, 14, 10, 800.0);
	T.AssertEquals(BothDiscs, Map.CountExplored(0), "explored: both discs");

	Map.RemoveObserver(Scout);
	T.AssertEquals(0, Map.CountVisible(0), "gone: nothing in sight");
	T.AssertEquals(BothDiscs, Map.CountExplored(0), "gone: the explored stays");
	T.AssertEquals(0, Map.GetNumObservers(), "gone: no observer");

	// A team: its members' sight is one (D371); the cells both see counted twice.
	const int A = Map.AddObserver(0, Map.GetCellCenter(4, 4), 800.0);
	const int B = Map.AddObserver(0, Map.GetCellCenter(6, 4), 800.0);
	T.AssertEquals(2, Map.GetCount(0, 5, 4), "team: the cell between seen twice");
	const int TeamUnion = HOLVisionUnion(HOLVisionSide, HOLVisionCell, 4, 4, 6, 4, 800.0);
	T.AssertEquals(TeamUnion, Map.CountVisible(0), "team: the union in sight");
	T.AssertEquals(0, Map.CountVisible(1), "team: the other camp still blind");
	Map.RemoveObserver(A);
	T.AssertEquals(1, Map.GetCount(0, 5, 4), "one gone: the other still sees");
	Map.SetObserverRadius(B, 0.0);
	T.AssertEquals(1, Map.CountVisible(0), "no sight: its own cell");
	Map.RemoveObserver(B);
	T.AssertEquals(0, Map.CountVisible(0), "all gone: every count back to 0");
}

void Test_VisionTreeCut(FUnitTest& T)
{
	UHOLVisionMapHolder Map = UHOLVisionMapHolder::Create(HOLVisionSide, HOLVisionCell, 1);
	for (int Y = 0; Y < HOLVisionSide; ++Y)
	{
		Map.SetBlocked(8, Y, true);
	}
	T.AssertTrue(Map.IsBlocked(8, 10), "forest");
	T.AssertFalse(Map.SetBlocked(8, 10, true), "forest again: no change");

	const int Eye = Map.AddObserver(0, Map.GetCellCenter(5, 10), 3200.0);
	T.AssertEquals(HOLVisionVisible, Map.GetVision(0, 8, 10), "the edge: in sight");
	T.AssertEquals(HOLVisionUnexplored, Map.GetVision(0, 9, 10), "behind the forest: black");

	T.AssertTrue(Map.SetBlocked(8, 10, false), "the last tree cut");
	T.AssertEquals(HOLVisionVisible, Map.GetVision(0, 9, 10), "through the gap: in sight");
	const int Seen = Map.CountSeenByRules(5, 10, 3200.0);
	T.AssertEquals(Seen, Map.CountVisible(0), "the counts match the forest now");

	T.AssertTrue(Map.SetBlocked(8, 10, true), "planted again");
	T.AssertEquals(HOLVisionExplored, Map.GetVision(0, 9, 10), "behind the forest again: explored");
	Map.RemoveObserver(Eye);
	T.AssertEquals(0, Map.CountVisible(0), "gone: exactly what it laid taken off");
}

void Test_VisionSightRadius(FUnitTest& T)
{
	// F2 (D523): every type of the real tables sees, beyond its reach (attack or riposte) by a tile at least.
	const double Tile = HOLVisionRulesRead::GetTileSize();
	TArray<FString> Names;
	TArray<double> Sight;
	TArray<double> Reach;
	TArray<double> SimSight;
	T.AssertTrue(HOLVisionRulesRead::ReadUnitSightRows(Names, Sight, Reach, SimSight), "the unit type table");
	T.AssertTrue(Names.Num() > 0, "the unit type table has rows");
	for (int Index = 0; Index < Names.Num(); ++Index)
	{
		T.AssertTrue(Sight[Index] >= Reach[Index] + Tile, Names[Index] + " sees (" + Sight[Index] + " cm) beyond its reach (" + Reach[Index] + " cm) by a tile");
		T.AssertTrue(SimSight[Index] == Sight[Index], Names[Index] + ": the simulation reads its sight");
	}
	TArray<FString> BuildingNames;
	TArray<double> BuildingSight;
	TArray<double> HalfSide;
	T.AssertTrue(HOLVisionRulesRead::ReadBuildingSightRows(BuildingNames, BuildingSight, HalfSide), "the building type table");
	T.AssertTrue(BuildingNames.Num() > 0, "the building type table has rows");
	for (int Index = 0; Index < BuildingNames.Num(); ++Index)
	{
		T.AssertTrue(BuildingSight[Index] >= HalfSide[Index] + Tile, BuildingNames[Index] + " sees (" + BuildingSight[Index] + " cm) beyond its walls by a tile");
	}
}
