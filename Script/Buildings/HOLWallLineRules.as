// Wall / palisade line placement, pure rule (F4, FortificationsInProgress.md; D691 axes and 45-degree diagonals on the 4 m grid).
// From the cell where the drag starts to the cell where it ends: the ordered list of cells to build, start first. Cells are integer grid
// coordinates (one per 4 m tile). No world access: not here the terrain / vision / enemy-centre refusals (D22), the footprint rotation, the
// network, the interface. Tested in Script\Tests\HOLWallLineRules_Test.as.
//
// The line is straight: horizontal, vertical or at 45 degrees. A drag that is on none of them snaps to the nearest of the 8 directions
// (octant limit tan 22.5 deg = 0.4142); the length is the drag projected on that direction, in whole cells (CHOICE TO CONFIRM, F4 does not say).
// The length cap and the cost of one cell are PARAMETERS (their values are the user's choice, not decided here); the cap cuts the line
// (the first cells from the start are kept). A cap of 0 or less keeps nothing.

// The cap of a dragged line in cells (D345 a setting; D867: no cap by default, 0 or less = no limit; every cell of the line is a foundation to build).
const FConsoleVariable CVar_WallMaxLineCells("hol.Wall.MaxLineCells", 0);

namespace HOLWallLine
{
	int SignOf(int Value)
	{
		if (Value > 0)
			return 1;
		return Value < 0 ? -1 : 0;
	}

	TArray<FIntPoint> Cells(FIntPoint Start, FIntPoint End, int MaxCells)
	{
		TArray<FIntPoint> Line;
		if (MaxCells <= 0)
			return Line;

		const int DX = End.X - Start.X;
		const int DY = End.Y - Start.Y;
		const int AX = DX < 0 ? -DX : DX;
		const int AY = DY < 0 ? -DY : DY;
		const int Hi = AX > AY ? AX : AY;
		const int Lo = AX > AY ? AY : AX;

		int StepX = 0;
		int StepY = 0;
		int Steps = 0;
		if (Hi > 0)
		{
			if (Lo * 1000 < Hi * 414)
			{
				// Nearest direction is an axis, along the longer side of the drag.
				if (AX >= AY)
					StepX = SignOf(DX);
				else
					StepY = SignOf(DY);
				Steps = Hi;
			}
			else
			{
				// Nearest direction is a diagonal: one step on each axis, the drag projected on it.
				StepX = SignOf(DX);
				StepY = SignOf(DY);
				Steps = (AX + AY + 1) / 2;
			}
		}

		const int Count = (Steps + 1 < MaxCells) ? Steps + 1 : MaxCells;
		for (int Index = 0; Index < Count; Index++)
			Line.Add(FIntPoint(Start.X + Index * StepX, Start.Y + Index * StepY));
		return Line;
	}

	// Total cost of the line as cut by the cap: the number of cells kept times the price of one cell.
	int Cost(FIntPoint Start, FIntPoint End, int MaxCells, int CostPerCell)
	{
		TArray<FIntPoint> Line = Cells(Start, End, MaxCells);
		return Line.Num() * CostPerCell;
	}

	// ---- Server validation of a line (F4): the same Cells rule, then the refusals. All or nothing: one refusal and no cell is laid.
	// Order of the gates: too long (the line BEFORE the cap cut is longer than MaxCells, so a drag is refused, not trimmed), out of the map
	// (MapMin and MapMax are the first and last valid cell, both included), occupied (a cell of Occupied lies on the line). The values of the
	// cap are PARAMETERS (Retours\Q-mur-ligne-longueur.md): no default is coded here. Pure: the caller reads the world (cells under an existing
	// building, the map size) and passes them in; the terrain, vision and enemy-centre refusals (D22) stay with CheckPlacement.
	enum EHOLWallLineRefusal
	{
		Ok,
		TooLong,
		OutOfMap,
		Occupied,
		// F4 request adaptation: the sender holds no seat (spectator or out of range), a point that is not finite or not on the grid.
		NoSeat,
		BadPoint
	}

	struct FHOLWallLinePlan
	{
		TArray<FIntPoint> Cells;
		EHOLWallLineRefusal Refusal = EHOLWallLineRefusal::Ok;
		// The first cell that caused the refusal (for TooLong: the first cell past the cap); the start cell when none.
		FIntPoint BadCell = FIntPoint(0, 0);
	}

	FHOLWallLinePlan Validate(FIntPoint Start, FIntPoint End, FIntPoint MapMin, FIntPoint MapMax, const TArray<FIntPoint>& Occupied, int MaxCells)
	{
		FHOLWallLinePlan Plan;
		Plan.BadCell = Start;
		if (MaxCells <= 0)
		{
			Plan.Refusal = EHOLWallLineRefusal::TooLong;
			return Plan;
		}

		// One cell more than the cap is enough to know the line is too long, and bounds the work whatever the drag the client sent.
		TArray<FIntPoint> Line = DragCells(Start, End, MaxCells);
		if (Line.Num() > MaxCells)
		{
			Plan.Refusal = EHOLWallLineRefusal::TooLong;
			Plan.BadCell = Line[MaxCells];
			return Plan;
		}

		for (int Index = 0; Index < Line.Num(); Index++)
		{
			const FIntPoint Cell = Line[Index];
			if (Cell.X < MapMin.X || Cell.X > MapMax.X || Cell.Y < MapMin.Y || Cell.Y > MapMax.Y)
			{
				Plan.Refusal = EHOLWallLineRefusal::OutOfMap;
				Plan.BadCell = Cell;
				return Plan;
			}
		}

		for (int Index = 0; Index < Line.Num(); Index++)
		{
			for (int Taken = 0; Taken < Occupied.Num(); Taken++)
			{
				if (Occupied[Taken].X == Line[Index].X && Occupied[Taken].Y == Line[Index].Y)
				{
					Plan.Refusal = EHOLWallLineRefusal::Occupied;
					Plan.BadCell = Line[Index];
					return Plan;
				}
			}
		}

		Plan.Cells = Line;
		return Plan;
	}
	// ---- Request adaptation (F4, D345, D867): the pure part of the server RPC ServerPlaceWallLine. D867: NO length cap by default. The cap is read from the cvar
	// hol.Wall.MaxLineCells: 0 (the default) or less = no limit, any other value is a cap in cells. "No limit" is a cap no drag can reach (the request refuses a point
	// past 100000 tiles, so a line is at most 200001 cells), which keeps Validate / DragCells (cap <= 0 = nothing) and their work bound unchanged.
	const int UnlimitedLineCells = 1000000;

	int MaxLineCells()
	{
		const int Cap = CVar_WallMaxLineCells.GetInt();
		return Cap <= 0 ? UnlimitedLineCells : Cap;
	}

	// The cell a world coordinate lies in: floor(Value / Tile), also for negatives.
	int CellOfCoordinate(double Value, double Tile)
	{
		const double Ratio = Value / Tile;
		int Cell = int(Ratio);
		if (double(Cell) > Ratio)
			Cell -= 1;
		return Cell;
	}

	// The world point of the centre of a cell (grid origin at the world origin, one cell = Tile, as UHOLConstructionSubsystem::SnapFootprint).
	FVector CellToPoint(FIntPoint Cell, double Tile, double Z)
	{
		return FVector((Cell.X + 0.5) * Tile, (Cell.Y + 0.5) * Tile, Z);
	}

	FHOLWallLinePlan PlanFromRequest(int SenderSlot, FVector Start, FVector End, double Tile, FIntPoint MapMin, FIntPoint MapMax, const TArray<FIntPoint>& Occupied, int MaxCells)
	{
		FHOLWallLinePlan Plan;
		Plan.BadCell = FIntPoint(0, 0);
		// The seat: slot out of range or no seat (spectator), the same test as every order RPC.
		if (HOLRpc::UnitOrderRefusal(SenderSlot, 1) != 0)
		{
			Plan.Refusal = EHOLWallLineRefusal::NoSeat;
			return Plan;
		}
		// Finite points on a real grid, and not so far that the cell index leaves the int range (a client can send anything).
		const double Limit = 100000.0 * Tile;
		if (Tile <= 0.0 || !HOLRpc::IsFiniteNumber(float(Start.X)) || !HOLRpc::IsFiniteNumber(float(Start.Y)) || !HOLRpc::IsFiniteNumber(float(End.X)) || !HOLRpc::IsFiniteNumber(float(End.Y))
			|| Math::Abs(Start.X) > Limit || Math::Abs(Start.Y) > Limit || Math::Abs(End.X) > Limit || Math::Abs(End.Y) > Limit)
		{
			Plan.Refusal = EHOLWallLineRefusal::BadPoint;
			return Plan;
		}
		return Validate(FIntPoint(CellOfCoordinate(Start.X, Tile), CellOfCoordinate(Start.Y, Tile)), FIntPoint(CellOfCoordinate(End.X, Tile), CellOfCoordinate(End.Y, Tile)), MapMin, MapMax, Occupied, MaxCells);
	}

	// ---- Drag gesture (F4, client): the cells the drag SHOWS. One source with Validate (it lays exactly these cells when the line is accepted); the C++
	// controller reads them through UHOLWallLineBridge (HOLWallLineBridge.as), it holds no copy of the rule. STUB (red first): empty.
	// The wall types the drag is armed for. Empty until F5 adds the first wall type (Palissade / Mur): then its row name goes here.
	TArray<FName> WallTypes()
	{
		TArray<FName> Types;
		return Types;
	}

	bool IsWallType(FName TypeName)
	{
		TArray<FName> Types = WallTypes();
		for (int Index = 0; Index < Types.Num(); Index++)
		{
			if (Types[Index] == TypeName)
				return true;
		}
		return false;
	}

	// The line BEFORE the cap cut, bounded to one cell past the cap (the drag of a too long line shows its refused cell, and the work stays bounded
	// whatever the drag): exactly the list Validate checks, and lays when it accepts.
	TArray<FIntPoint> DragCells(FIntPoint Start, FIntPoint End, int MaxCells)
	{
		if (MaxCells <= 0)
		{
			TArray<FIntPoint> Nothing;
			return Nothing;
		}
		return Cells(Start, End, MaxCells + 1);
	}

	// The same from the two world points the client holds (the ground under the press and under the cursor), as PlanFromRequest reads them.
	TArray<FIntPoint> DragCellsFromPoints(FVector Start, FVector End, double Tile, int MaxCells)
	{
		if (Tile <= 0.0)
		{
			TArray<FIntPoint> Nothing;
			return Nothing;
		}
		return DragCells(FIntPoint(CellOfCoordinate(Start.X, Tile), CellOfCoordinate(Start.Y, Tile)), FIntPoint(CellOfCoordinate(End.X, Tile), CellOfCoordinate(End.Y, Tile)), MaxCells);
	}
}