// One table of talent names (user's trial of 2026-10-31: "the names are not the same everywhere"): every place that shows a talent
// (the tree window's cells and the icons' hovers, the notice "Talent pris", the list of the talents taken, the reminder's tooltip) takes its name from
// HOLTalentNames (HOLTalentNames.as), and that table is the one of Tools/Data/Talents.csv (read by HOLHero::GetTalentCsvName, the
// dev table of GDD 13.2). Families: 0 Guardian, 1 Captain, 2 Champion; rows 2 to 9.

// The 24 names of the game are the 24 names of the CSV, one by one, and no two are alike.
void Test_TalentNamesAreTheTablesNames(FUnitTest& T)
{
	TArray<FString> Seen;
	for (int Row = HOLTalent::FirstRow; Row < HOLTalent::FirstRow + 8; Row++)
	{
		for (int Family = 0; Family < HOLTalent::FamilyCount; Family++)
		{
			const FString Name = HOLTalentNames::Name(Row, Family).ToString();
			const FString CsvName = HOLHero::GetTalentCsvName(Row, Family);
			T.AssertFalse(CsvName.IsEmpty(), f"the CSV has a name for row {Row} family {Family}");
			T.AssertEquals(CsvName, Name, f"row {Row} family {Family}: the name shown is the table's");
			T.AssertFalse(Seen.Contains(Name), f"{Name} is the name of one talent only");
			Seen.Add(Name);
			T.AssertFalse(HOLTalentNames::Effect(Row, Family).ToString().IsEmpty(), f"{Name} has a readable effect");
		}
	}
}

// The names the hero's block shows come from that same table: the hover of a cell of the tree names its talent, the
// notice names the one just taken, the list names all of them in order, the advised button's tooltip names the one it takes.
void Test_TalentTextsShowTheTablesNames(FUnitTest& T)
{
	TArray<int> Picks;
	Picks.Add(1);
	Picks.Add(1);
	const FHOLTalentPanelView View = HOLTalentPanel::Make(5, Picks);
	T.AssertEquals(4, View.NextRow, "the next row after two talents is row 4");
	const TArray<FHOLTalentCellView> Cells = HOLTalentTree::Make(5, Picks);
	for (int Index = 0; Index < Cells.Num(); Index++)
	{
		const FString Name = HOLTalentNames::Name(Cells[Index].Row, Cells[Index].Family).ToString();
		T.AssertTrue(Cells[Index].Tooltip.Contains(Name), f"the hover of the cell row {Cells[Index].Row} family {Cells[Index].Family} names {Name}");
		T.AssertEquals(Name, Cells[Index].Name, "and its label is the same name");
	}
	const FString Notice = HOLTalentText::NoticeText(2, 1);
	T.AssertTrue(Notice.Contains(HOLTalentNames::Name(2, 1).ToString()), "the notice names Boucliers levés");
	T.AssertTrue(Notice.Contains("Boucliers levés"), "the notice of row 2, Captain, is Boucliers levés");
	T.AssertTrue(Notice.Contains(HOLTalentNames::Effect(2, 1).ToString()), "the notice says what it does");
	const FString List = HOLTalentText::ListText(Picks);
	T.AssertTrue(List.Contains(HOLTalentNames::Name(2, 1).ToString()), "the list names row 2");
	T.AssertTrue(List.Contains(HOLTalentNames::Name(3, 1).ToString()), "the list names row 3");
	T.AssertTrue(List.Find(HOLTalentNames::Name(2, 1).ToString()) < List.Find(HOLTalentNames::Name(3, 1).ToString()), "in the order of the rows");
	TArray<int> NoPicks;
	T.AssertTrue(HOLTalentText::ListText(NoPicks).IsEmpty(), "no talent, no list");
	const FHOLTalentPanelView Advised = HOLTalentPanel::Make(5, Picks);
	T.AssertTrue(HOLTalentText::TooltipText(Advised).Contains(HOLTalentNames::Name(Advised.NextRow, Advised.AdvisedFamily).ToString()), "the advised button names its talent");
}

// A talent the prototype cannot show yet says so (D194: no talent is silently dead); one that works does not.
void Test_TalentNoticeSaysWhenNothingHappensYet(FUnitTest& T)
{
	T.AssertFalse(HOLTalentText::NoticeText(2, 1).Contains(HOLTalentNames::NoEffectYet().ToString()), "Boucliers levés works: no warning");
	T.AssertFalse(HOLTalentText::NoticeText(4, 1).Contains(HOLTalentNames::NoEffectYet().ToString()), "Pas de l'Aube works: no warning");
	T.AssertFalse(HOLTalentText::NoticeText(3, 1).Contains(HOLTalentNames::NoEffectYet().ToString()), "Healers Hands works since T4b: no warning");
	T.AssertTrue(HOLTalentText::NoticeText(5, 1).Contains(HOLTalentNames::NoEffectYet().ToString()), "row 5 Captain has no effect yet: it says so");
}
