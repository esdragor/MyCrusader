// The Forge, F1 of FortificationsInProgress.md (GDD 11.1, D691, D695): the building row of the type table and the four attack technologies it researches,
// on the real chain (Tools/Data/BuildingTypes.csv and Technologies.csv, read as the simulation reads them), never from typed copies of a figure.
// What script cannot see (the Age, the grid slot, the queue length, the footprint of the row; the effect on a hit's damage: owner only) stays in the C++
// tests (HOLBuildingTests, HOLTechnologyTableTests): FortificationsInProgress.md F1 lists them as waiting for a C++ slot.
// The first test needs the regenerated Content/Buildings/BuildingTypes.uasset (Tools/Editor/make_building_types.py): red until then.

FHOLAITechnology ForgeRow(const TArray<FHOLAITechnology>& Table, FName Name)
{
	for (int Row = 0; Row < Table.Num(); Row++)
	{
		if (Table[Row].Name == Name)
		{
			return Table[Row];
		}
	}
	FHOLAITechnology Missing;
	return Missing;
}

// The Forge is a row of the building table (it was not: its technologies pointed at an absent building), with a price in wood and a build time, and no
// population (a technology building houses nobody).
void Test_ForgeIsInTheBuildingTable(FUnitTest& T)
{
	FHOLAITypeFigures Forge;
	T.AssertTrue(HOLAIOrder::ReadAIBuildingFigures(n"Forge", Forge), "the Forge is a known building type");
	T.AssertTrue(Forge.CostWood > 0, "the Forge is paid in wood");
	T.AssertTrue(Forge.BuildTime > 0.f, "the Forge takes time to build");
	T.AssertEquals(0, Forge.Population, "the Forge provides no population");
}

// The four attack lines are researched by the Forge, I at tier 1 and II at tier 2 after I, and their effect is coded (the table's Implemented column).
void Test_ForgeAttackLinesOnTheRealTable(FUnitTest& T)
{
	TArray<FHOLAITechnology> Table;
	T.AssertTrue(HOLAITechnology::ReadAITechnologyTable(Table), "the real table is there");
	TArray<FName> Names;
	Names.Add(n"MeleeAttackI");
	Names.Add(n"MeleeAttackII");
	Names.Add(n"RangedAttackI");
	Names.Add(n"RangedAttackII");
	for (int Index = 0; Index < 4; Index++)
	{
		const FHOLAITechnology Row = ForgeRow(Table, Names[Index]);
		const FString Name = Names[Index].ToString();
		T.AssertTrue(Row.Name == Names[Index], Name + " is a row of the table");
		T.AssertTrue(Row.Building == n"Forge", Name + " is researched by the Forge");
		T.AssertTrue(Row.bImplemented, Name + " has a coded effect");
		T.AssertEquals(1, Row.Category, Name + " is a military technology");
		T.AssertEquals(Index % 2 == 0 ? 1 : 2, Row.MinTier, Name + ": level I opens at tier 1, level II at tier 2");
		if (Index % 2 == 1)
		{
			T.AssertTrue(Row.Requires == Names[Index - 1], Name + " follows its level I");
		}
		else
		{
			T.AssertTrue(Row.Requires.IsNone(), Name + " is a first level");
		}
	}
}

// The availability on those rows, one reason at a time: a finished Forge is needed (and none, before the building exists, is the reason given), the
// tier opens level I, level I opens level II, the purse is the last word.
void Test_ForgeAttackAvailability(FUnitTest& T)
{
	TArray<FHOLAITechnology> Table;
	HOLAITechnology::ReadAITechnologyTable(Table);
	const FHOLAITechnology One = ForgeRow(Table, n"MeleeAttackI");
	const FHOLAITechnology Two = ForgeRow(Table, n"MeleeAttackII");
	T.AssertEquals(HOLTechnologyRules::Available, HOLTechnologyRules::CheckImplemented(One.bImplemented, 1, 3, One.MinTier, One.MinHeroLevel, true, true, true, false, false), "tier 1, Forge, paid: level I");
	T.AssertEquals(HOLTechnologyRules::TierTooLow, HOLTechnologyRules::CheckImplemented(One.bImplemented, 0, 0, One.MinTier, One.MinHeroLevel, true, true, true, false, false), "tier 0: closed");
	T.AssertEquals(HOLTechnologyRules::NoBuilding, HOLTechnologyRules::CheckImplemented(One.bImplemented, 1, 3, One.MinTier, One.MinHeroLevel, true, false, true, false, false), "no finished Forge");
	T.AssertEquals(HOLTechnologyRules::CannotPay, HOLTechnologyRules::CheckImplemented(One.bImplemented, 1, 3, One.MinTier, One.MinHeroLevel, true, true, false, false, false), "an empty purse");
	T.AssertEquals(HOLTechnologyRules::TierTooLow, HOLTechnologyRules::CheckImplemented(Two.bImplemented, 1, 3, Two.MinTier, Two.MinHeroLevel, true, true, true, false, false), "level II is closed at tier 1");
	T.AssertEquals(HOLTechnologyRules::RequiresMissing, HOLTechnologyRules::CheckImplemented(Two.bImplemented, 2, 6, Two.MinTier, Two.MinHeroLevel, false, true, true, false, false), "level II without level I");
	T.AssertEquals(HOLTechnologyRules::Available, HOLTechnologyRules::CheckImplemented(Two.bImplemented, 2, 6, Two.MinTier, Two.MinHeroLevel, true, true, true, false, false), "tier 2 after level I: level II");
}

// The armour lines are in the table but no code reads them yet (F2): they stay refused, "Soon" (D601), with the Forge standing and the purse full.
void Test_ForgeArmourLinesStayGreyedUntilF2(FUnitTest& T)
{
	TArray<FHOLAITechnology> Table;
	HOLAITechnology::ReadAITechnologyTable(Table);
	TArray<FName> Names;
	Names.Add(n"InfantryArmorI");
	Names.Add(n"InfantryArmorII");
	Names.Add(n"CavalryArmorI");
	Names.Add(n"CavalryArmorII");
	for (int Index = 0; Index < 4; Index++)
	{
		const FHOLAITechnology Row = ForgeRow(Table, Names[Index]);
		T.AssertTrue(Row.Building == n"Forge", Names[Index].ToString() + " is a Forge line");
		T.AssertEquals(HOLTechnologyRules::NotImplemented, HOLTechnologyRules::CheckImplemented(Row.bImplemented, 2, 6, Row.MinTier, Row.MinHeroLevel, true, true, true, false, false),
			Names[Index].ToString() + " is not coded yet: refused");
	}
}
