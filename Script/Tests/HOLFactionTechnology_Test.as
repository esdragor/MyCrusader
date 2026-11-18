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

// The faction filter (ports the rest of Tests/HOLFactionTechnologyTests.cpp, "HeroesOfLiberty.Match.Factions.TechnologyGrid"), read through the wrappers of
// UHOLFactionTechnologyReadLibrary over the one source of the rule (HOLFactionRules::IsTechnologyOpenTo, UHOLTechnologySettings::GetBuildingTechnologies).
// A technology of the Dawn (TemperedHalberds) is not offered to the Legions; Common technologies stay in both grids.
void Test_FactionTechnologyFilterGrid(FUnitTest& T)
{
	TArray<FName> DawnBarracks = HOLFactionTechnologyRead::GetBuildingTechnologies(n"Barracks", n"Dawn");
	TArray<FName> LegionsBarracks = HOLFactionTechnologyRead::GetBuildingTechnologies(n"Barracks", n"Legions");
	TArray<FName> AllBarracks = HOLFactionTechnologyRead::GetBuildingTechnologies(n"Barracks", NAME_None);
	T.AssertTrue(DawnBarracks.Contains(n"TemperedHalberds"), "the Dawn's Barracks research the halberds");
	T.AssertFalse(LegionsBarracks.Contains(n"TemperedHalberds"), "the Legions' Barracks do not offer the halberds (a technology of the Dawn)");
	T.AssertTrue(AllBarracks.Contains(n"TemperedHalberds"), "no faction: nothing is filtered");
	T.AssertTrue(AllBarracks.Num() >= DawnBarracks.Num(), "a faction filter only removes technologies");

	// A technology of every faction stays in the grid of both.
	TArray<FName> DawnForge = HOLFactionTechnologyRead::GetBuildingTechnologies(n"Forge", n"Dawn");
	TArray<FName> LegionsForge = HOLFactionTechnologyRead::GetBuildingTechnologies(n"Forge", n"Legions");
	T.AssertTrue(DawnForge.Num() > 0, "the Forge has technologies");
	T.AssertEquals(DawnForge.Num(), LegionsForge.Num(), "the Forge's technologies are the same count for both factions (Common)");
	for (int Index = 0; Index < DawnForge.Num() && Index < LegionsForge.Num(); Index++)
	{
		T.AssertTrue(DawnForge[Index] == LegionsForge[Index], "the Forge's technologies are the same, in the same order, for both factions");
	}
}

// The server's question (IsTechnologyOpenTo) gives the same answer on the same table as the grid.
void Test_FactionTechnologyOpenTo(FUnitTest& T)
{
	T.AssertTrue(HOLFactionTechnologyRead::IsTechnologyOpenTo(n"TemperedHalberds", n"Dawn"), "the Dawn may research the halberds");
	T.AssertFalse(HOLFactionTechnologyRead::IsTechnologyOpenTo(n"TemperedHalberds", n"Legions"), "the Legions may not");

	TArray<FName> DawnForge = HOLFactionTechnologyRead::GetBuildingTechnologies(n"Forge", n"Dawn");
	T.AssertTrue(DawnForge.Num() > 0, "the Forge has technologies");
	if (DawnForge.Num() > 0)
	{
		T.AssertTrue(HOLFactionTechnologyRead::IsTechnologyOpenTo(DawnForge[0], n"Legions"), "a Common technology is open to the Legions");
	}
	T.AssertTrue(HOLFactionTechnologyRead::IsTechnologyOpenTo(n"NoSuchTechnology", n"Legions"), "a technology the table does not know is left open");
}
