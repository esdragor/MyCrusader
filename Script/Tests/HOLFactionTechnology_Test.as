// Partial port of Tests/HOLFactionTechnologyTests.cpp (E3-06, "HeroesOfLiberty.Match.Factions.TechnologyGrid"), the half script can see: the real technology
// table (Tools/Data/Technologies.csv, read as the simulation reads it through HOLAITechnology::ReadAITechnologyTable), never a typed copy of a row.
// NOT ported (no UFUNCTION exposes them, no C++ written here): the faction filter itself, i.e. UHOLTechnologySettings::GetBuildingTechnologies(Building,
// FactionKey, Out) (the Dawn's Barracks offer the halberds, the Legions' do not, the Forge's list is the same for both) and
// HOLFactionRules::IsTechnologyOpenTo(Technology, FactionKey). FHOLAITechnology carries no `Faction` column. The rule stays in C++ (one source, HOLFactionRules);
// the C++ test stays until a reading library for it exists (TYPES_A_EXPOSER: IsTechnologyOpenTo + the faction overload of GetBuildingTechnologies).

// The Faction technology of the Dawn (TemperedHalberds) is a row of the real table, a technology of the Barracks, of the "Faction" category (4), and with no
// faction asked ("no faction: nothing is filtered") the Barracks list holds it; the Forge, whose technologies are `Common`, has some.
void Test_FactionTechnologyGrid(FUnitTest& T)
{
	TArray<FHOLAITechnology> Table;
	T.AssertTrue(HOLAITechnology::ReadAITechnologyTable(Table), "the technology table is loaded");

	bool bHalberds = false;
	int BarracksCount = 0;
	int ForgeCount = 0;
	bool bHalberdsInBarracks = false;
	for (int Row = 0; Row < Table.Num(); Row++)
	{
		if (Table[Row].Building == n"Barracks")
		{
			BarracksCount++;
		}
		if (Table[Row].Building == n"Forge")
		{
			ForgeCount++;
		}
		if (Table[Row].Name == n"TemperedHalberds")
		{
			bHalberds = true;
			bHalberdsInBarracks = Table[Row].Building == n"Barracks";
			T.AssertEquals(4, Table[Row].Category, "TemperedHalberds is a faction technology (category 4)");
			T.AssertEquals(1, Table[Row].MinTier, "TemperedHalberds opens at tier 1");
		}
	}
	T.AssertTrue(bHalberds, "TemperedHalberds is a row");
	T.AssertTrue(bHalberdsInBarracks, "the Barracks research the halberds (no faction asked: nothing is filtered)");
	T.AssertTrue(BarracksCount > 0, "the Barracks have technologies");
	T.AssertTrue(ForgeCount > 0, "the Forge has technologies");
}
