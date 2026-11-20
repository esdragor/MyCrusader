// The wall drag ghost, pure rule (F4 client drawing, HALDIR-1120pc, D839): which cells the player sees while dragging a wall line, and in
// which colour. One source with the server: the cells are exactly the line HOLWallLine::Validate checks (a refused line still shows its
// cells, so the player sees where it breaks), the colour is Validate's verdict (all or nothing, as the release will be). The drawing
// itself (instanced boxes on the ground) is Script\Buildings\HOLWallGhostDrawer.as. Tested in Script\Tests\HOLWallGhostRules_Test.as.

namespace HOLWallGhost
{
	struct FHOLWallGhost
	{
		// The cells to draw, start first (one past the cap when the line is too long: that cell is the refused one).
		TArray<FIntPoint> Cells;
		// True: the release lays the whole line (green); false: it lays nothing (red).
		bool bValid = true;
		// The cell that causes the refusal (the start cell when valid): drawn apart in red.
		FIntPoint BadCell = FIntPoint(0, 0);
	}

	FHOLWallGhost Build(FIntPoint Start, FIntPoint End, FIntPoint MapMin, FIntPoint MapMax, const TArray<FIntPoint>& Occupied, int MaxCells)
	{
		FHOLWallGhost Ghost;
		HOLWallLine::FHOLWallLinePlan Plan = HOLWallLine::Validate(Start, End, MapMin, MapMax, Occupied, MaxCells);
		// DragCells is the very list Validate checks (bounded to one cell past the cap), so a refused line is shown whole.
		Ghost.Cells = HOLWallLine::DragCells(Start, End, MaxCells);
		Ghost.bValid = Plan.Refusal == HOLWallLine::EHOLWallLineRefusal::Ok;
		Ghost.BadCell = Plan.BadCell;
		return Ghost;
	}

	// From the two ground points the client holds (the press and the cursor), as HOLWallLine::PlanFromRequest reads them.
	FHOLWallGhost BuildFromPoints(FVector Start, FVector End, double Tile, FIntPoint MapMin, FIntPoint MapMax, const TArray<FIntPoint>& Occupied, int MaxCells)
	{
		if (Tile <= 0.0)
		{
			FHOLWallGhost Nothing;
			Nothing.bValid = false;
			return Nothing;
		}
		return Build(FIntPoint(HOLWallLine::CellOfCoordinate(Start.X, Tile), HOLWallLine::CellOfCoordinate(Start.Y, Tile)),
			FIntPoint(HOLWallLine::CellOfCoordinate(End.X, Tile), HOLWallLine::CellOfCoordinate(End.Y, Tile)), MapMin, MapMax, Occupied, MaxCells);
	}
}
