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

// D901: one occupied cell on the line is a hole: the other 6 cells are laid, the occupied one is skipped (no longer all or nothing).
void Test_WallLineValidateOccupiedCellLeavesAHole(FUnitTest& T)
{
	TArray<FIntPoint> Taken;
	Taken.Add(FIntPoint(5, 5));
	HOLWallLine::FHOLWallLinePlan Plan = HOLWallLine::Validate(FIntPoint(2, 5), FIntPoint(8, 5), WallLineMapMin, WallLineMapMax, Taken, 16);
	T.AssertTrue(Plan.Refusal == HOLWallLine::EHOLWallLineRefusal::Ok, "a building on (5,5) does not refuse the line");
	T.AssertEquals(6, Plan.Cells.Num(), "the 6 other cells are laid");
	T.AssertTrue(Plan.Skipped.Num() == 1 && Plan.Skipped[0].X == 5 && Plan.Skipped[0].Y == 5, "the occupied cell is the one skipped");

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

// ---- Request adaptation (F4): ServerPlaceWallLine's pure part, world points in, cells out.
const double WallLineTile = 400.0;

// A request from a sender with no seat is refused whatever the line; a seated sender with the same line is accepted: the refusal comes from the sender.
void Test_WallLinePlanRefusesSenderOutOfRange(FUnitTest& T)
{
	TArray<FIntPoint> None;
	FVector A = HOLWallLine::CellToPoint(FIntPoint(2, 3), WallLineTile, 0.0);
	FVector B = HOLWallLine::CellToPoint(FIntPoint(6, 3), WallLineTile, 0.0);
	HOLWallLine::FHOLWallLinePlan Gone = HOLWallLine::PlanFromRequest(-1, A, B, WallLineTile, WallLineMapMin, WallLineMapMax, None, 16);
	T.AssertTrue(Gone.Refusal == HOLWallLine::EHOLWallLineRefusal::NoSeat && Gone.Cells.Num() == 0, "slot -1 (spectator): refused, no cell");
	HOLWallLine::FHOLWallLinePlan Far = HOLWallLine::PlanFromRequest(1000, A, B, WallLineTile, WallLineMapMin, WallLineMapMax, None, 16);
	T.AssertTrue(Far.Refusal == HOLWallLine::EHOLWallLineRefusal::NoSeat && Far.Cells.Num() == 0, "slot 1000 (out of range): refused, no cell");
	HOLWallLine::FHOLWallLinePlan Seated = HOLWallLine::PlanFromRequest(0, A, B, WallLineTile, WallLineMapMin, WallLineMapMax, None, 16);
	T.AssertTrue(Seated.Refusal == HOLWallLine::EHOLWallLineRefusal::Ok, "slot 0, same line: accepted");
}

// The accepted plan lays the cells Validate lays for the same two cells (computed by the rule, not copied).
void Test_WallLinePlanAcceptedReturnsTheCells(FUnitTest& T)
{
	TArray<FIntPoint> None;
	FVector A = HOLWallLine::CellToPoint(FIntPoint(2, 3), WallLineTile, 0.0);
	FVector B = HOLWallLine::CellToPoint(FIntPoint(6, 3), WallLineTile, 0.0);
	HOLWallLine::FHOLWallLinePlan Expected = HOLWallLine::Validate(FIntPoint(2, 3), FIntPoint(6, 3), WallLineMapMin, WallLineMapMax, None, 16);
	HOLWallLine::FHOLWallLinePlan Plan = HOLWallLine::PlanFromRequest(0, A, B, WallLineTile, WallLineMapMin, WallLineMapMax, None, 16);
	T.AssertTrue(Plan.Refusal == HOLWallLine::EHOLWallLineRefusal::Ok, "a free line in the map: accepted");
	T.AssertEquals(Expected.Cells.Num(), Plan.Cells.Num(), "the plan has as many cells as Validate's");
	T.AssertTrue(Plan.Cells.Num() == 5 && WallLineHasCell(Plan.Cells, 2, 3) && WallLineHasCell(Plan.Cells, 6, 3), "cells 2..6 on row 3, both ends included");
}

// A point is mapped to the cell it lies in (floor), not the nearest centre: 399 is still in cell 0, 1201 is in cell 3.
void Test_WallLinePlanFloorsWorldPointsToCells(FUnitTest& T)
{
	TArray<FIntPoint> None;
	HOLWallLine::FHOLWallLinePlan Plan = HOLWallLine::PlanFromRequest(0, FVector(399.0, 10.0, 0.0), FVector(1201.0, 10.0, 0.0), WallLineTile, WallLineMapMin, WallLineMapMax, None, 16);
	T.AssertTrue(Plan.Refusal == HOLWallLine::EHOLWallLineRefusal::Ok && Plan.Cells.Num() == 4, "cell 0 to cell 3 on row 0: 4 cells");
	T.AssertTrue(WallLineHasCell(Plan.Cells, 0, 0) && WallLineHasCell(Plan.Cells, 3, 0), "first cell 0, last cell 3");
}

// The refusals of Validate reach the player through the request: too long, out of the map; an occupied cell is skipped (D901), not refused.
void Test_WallLinePlanCarriesTheRefusals(FUnitTest& T)
{
	TArray<FIntPoint> None;
	TArray<FIntPoint> Taken;
	Taken.Add(FIntPoint(4, 3));
	FVector A = HOLWallLine::CellToPoint(FIntPoint(2, 3), WallLineTile, 0.0);
	FVector B = HOLWallLine::CellToPoint(FIntPoint(6, 3), WallLineTile, 0.0);
	HOLWallLine::FHOLWallLinePlan Occupied = HOLWallLine::PlanFromRequest(0, A, B, WallLineTile, WallLineMapMin, WallLineMapMax, Taken, 16);
	T.AssertTrue(Occupied.Refusal == HOLWallLine::EHOLWallLineRefusal::Ok && Occupied.Cells.Num() == 4 && Occupied.Skipped.Num() == 1, "a building on the line: its cell is skipped, the 4 others are laid");
	HOLWallLine::FHOLWallLinePlan Long = HOLWallLine::PlanFromRequest(0, A, B, WallLineTile, WallLineMapMin, WallLineMapMax, None, 3);
	T.AssertTrue(Long.Refusal == HOLWallLine::EHOLWallLineRefusal::TooLong, "5 cells with a cap of 3: refused");
	FVector Out = HOLWallLine::CellToPoint(FIntPoint(65, 3), WallLineTile, 0.0);
	FVector Edge = HOLWallLine::CellToPoint(FIntPoint(62, 3), WallLineTile, 0.0);
	HOLWallLine::FHOLWallLinePlan Off = HOLWallLine::PlanFromRequest(0, Edge, Out, WallLineTile, WallLineMapMin, WallLineMapMax, None, 16);
	T.AssertTrue(Off.Refusal == HOLWallLine::EHOLWallLineRefusal::OutOfMap, "a line leaving the map: refused");
	HOLWallLine::FHOLWallLinePlan NoGrid = HOLWallLine::PlanFromRequest(0, A, B, 0.0, WallLineMapMin, WallLineMapMax, None, 16);
	T.AssertTrue(NoGrid.Refusal == HOLWallLine::EHOLWallLineRefusal::BadPoint, "a tile size of 0: refused (no division by zero)");
}

// D867: no length cap by default. hol.Wall.MaxLineCells = 0 (its default) or a very large value means no limit; the request rule fed with the cap the server
// reads (HOLWallLine::MaxLineCells()) accepts a 151-cell line, and lays every cell of it (the effect, computed by the rule).
void Test_WallLineDefaultHasNoLengthCap(FUnitTest& T)
{
	TArray<FIntPoint> None;
	FIntPoint Low(-200, -200);
	FIntPoint High(200, 200);
	FVector A = HOLWallLine::CellToPoint(FIntPoint(-75, 3), WallLineTile, 0.0);
	FVector B = HOLWallLine::CellToPoint(FIntPoint(75, 3), WallLineTile, 0.0);
	HOLWallLine::FHOLWallLinePlan Plan = HOLWallLine::PlanFromRequest(0, A, B, WallLineTile, Low, High, None, HOLWallLine::MaxLineCells());
	T.AssertTrue(Plan.Refusal == HOLWallLine::EHOLWallLineRefusal::Ok, "a 151-cell line with the default cap: accepted, not TooLong");
	T.AssertEquals(151, Plan.Cells.Num(), "every cell of the long line is laid");
	T.AssertTrue(WallLineHasCell(Plan.Cells, -75, 3) && WallLineHasCell(Plan.Cells, 75, 3) && WallLineIsContiguous(Plan.Cells), "both ends included, no hole");
}

// D867: the drag shows the whole long line (no cut at 17 cells), the same list the server lays.
void Test_WallLineDefaultDragShowsTheWholeLongLine(FUnitTest& T)
{
	TArray<FIntPoint> None;
	TArray<FIntPoint> Shown = HOLWallLine::DragCells(FIntPoint(0, 0), FIntPoint(99, 0), HOLWallLine::MaxLineCells());
	HOLWallLine::FHOLWallLinePlan Plan = HOLWallLine::Validate(FIntPoint(0, 0), FIntPoint(99, 0), FIntPoint(-200, -200), FIntPoint(200, 200), None, HOLWallLine::MaxLineCells());
	T.AssertEquals(100, Shown.Num(), "the default drag shows the 100 cells of the line");
	T.AssertEquals(Shown.Num(), Plan.Cells.Num(), "the server lays as many cells as the drag shows");
}

// A cell centre mapped back lands in the same cell, for a negative cell too.
void Test_WallLineCellPointRoundTrip(FUnitTest& T)
{
	TArray<FIntPoint> None;
	FIntPoint Wide(-5, -2);
	FVector P = HOLWallLine::CellToPoint(Wide, WallLineTile, 0.0);
	HOLWallLine::FHOLWallLinePlan Plan = HOLWallLine::PlanFromRequest(0, P, P, WallLineTile, FIntPoint(-100, -100), FIntPoint(100, 100), None, 16);
	T.AssertTrue(Plan.Refusal == HOLWallLine::EHOLWallLineRefusal::Ok && Plan.Cells.Num() == 1 && WallLineHasCell(Plan.Cells, -5, -2), "the centre of cell (-5,-2) is read back as (-5,-2)");
}

// ---- The drag gesture shows the cells the server lays (F4 client). The shown list comes from DragCells, the laid list from Validate.
void Test_WallLineDragShowsTheCellsValidateLays(FUnitTest& T)
{
	TArray<FIntPoint> None;
	FIntPoint Origin(2, 3);
	TArray<FIntPoint> Ends;
	Ends.Add(FIntPoint(7, 3));
	Ends.Add(FIntPoint(6, 7));
	Ends.Add(FIntPoint(7, 4));
	for (int Case = 0; Case < 3; Case++)
	{
		HOLWallLine::FHOLWallLinePlan Plan = HOLWallLine::Validate(Origin, Ends[Case], FIntPoint(-100, -100), FIntPoint(100, 100), None, 16);
		TArray<FIntPoint> Shown = HOLWallLine::DragCells(Origin, Ends[Case], 16);
		T.AssertTrue(Plan.Refusal == HOLWallLine::EHOLWallLineRefusal::Ok && Plan.Cells.Num() > 1, "the drag is accepted and lays several cells");
		T.AssertEquals(Plan.Cells.Num(), Shown.Num(), "the drag shows as many cells as Validate lays");
		bool bSame = Shown.Num() == Plan.Cells.Num();
		for (int Index = 0; bSame && Index < Shown.Num(); Index++)
			bSame = Shown[Index].X == Plan.Cells[Index].X && Shown[Index].Y == Plan.Cells[Index].Y;
		T.AssertTrue(bSame, "the shown cells are the laid cells, in the same order");
	}
}

// A line too long is refused: the drag shows one cell past the cap, and that cell is the one Validate names.
void Test_WallLineDragOfATooLongLineEndsAtTheRefusedCell(FUnitTest& T)
{
	TArray<FIntPoint> None;
	HOLWallLine::FHOLWallLinePlan Plan = HOLWallLine::Validate(FIntPoint(0, 0), FIntPoint(12, 0), FIntPoint(-100, -100), FIntPoint(100, 100), None, 4);
	TArray<FIntPoint> Shown = HOLWallLine::DragCells(FIntPoint(0, 0), FIntPoint(12, 0), 4);
	T.AssertTrue(Plan.Refusal == HOLWallLine::EHOLWallLineRefusal::TooLong, "a 13-cell drag with a cap of 4 is refused");
	T.AssertEquals(5, Shown.Num(), "the drag shows the cap plus the one refused cell, not the whole 13 cells");
	T.AssertTrue(Shown.Num() == 5 && Shown[4].X == Plan.BadCell.X && Shown[4].Y == Plan.BadCell.Y, "the last shown cell is the refused cell");
}

// From the world points the client holds: the same cells as the server reads from the same two points.
void Test_WallLineDragFromWorldPointsMatchesThePlanOfTheRequest(FUnitTest& T)
{
	TArray<FIntPoint> None;
	FVector A = HOLWallLine::CellToPoint(FIntPoint(-3, 5), WallLineTile, 0.0);
	FVector B = HOLWallLine::CellToPoint(FIntPoint(4, 5), WallLineTile, 0.0);
	HOLWallLine::FHOLWallLinePlan Plan = HOLWallLine::PlanFromRequest(0, A, B, WallLineTile, FIntPoint(-100, -100), FIntPoint(100, 100), None, 16);
	TArray<FIntPoint> Shown = HOLWallLine::DragCellsFromPoints(A, B, WallLineTile, 16);
	T.AssertTrue(Plan.Refusal == HOLWallLine::EHOLWallLineRefusal::Ok, "the request is accepted");
	T.AssertEquals(Plan.Cells.Num(), Shown.Num(), "the drag from the world points shows the cells the request lays");
	bool bSame = Shown.Num() == Plan.Cells.Num();
	for (int Index = 0; bSame && Index < Shown.Num(); Index++)
		bSame = Shown[Index].X == Plan.Cells[Index].X && Shown[Index].Y == Plan.Cells[Index].Y;
	T.AssertTrue(bSame, "the same cells in the same order");
}

// S3-b: F5 gave the drag its first wall type. WallTypes() lists the Palisade, and every listed name is a real row of the building type table
// (read as the simulation reads it, Tools/Data/BuildingTypes.csv regenerated into BuildingTypes.uasset), so the drag never arms for a name
// the placement could not build. A tower is not a wall (it is placed one click at a time).
void Test_WallTypesListsPalisade(FUnitTest& T)
{
	TArray<FName> Types = HOLWallLine::WallTypes();
	T.AssertTrue(Types.Num() > 0, "the list of wall types is not empty: the Palisade is the first wall type");
	T.AssertTrue(HOLWallLine::IsWallType(n"Palisade"), "Palisade is a wall type");
	T.AssertTrue(!HOLWallLine::IsWallType(NAME_None), "None is not a wall type");
	T.AssertTrue(!HOLWallLine::IsWallType(n"TowerWood"), "a Wood watch tower is not a wall type");
	T.AssertTrue(!HOLWallLine::IsWallType(n"House"), "a House is not a wall type");

	FHOLBuildingTypeRow Row;
	T.AssertTrue(HOLBuildingType::FindRow(n"Palisade", Row), "Palisade is a row of the building type table");
	T.AssertEquals(0, Row.Age, "the Palisade is a tier 0 building (D691)");
	T.AssertTrue(Row.MaxHealth > 0, "the Palisade has hit points");
	T.AssertFalse(Row.Walkable, "a Palisade is not walkable (D692: only the stone rampart becomes walkable, last)");
	FHOLAITypeFigures Figures;
	T.AssertTrue(HOLAIOrder::ReadAIBuildingFigures(n"Palisade", Figures), "the AI reads Palisade as a known building type");
	T.AssertTrue(Figures.CostWood > 0, "the Palisade is paid in wood");
	T.AssertTrue(Figures.BuildTime > 0.f, "the Palisade takes time to build (D867: each cell is a building site)");

	bool bEveryTypeIsARow = true;
	for (int Index = 0; Index < Types.Num(); Index++)
	{
		FHOLBuildingTypeRow Listed;
		if (!HOLBuildingType::FindRow(Types[Index], Listed))
			bEveryTypeIsARow = false;
	}
	T.AssertTrue(bEveryTypeIsARow, "every wall type of the list is a row of the building type table");
}

// F9: the palisade gate is a row of the table (Tools\Data\BuildingTypes.csv regenerated into the .uasset), tier 0, paid in wood, solid by default.
void Test_PalisadeGateIsATableRow(FUnitTest& T)
{
	T.AssertTrue(HOLWallLine::IsGateType(HOLWallLine::GateTypeName()), "the gate type name is a gate type");
	T.AssertTrue(!HOLWallLine::IsGateType(n"Palisade"), "a plain palisade is not a gate");
	T.AssertTrue(!HOLWallLine::IsWallType(HOLWallLine::GateTypeName()), "a gate is not dragged as a line of wall: it is placed one at a time");
	FHOLBuildingTypeRow Row;
	T.AssertTrue(HOLBuildingType::FindRow(HOLWallLine::GateTypeName(), Row), "the gate is a row of the building type table");
	T.AssertEquals(0, Row.Age, "the palisade gate is a tier 0 building");
	T.AssertTrue(Row.MaxHealth > 0, "the gate has hit points");
	T.AssertFalse(Row.Walkable, "the gate is not a walkable rampart");
	FHOLAITypeFigures Figures;
	T.AssertTrue(HOLAIOrder::ReadAIBuildingFigures(HOLWallLine::GateTypeName(), Figures), "the AI reads the gate as a known building type");
	T.AssertTrue(Figures.CostWood > 0, "the gate is paid in wood");
}

// F9: a gate replaces a section of palisade of the same seat, nothing else.
void Test_GateReplacesOnlyOwnPalisadeSection(FUnitTest& T)
{
	T.AssertTrue(HOLWallLine::GateReplaces(n"Palisade", 0, 0), "a gate replaces a palisade section of its own seat");
	T.AssertTrue(!HOLWallLine::GateReplaces(n"Palisade", 1, 0), "a gate does not replace an enemy or foreign palisade");
	T.AssertTrue(!HOLWallLine::GateReplaces(n"House", 0, 0), "a gate does not replace a House");
	T.AssertTrue(!HOLWallLine::GateReplaces(HOLWallLine::GateTypeName(), 0, 0), "a gate does not replace another gate");
	T.AssertTrue(!HOLWallLine::GateReplaces(NAME_None, 0, 0), "a gate does not replace nothing");
}

// F9: the gate lets the owner's units through, no other seat's, and a plain palisade lets nobody through.
void Test_GatePassesOnlyTheOwnersUnits(FUnitTest& T)
{
	T.AssertTrue(HOLWallLine::GatePassesUnit(HOLWallLine::GateTypeName(), 0, 0), "the gate lets its owner's unit through");
	T.AssertTrue(!HOLWallLine::GatePassesUnit(HOLWallLine::GateTypeName(), 0, 1), "the gate stops another seat's unit");
	T.AssertTrue(!HOLWallLine::GatePassesUnit(n"Palisade", 0, 0), "a plain palisade lets nobody through, not even its owner");
}

// F10: the stone wall is a row of the table (D691: Mur de pierre, tier 1, paid in stone), a wall type, solid and sturdier than the palisade.
void Test_StoneWallIsATableRow(FUnitTest& T)
{
	T.AssertTrue(HOLWallLine::IsWallType(n"StoneWall"), "the stone wall is a wall type (it is dragged as a line)");
	FHOLBuildingTypeRow Row;
	T.AssertTrue(HOLBuildingType::FindRow(n"StoneWall", Row), "the stone wall is a row of the building type table");
	FHOLBuildingTypeRow Wood;
	T.AssertTrue(HOLBuildingType::FindRow(n"Palisade", Wood), "the palisade row is there to compare with");
	T.AssertEquals(1, Row.Age, "the stone wall is a tier 1 building (D691)");
	T.AssertTrue(Row.MaxHealth > Wood.MaxHealth, "a stone wall has more hit points than a palisade");
	T.AssertFalse(Row.Walkable, "the stone wall is not walkable yet (D692: the walkable rampart comes last)");
	FHOLAITypeFigures Figures;
	T.AssertTrue(HOLAIOrder::ReadAIBuildingFigures(n"StoneWall", Figures), "the AI reads the stone wall as a known building type");
	T.AssertTrue(Figures.CostStone > 0 && Figures.CostWood == 0, "the stone wall is paid in stone alone");
	T.AssertTrue(Figures.BuildTime > 0.f, "the stone wall takes time to build (D867: each cell is a building site)");
}

// F10: the wooden gate only replaces a section of wooden palisade (D691: the stone gate is another building), never a stone wall.
void Test_WoodGateDoesNotReplaceStoneWall(FUnitTest& T)
{
	T.AssertTrue(!HOLWallLine::GateReplaces(n"StoneWall", 0, 0), "the wooden gate does not replace a stone wall section");
	T.AssertTrue(HOLWallLine::GateReplaces(n"Palisade", 0, 0), "the wooden gate still replaces its own palisade section");
}

// F4 (ARWEN-1123pe, the user's bug of 2026-11-23): the released line must send its Peasants to build. Two selected Peasants and a line of 3 cells: each of the 3
// cells (the plan computed by the rule) gives its builders to BOTH Peasants (6 orders), the first cell obeys the player's Shift, the next ones queue behind it.
// The RPC itself needs a seat and a world (not reachable from a script test): ServerPlaceWallLine calls exactly these two rules.
void Test_WallLineBuildersGetOrder(FUnitTest& T)
{
	TArray<FIntPoint> None;
	FVector A = HOLWallLine::CellToPoint(FIntPoint(2, 3), WallLineTile, 0.0);
	FVector B = HOLWallLine::CellToPoint(FIntPoint(4, 3), WallLineTile, 0.0);
	HOLWallLine::FHOLWallLinePlan Plan = HOLWallLine::PlanFromRequest(0, A, B, WallLineTile, WallLineMapMin, WallLineMapMax, None, 16);
	T.AssertEquals(3, Plan.Cells.Num(), "a line of 3 cells: 3 foundations");
	TArray<uint32> Peasants;
	Peasants.Add(11);
	Peasants.Add(12);
	int Orders = 0;
	for (int Index = 0; Index < Plan.Cells.Num(); Index++)
	{
		TArray<uint32> Builders = HOLWallLine::BuildersForCell(Index, Peasants);
		for (int Seat = 0; Seat < Peasants.Num(); Seat++)
		{
			if (Builders.Contains(Peasants[Seat]))
				Orders++;
		}
	}
	T.AssertEquals(6, Orders, "3 foundations x 2 Peasants: every selected Peasant has a build order on every foundation");
	bool bFirst = HOLWallLine::QueueForCell(0, false);
	bool bSecond = HOLWallLine::QueueForCell(1, false);
	bool bThird = HOLWallLine::QueueForCell(2, false);
	T.AssertFalse(bFirst, "no Shift: the first foundation takes the Peasants off what they were doing");
	T.AssertTrue(bSecond && bThird, "the next foundations are queued behind the first (the band is built in the order of the drag)");
	bool bKept = HOLWallLine::QueueForCell(0, true);
	T.AssertTrue(bKept, "Shift: the first foundation is queued too");
}

// A line laid with nobody selected still lays its foundations (as a single building without selection): no builder, no order.
void Test_WallLineNoSelectionNoBuilders(FUnitTest& T)
{
	TArray<uint32> Nobody;
	TArray<uint32> Builders = HOLWallLine::BuildersForCell(1, Nobody);
	T.AssertEquals(0, Builders.Num(), "nobody selected: no builder is named");
}

// D901 (user's answer B): a building on the line no longer cancels it. The free cells are laid (and paid), the occupied cell is a hole listed in Skipped.
void Test_WallLineValidateOccupiedCellIsSkippedNotRefused(FUnitTest& T)
{
	TArray<FIntPoint> Taken;
	Taken.Add(FIntPoint(2, 0));
	HOLWallLine::FHOLWallLinePlan Plan = HOLWallLine::Validate(FIntPoint(0, 0), FIntPoint(4, 0), WallLineMapMin, WallLineMapMax, Taken, 16);
	T.AssertTrue(Plan.Refusal == HOLWallLine::EHOLWallLineRefusal::Ok, "one building on a 5-cell line: not refused");
	T.AssertEquals(4, Plan.Cells.Num(), "the 4 free cells are laid");
	T.AssertFalse(WallLineHasCell(Plan.Cells, 2, 0), "the occupied cell is not laid");
	T.AssertTrue(WallLineHasCell(Plan.Cells, 0, 0) && WallLineHasCell(Plan.Cells, 1, 0) && WallLineHasCell(Plan.Cells, 3, 0) && WallLineHasCell(Plan.Cells, 4, 0), "the cells around the hole are laid, in order from the start");
	T.AssertEquals(1, Plan.Skipped.Num(), "one cell skipped");
	T.AssertTrue(WallLineHasCell(Plan.Skipped, 2, 0), "the skipped cell is the occupied one (the client draws it red)");
}

// Every cell of the line occupied: nothing to lay, the line is refused as Occupied (nothing laid, nothing paid).
void Test_WallLineValidateAllCellsOccupiedLaysNothing(FUnitTest& T)
{
	TArray<FIntPoint> Taken;
	for (int X = 0; X <= 2; X++)
		Taken.Add(FIntPoint(X, 0));
	HOLWallLine::FHOLWallLinePlan Plan = HOLWallLine::Validate(FIntPoint(0, 0), FIntPoint(2, 0), WallLineMapMin, WallLineMapMax, Taken, 16);
	T.AssertTrue(Plan.Refusal == HOLWallLine::EHOLWallLineRefusal::Occupied, "all 3 cells occupied: refused");
	T.AssertEquals(0, Plan.Cells.Num(), "nothing is laid");
}

// The cost and the order of the Peasants follow the cells actually laid: a hole at the start shifts the first order to the next free cell.
void Test_WallLineSkippedCellIsNotPaidNorQueued(FUnitTest& T)
{
	TArray<FIntPoint> Taken;
	Taken.Add(FIntPoint(0, 0));
	Taken.Add(FIntPoint(3, 0));
	HOLWallLine::FHOLWallLinePlan Plan = HOLWallLine::Validate(FIntPoint(0, 0), FIntPoint(5, 0), WallLineMapMin, WallLineMapMax, Taken, 16);
	T.AssertEquals(4, Plan.Cells.Num(), "6 cells, 2 occupied: 4 laid");
	T.AssertEquals(2, Plan.Skipped.Num(), "two holes");
	T.AssertEquals(4 * 7, Plan.Cells.Num() * 7, "the price is the laid cells times the price of one cell");
	T.AssertTrue(Plan.Cells[0].X == 1, "the first laid cell is the first free one (the first order, index 0, obeys Shift)");
	T.AssertFalse(HOLWallLine::QueueForCell(0, false), "the first LAID cell is not queued without Shift");
}