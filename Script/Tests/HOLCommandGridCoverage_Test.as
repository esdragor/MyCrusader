// L2 coverage (SAURON-1119ce): the two readings the C++ tests of the command grid lost when their extracts moved to script (SAURON-1119cd).
// NOT COMPILED, NOT RUN when written. The expected values are in clear and come from the data, never from the rule's code:
//  - the Peasant's Population = 1 (Tools/Data/UnitTypes.csv, column Population of the row Peasant; GDD: one place per Peasant),
//  - the cvar hol.Nomad.ForumCell = 2 (Config/DefaultEngine.ini line 57, D831: the Nomad's first forum stands in cell E; the C++ default is 10 = C).
// To SEE RED (rouge a voir): in Tools/Data/UnitTypes.csv set the Peasant's Population to 2 and regenerate the table (Test_CGCoveragePeasantPopulation),
// or in Config/DefaultEngine.ini change `hol.Nomad.ForumCell=2` to `=3` (Test_CGCoverageNomadForumCell).
// Names are prefixed CGCOV: a global function or constant of a test file is seen by no other file but its name must not clash.

const FConsoleVariable CGCOVNomadForumCell("hol.Nomad.ForumCell", 10);

// HeroesOfLiberty.CommandGridCoverage.PeasantPopulation
// Covers HOLProductionPopulationEndTests.cpp 156-157 (Row->Population of the Peasant, no longer read by any test): the table gives 1 place, and
// with that table value the queue is blocked at 10 / 10 and free at 9 / 10.
void Test_CGCoveragePeasantPopulation(FUnitTest& T)
{
	FHOLUnitTypeSummary Peasant;
	bool bFound = HOLProductionTables::FindUnitType(n"Peasant", Peasant);
	T.AssertTrue(bFound, "Peasant in the unit type table");
	T.AssertEquals(1, Peasant.Population, "UnitTypes.csv: a Peasant takes one population place");
	bool bBlockedFull = HOLCommandGridKeys::IsPopulationBlocked(10, 10, Peasant.Population);
	T.AssertTrue(bBlockedFull, "10 / 10 with the table's Peasant: the queue says population full");
	bool bBlockedRoom = HOLCommandGridKeys::IsPopulationBlocked(9, 10, Peasant.Population);
	T.AssertFalse(bBlockedRoom, "9 / 10 with the table's Peasant: room for one");
}

// HeroesOfLiberty.CommandGridCoverage.NomadForumCell
// Covers the setting hol.Nomad.ForumCell, only tested by HOLStartingHeroTests.cpp ~647 (the C++ test reads the same cvar to build its own expectation):
// the setting itself is E = 2 (DefaultEngine.ini, D831), and a Nomad's builders list with the forum at that cell shows the forum THERE and
// nowhere else of page II (cell 10, the old C++ default, shows nothing).
void Test_CGCoverageNomadForumCell(FUnitTest& T)
{
	int ForumCell = CGCOVNomadForumCell.GetInt();
	T.AssertEquals(2, ForumCell, "DefaultEngine.ini: the Nomad's first forum is in cell E (2), D831");

	FHOLGridContextView Grid;
	Grid.bBuilders = true;
	Grid.bOwnUnits = true;
	Grid.ReachedAge = 0;
	Grid.BuildableTypes.Add(n"MainCentre");
	Grid.BuildableCells.Add(ForumCell);
	Grid.BuildableAges.Add(1);
	Grid.BuildableAlwaysOpen.Add(true);

	FHOLGridCommandView AtE = HOLCommandGridRules::GetAgeBuilding(Grid, 1, 2);
	bool bShownAtE = AtE.Command != EHOLGridCommandKind::None && AtE.TypeName == n"MainCentre";
	T.AssertTrue(bShownAtE, "page II, cell E (2): the Nomad's forum is shown");
	FHOLGridCommandView AtC = HOLCommandGridRules::GetAgeBuilding(Grid, 1, 10);
	T.AssertTrue(AtC.Command == EHOLGridCommandKind::None, "page II, cell C (10): nothing, the forum moved to E");
	FHOLGridKeyResultView KeyE = HOLCommandGridKeys::PressKey(Grid, 1, 2);
	bool bKeyEBuilds = KeyE.Target == EHOLGridKeyTargetKind::Build && KeyE.TypeName == n"MainCentre";
	T.AssertTrue(bKeyEBuilds, "page II, key E: builds the forum");
}