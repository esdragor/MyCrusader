// Wall / palisade line placement, pure rule (F4, HOLWallLineRules.as; D691). Cells are integer grid coordinates (one per 4 m tile). The cap, the
// per-cell cost and every value below are FIXTURES: the real length cap, cost and corner shape are the user's choice (D345), not decided here.

bool WallLineHasCell(const TArray<FIntPoint>& Line, int X, int Y)
{
	for (int Index = 0; Index < Line.Num(); Index++)
	{
		if (Line[Index].X == X && Line[Index].Y == Y)
			return true;
	}
	return false;
}

bool WallLineHasDuplicate(const TArray<FIntPoint>& Line)
{
	for (int A = 0; A < Line.Num(); A++)
	{
		for (int B = A + 1; B < Line.Num(); B++)
		{
			if (Line[A].X == Line[B].X && Line[A].Y == Line[B].Y)
				return true;
		}
	}
	return false;
}

// Every cell touches the previous one (8 neighbours): no hole in the line.
bool WallLineIsContiguous(const TArray<FIntPoint>& Line)
{
	for (int Index = 1; Index < Line.Num(); Index++)
	{
		int DX = Line[Index].X - Line[Index - 1].X;
		int DY = Line[Index].Y - Line[Index - 1].Y;
		if (DX < -1 || DX > 1 || DY < -1 || DY > 1 || (DX == 0 && DY == 0))
			return false;
	}
	return true;
}

// Start == End: the line is the one cell where the player pressed and released.
void Test_WallLineSingleCell(FUnitTest& T)
{
	TArray<FIntPoint> Line = HOLWallLine::Cells(FIntPoint(3, 4), FIntPoint(3, 4), 10);
	T.AssertEquals(1, Line.Num(), "Start == End gives exactly one cell");
	T.AssertTrue(WallLineHasCell(Line, 3, 4), "the one cell is the start cell");
}

// Horizontal and vertical lines run from the start to the end, in that order, whichever way the player drags, with no hole.
void Test_WallLineStraightIsOrderedFromStart(FUnitTest& T)
{
	TArray<FIntPoint> East = HOLWallLine::Cells(FIntPoint(2, 5), FIntPoint(6, 5), 64);
	T.AssertEquals(5, East.Num(), "(2,5) -> (6,5): five cells, both ends included");
	T.AssertTrue(East.Num() == 5 && East[0].X == 2 && East[0].Y == 5 && East[4].X == 6 && East[4].Y == 5, "eastward: first cell is the start, last is the end");
	T.AssertTrue(WallLineIsContiguous(East), "eastward: no hole");

	TArray<FIntPoint> West = HOLWallLine::Cells(FIntPoint(6, 5), FIntPoint(2, 5), 64);
	T.AssertEquals(5, West.Num(), "(6,5) -> (2,5): the same five cells");
	T.AssertTrue(West.Num() == 5 && West[0].X == 6 && West[4].X == 2, "westward: the order starts at the start (x = 6 first, x = 2 last)");

	TArray<FIntPoint> South = HOLWallLine::Cells(FIntPoint(0, 0), FIntPoint(0, -3), 64);
	T.AssertEquals(4, South.Num(), "(0,0) -> (0,-3): four cells");
	T.AssertTrue(South.Num() == 4 && South[0].Y == 0 && South[1].Y == -1 && South[3].Y == -3 && South[2].X == 0, "vertical: y goes 0, -1, -2, -3 on column 0");
}

// A 45-degree line takes one cell per step on each axis, no cell twice. A drag that is not on an axis or a diagonal snaps to the nearest of the
// 8 directions (CHOICE TO CONFIRM: the length is the drag projected on that direction, in whole cells).
void Test_WallLineDiagonalHasNoDuplicate(FUnitTest& T)
{
	TArray<FIntPoint> Up = HOLWallLine::Cells(FIntPoint(0, 0), FIntPoint(4, 4), 64);
	T.AssertEquals(5, Up.Num(), "(0,0) -> (4,4): five cells");
	T.AssertTrue(!WallLineHasDuplicate(Up), "the diagonal has no duplicate cell");
	T.AssertTrue(WallLineHasCell(Up, 0, 0) && WallLineHasCell(Up, 2, 2) && WallLineHasCell(Up, 4, 4), "the diagonal is (i, i)");
	T.AssertTrue(WallLineIsContiguous(Up), "the diagonal has no hole");

	TArray<FIntPoint> Anti = HOLWallLine::Cells(FIntPoint(4, 0), FIntPoint(0, 4), 64);
	T.AssertEquals(5, Anti.Num(), "(4,0) -> (0,4): five cells");
	T.AssertTrue(Anti.Num() == 5 && Anti[0].X == 4 && Anti[0].Y == 0 && Anti[4].X == 0 && Anti[4].Y == 4 && Anti[2].X == 2 && Anti[2].Y == 2, "the other diagonal runs (4,0), (3,1), ... (0,4)");
	T.AssertTrue(!WallLineHasDuplicate(Anti), "the other diagonal has no duplicate cell");

	TArray<FIntPoint> Snapped = HOLWallLine::Cells(FIntPoint(0, 0), FIntPoint(5, 3), 64);
	T.AssertEquals(5, Snapped.Num(), "(0,0) -> (5,3) snaps to the diagonal, 4 steps (projection of the drag): five cells");
	T.AssertTrue(Snapped.Num() == 5 && Snapped[4].X == 4 && Snapped[4].Y == 4, "the snapped diagonal ends on (4,4)");
	TArray<FIntPoint> Flat = HOLWallLine::Cells(FIntPoint(0, 0), FIntPoint(10, 2), 64);
	T.AssertEquals(11, Flat.Num(), "(0,0) -> (10,2) snaps to the axis: eleven cells");
	T.AssertTrue(Flat.Num() == 11 && Flat[10].X == 10 && Flat[10].Y == 0, "the snapped axis line ends on (10,0)");
}

// Past the cap the line is cut: the cells kept are the first ones from the start.
void Test_WallLineIsCutAtTheCap(FUnitTest& T)
{
	TArray<FIntPoint> Cut = HOLWallLine::Cells(FIntPoint(0, 0), FIntPoint(9, 0), 4);
	T.AssertEquals(4, Cut.Num(), "ten cells asked, cap 4: four kept");
	T.AssertTrue(Cut.Num() == 4 && Cut[0].X == 0 && Cut[3].X == 3, "the kept cells are the first four from the start");

	T.AssertEquals(10, HOLWallLine::Cells(FIntPoint(0, 0), FIntPoint(9, 0), 10).Num(), "a line of exactly the cap is whole");
	T.AssertEquals(10, HOLWallLine::Cells(FIntPoint(0, 0), FIntPoint(9, 0), 50).Num(), "a line under the cap is whole");
	T.AssertEquals(1, HOLWallLine::Cells(FIntPoint(0, 0), FIntPoint(9, 0), 1).Num(), "cap 1: only the start cell");
	T.AssertEquals(0, HOLWallLine::Cells(FIntPoint(0, 0), FIntPoint(9, 0), 0).Num(), "cap 0: nothing may be built");

	TArray<FIntPoint> Diagonal = HOLWallLine::Cells(FIntPoint(0, 0), FIntPoint(5, 5), 3);
	T.AssertTrue(Diagonal.Num() == 3 && Diagonal[2].X == 2 && Diagonal[2].Y == 2, "a diagonal line is cut the same way: (0,0), (1,1), (2,2)");
}

// The total cost is the number of cells actually kept times the price of one cell: a cut line costs less.
void Test_WallLineCostIsPerCell(FUnitTest& T)
{
	T.AssertEquals(15, HOLWallLine::Cost(FIntPoint(0, 0), FIntPoint(4, 0), 64, 3), "five cells at 3 each cost 15");
	T.AssertEquals(3, HOLWallLine::Cost(FIntPoint(2, 2), FIntPoint(2, 2), 64, 3), "one cell at 3 costs 3");
	T.AssertEquals(6, HOLWallLine::Cost(FIntPoint(0, 0), FIntPoint(4, 0), 2, 3), "the line cut to 2 cells costs 6, the cut part is not paid");
	T.AssertEquals(0, HOLWallLine::Cost(FIntPoint(0, 0), FIntPoint(4, 0), 64, 0), "a free cell makes a free line");
	T.AssertEquals(0, HOLWallLine::Cost(FIntPoint(0, 0), FIntPoint(4, 0), 0, 3), "cap 0: no cell, nothing to pay");
	T.AssertEquals(12, HOLWallLine::Cost(FIntPoint(0, 0), FIntPoint(3, 3), 64, 3), "a diagonal of four cells at 3 costs 12");
}

// ---- Server validation of a line (HOLWallLine::Validate). Fixtures only: the map is 0..63 and the cap 16 are NOT the real values (Q-mur-ligne-longueur).
const FIntPoint WallLineMapMin = FIntPoint(0, 0);
const FIntPoint WallLineMapMax = FIntPoint(63, 63);

// A free drag of N cells gives N aligned cells, start first, and no refusal.
void Test_WallLineValidateFreeDragLaysEveryCell(FUnitTest& T)
{
	TArray<FIntPoint> None;
	HOLWallLine::FHOLWallLinePlan Plan = HOLWallLine::Validate(FIntPoint(2, 5), FIntPoint(8, 5), WallLineMapMin, WallLineMapMax, None, 16);
	T.AssertTrue(Plan.Refusal == HOLWallLine::EHOLWallLineRefusal::Ok, "a free drag inside the map and the cap is accepted");
	T.AssertEquals(7, Plan.Cells.Num(), "(2,5) -> (8,5): seven cells laid");
	T.AssertTrue(Plan.Cells.Num() == 7 && Plan.Cells[0].X == 2 && Plan.Cells[6].X == 8 && WallLineIsContiguous(Plan.Cells), "the cells run from the start to the end without a hole");
	bool bAligned = Plan.Cells.Num() > 0;
	for (int Index = 0; Index < Plan.Cells.Num(); Index++)
		bAligned = bAligned && Plan.Cells[Index].Y == 5;
	T.AssertTrue(bAligned, "all the cells are on row 5");
}

// An off-axis drag is laid on the nearest of the 8 directions, and the occupancy is read on the LAID cells, not on the drag end.
void Test_WallLineValidateOffAxisDragSnaps(FUnitTest& T)
{
	TArray<FIntPoint> OnDragEndOnly;
	OnDragEndOnly.Add(FIntPoint(10, 2));
	HOLWallLine::FHOLWallLinePlan Flat = HOLWallLine::Validate(FIntPoint(0, 0), FIntPoint(10, 2), WallLineMapMin, WallLineMapMax, OnDragEndOnly, 16);
	T.AssertTrue(Flat.Refusal == HOLWallLine::EHOLWallLineRefusal::Ok, "(10,2) is occupied but the laid line is row 0: accepted");
	T.AssertEquals(11, Flat.Cells.Num(), "(0,0) -> (10,2) snaps to the axis: eleven cells");
	T.AssertTrue(Flat.Cells.Num() == 11 && Flat.Cells[10].X == 10 && Flat.Cells[10].Y == 0, "the snapped line ends on (10,0)");

	TArray<FIntPoint> None;
	HOLWallLine::FHOLWallLinePlan Diagonal = HOLWallLine::Validate(FIntPoint(10, 10), FIntPoint(15, 13), WallLineMapMin, WallLineMapMax, None, 16);
	T.AssertEquals(5, Diagonal.Cells.Num(), "(10,10) -> (15,13) snaps to the diagonal, 4 steps: five cells");
	T.AssertTrue(Diagonal.Cells.Num() == 5 && Diagonal.Cells[4].X == 14 && Diagonal.Cells[4].Y == 14, "the snapped diagonal ends on (14,14)");
}

// One occupied cell on the line refuses the WHOLE line: no cell is laid, the bad cell is named.
void Test_WallLineValidateOccupiedCellRefusesTheLine(FUnitTest& T)
{
	TArray<FIntPoint> Taken;
	Taken.Add(FIntPoint(5, 5));
	HOLWallLine::FHOLWallLinePlan Plan = HOLWallLine::Validate(FIntPoint(2, 5), FIntPoint(8, 5), WallLineMapMin, WallLineMapMax, Taken, 16);
	T.AssertTrue(Plan.Refusal == HOLWallLine::EHOLWallLineRefusal::Occupied, "a building on (5,5) refuses the line");
	T.AssertEquals(0, Plan.Cells.Num(), "all or nothing: no cell is laid");
	T.AssertTrue(Plan.BadCell.X == 5 && Plan.BadCell.Y == 5, "the refusal names the occupied cell");

	TArray<FIntPoint> Beside;
	Beside.Add(FIntPoint(5, 6));
	HOLWallLine::FHOLWallLinePlan Free = HOLWallLine::Validate(FIntPoint(2, 5), FIntPoint(8, 5), WallLineMapMin, WallLineMapMax, Beside, 16);
	T.AssertTrue(Free.Refusal == HOLWallLine::EHOLWallLineRefusal::Ok && Free.Cells.Num() == 7, "a building on the next row does not refuse");
}

// A line that leaves the map is refused whole, even when only its last cell is outside.
void Test_WallLineValidateOutOfMapRefusesTheLine(FUnitTest& T)
{
	TArray<FIntPoint> None;
	HOLWallLine::FHOLWallLinePlan Plan = HOLWallLine::Validate(FIntPoint(60, 10), FIntPoint(64, 10), WallLineMapMin, WallLineMapMax, None, 16);
	T.AssertTrue(Plan.Refusal == HOLWallLine::EHOLWallLineRefusal::OutOfMap, "(64,10) is outside 0..63: refused");
	T.AssertEquals(0, Plan.Cells.Num(), "no cell is laid");
	T.AssertTrue(Plan.BadCell.X == 64 && Plan.BadCell.Y == 10, "the refusal names the first cell outside");
	HOLWallLine::FHOLWallLinePlan Edge = HOLWallLine::Validate(FIntPoint(60, 10), FIntPoint(63, 10), WallLineMapMin, WallLineMapMax, None, 16);
	T.AssertTrue(Edge.Refusal == HOLWallLine::EHOLWallLineRefusal::Ok && Edge.Cells.Num() == 4, "the last cell of the map is inside");
}

// Too long: a drag longer than the cap is refused (not trimmed, unlike Cells); exactly the cap is accepted; a cap of 0 refuses everything.
void Test_WallLineValidateTooLongRefusesTheLine(FUnitTest& T)
{
	TArray<FIntPoint> None;
	HOLWallLine::FHOLWallLinePlan Over = HOLWallLine::Validate(FIntPoint(0, 0), FIntPoint(5, 0), WallLineMapMin, WallLineMapMax, None, 5);
	T.AssertTrue(Over.Refusal == HOLWallLine::EHOLWallLineRefusal::TooLong, "6 cells with a cap of 5: refused");
	T.AssertEquals(0, Over.Cells.Num(), "no cell is laid, the line is not trimmed");
	T.AssertTrue(Over.BadCell.X == 5 && Over.BadCell.Y == 0, "the refusal names the first cell past the cap");
	HOLWallLine::FHOLWallLinePlan Exact = HOLWallLine::Validate(FIntPoint(0, 0), FIntPoint(4, 0), WallLineMapMin, WallLineMapMax, None, 5);
	T.AssertTrue(Exact.Refusal == HOLWallLine::EHOLWallLineRefusal::Ok && Exact.Cells.Num() == 5, "5 cells with a cap of 5: accepted");
	HOLWallLine::FHOLWallLinePlan Zero = HOLWallLine::Validate(FIntPoint(0, 0), FIntPoint(0, 0), WallLineMapMin, WallLineMapMax, None, 0);
	T.AssertTrue(Zero.Refusal == HOLWallLine::EHOLWallLineRefusal::TooLong && Zero.Cells.Num() == 0, "a cap of 0 refuses even one cell");
}