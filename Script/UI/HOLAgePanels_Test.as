// NOT COMPILED YET (needs the C++ lot of GALADRIEL-1119a for HOLAgePanels.as, same module). H-8: the age panels' rules (HOLAgePanels.as), pure, in literals copied from
// the C++ screen's behaviour (HOLGameScreen.cpp RefreshAges / HandleAgeClicked / Describe, quick_check.ps1:4254 "grid ages I=open [A=House greyed Z=LumberCamp ...") and, for the
// chain, through the real rule of the grid (HOLCommandGridRead) with the builders' context: panel I has its three buildings, II to IV are locked at tier 0 (tiers.txt).

FHOLGridContextView APMakeBuilders()
{
	FHOLGridContextView Context;
	Context.bBuilders = true;
	Context.BuildableTypes.Add(n"House");
	Context.BuildableCells.Add(0);
	return Context;
}

void Test_AgePanelsNumeralsKeysAndHeader(FUnitTest& T)
{
	T.AssertEquals(HOLAgePanels::Numeral(0), "I", "the first age");
	T.AssertEquals(HOLAgePanels::Numeral(1), "II", "the second");
	T.AssertEquals(HOLAgePanels::Numeral(2), "III", "the third");
	T.AssertEquals(HOLAgePanels::Numeral(3), "IV", "the fourth");
	T.AssertEquals(HOLAgePanels::CellKey(0), "A", "the medallion's key of age I");
	T.AssertEquals(HOLAgePanels::CellKey(3), "R", "the medallion's key of age IV");
	T.AssertEquals(HOLAgePanels::CellKey(4), "Q", "the second row starts at Q");
	T.AssertEquals(HOLAgePanels::CellKey(11), "V", "the last cell");
	T.AssertEquals(HOLAgePanels::CellKey(12), "?", "outside the grid");
	T.AssertEquals(HOLAgePanels::HeaderName(1), "Palier II — Essor", "the header of age II as the C++ LOCTEXT AgeHeaderName gives it");
	T.AssertEquals(HOLAgePanels::HeaderName(0), "Palier I — Fondation", "age I");
	T.AssertEquals(HOLAgePanels::HeaderName(3), "Palier IV — Légende", "age IV");
}

void Test_AgePanelsTooltipSaysLockedUnderTheHeader(FUnitTest& T)
{
	T.AssertEquals(HOLAgePanels::TooltipText(2, true), "Palier III — Puissance", "reached: the header alone");
	T.AssertEquals(HOLAgePanels::TooltipText(2, false), "Palier III — Puissance\nPalier non atteint", "not reached: the line under it (the widget's tooltip)");
	T.AssertEquals(HOLAgePanels::TooltipLine(2, false), "Palier III — Puissance | Palier non atteint", "the report keeps it on one line, ' | '");
	T.AssertEquals(HOLAgePanels::TooltipLine(2, true), "Palier III — Puissance", "reached: the same on one line");
}

void Test_AgePanelsLookFollowsReachedAndChosen(FUnitTest& T)
{
	T.AssertEquals(HOLAgePanels::Look(false, 1, 1), HOLAgePanels::LookLocked(), "a locked age is never lit, even the chosen one's number");
	T.AssertEquals(HOLAgePanels::Look(true, 0, 0), HOLAgePanels::LookChosen(), "reached and chosen: lit");
	T.AssertEquals(HOLAgePanels::Look(true, 0, 1), HOLAgePanels::LookReached(), "reached, another chosen: reached");
	T.AssertEquals(HOLAgePanels::Look(true, 0, -1), HOLAgePanels::LookReached(), "panels hidden or nothing chosen: reached");
}

void Test_AgePanelsLookThroughTheGridRule(FUnitTest& T)
{
	// The chain: the grid's rule (C++ HOLCommandGrid::IsAgeReached) then the look; tier 0 (tiers.txt step 1): only I open.
	FHOLGridContextView Builders = APMakeBuilders();
	T.AssertEquals(HOLAgePanels::LookIn(Builders, 0, 0), HOLAgePanels::LookChosen(), "tier 0: I open and chosen");
	T.AssertEquals(HOLAgePanels::LookIn(Builders, 0, -1), HOLAgePanels::LookReached(), "tier 0: I open, none chosen");
	T.AssertEquals(HOLAgePanels::LookIn(Builders, 1, 1), HOLAgePanels::LookLocked(), "tier 0: II locked");
	T.AssertEquals(HOLAgePanels::LookIn(Builders, 3, -1), HOLAgePanels::LookLocked(), "tier 0: IV locked");
}

void Test_AgePanelsColumnsArePackedAndFirstKept(FUnitTest& T)
{
	// Panel I holds A Z E (cells 0 1 2): three columns used, the 4th collapsed.
	TArray<bool> Has;
	for (int Index = 0; Index < 12; Index++)
		Has.Add(Index <= 2);
	TArray<bool> Used = HOLAgePanels::UsedColumns(Has);
	T.AssertEquals(Used.Num(), 4, "four columns");
	T.AssertTrue(Used[0] && Used[1] && Used[2] && !Used[3], "A Z E used, R collapsed");
	// An empty age: its medallion alone, one column.
	TArray<bool> Empty;
	for (int Index = 0; Index < 12; Index++)
		Empty.Add(false);
	TArray<bool> Kept = HOLAgePanels::UsedColumns(Empty);
	T.AssertTrue(Kept[0] && !Kept[1] && !Kept[2] && !Kept[3], "an empty age keeps one column");
	// Only the third column used (cell 2): the 1st column is not forced (the others are used), the cell moves to place 0.
	TArray<bool> Third;
	for (int Index = 0; Index < 12; Index++)
		Third.Add(Index == 2);
	TArray<bool> UsedThird = HOLAgePanels::UsedColumns(Third);
	T.AssertTrue(!UsedThird[0] && !UsedThird[1] && UsedThird[2] && !UsedThird[3], "only E: the first column is not added");
	int Packed = HOLAgePanels::PackedColumn(UsedThird, 2);
	T.AssertEquals(Packed, 0, "E moves to the first place");
	int PackedAfter = HOLAgePanels::PackedColumn(Used, 2);
	T.AssertEquals(PackedAfter, 2, "A Z E: E stays third");
}

void Test_AgePanelsCellStateWordsAndReportLine(FUnitTest& T)
{
	T.AssertTrue(HOLAgePanels::IsCellReady(false, true), "an affordable building of a reached age is ready");
	T.AssertTrue(!HOLAgePanels::IsCellReady(true, true), "age not reached: greyed");
	T.AssertTrue(!HOLAgePanels::IsCellReady(false, false), "stock short: greyed");
	T.AssertEquals(HOLAgePanels::CellPart(0, "House", 0), "A=House enabled", "a ready cell");
	T.AssertEquals(HOLAgePanels::CellPart(1, "LumberCamp", 1), "Z=LumberCamp greyed", "a greyed cell");
	T.AssertEquals(HOLAgePanels::CellPart(2, "MiningCamp", 2), "E=MiningCamp disabled", "a disabled cell");
	TArray<FString> Segments;
	Segments.Add(HOLAgePanels::AgeSegment(0, true, "A=House greyed Z=LumberCamp greyed"));
	Segments.Add(HOLAgePanels::AgeSegment(1, false, ""));
	Segments.Add(HOLAgePanels::AgeSegment(2, false, ""));
	Segments.Add(HOLAgePanels::AgeSegment(3, false, ""));
	T.AssertEquals(HOLAgePanels::GridLine(Segments, 0), "ages I=open [A=House greyed Z=LumberCamp greyed] II=locked [] III=locked [] IV=locked [] (chosen I)", "the 'grid' part of hol.HUD.Report, as quick_check.ps1:4254 reads it");
	T.AssertEquals(HOLAgePanels::GridLine(Segments, -1), "ages I=open [A=House greyed Z=LumberCamp greyed] II=locked [] III=locked [] IV=locked [] (chosen none)", "no age chosen");
	T.AssertEquals(HOLAgePanels::GridLine(Segments, 4), "ages I=open [A=House greyed Z=LumberCamp greyed] II=locked [] III=locked [] IV=locked [] (chosen none)", "an age outside 0..3 is none");
}

void Test_AgePanelsClickJournalAndGate(FUnitTest& T)
{
	T.AssertEquals(HOLAgePanels::ClickLine(0, 0), "HUD: age I A clicked", "the journal line of AgeCell000");
	T.AssertEquals(HOLAgePanels::ClickLine(0, 1), "HUD: age I Z clicked", "AgeCell001 (the NotEnoughWood scenario of quick_check.ps1:1672)");
	T.AssertEquals(HOLAgePanels::ClickLine(2, 11), "HUD: age III V clicked", "the last cell of III");
	T.AssertEquals(HOLAgePanels::ClickLine(1, 12), "HUD: age II ? clicked", "a key outside the grid");
	T.AssertTrue(HOLAgePanels::ClickAllowed(true, true), "a building shown takes the click");
	T.AssertTrue(!HOLAgePanels::ClickAllowed(false, true), "panels hidden: not ours");
	T.AssertTrue(!HOLAgePanels::ClickAllowed(true, false), "a cell without a building: not taken");
}

void Test_AgePanelsWidgetNamesAndReportParts(FUnitTest& T)
{
	T.AssertEquals(HOLAgePanels::CellNameText(0, 0), "AgeCell000", "AgeCell000, the house (age 0, A)");
	T.AssertEquals(HOLAgePanels::CellNameText(0, 4), "AgeCell004", "AgeCell004, the barracks (Q)");
	T.AssertEquals(HOLAgePanels::CellNameText(1, 11), "AgeCell111", "age II's last cell");
	FName Panel = HOLAgePanels::PanelName(3);
	T.AssertTrue(Panel == n"AgePanel3", "AgePanel<age>");
	FName Medallion = HOLAgePanels::MedallionName(2);
	T.AssertTrue(Medallion == n"AgeMedallion2", "AgeMedallion<age>");
	T.AssertEquals(HOLAgePanels::ContentPart(0, 0, "Maison\nBois 25"), "I A: Maison Bois 25", "the content line: the breaks replaced by a space");
	T.AssertEquals(HOLAgePanels::MedallionPart(0, "Palier I — Fondation"), "I A: Palier I — Fondation", "the medallion's report part uses the age's key");
}

// The letter a cell shows in its top left corner (H-8: the cell was keyed before its widget existed and showed none): one letter per cell, in the grid's order A Z E R / Q S D F / W X C V.
void Test_AgePanelsEveryCellKeyIsItsGridLetter(FUnitTest& T)
{
	FString All = "";
	for (int Cell = 0; Cell < HOLAgePanels::NumCells(); Cell++)
	{
		FString Key = HOLAgePanels::CellKey(Cell);
		T.AssertEquals(Key.Len(), 1, "one letter per cell");
		All += Key;
	}
	T.AssertEquals(All, "AZERQSDFWXCV", "the 12 cells' letters in the grid's order");
}
