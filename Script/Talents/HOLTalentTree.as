// The talent tree window's data (TQ4 = A, the user's decision of 2026-11-01: a window of 5 rows x 3 cells, Guardian / Captain / Champion, names and
// effects on hover; the WINDOW ITSELF IS NOT BUILT YET, the family buttons of the hero's block stand in). Pure, from the level and the picks alone
// (tested in HOLTalentTree_Test.as): the cells the window will draw, row by row (the rows the prototype's maximum level reaches), each with the
// table's name (HOLTalentNames) and its hover text (name, effect, and when the prototype cannot show it yet, that too: D194).

// How a cell stands: taken (the talent chosen in its row), passed (the others of a row already filled), pickable (next row, a point in reserve),
// waiting (next row, no point yet), later (the rows after).
enum EHOLTalentCellState
{
	Taken,
	Passed,
	Pickable,
	Waiting,
	Later
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
}

namespace HOLTalentTree
{
	FText HoverPattern()
	{
		return NSLOCTEXT("HOLTalentTree", "Hover", "{0} — {1}");
	}

	// The cells of a hero of Level who took Picks (the families, row 2 first): row by row, the families in the column order.
	TArray<FHOLTalentCellView> Make(int Level, const TArray<int>& Picks)
	{
		TArray<FHOLTalentCellView> Cells;
		const int Points = HOLTalent::PointsAvailable(Level, Picks);
		const int NextRow = HOLTalent::NextRow(Picks);
		const int RowsShown = HOLTalent::PointsEarned(HOLHero::GetMaxLevel());
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

	// The stable line of the tests: "row 2: . T . | row 3: ..." (T taken, . passed, P pickable, w waiting, l later).
	FString Describe(const TArray<FHOLTalentCellView>& Cells)
	{
		FString Text = "";
		for (int Index = 0; Index < Cells.Num(); Index++)
		{
			if (Index % HOLTalent::FamilyCount == 0)
				Text += (Index > 0 ? " | " : "") + f"row {Cells[Index].Row}:";
			const EHOLTalentCellState State = Cells[Index].State;
			Text += State == EHOLTalentCellState::Taken ? " T" : State == EHOLTalentCellState::Passed ? " ." : State == EHOLTalentCellState::Pickable ? " P" : State == EHOLTalentCellState::Waiting ? " w" : " l";
		}
		return Text;
	}
}
