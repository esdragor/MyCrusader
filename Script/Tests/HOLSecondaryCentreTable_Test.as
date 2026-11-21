// SC2 (HALDIR-1121c, GDD 5.4 D384, D783, 11.1): the building type table knows the SecondaryCentre type and agrees with the pure rules of
// Script/SecondaryCentres/HOLSecondaryCentreRules.as (cost, population, unlock tier). Two sources, compared: the table is read as the
// simulation reads it (HOLConstructionRead::FindBuildingType, from Tools/Data/BuildingTypes.csv through the regenerated table), the
// expected values come from the rules (150 wood + 400 stone, 10 population, tier 1), never typed again here.
// Red until BuildingTypes.csv holds the SecondaryCentre row AND the table is regenerated (Tools/AutoJourney/regen_tables.ps1).

void Test_SecondaryCentreTable_RowExistsAndMatchesRules(FUnitTest& T)
{
	FHOLBuildingTypeRow Row;
	bool bFound = HOLConstructionRead::FindBuildingType(n"SecondaryCentre", Row);
	T.AssertTrue(bFound, "SecondaryCentre is in the building type table (SC2)");
	if (!bFound)
	{
		return;
	}
	HOLSecondaryCentre::FHOLCentreCost Cost = HOLSecondaryCentre::Cost();
	T.AssertEquals(Cost.Food, Row.Cost.Food, "SecondaryCentre: food cost = the rules'");
	T.AssertEquals(Cost.Wood, Row.Cost.Wood, "SecondaryCentre: wood cost = the rules' (D783, 150)");
	T.AssertEquals(Cost.Stone, Row.Cost.Stone, "SecondaryCentre: stone cost = the rules' (D783, 400)");
	T.AssertEquals(Cost.Gold, Row.Cost.Gold, "SecondaryCentre: gold cost = the rules'");
	T.AssertEquals(HOLSecondaryCentre::PopulationBase, Row.PopulationProvided, "SecondaryCentre: population given = the rules' (D783, 10)");
	T.AssertEquals(HOLSecondaryCentre::UnlockTier, Row.Age, "SecondaryCentre: unlocked at the rules' tier (GDD 11.1, tier 1 Essor)");
	T.AssertTrue(HOLConstructionRead::IsBuildingTypeBuildable(Row), "SecondaryCentre: built by Peasants (unlike the main centre)");
	T.AssertTrue(Row.BuildTime > 0.0f, "SecondaryCentre: a build time");
}

void Test_SecondaryCentreTable_IsAForumMore(FUnitTest& T)
{
	FHOLBuildingTypeRow Main;
	FHOLBuildingTypeRow Row;
	bool bMain = HOLConstructionRead::FindBuildingType(n"MainCentre", Main);
	bool bFound = HOLConstructionRead::FindBuildingType(n"SecondaryCentre", Row);
	T.AssertTrue(bMain, "MainCentre in the table");
	T.AssertTrue(bFound, "SecondaryCentre in the table (SC2)");
	if (!bMain || !bFound)
	{
		return;
	}
	// D384: a real forum more: takes every resource (a deposit), same footprint and sight as the forum (D523: forum and secondary centre 16).
	TArray<int32> MainAccepts = HOLConstructionRead::GetBuildingTypeAccepts(Main);
	TArray<int32> Accepts = HOLConstructionRead::GetBuildingTypeAccepts(Row);
	T.AssertEquals(MainAccepts.Num(), Accepts.Num(), "SecondaryCentre accepts as many resources as the main centre (all four)");
	T.AssertEquals(4, Accepts.Num(), "SecondaryCentre takes all four resources (D384)");
	T.AssertTrue(Math::Abs(Main.Footprint.X - Row.Footprint.X) < 0.5 && Math::Abs(Main.Footprint.Y - Row.Footprint.Y) < 0.5, "SecondaryCentre: same footprint as the main centre");
	T.AssertTrue(Math::Abs(Main.SightRadius - Row.SightRadius) < 0.5f, "SecondaryCentre: same sight radius as the main centre (D523)");
}
