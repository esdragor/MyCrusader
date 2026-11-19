// Wired by SAURON-1119q (HUD H-10): HOLCommandGridPanel.as and its C++ bindings exist; not compiled until the session's build.
// NOT COMPILED YET (SAURON-1119m, H-10 wave 1): needs the C++ lot of Docs\Rapports\SAURON-1119m.md for HOLCommandGridPanel.as (same module: UHOLCommandGridPanel calls bindings that
// do not exist yet), so the module does not compile with these two files and without the lot. The rules of the view (HOLCommandGridPanelRules), pure, in literals copied from the C++
// screen's behaviour: GridKeys (HOLGameScreen.cpp:242-245), GridCellName (:278-281), HandleGridCellClicked (:2191-2208: "HUD: order cell %s clicked" / "HUD: grid cell %s clicked"),
// ClickWidget (:2345-2355), DescribeCells (:2461-2467: "A: Peasant", breaks replaced by a space), RefreshGrid's empty cell (default: ClearUnit, dark) and the panel's Hidden / Visible (:1810-1816).
// Red to see (once the lot is there): fake the "AZERQSDFWXCV" expectation of Test_GridPanelKeysFollowTheAzertyRows by one letter, run, restore.

void Test_GridPanelKeysAndNamesFollowTheCells(FUnitTest& T)
{
	T.AssertEquals(HOLCommandGridPanelRules::CellKey(0), "A", "the first row starts at A");
	T.AssertEquals(HOLCommandGridPanelRules::CellKey(3), "R", "the first row ends at R");
	T.AssertEquals(HOLCommandGridPanelRules::CellKey(4), "Q", "the second row starts at Q (AZERTY)");
	T.AssertEquals(HOLCommandGridPanelRules::CellKey(7), "F", "the second row ends at F");
	T.AssertEquals(HOLCommandGridPanelRules::CellKey(8), "W", "the third row starts at W");
	T.AssertEquals(HOLCommandGridPanelRules::CellKey(11), "V", "the last cell");
	T.AssertEquals(HOLCommandGridPanelRules::CellKey(12), "?", "outside the grid");
	T.AssertEquals(HOLCommandGridPanelRules::CellKey(-1), "?", "before the grid");
	T.AssertEquals(HOLCommandGridPanelRules::CellNameText(0), "GridCell00", "the tools click GridCell00 (ecran_hud_file.txt:17, quick_check.ps1:1374)");
	T.AssertEquals(HOLCommandGridPanelRules::CellNameText(5), "GridCell05", "quick_check.ps1:1694 clicks GridCell05");
	T.AssertEquals(HOLCommandGridPanelRules::CellNameText(11), "GridCell11", "quick_check.ps1:1705 clicks GridCell11 (the site's cancel, V)");
	T.AssertEquals(HOLCommandGridPanelRules::CellNameText(10), "GridCell10", "two digits are not padded again");
}

void Test_GridPanelKeysFollowTheAzertyRows(FUnitTest& T)
{
	// Every cell of the 3 x 4 table carries the key of its row and column: the keys in reading order are the keyboard's rows.
	FString Keys = "";
	for (int Cell = 0; Cell < HOLCommandGridPanelRules::NumCells(); Cell++)
		Keys += HOLCommandGridPanelRules::CellKey(Cell);
	T.AssertEquals(Keys, "AZERQSDFWXCV", "the 12 keys, row by row");
	T.AssertEquals(HOLCommandGridPanelRules::NumCells() / HOLCommandGridPanelRules::GridColumns(), 3, "3 rows of 4");
}

void Test_GridPanelCellStateWordsAndEmptyCellIsDark(FUnitTest& T)
{
	T.AssertEquals(HOLCommandGridPanelRules::StateWord(HOLCommandGridPanelRules::StateReady()), "enabled", "Ready is 'enabled' in hol.HUD.Report (quick_check.ps1:3694 'grid A=Peasant enabled')");
	T.AssertEquals(HOLCommandGridPanelRules::StateWord(HOLCommandGridPanelRules::StateGreyed()), "greyed", "Greyed is 'greyed' (quick_check.ps1:4849 'A=CloseRanks enabled Z=Banner greyed')");
	T.AssertEquals(HOLCommandGridPanelRules::StateWord(HOLCommandGridPanelRules::StateDisabled()), "disabled", "any other is 'disabled'");
	// An empty cell is dark whatever state the frame carried; a command keeps its own.
	T.AssertEquals(HOLCommandGridPanelRules::EffectiveState(HOLCommandGridPanelRules::KindNone(), HOLCommandGridPanelRules::StateReady()), HOLCommandGridPanelRules::StateDisabled(), "no command: dark even if the frame said Ready");
	T.AssertEquals(HOLCommandGridPanelRules::EffectiveState(HOLCommandGridPanelRules::KindCommand(), HOLCommandGridPanelRules::StateGreyed()), HOLCommandGridPanelRules::StateGreyed(), "a greyed command stays greyed (still clickable: the click says why)");
	T.AssertEquals(HOLCommandGridPanelRules::EffectiveState(HOLCommandGridPanelRules::KindCommand(), HOLCommandGridPanelRules::StateReady()), HOLCommandGridPanelRules::StateReady(), "a ready command stays ready");
	T.AssertTrue(HOLCommandGridPanelRules::NeedsFill(HOLCommandGridPanelRules::KindCommand()), "a command is filled (name, tooltip)");
	T.AssertTrue(!HOLCommandGridPanelRules::NeedsFill(HOLCommandGridPanelRules::KindNone()), "an empty cell is cleared, not filled: no tooltip stays on it");
}

void Test_GridPanelShownOnlyWithGridOrOrders(FUnitTest& T)
{
	T.AssertTrue(HOLCommandGridPanelRules::PanelVisible(true, false), "a building's grid: shown");
	T.AssertTrue(HOLCommandGridPanelRules::PanelVisible(false, true), "the units' orders: shown");
	T.AssertTrue(!HOLCommandGridPanelRules::PanelVisible(false, false), "nothing to command: hidden (never collapsed: nothing moves in the bar)");
}

void Test_GridPanelClickLineIsTheCppsWordForWord(FUnitTest& T)
{
	T.AssertEquals(HOLCommandGridPanelRules::ClickLine(0, false), "HUD: grid cell A clicked", "HandleGridCellClicked's grid line");
	T.AssertEquals(HOLCommandGridPanelRules::ClickLine(4, true), "HUD: order cell Q clicked", "its order line (Q attack-move)");
	T.AssertEquals(HOLCommandGridPanelRules::ClickLine(7, true), "HUD: order cell F clicked", "the key comes from the cell's index");
	T.AssertEquals(HOLCommandGridPanelRules::ClickLine(12, false), "HUD: grid cell ? clicked", "a cell outside the grid: '?'");
}

void Test_GridPanelOrderClickOnlyWithoutGridAndWithAnOrder(FUnitTest& T)
{
	T.AssertTrue(HOLCommandGridPanelRules::IsOrderClick(false, true, n"Stop"), "units selected, an order in the cell: the order's click");
	T.AssertTrue(!HOLCommandGridPanelRules::IsOrderClick(true, true, n"Stop"), "a building's grid showing wins: the grid's click");
	T.AssertTrue(!HOLCommandGridPanelRules::IsOrderClick(false, true, NAME_None), "no order in that cell: the grid's activation (which does nothing)");
	T.AssertTrue(!HOLCommandGridPanelRules::IsOrderClick(false, false, n"Stop"), "orders hidden: not an order click");
}

void Test_GridPanelClickAllowedNeedsAShownPanelAndAnEnabledCell(FUnitTest& T)
{
	T.AssertTrue(HOLCommandGridPanelRules::ClickAllowed(true, false, true), "grid shown, cell enabled");
	T.AssertTrue(HOLCommandGridPanelRules::ClickAllowed(false, true, true), "orders shown, cell enabled");
	T.AssertTrue(!HOLCommandGridPanelRules::ClickAllowed(false, false, true), "nothing shown: hol.HUD.Click GridCell00 returns false (C++ ClickWidget)");
	T.AssertTrue(!HOLCommandGridPanelRules::ClickAllowed(true, false, false), "a disabled cell takes no click");
}

void Test_GridPanelContentPartIsTheReportsCellText(FUnitTest& T)
{
	T.AssertEquals(HOLCommandGridPanelRules::ContentPart(0, "Paysan"), "A: Paysan", "'A: Paysan' in 'HUD cells: A: ...' (legions_roster.txt:80 'A: Archer squelette')");
	T.AssertEquals(HOLCommandGridPanelRules::ContentPart(1, "Rangs\n59 s"), "Z: Rangs 59 s", "breaks replaced by a space (quick_check.ps1:4853 'A: Rangs (59|60) s')");
	T.AssertEquals(HOLCommandGridPanelRules::ContentPart(2, ""), "", "an empty content is not listed");
}
