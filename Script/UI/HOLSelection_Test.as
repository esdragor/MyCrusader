// H-9 lot A: the selection block's pure rules (HOLSelectionRules in HOLSelection.as), in literals: the name, the report's ShownSelection / ShownHealth / ShownActivity strings word for word
// (quick_check reads them), the detail line of a building and of a source, the forum's report. NOT COMPILED YET (see HOLSelection.as).

void Test_SelectionNameFollowsConstructionState(FUnitTest& T)
{
	T.AssertEquals(HOLSelectionRules::NameText("Maison", true, false), "Maison", "a finished building: its name alone");
	T.AssertEquals(HOLSelectionRules::NameText("Maison", false, true), "Fondation : Maison", "a foundation (J6a.3)");
	T.AssertEquals(HOLSelectionRules::NameText("Maison", false, false), "Chantier : Maison", "a site (J6a.5)");
	T.AssertEquals(HOLSelectionRules::ProgressText(40), "40 %", "the percentage, a space before the sign");
}

void Test_SelectionBuildingShownKindWordsOfQuickCheck(FUnitTest& T)
{
	T.AssertEquals(HOLSelectionRules::BuildingShownKind("MainCentre", true, true, false, 100), "MainCentre (own)", "finished and own");
	T.AssertEquals(HOLSelectionRules::BuildingShownKind("MainCentre", false, true, false, 100), "MainCentre (other)", "finished, another player's");
	T.AssertEquals(HOLSelectionRules::BuildingShownKind("House", true, false, true, 0), "House (own), foundation", "a foundation");
	T.AssertEquals(HOLSelectionRules::BuildingShownKind("House", true, false, false, 40), "House (own), under construction 40%", "a site, its percentage");
	T.AssertEquals(HOLSelectionRules::SourceShownKind("Tree", 12), "Tree source 12", "a source: key, then index");
	T.AssertEquals(HOLSelectionRules::NoneShownKind(), "none", "nothing selected");
}

void Test_SelectionPercentFloorsInSingle(FUnitTest& T)
{
	T.AssertEquals(HOLSelectionRules::PercentOf(float32(0.5)), 50, "half");
	T.AssertEquals(HOLSelectionRules::PercentOf(float32(0.999)), 99, "floored, never rounded up to 100 before the end");
	T.AssertEquals(HOLSelectionRules::PercentOf(float32(1.0)), 100, "done");
}

void Test_SelectionHealthTexts(FUnitTest& T)
{
	T.AssertEquals(HOLSelectionRules::HealthText(250, 250), "250 / 250", "a building's health row");
	T.AssertEquals(HOLSelectionRules::HealthShown(250, 250, true), ", hp 250/250", "the report's unit health");
	T.AssertEquals(HOLSelectionRules::HealthShown(40, 100, false), ", hp 40/100 (share)", "a client's hero known by a share");
	T.AssertEquals(HOLSelectionRules::HealthShown(10, 0, true), "", "an unknown maximum: no health");
}

void Test_SelectionBuildingDetailForumAndBuilders(FUnitTest& T)
{
	const FHOLSelectionDetail Plain = HOLSelectionRules::BuildingDetail(true, true, 0, false, "", "");
	T.AssertEquals(Plain.Shown, "", "a finished building without a forum: no line");
	T.AssertFalse(Plain.bVisible, "the row is collapsed");
	const FHOLSelectionDetail Standing = HOLSelectionRules::BuildingDetail(true, true, 0, true, ", lives 2", "");
	T.AssertEquals(Standing.Shown, ", lives 2", "a standing forum: the report keeps its lives");
	T.AssertFalse(Standing.bVisible, "but no line of its own (its lives alone)");
	const FHOLSelectionDetail Ruin = HOLSelectionRules::BuildingDetail(true, false, 0, true, ", in ruin 25 s, lives 1", "En ruine");
	T.AssertTrue(Ruin.bVisible, "a ruin has its line");
	T.AssertEquals(Ruin.Text, "En ruine", "its text");
	const FHOLSelectionDetail Site = HOLSelectionRules::BuildingDetail(false, true, 2, false, "", "");
	T.AssertEquals(Site.Shown, ", builders 2", "an own site: its builders");
	T.AssertEquals(Site.Text, "Bâtisseurs : 2", "the text of the C++ screen");
	const FHOLSelectionDetail Foreign = HOLSelectionRules::BuildingDetail(false, false, 2, false, "", "");
	T.AssertFalse(Foreign.bVisible, "another player's site: builders are not received, no line");
}

void Test_SelectionSourceDetailAndForumReport(FUnitTest& T)
{
	const FHOLSelectionDetail Detail = HOLSelectionRules::SourceDetail(1840);
	T.AssertEquals(Detail.Shown, ", 1840 left", "the report's remaining amount, no thousands separator");
	T.AssertEquals(Detail.Text, "Reste : 1840", "the text of the C++ screen");
	T.AssertTrue(Detail.bVisible, "always on the screen");
	T.AssertEquals(HOLSelectionRules::ForumReport(true, ", lives 3"), ", lives 3", "a forum: its Shown line");
	T.AssertEquals(HOLSelectionRules::ForumReport(false, ", lives 3"), "", "not a forum: empty");
	T.AssertTrue(HOLSelectionRules::LivesChanged(-1, 0), "never shown: set, even at 0");
	T.AssertFalse(HOLSelectionRules::LivesChanged(3, 3), "unchanged: not set again");
}

void Test_SelectionWorkerActivityShown(FUnitTest& T)
{
	T.AssertEquals(HOLSelectionRules::WorkerActivityShown(false, "x", false, "", 0, 0), "", "no unit: empty");
	T.AssertEquals(HOLSelectionRules::WorkerActivityShown(true, "EHOLUnitActivity::Attacking", false, "", 0, 0), ", EHOLUnitActivity::Attacking", "no load: the activity alone");
	T.AssertEquals(HOLSelectionRules::WorkerActivityShown(true, "EHOLUnitActivity::Gathering", true, "EHOLResource::Wood", 4, 10),
		", EHOLUnitActivity::Gathering EHOLResource::Wood 4/10", "a load: activity, resource, amount / capacity");
}
