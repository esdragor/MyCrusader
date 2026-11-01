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
