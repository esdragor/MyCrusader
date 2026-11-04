// The talent tree window's data (TQ4 = A, the user's decision of 2026-11-01: a window of 5 rows x 3 cells, Guardian / Captain / Champion, names and
// effects on hover; the window is built in UHOLGameScreenScript on these views, opened by the reminder or the T key). Pure, from the level and the picks alone
// (tested in HOLTalentTree_Test.as): the cells the window will draw, row by row (the rows the prototype's maximum level reaches), each with the
// table's name (HOLTalentNames) and its hover text (name, effect, and when the prototype cannot show it yet, that too: D194).

// How a cell stands: taken (the talent chosen in its row), passed (the others of a row already filled), pickable (next row, a point in reserve),
// waiting (next row, no point yet), later (the rows after), locked (the rows past the game's maximum level, shown greyed "plus tard": the
// prototype's window draws rows 7 to 9 so, 2026-11-04).
enum EHOLTalentCellState
{
	Taken,
	Passed,
	Pickable,
	Waiting,
	Later,
	Locked
}

struct FHOLTalentCellView
{
	int Row = 0;
	int Family = 0;
	FString Name;
	// The hover: "<Name> — <effect>" (and "(pas encore actif dans ce prototype)").
	FString Tooltip;
	EHOLTalentCellState State = EHOLTalentCellState::Later;
	bool bPickable = false;
	bool bHasEffect = false;
	// The icon's glyph (no art yet: the first letter of the name, over the family's colour) of the talents taken (TQ5 = A, D618).
	FString Glyph;
}

namespace HOLTalentTree
{
	FText HoverPattern()
	{
		return NSLOCTEXT("HOLTalentTree", "Hover", "{0} — {1}");
	}

	// The cells of a hero of Level who took Picks (the families, row 2 first): row by row, the families in the column order, for a game whose
	// heroes reach MaxLevel at the most: the rows 2 to MaxLevel (the full game's 8 rows, 2 to 9, at 9 or 10; the rows 7 to 9 are D195's).
	TArray<FHOLTalentCellView> MakeFor(int MaxLevel, int Level, const TArray<int>& Picks)
	{
		TArray<FHOLTalentCellView> Cells;
		const int Points = HOLTalent::PointsAvailable(Level, Picks);
		const int NextRow = HOLTalent::NextRow(Picks);
		const int RowsShown = HOLTalent::PointsEarned(MaxLevel);
		for (int Index = 0; Index < RowsShown; Index++)
		{
			const int Row = HOLTalent::FirstRow + Index;
			for (int Family = 0; Family < HOLTalent::FamilyCount; Family++)
			{
				FHOLTalentCellView Cell;
				Cell.Row = Row;
				Cell.Family = Family;
				Cell.Name = HOLTalentNames::Name(Row, Family).ToString();
				Cell.bHasEffect = HOLHero::TalentHasEffect(Row, Family);
				Cell.Tooltip = HoverPattern().ToString().Replace("{0}", Cell.Name).Replace("{1}", HOLTalentText::EffectText(Row, Family));
				if (Index < Picks.Num())
					Cell.State = Picks[Index] == Family ? EHOLTalentCellState::Taken : EHOLTalentCellState::Passed;
				else if (Row == NextRow)
					Cell.State = Points > 0 ? EHOLTalentCellState::Pickable : EHOLTalentCellState::Waiting;
				else
					Cell.State = EHOLTalentCellState::Later;
				Cell.bPickable = Cell.State == EHOLTalentCellState::Pickable;
				Cells.Add(Cell);
			}
		}
		return Cells;
	}

	// "plus tard (niveau N)": what the hover of a locked cell says of its row (the row is the hero level that gives the point).
	FString LaterNote(int Row)
	{
		return NSLOCTEXT("HOLTalentTree", "Later", "plus tard, niveau {0}").ToString().Replace("{0}", f"{Row}");
	}

	// The window's cells: all the rows of the game (2 to 9), those past GameMaxLevel (the prototype's 6) locked: greyed, never pickable (not even
	// "waiting for a point": no level gives it), hover with the table's name and effect and the row's level.
	TArray<FHOLTalentCellView> MakeWithLater(int GameMaxLevel, int Level, const TArray<int>& Picks)
	{
		TArray<FHOLTalentCellView> Cells = MakeFor(HOLTalent::LastRow, Level, Picks);
		for (int Index = 0; Index < Cells.Num(); Index++)
		{
			if (Cells[Index].Row > GameMaxLevel && Cells[Index].State != EHOLTalentCellState::Taken)
			{
				Cells[Index].State = EHOLTalentCellState::Locked;
				Cells[Index].bPickable = false;
				Cells[Index].Tooltip += " (" + LaterNote(Cells[Index].Row) + ")";
			}
		}
		return Cells;
	}

	// The prototype's window: the rows its maximum level reaches (2 to 6).
	TArray<FHOLTalentCellView> Make(int Level, const TArray<int>& Picks)
	{
		return MakeFor(HOLHero::GetMaxLevel(), Level, Picks);
	}

	// The family a click on the cell (Row, Family) asks for: its own column when the cell is pickable, FamilyNone for any other state or a cell
	// outside the window (the server checks the rest, HOLTalent::CanPick).
	int ClickedFamily(const TArray<FHOLTalentCellView>& Cells, int Row, int Family)
	{
		for (int Index = 0; Index < Cells.Num(); Index++)
		{
			if (Cells[Index].Row == Row && Cells[Index].Family == Family)
				return Cells[Index].bPickable ? Cells[Index].Family : HOLTalent::FamilyNone;
		}
		return HOLTalent::FamilyNone;
	}

	// The talents taken stay listed and readable after the purchase (TQ5 = A, D618): one entry per talent taken, the rows in order, each with
	// a glyph for its icon and the hover of its cell in the tree.
	TArray<FHOLTalentCellView> TakenEntries(int Level, const TArray<int>& Picks)
	{
		TArray<FHOLTalentCellView> Taken;
		const TArray<FHOLTalentCellView> Cells = Make(Level, Picks);
		for (int Index = 0; Index < Cells.Num(); Index++)
		{
			if (Cells[Index].State == EHOLTalentCellState::Taken)
			{
				FHOLTalentCellView Entry = Cells[Index];
				Entry.Glyph = Entry.Name.Left(1);
				Taken.Add(Entry);
			}
		}
		return Taken;
	}

	// The stable line of the tests: "row 2: . T . | row 3: ..." (T taken, . passed, P pickable, w waiting, l later, x locked).
	FString Describe(const TArray<FHOLTalentCellView>& Cells)
	{
		FString Text = "";
		for (int Index = 0; Index < Cells.Num(); Index++)
		{
			if (Index % HOLTalent::FamilyCount == 0)
				Text += (Index > 0 ? " | " : "") + f"row {Cells[Index].Row}:";
			const EHOLTalentCellState State = Cells[Index].State;
			Text += State == EHOLTalentCellState::Taken ? " T" : State == EHOLTalentCellState::Passed ? " ." : State == EHOLTalentCellState::Pickable ? " P" : State == EHOLTalentCellState::Waiting ? " w" : State == EHOLTalentCellState::Locked ? " x" : " l";
		}
		return Text;
	}
}
