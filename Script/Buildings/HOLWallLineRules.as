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

// D898 (fine grid, G1): the pitch of a wall line is Tile / SubDivision. 1 = the 4 m grid (default until G10 validates the fine grid), 4 = 1 m sub-cells.
const FConsoleVariable CVar_WallSubDivision("hol.Wall.SubDivision", 1);
// D898 G7 (Q-mur-diagonale.md, not answered): 1 (the default) = a diagonal line of sub-cells gets a filler piece at each corner so no unit slips between two corner-touching pieces; 0 = the bare diagonal.
const FConsoleVariable CVar_WallDiagonalFill("hol.Wall.DiagonalFill", 1);

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

	// ---- Server validation of a line (F4): the same Cells rule, then the refusals. Too long and out of the map: all or nothing, no cell is laid. Occupied (D901): the occupied cells are skipped, the others are laid.
	// Order of the gates: too long (the line BEFORE the cap cut is longer than MaxCells, so a drag is refused, not trimmed), out of the map
	// (MapMin and MapMax are the first and last valid cell, both included), occupied (EVERY cell of the line is in Occupied; a partial overlap only skips those cells). The values of the
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
		// D901: the cells of the line left out because a building stands on them (holes: not laid, not paid).
		TArray<FIntPoint> Skipped;
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

		// D901 (the user's answer B): an occupied cell is a hole, not a refusal. The free cells are laid (and paid: the cost is the laid cells), the occupied ones
		// go to Skipped (the client draws them red). Every cell occupied: nothing to lay, the line is refused as Occupied (BadCell = the first of them).
		for (int Index = 0; Index < Line.Num(); Index++)
		{
			bool bTaken = false;
			for (int Taken = 0; Taken < Occupied.Num(); Taken++)
			{
				if (Occupied[Taken].X == Line[Index].X && Occupied[Taken].Y == Line[Index].Y)
				{
					bTaken = true;
					break;
				}
			}
			if (bTaken)
				Plan.Skipped.Add(Line[Index]);
			else
				Plan.Cells.Add(Line[Index]);
		}
		if (Plan.Cells.Num() == 0 && Plan.Skipped.Num() > 0)
		{
			Plan.Refusal = EHOLWallLineRefusal::Occupied;
			Plan.BadCell = Plan.Skipped[0];
		}
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

	// F4 (ARWEN-1123pe; the user's bug of 2026-11-23, wall_line_long: "when I release the line they do not build"): the Peasants a line's cell is given to,
	// and whether the order is queued. One rule for ServerPlaceWallLine (HOLRpcPlayer.as), as ServerPlaceBuilding sends its UnitIds with one building.
	// Rule not decided by the GDD (3348 says only "several Peasants in parallel"): every selected Peasant gets every cell, the first cell obeys the Shift
	// of the player, the next cells queue behind it (so the band is built in the order of the drag). See Docs\Rapports\ARWEN-1123pe.md.
	TArray<uint32> BuildersForCell(int CellIndex, const TArray<uint32>& UnitIds)
	{
		return UnitIds;
	}

	bool QueueForCell(int CellIndex, bool bQueue)
	{
		return bQueue || CellIndex > 0;
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

	// D898 G2: every occupied 4 m tile becomes SubDivision x SubDivision sub-cells (tile (2,0) at 4 = the sub-cells 8..11 x 0..3). SubDivision 1 or less: the tiles as they are.
	TArray<FIntPoint> ExpandOccupied(const TArray<FIntPoint>& OccupiedTiles, int SubDivision)
	{
		TArray<FIntPoint> Result;
		if (SubDivision <= 1)
		{
			Result = OccupiedTiles;
			return Result;
		}
		for (int Tile = 0; Tile < OccupiedTiles.Num(); Tile++)
		{
			for (int SubX = 0; SubX < SubDivision; SubX++)
			{
				for (int SubY = 0; SubY < SubDivision; SubY++)
					Result.Add(FIntPoint(OccupiedTiles[Tile].X * SubDivision + SubX, OccupiedTiles[Tile].Y * SubDivision + SubY));
			}
		}
		return Result;
	}

	// D898 G3: the ground footprint of the wall piece of a cell, in world units: [Cell x Pitch, Cell x Pitch + Pitch] on both axes (grid origin at the world origin, as CellToPoint).
	// The boxes of neighbouring cells touch without overlapping, so the first box of a line starts under the mouse cell and the last one ends under the end cell.
	struct FHOLWallPieceBox
	{
		double MinX = 0.0;
		double MinY = 0.0;
		double MaxX = 0.0;
		double MaxY = 0.0;
	}

	FHOLWallPieceBox PieceBox(FIntPoint Cell, double Pitch)
	{
		FHOLWallPieceBox Box;
		Box.MinX = Cell.X * Pitch;
		Box.MinY = Cell.Y * Pitch;
		Box.MaxX = Box.MinX + Pitch;
		Box.MaxY = Box.MinY + Pitch;
		return Box;
	}

	// ---- Fine grid (D898, G1, PLAN-GRILLE-FINE-1124.md): the line on a pitch of Tile / SubDivision (4 m tile, SubDivision 4 = 1 m cells). Cells are then sub-cells.
	// MapMin / MapMax stay in 4 m tiles (the caller does not change): the last valid tile holds SubDivision sub-cells. SubDivision 1 or less = the 4 m plan, unchanged.
	// Occupied comes in 4 m tiles (G2: ExpandOccupied makes sub-cells of them); the wall pieces already laid are seen as 4 m tiles until G4.
	FHOLWallLinePlan PlanFromRequestFine(int SenderSlot, FVector Start, FVector End, double Tile, int SubDivision, FIntPoint MapMin, FIntPoint MapMax, const TArray<FIntPoint>& Occupied, int MaxCells)
	{
		if (SubDivision <= 1)
			return PlanFromRequest(SenderSlot, Start, End, Tile, MapMin, MapMax, Occupied, MaxCells);
		const TArray<FIntPoint> FineOccupied = ExpandOccupied(Occupied, SubDivision);
		const FIntPoint FineMin = FIntPoint(MapMin.X * SubDivision, MapMin.Y * SubDivision);
		const FIntPoint FineMax = FIntPoint(MapMax.X * SubDivision + SubDivision - 1, MapMax.Y * SubDivision + SubDivision - 1);
		FHOLWallLinePlan FinePlan = PlanFromRequest(SenderSlot, Start, End, Tile / double(SubDivision), FineMin, FineMax, FineOccupied, MaxCells);
		if (FinePlan.Refusal == EHOLWallLineRefusal::Ok && CVar_WallDiagonalFill.GetInt() != 0)
			{
				const double FineTile = Tile / double(SubDivision);
				const FIntPoint FineStart = FIntPoint(CellOfCoordinate(Start.X, FineTile), CellOfCoordinate(Start.Y, FineTile));
				const FIntPoint FineEnd = FIntPoint(CellOfCoordinate(End.X, FineTile), CellOfCoordinate(End.Y, FineTile));
				FillDiagonalGaps(FinePlan, DragCells(FineStart, FineEnd, MaxCells), FineOccupied, MaxCells);
			}
		return FinePlan;
	}

	// D898 G7: between two consecutive pieces that touch only by a corner (one step on both axes) a filler piece is laid at (next.X, previous.Y), so the line is edge-connected and no unit slips through the slit.
	// The filler lies inside the box of the two pieces, so it is inside the map when they are. An occupied filler is skipped like any occupied cell (D901) and listed in Skipped. The cap counts the drag
	// pieces only (the check came before): a diagonal can hold up to twice that many pieces.
	// D898 G8: the cap is recounted AFTER the fill (a diagonal holds up to twice its drag pieces): over MaxCells the line is refused TooLong and lays nothing, as Validate refuses a long line.
	void FillDiagonalGaps(FHOLWallLinePlan& Plan, const TArray<FIntPoint>& FullLine, const TArray<FIntPoint>& FineOccupied, int MaxCells)
	{
		// The fill runs on the WHOLE line (occupied cells included), then the occupied pieces (line cells and fillers alike) go to Skipped: a filler next to a hole is not lost.
		Plan.Skipped.Empty();
		Plan.Cells = FillLine(FullLine, FineOccupied, Plan.Skipped);
		if (Plan.Cells.Num() > MaxCells)
		{
			Plan.Refusal = EHOLWallLineRefusal::TooLong;
			Plan.BadCell = Plan.Cells[MaxCells];
			Plan.Cells.Empty();
		}
	}

	// The one fill rule (server plan and client drag): the line with a filler at each corner-only step; an occupied piece (line cell or filler) goes to Skipped, not to the result.
	TArray<FIntPoint> FillLine(const TArray<FIntPoint>& Line, const TArray<FIntPoint>& FineOccupied, TArray<FIntPoint>& Skipped)
	{
		TArray<FIntPoint> Filled;
		for (int Index = 0; Index < Line.Num(); Index++)
		{
			bool bLineTaken = false;
			for (int Held = 0; Held < FineOccupied.Num(); Held++)
			{
				if (FineOccupied[Held] == Line[Index])
					bLineTaken = true;
			}
			if (Index > 0)
			{
				const FIntPoint Previous = Line[Index - 1];
				const FIntPoint Current = Line[Index];
				const int StepX = Current.X - Previous.X;
				const int StepY = Current.Y - Previous.Y;
				if ((StepX == 1 || StepX == -1) && (StepY == 1 || StepY == -1))
				{
					const FIntPoint Filler = FIntPoint(Current.X, Previous.Y);
					bool bTaken = false;
					for (int Taken = 0; Taken < FineOccupied.Num(); Taken++)
					{
						if (FineOccupied[Taken] == Filler)
							bTaken = true;
					}
					if (bTaken)
						Skipped.Add(Filler);
					else
						Filled.Add(Filler);
				}
			}
			if (bLineTaken)
				Skipped.Add(Line[Index]);
			else
				Filled.Add(Line[Index]);
		}
		return Filled;
	}

	// True when the diagonal fill is on (hol.Wall.DiagonalFill), read by the ghost so it follows the server.
	bool FillIsOn()
	{
		return CVar_WallDiagonalFill.GetInt() != 0;
	}

	// The setting read by the callers (D898, Q-grille-fine-structure.md, not answered): 1 (the default) = the 4 m grid of today, 4 = 1 m sub-cells. The default moves to 4 at G10.
	int SubDivision()
	{
		const int Value = CVar_WallSubDivision.GetInt();
		return Value < 1 ? 1 : Value;
	}

	// D898 G5 (PLAN-GRILLE-FINE-1124.md): the server orders of a line: one place-building order per cell of the plan, each carrying the Peasants of BuildersForCell and the queue flag of QueueForCell.
	// The one rule for ServerPlaceWallLine (HOLRpcPlayer.as) at any SubDivision: a cell is a sub-cell when SubDivision > 1.
	struct FHOLWallOrder
	{
		FIntPoint Cell;
		TArray<uint32> Builders;
		bool bQueue = false;
	}

	TArray<FHOLWallOrder> OrdersForLine(const FHOLWallLinePlan& Plan, const TArray<uint32>& UnitIds, bool bQueue)
	{
		TArray<FHOLWallOrder> Orders;
		for (int Index = 0; Index < Plan.Cells.Num(); Index++)
		{
			FHOLWallOrder Order;
			Order.Cell = Plan.Cells[Index];
			Order.Builders = BuildersForCell(Index, UnitIds);
			Order.bQueue = QueueForCell(Index, bQueue);
			Orders.Add(Order);
		}
		return Orders;
	}
	// ---- Drag gesture (F4, client): the cells the drag SHOWS. One source with Validate (it lays exactly these cells when the line is accepted); the C++
	// controller reads them through UHOLWallLineBridge (HOLWallLineBridge.as), it holds no copy of the rule. STUB (red first): empty.
	// The wall types the drag is armed for (S3-b): the rows of BuildingTypes.csv placed as a line. The Palisade is the first; the stone Wall and
	// the gates (F9 / F10) join this list when their rows exist.
	TArray<FName> WallTypes()
	{
		TArray<FName> Types;
		Types.Add(n"Palisade");
		Types.Add(n"StoneWall");
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
		return DragCellsFine(FIntPoint(CellOfCoordinate(Start.X, Tile), CellOfCoordinate(Start.Y, Tile)), FIntPoint(CellOfCoordinate(End.X, Tile), CellOfCoordinate(End.Y, Tile)), MaxCells, SubDivision());
	}

	// D898 G8: the drag the client shows on the fine grid = the cells the server lays (same FillLine), bounded to one piece past the cap.
	TArray<FIntPoint> DragCellsFine(FIntPoint Start, FIntPoint End, int MaxCells, int SubDivision)
	{
		TArray<FIntPoint> Line = DragCells(Start, End, MaxCells);
		if (SubDivision <= 1 || CVar_WallDiagonalFill.GetInt() == 0)
			return Line;
		TArray<FIntPoint> NoOccupied;
		TArray<FIntPoint> NoSkipped;
		TArray<FIntPoint> Filled = FillLine(Line, NoOccupied, NoSkipped);
		TArray<FIntPoint> Shown;
		for (int Index = 0; Index < Filled.Num() && Index <= MaxCells; Index++)
			Shown.Add(Filled[Index]);
		return Shown;
	}

	// F9 (S3-b, FortificationsInProgress.md): the palisade GATE. PROVISIONAL, no GDD value (D867 neighbours only say: gates are solid, units climb stone walls
	// by towers and gates); the figures live in Tools\Data\BuildingTypes.csv, the open points in Retours\Q-porte-palissade.md (default A).
	FName GateTypeName()
	{
		return n"PalisadeGate";
	}

	bool IsGateType(FName TypeName)
	{
		return TypeName == GateTypeName();
	}

	// A gate is placed ON a section of palisade already standing and replaces it: the section is a wall type of the same seat, and not a gate already.
	bool GateReplaces(FName ExistingType, int ExistingOwner, int GateOwner)
	{
		return ExistingOwner == GateOwner && ExistingType == n"Palisade";
	}

	// A gate lets its owner's units through (other seats meet a solid building, D691 / D867 neighbours: walls and gates are solid for the enemy).
	bool GatePassesUnit(FName GateType, int GateOwner, int UnitOwner)
	{
		return IsGateType(GateType) && GateOwner >= 0 && GateOwner == UnitOwner;
	}}