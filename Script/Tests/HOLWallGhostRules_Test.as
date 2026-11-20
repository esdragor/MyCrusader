// The wall drag ghost (HOLWallGhostRules.as, F4 client drawing): which cells to show for a given drag. The values are fixtures. The expected cells and
// verdict are read from the server's own rule (HOLWallLine::Validate), never typed by hand.

bool WallGhostSameCells(const TArray<FIntPoint>& A, const TArray<FIntPoint>& B)
{
	if (A.Num() != B.Num())
		return false;
	for (int Index = 0; Index < A.Num(); Index++)
	{
		if (A[Index].X != B[Index].X || A[Index].Y != B[Index].Y)
			return false;
	}
	return true;
}

// The ghost of an accepted drag is the line the release lays, in the same order, in the valid colour.
void Test_WallGhostShowsTheLineTheReleaseLays(FUnitTest& T)
{
	TArray<FIntPoint> None;
	FIntPoint Origin(2, 3);
	TArray<FIntPoint> Ends;
	Ends.Add(FIntPoint(7, 3));
	Ends.Add(FIntPoint(6, 7));
	Ends.Add(FIntPoint(7, 4));
	Ends.Add(FIntPoint(2, 3));
	for (int Case = 0; Case < Ends.Num(); Case++)
	{
		HOLWallLine::FHOLWallLinePlan Plan = HOLWallLine::Validate(Origin, Ends[Case], FIntPoint(-100, -100), FIntPoint(100, 100), None, 16);
		HOLWallGhost::FHOLWallGhost Ghost = HOLWallGhost::Build(Origin, Ends[Case], FIntPoint(-100, -100), FIntPoint(100, 100), None, 16);
		T.AssertTrue(Plan.Refusal == HOLWallLine::EHOLWallLineRefusal::Ok && Plan.Cells.Num() >= 1, "the drag is accepted and lays at least its start cell");
		T.AssertTrue(WallGhostSameCells(Ghost.Cells, Plan.Cells), "the ghost cells are the laid cells, in the same order");
		T.AssertTrue(Ghost.bValid, "an accepted drag is shown valid");
	}
}

// A refused line still shows its cells (the player sees where it breaks), in the refused colour, with the refused cell named as Validate names it.
void Test_WallGhostOfAnOccupiedLineIsRedAndNamesTheCell(FUnitTest& T)
{
	TArray<FIntPoint> Taken;
	Taken.Add(FIntPoint(5, 0));
	HOLWallLine::FHOLWallLinePlan Plan = HOLWallLine::Validate(FIntPoint(0, 0), FIntPoint(8, 0), FIntPoint(-100, -100), FIntPoint(100, 100), Taken, 16);
	HOLWallGhost::FHOLWallGhost Ghost = HOLWallGhost::Build(FIntPoint(0, 0), FIntPoint(8, 0), FIntPoint(-100, -100), FIntPoint(100, 100), Taken, 16);
	T.AssertTrue(Plan.Refusal == HOLWallLine::EHOLWallLineRefusal::Occupied, "Validate refuses a line over an occupied cell");
	T.AssertFalse(Ghost.bValid, "the ghost is shown refused");
	int Shown = Ghost.Cells.Num();
	T.AssertEquals(9, Shown, "the 9 cells of the dragged row are all shown");
	T.AssertEquals(Plan.BadCell.X, Ghost.BadCell.X, "the refused cell is the one Validate names (x)");
	T.AssertEquals(Plan.BadCell.Y, Ghost.BadCell.Y, "the refused cell is the one Validate names (y)");
}

// Past the map edge: refused, the cells up to and beyond the edge are shown.
void Test_WallGhostOutOfTheMapIsRed(FUnitTest& T)
{
	TArray<FIntPoint> None;
	HOLWallLine::FHOLWallLinePlan Plan = HOLWallLine::Validate(FIntPoint(0, 0), FIntPoint(6, 0), FIntPoint(0, 0), FIntPoint(3, 3), None, 16);
	HOLWallGhost::FHOLWallGhost Ghost = HOLWallGhost::Build(FIntPoint(0, 0), FIntPoint(6, 0), FIntPoint(0, 0), FIntPoint(3, 3), None, 16);
	T.AssertTrue(Plan.Refusal == HOLWallLine::EHOLWallLineRefusal::OutOfMap, "Validate refuses a line leaving the map");
	T.AssertFalse(Ghost.bValid, "the ghost is shown refused");
	int Shown = Ghost.Cells.Num();
	T.AssertEquals(7, Shown, "the whole dragged row is shown, edge included");
	T.AssertEquals(Plan.BadCell.X, Ghost.BadCell.X, "the refused cell is the first one out of the map");
}

// Too long: the ghost ends at the cell past the cap (the refused one), as the line DragCells hands to Validate.
void Test_WallGhostTooLongShowsOneCellPastTheCap(FUnitTest& T)
{
	TArray<FIntPoint> None;
	HOLWallLine::FHOLWallLinePlan Plan = HOLWallLine::Validate(FIntPoint(0, 0), FIntPoint(12, 0), FIntPoint(-100, -100), FIntPoint(100, 100), None, 4);
	HOLWallGhost::FHOLWallGhost Ghost = HOLWallGhost::Build(FIntPoint(0, 0), FIntPoint(12, 0), FIntPoint(-100, -100), FIntPoint(100, 100), None, 4);
	T.AssertTrue(Plan.Refusal == HOLWallLine::EHOLWallLineRefusal::TooLong, "Validate refuses a line past the cap");
	T.AssertFalse(Ghost.bValid, "the ghost is shown refused");
	int Shown = Ghost.Cells.Num();
	T.AssertEquals(5, Shown, "the cap (4) plus the refused cell");
	T.AssertEquals(Plan.BadCell.X, Ghost.BadCell.X, "the refused cell is the one past the cap");
}
