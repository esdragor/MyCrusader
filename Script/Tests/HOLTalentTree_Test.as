// The talent tree window's data (TQ4 = A, the user's decision: 5 rows x 3 cells, Guardian / Captain / Champion, names and effects on hover;
// HOLTalentTree.as). Pure: from the level and the picks alone, the cells the window will draw. Families: 0 Guardian, 1 Captain, 2 Champion.

TArray<int> TreePicks(int F0 = -1, int F1 = -1, int F2 = -1, int F3 = -1, int F4 = -1)
{
	TArray<int> Picks;
	if (F0 >= 0) Picks.Add(F0);
	if (F1 >= 0) Picks.Add(F1);
	if (F2 >= 0) Picks.Add(F2);
	if (F3 >= 0) Picks.Add(F3);
	if (F4 >= 0) Picks.Add(F4);
	return Picks;
}

// 5 rows (levels 2 to 6 of the prototype) of 3 cells, row by row, the families in the column order; each cell carries the table's name and effect.
void Test_TalentTreeIsFiveRowsOfThree(FUnitTest& T)
{
	const TArray<FHOLTalentCellView> Cells = HOLTalentTree::Make(1, TreePicks());
	T.AssertEquals(5 * 3, Cells.Num(), "5 rows x 3 cells");
	for (int Index = 0; Index < Cells.Num(); Index++)
	{
		T.AssertEquals(HOLTalent::FirstRow + Index / 3, Cells[Index].Row, f"cell {Index}: its row");
		T.AssertEquals(Index % 3, Cells[Index].Family, f"cell {Index}: its column is its family");
		T.AssertEquals(HOLTalentNames::Name(Cells[Index].Row, Cells[Index].Family).ToString(), Cells[Index].Name, f"cell {Index}: the table's name");
		T.AssertTrue(Cells[Index].Tooltip.Contains(Cells[Index].Name), f"cell {Index}: the hover names the talent");
		T.AssertTrue(Cells[Index].Tooltip.Contains(HOLTalentNames::Effect(Cells[Index].Row, Cells[Index].Family).ToString()), f"cell {Index}: the hover says what it does");
	}
}

// The state of each cell: taken (the one chosen in its row), passed (the others of a filled row), pickable (the next row, a point in reserve), waiting (the
// next row, no point yet) or later (the rows after).
void Test_TalentTreeStates(FUnitTest& T)
{
	// Level 3, two taken (Captain, Captain): row 4 is next, no point left.
	const TArray<FHOLTalentCellView> Waiting = HOLTalentTree::Make(3, TreePicks(1, 1));
	T.AssertEquals("row 2: . T . | row 3: . T . | row 4: w w w | row 5: l l l | row 6: l l l", HOLTalentTree::Describe(Waiting), "level 3, two taken");
	// Level 4: the point for row 4 is in reserve; the three cells of row 4 can be taken.
	const TArray<FHOLTalentCellView> Open = HOLTalentTree::Make(4, TreePicks(1, 1));
	T.AssertEquals("row 2: . T . | row 3: . T . | row 4: P P P | row 5: l l l | row 6: l l l", HOLTalentTree::Describe(Open), "level 4, two taken: row 4 open");
	T.AssertTrue(Open[6].bPickable && Open[7].bPickable && Open[8].bPickable, "the three cells of row 4 are pickable");
	T.AssertFalse(Open[0].bPickable, "a cell of a filled row is not");
	// Nothing taken at level 1: row 2 waits.
	T.AssertEquals("row 2: w w w | row 3: l l l | row 4: l l l | row 5: l l l | row 6: l l l", HOLTalentTree::Describe(HOLTalentTree::Make(1, TreePicks())), "level 1");
}

// A cell says whether its talent works in the prototype (D194: nothing is silently dead): Boucliers levés and Pas de l'Aube do, Mains du guérisseur does not.
void Test_TalentTreeCellsSayWhetherTheyWork(FUnitTest& T)
{
	const TArray<FHOLTalentCellView> Cells = HOLTalentTree::Make(1, TreePicks());
	T.AssertTrue(Cells[1].bHasEffect, "row 2 Captain, Boucliers levés, works");
	T.AssertTrue(Cells[7].bHasEffect, "row 4 Captain, Pas de l'Aube, works");
	T.AssertFalse(Cells[4].bHasEffect, "row 3 Captain, Mains du guérisseur, has no Monk yet");
	T.AssertTrue(Cells[4].Tooltip.Contains(HOLTalentNames::NoEffectYet().ToString()), "and its hover says so");
	T.AssertFalse(Cells[1].Tooltip.Contains(HOLTalentNames::NoEffectYet().ToString()), "Boucliers levés' hover does not");
}

// A click on a cell (the window's, TQ4 = A): only a pickable cell sends a request, with its own family column; every other state sends nothing.
void Test_TalentTreeClickRequestsOnlyAPickableCell(FUnitTest& T)
{
	// Level 4, two taken (Captain, Captain): row 4 is open, rows 2 and 3 are filled, rows 5 and 6 are later.
	const TArray<FHOLTalentCellView> Cells = HOLTalentTree::Make(4, TreePicks(1, 1));
	T.AssertEquals(HOLTalent::FamilyNone, HOLTalentTree::ClickedFamily(Cells, 2, 1), "a taken cell sends nothing");
	T.AssertEquals(HOLTalent::FamilyNone, HOLTalentTree::ClickedFamily(Cells, 2, 0), "a passed cell sends nothing");
	T.AssertEquals(0, HOLTalentTree::ClickedFamily(Cells, 4, 0), "row 4, Guardian column: asks for the Guardian");
	T.AssertEquals(1, HOLTalentTree::ClickedFamily(Cells, 4, 1), "row 4, Captain column: asks for the Captain");
	T.AssertEquals(2, HOLTalentTree::ClickedFamily(Cells, 4, 2), "row 4, Champion column: asks for the Champion");
	T.AssertEquals(HOLTalent::FamilyNone, HOLTalentTree::ClickedFamily(Cells, 5, 1), "a later row sends nothing");
	T.AssertEquals(HOLTalent::FamilyNone, HOLTalentTree::ClickedFamily(Cells, 9, 1), "a row outside the window sends nothing");
	// No point in reserve: the next row waits and sends nothing.
	T.AssertEquals(HOLTalent::FamilyNone, HOLTalentTree::ClickedFamily(HOLTalentTree::Make(3, TreePicks(1, 1)), 4, 1), "waiting for a point: nothing");
}

// The talents taken stay listed and readable after the purchase (TQ5 = A, D618): one entry per talent taken, in the order of the rows, each with a
// glyph for its icon, its family and the same hover text as its cell in the tree (name and effect, and "pas encore actif" when it has none).
void Test_TalentTakenEntriesOnePerTalentWithAHover(FUnitTest& T)
{
	T.AssertEquals(0, HOLTalentTree::TakenEntries(1, TreePicks()).Num(), "nothing taken, nothing listed");
	const TArray<int> Picks = TreePicks(1, 1, 1);
	const TArray<FHOLTalentCellView> Cells = HOLTalentTree::Make(6, Picks);
	const TArray<FHOLTalentCellView> Taken = HOLTalentTree::TakenEntries(6, Picks);
	T.AssertEquals(3, Taken.Num(), "one entry per talent taken");
	for (int Index = 0; Index < Taken.Num(); Index++)
	{
		T.AssertEquals(HOLTalent::FirstRow + Index, Taken[Index].Row, f"entry {Index}: the rows in order");
		T.AssertEquals(1, Taken[Index].Family, f"entry {Index}: its family");
		T.AssertTrue(Taken[Index].State == EHOLTalentCellState::Taken, f"entry {Index}: taken");
		T.AssertFalse(Taken[Index].Glyph.IsEmpty(), f"entry {Index}: it has an icon glyph");
		T.AssertEquals(Cells[Index * 3 + 1].Tooltip, Taken[Index].Tooltip, f"entry {Index}: the same hover as its cell in the tree");
		T.AssertTrue(Taken[Index].Tooltip.Contains(Taken[Index].Name), f"entry {Index}: the hover names the talent");
	}
	T.AssertTrue(Taken[1].Tooltip.Contains(HOLTalentNames::NoEffectYet().ToString()), "Mains du guerisseur says it has no effect yet");
	T.AssertFalse(Taken[0].Tooltip.Contains(HOLTalentNames::NoEffectYet().ToString()), "Boucliers leves does not");
}
