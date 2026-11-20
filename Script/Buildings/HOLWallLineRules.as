// Wall / palisade line placement, pure rule (F4, FortificationsInProgress.md; D691 axes and 45-degree diagonals on the 4 m grid).
// From the cell where the drag starts to the cell where it ends: the ordered list of cells to build, start first. Cells are integer grid
// coordinates (one per 4 m tile). No world access: not here the terrain / vision / enemy-centre refusals (D22), the footprint rotation, the
// network, the interface. Tested in Script\Tests\HOLWallLineRules_Test.as.
//
// The line is straight: horizontal, vertical or at 45 degrees. A drag that is on none of them snaps to the nearest of the 8 directions
// (octant limit tan 22.5 deg = 0.4142); the length is the drag projected on that direction, in whole cells (CHOICE TO CONFIRM, F4 does not say).
// The length cap and the cost of one cell are PARAMETERS (their values are the user's choice, not decided here); the cap cuts the line
// (the first cells from the start are kept). A cap of 0 or less keeps nothing.

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
		Occupied
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
		TArray<FIntPoint> Line = Cells(Start, End, MaxCells + 1);
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
	}}
