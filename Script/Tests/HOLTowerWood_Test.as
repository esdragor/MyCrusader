// The Wood watch tower, F8 of FortificationsInProgress.md (GDD D691, D693, D694, D793): its row of the building type table, read as the simulation reads it
// (Tools/Data/BuildingTypes.csv, regenerated into BuildingTypes.uasset by Tools/AutoJourney/regen_tables.ps1), never from a typed copy of a figure.
// Red until the TowerWood row exists AND the table is regenerated. The shooting itself (attack fields, projectile) is C++ and stays in F8's C++ lot.

// The table knows the tower, it is a tier 0 building (D691: palier 0), paid in wood, takes time to build, has hit points and houses nobody (population 0).
void Test_TowerWoodIsInTheBuildingTable(FUnitTest& T)
{
	FHOLBuildingTypeRow Row;
	T.AssertTrue(HOLBuildingType::FindRow(n"TowerWood", Row), "TowerWood is a row of the building type table");
	T.AssertEquals(0, Row.Age, "the Wood watch tower is a tier 0 building");
	T.AssertTrue(Row.MaxHealth > 0, "the Wood watch tower has hit points");
	T.AssertFalse(Row.Walkable, "a tower is not walkable (D692: non praticable)");

	FHOLAITypeFigures Figures;
	T.AssertTrue(HOLAIOrder::ReadAIBuildingFigures(n"TowerWood", Figures), "the AI reads TowerWood as a known building type");
	T.AssertTrue(Figures.CostWood > 0, "the Wood watch tower is paid in wood");
	T.AssertTrue(Figures.BuildTime > 0.f, "the Wood watch tower takes time to build");
	T.AssertEquals(0, Figures.Population, "the Wood watch tower provides no population");
}

// The garrison places come from the table row (D793: 3 for the Wood watch tower), computed by the garrison rule from the row, then the same
// number through the type name; two different paths to one figure, neither copied from the other.
void Test_TowerWoodHasGarrisonPlacesFromTheTable(FUnitTest& T)
{
	FHOLBuildingTypeRow Row;
	T.AssertTrue(HOLBuildingType::FindRow(n"TowerWood", Row), "TowerWood is a row of the building type table");
	const int FromRow = HOLGarrison::SlotsOfRow(Row);
	T.AssertTrue(FromRow > 0, "the Wood watch tower has garrison places in the table (GarrisonSlots > 0)");
	T.AssertEquals(3, FromRow, "D793: the Wood watch tower has 3 places");
	const int FromName = HOLGarrison::SlotsOf(n"TowerWood");
	T.AssertEquals(FromRow, FromName, "the places by type name are the places of the table row");
}

// The effect on the order: a Wood watch tower of the table admits foot soldiers up to its places (the real rule, the real row).
void Test_TowerWoodAdmitsGarrisonersUpToTheTablePlaces(FUnitTest& T)
{
	FHOLBuildingTypeRow Row;
	T.AssertTrue(HOLBuildingType::FindRow(n"TowerWood", Row), "TowerWood is a row of the building type table");
	const int Places = HOLGarrison::SlotsOfRow(Row);
	T.AssertTrue(Places > 0, "the row has places");
	TArray<int> Slots;
	TArray<int> Kinds;
	for (int Index = 0; Index < Places + 2; Index++)
	{
		Slots.Add(0);
		Kinds.Add(HOLGarrison::KindMeleeInfantry);
	}
	const TArray<int> In = HOLGarrison::AdmittedInto(n"TowerWood", Slots, Kinds, 0, 0);
	T.AssertEquals(Places, In.Num(), "an empty Wood watch tower admits as many own soldiers as the table gives places");
}
