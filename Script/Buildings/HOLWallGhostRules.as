// The wall drag ghost, pure rule (F4 client drawing, HALDIR-1120pc, D839): which cells the player sees while dragging a wall line, and in
// which colour. One source with the server: the cells are exactly the line HOLWallLine::Validate checks (a refused line still shows its
// cells, so the player sees where it breaks), the colour is Validate's verdict (D901: the release lays the free cells, the occupied cells stay holes, listed in Skipped and drawn red). The drawing
// itself (instanced boxes on the ground) is Script\Buildings\HOLWallGhostDrawer.as. Tested in Script\Tests\HOLWallGhostRules_Test.as.

namespace HOLWallGhost
{
	struct FHOLWallGhost
	{
		// The cells to draw, start first (one past the cap when the line is too long: that cell is the refused one).
		TArray<FIntPoint> Cells;
		// True: the release lays the whole line (green); false: it lays nothing (red).
		bool bValid = true;
		// D901: the occupied cells of the line, left as holes by the release: drawn red inside a valid (green) line.
		TArray<FIntPoint> Skipped;
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
		Ghost.Skipped = Plan.Skipped;
		return Ghost;
	}

	// D898 G8: the ghost on the fine grid draws the filled diagonal the server lays; the cap is counted on it (over the cap = refused, the cell past the cap is the bad one).
	FHOLWallGhost BuildFine(FIntPoint Start, FIntPoint End, FIntPoint MapMin, FIntPoint MapMax, const TArray<FIntPoint>& Occupied, int MaxCells, int SubDivision)
	{
		if (SubDivision <= 1)
		{
			FHOLWallGhost Coarse = Build(Start, End, MapMin, MapMax, Occupied, MaxCells);
			Coarse.Cells = HOLWallLine::DragCellsFine(Start, End, MaxCells, SubDivision);
			if (Coarse.bValid && Coarse.Cells.Num() > MaxCells)
			{
				Coarse.bValid = false;
				Coarse.BadCell = Coarse.Cells[MaxCells];
			}
			return Coarse;
		}
		// Occupied and the map come in 4 m tiles, as for HOLWallLine::PlanFromRequestFine: the sub-cells of the occupied tiles are the occupied cells of the fine line (fillers included).
		const TArray<FIntPoint> FineOccupied = HOLWallLine::ExpandOccupied(Occupied, SubDivision);
		const FIntPoint FineMin = FIntPoint(MapMin.X * SubDivision, MapMin.Y * SubDivision);
		const FIntPoint FineMax = FIntPoint(MapMax.X * SubDivision + SubDivision - 1, MapMax.Y * SubDivision + SubDivision - 1);
		FHOLWallGhost Ghost = Build(Start, End, FineMin, FineMax, FineOccupied, MaxCells);
		Ghost.Cells = HOLWallLine::DragCellsFine(Start, End, MaxCells, SubDivision);
		if (Ghost.bValid && HOLWallLine::FillIsOn())
		{
			// The very fill the server runs (one rule): its holes, fillers on occupied tiles included, are the red pieces.
			TArray<FIntPoint> Holes;
			HOLWallLine::FillLine(HOLWallLine::DragCells(Start, End, MaxCells), FineOccupied, Holes);
			Ghost.Skipped = Holes;
		}
		if (Ghost.bValid && Ghost.Cells.Num() > MaxCells)
		{
			Ghost.bValid = false;
			Ghost.BadCell = Ghost.Cells[MaxCells];
		}
		return Ghost;
	}

	// D898 G8 (CELEBORN-1125md): the ghost from the two ground points at an explicit SubDivision, cells read at the pitch Tile / SubDivision.
	FHOLWallGhost BuildFromPointsAt(FVector Start, FVector End, double Tile, int SubDivision, FIntPoint MapMin, FIntPoint MapMax, const TArray<FIntPoint>& Occupied, int MaxCells)
	{
		if (Tile <= 0.0)
		{
			FHOLWallGhost Nothing;
			Nothing.bValid = false;
			return Nothing;
		}
		const int Divide = SubDivision < 1 ? 1 : SubDivision;
		const double Pitch = Tile / double(Divide);
		return BuildFine(FIntPoint(HOLWallLine::CellOfCoordinate(Start.X, Pitch), HOLWallLine::CellOfCoordinate(Start.Y, Pitch)),
			FIntPoint(HOLWallLine::CellOfCoordinate(End.X, Pitch), HOLWallLine::CellOfCoordinate(End.Y, Pitch)), MapMin, MapMax, Occupied, MaxCells, Divide);
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
		return BuildFromPointsAt(Start, End, Tile, HOLWallLine::SubDivision(), MapMin, MapMax, Occupied, MaxCells);
	}
}
