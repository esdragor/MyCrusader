// Port of the pure parts of HOLBuildingTests.cpp (Buildings.Types, the table half of Buildings.Walkable) and of the main centre
// check of HOLGatherTests.cpp (Gather.Tables), L17 migration (HALDIR-1117r, D830, D838, D839).
// Same values as the C++ tests; they read the building type table through HOLBuildingType::FindRow (HOLBuildingTypeLibrary.h);
// the row fields are BlueprintReadOnly since the lot of 2026-11-17 (HOLBuildingTypes.h). Resources as ints: 0 Food, 1 Wood, 2 Stone, 3 Gold.
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
	bool bFound = HOLBuildingType::FindRow(n"MainCentre", MainCentre);
	T.AssertTrue(bFound, "main centre in the building type table");
	if (!bFound)
	{
		return;
	}
	T.AssertFalse(MainCentre.DisplayName.IsEmpty(), "named");
	T.AssertTrue(MainCentre.Footprint.X == 1600.0 && MainCentre.Footprint.Y == 1600.0, "16 x 16 m: 4 x 4 tiles of 4 m (D261, D311, D344)");
	T.AssertEquals(1000.0f, MainCentre.Height, "~10 m high (D261)");
	T.AssertTrue(MainCentre.MaxHealth > 0, "health points");
	UHOLArtVisual MainCentreArt = UHOLArtVisualRegistry::FindArtVisual(n"MainCentre", NAME_None);
	T.AssertTrue(MainCentreArt != nullptr && !MainCentreArt.Mesh.IsNull(), "the Forum has its model: SM_Forum, composed from the kit (its DA_Art)");
}

// HeroesOfLiberty.Buildings.Types (house): the provisional shape, no final mesh yet.
void Test_Building_TypesHouseProvisional(FUnitTest& T)
{
	FHOLBuildingTypeRow House;
	bool bFound = HOLBuildingType::FindRow(n"House", House);
	T.AssertTrue(bFound, "house in the building type table");
	if (!bFound)
	{
		return;
	}
	UHOLArtVisual HouseArt = UHOLArtVisualRegistry::FindArtVisual(n"House", NAME_None);
	T.AssertTrue(HouseArt == nullptr || HouseArt.Mesh.IsNull(), "provisional shape: no final mesh yet (its DA_Art)");
}

// HeroesOfLiberty.Buildings.Walkable (table half): the farm is walked over, the mill is not. (The BlocksNavigation rule half is
// already in HOLConstruction_Test.as; the world half stays in C++.)
void Test_Building_WalkableTable(FUnitTest& T)
{
	FHOLBuildingTypeRow Farm;
	FHOLBuildingTypeRow Mill;
	bool bFoundFarm = HOLBuildingType::FindRow(n"Farm", Farm);
	bool bFoundMill = HOLBuildingType::FindRow(n"Mill", Mill);
	T.AssertTrue(bFoundFarm, "farm in the building type table");
	T.AssertTrue(bFoundMill, "mill in the building type table");
	if (!bFoundFarm || !bFoundMill)
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
	bool bFound = HOLBuildingType::FindRow(n"MainCentre", MainCentre);
	T.AssertTrue(bFound, "main centre in the building type table");
	if (!bFound)
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
