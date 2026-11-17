// Port of the pure parts of HOLBuildingTests.cpp (Buildings.Types, the table half of Buildings.Walkable) and of the main centre
// check of HOLGatherTests.cpp (Gather.Tables), L17 migration (HALDIR-1117r, D830, D838, D839). NOT COMPILED, NOT RUN when written.
// Same values as the C++ tests; they read the building type table through HOLBuildingType::FindRow (HOLBuildingTypeLibrary.h).
// The row fields are read directly (EditAnywhere UPROPERTYs of FHOLBuildingTypeRow): if the script refuses one, see TYPES_A_EXPOSER of
// the report (BlueprintReadOnly on the row's fields). Resources as ints: 0 Food, 1 Wood, 2 Stone, 3 Gold.
// The rest of both C++ files needs a world (spawned AHOLBuilding, source registry, Mass) and stays in C++ (D838); Gather.* has no
// script library (HOLGather rules, FHOLSourceMassifs, the source type table are not exposed).

bool BuildingTestAccepts(const FHOLBuildingTypeRow& Row, int Resource)
{
	for (int Index = 0; Index < Row.Accepts.Num(); ++Index)
	{
		if (int(Row.Accepts[Index]) == Resource)
		{
			return true;
		}
	}
	return false;
}

// HeroesOfLiberty.Buildings.Types (main centre): named, 16 x 16 m (4 x 4 tiles of 4 m, D261, D311, D344), 10 m high, health points,
// and its own model (SM_Forum).
void Test_Building_TypesMainCentre(FUnitTest& T)
{
	FHOLBuildingTypeRow MainCentre;
	if (!T.AssertTrue(HOLBuildingType::FindRow(n"MainCentre", MainCentre), "main centre in the building type table"))
	{
		return;
	}
	T.AssertFalse(MainCentre.DisplayName.IsEmpty(), "named");
	T.AssertTrue(MainCentre.Footprint.X == 1600.0 && MainCentre.Footprint.Y == 1600.0, "16 x 16 m: 4 x 4 tiles of 4 m (D261, D311, D344)");
	T.AssertEquals(1000.0f, MainCentre.Height, "~10 m high (D261)");
	T.AssertTrue(MainCentre.MaxHealth > 0, "health points");
	T.AssertFalse(MainCentre.Mesh.IsNull(), "the Forum has its model: SM_Forum, composed from the kit");
}

// HeroesOfLiberty.Buildings.Types (house): the provisional shape, no final mesh yet.
void Test_Building_TypesHouseProvisional(FUnitTest& T)
{
	FHOLBuildingTypeRow House;
	if (!T.AssertTrue(HOLBuildingType::FindRow(n"House", House), "house in the building type table"))
	{
		return;
	}
	T.AssertTrue(House.Mesh.IsNull(), "provisional shape: no final mesh yet");
}

// HeroesOfLiberty.Buildings.Walkable (table half): the farm is walked over, the mill is not. (The BlocksNavigation rule half is
// already in HOLConstruction_Test.as; the world half stays in C++.)
void Test_Building_WalkableTable(FUnitTest& T)
{
	FHOLBuildingTypeRow Farm;
	FHOLBuildingTypeRow Mill;
	if (!T.AssertTrue(HOLBuildingType::FindRow(n"Farm", Farm), "farm in the building type table")
		|| !T.AssertTrue(HOLBuildingType::FindRow(n"Mill", Mill), "mill in the building type table"))
	{
		return;
	}
	T.AssertTrue(Farm.Walkable, "the farm is walked over (table)");
	T.AssertFalse(Mill.Walkable, "the mill is not (table)");
}

// HeroesOfLiberty.Gather.Tables (last block): the main centre takes the four resources (D295); not a deposit for a house.
void Test_Building_MainCentreAcceptsFourResources(FUnitTest& T)
{
	FHOLBuildingTypeRow MainCentre;
	if (!T.AssertTrue(HOLBuildingType::FindRow(n"MainCentre", MainCentre), "main centre in the building type table"))
	{
		return;
	}
	T.AssertTrue(BuildingTestAccepts(MainCentre, 0) && BuildingTestAccepts(MainCentre, 1) && BuildingTestAccepts(MainCentre, 2)
		&& BuildingTestAccepts(MainCentre, 3), "the main centre takes the four resources (D295)");
	FHOLBuildingTypeRow Mill;
	if (HOLBuildingType::FindRow(n"Mill", Mill))
	{
		T.AssertTrue(BuildingTestAccepts(Mill, 0) && !BuildingTestAccepts(Mill, 1), "the mill takes food only (BuildingTypes.csv)");
	}
}
