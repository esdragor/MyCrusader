// Script port of HeroesOfLiberty.Match.Factions.Tables (HOLFactionRulesTests.cpp, FHOLFactionTablesTest) (D830, D836, D839): run against
// UHOLFactionRulesReadLibrary (namespace HOLFactionRulesRead, ELROND-1117s), wrappers of the pure rules of HOLFactionRules (the CSV
// parsers, FTable, the built-in table), which STAY in C++. Same vectors as the C++ test; the parsing is the real C++ parsing (the script
// only hands the text over and reads the result). The 7 other tests of the C++ file (world, Mass) stay in C++ (D838).
// The C++ test is removed by the main session after a red seen then green.

const FString FixtureRolesText =
	"Name,Faction,Role,Type\n" +
	"Dawn_Worker,Dawn,Worker,Peasant\n" +
	"Dawn_Hero,Dawn,Hero,Hero\n" +
	"Dawn_MainCentre,Dawn,MainCentre,MainCentre\n" +
	"Legions_Worker,Legions,Worker,Spearman\n" +
	"Legions_Hero,Legions,Hero,ManAtArms\n" +
	"Legions_MainCentre,Legions,MainCentre,MainCentre\n";

const FString FixtureOverridesText =
	"Name,Faction,Type,Field,Value\n" +
	"Legions_Spearman_Name,Legions,Spearman,DisplayName,Fixture Legion Spearman\n" +
	"Legions_Spearman_Cost,Legions,Spearman,Cost,\"(Food=0,Wood=7,Stone=0,Gold=9)\"\n";

// Section 1 of the C++ test: the real files (Tools/Data) read without a problem; the Dawn's three roles and the Legions'.
void Test_FactionTablesRealFiles(FUnitTest& T)
{
	FString RolesText;
	FString OverridesText;
	bool bRoles = HOLMatchConfigRead::LoadProjectFileText("Tools/Data/Factions.csv", RolesText);
	bool bOverrides = HOLMatchConfigRead::LoadProjectFileText("Tools/Data/FactionOverrides.csv", OverridesText);
	T.AssertTrue(bRoles, "Tools/Data/Factions.csv is there");
	T.AssertTrue(bOverrides, "Tools/Data/FactionOverrides.csv is there");
	if (!bRoles || !bOverrides)
	{
		return;
	}
	T.AssertEquals(0, HOLFactionRulesRead::CountRoleProblems(RolesText), "Factions.csv reads without a problem");
	T.AssertEquals(0, HOLFactionRulesRead::CountOverrideProblems(OverridesText), "FactionOverrides.csv reads without a problem");
	T.AssertTrue(HOLFactionRulesRead::CountRoleRows(RolesText) > 0, "Factions.csv has rows");

	T.AssertEquals("Peasant", HOLFactionRulesRead::FindRoleType(RolesText, "Dawn", "Worker"), "Dawn Worker");
	T.AssertEquals("Hero", HOLFactionRulesRead::FindRoleType(RolesText, "Dawn", "Hero"), "Dawn Hero");
	T.AssertEquals("MainCentre", HOLFactionRulesRead::FindRoleType(RolesText, "Dawn", "MainCentre"), "Dawn MainCentre");
	T.AssertTrue(HOLFactionRulesRead::HasFaction(RolesText, "Legions"), "the Legions have rows of their own");
	T.AssertEquals("Zombie", HOLFactionRulesRead::FindRoleType(RolesText, "Legions", "Worker"), "Legions Worker: the Zombie (L1)");
	T.AssertEquals("Hero", HOLFactionRulesRead::FindRoleType(RolesText, "Legions", "Hero"), "Legions Hero, provisional until L8a");
}

// Section 2: the figures built in (a packaged game) are the files'.
void Test_FactionTablesBuiltIn(FUnitTest& T)
{
	FString RolesText;
	FString OverridesText;
	bool bRoles = HOLMatchConfigRead::LoadProjectFileText("Tools/Data/Factions.csv", RolesText);
	bool bOverrides = HOLMatchConfigRead::LoadProjectFileText("Tools/Data/FactionOverrides.csv", OverridesText);
	T.AssertTrue(bRoles && bOverrides, "the real files are there");
	if (!bRoles || !bOverrides)
	{
		return;
	}
	int RoleRows = HOLFactionRulesRead::CountRoleRows(RolesText);
	T.AssertEquals(RoleRows, HOLFactionRulesRead::CountBuiltInRoleRows(), "built-in roles: the same number of rows");
	for (int Index = 0; Index < RoleRows; ++Index)
	{
		FString Faction = HOLFactionRulesRead::GetRoleCell(RolesText, Index, "Faction");
		FString Role = HOLFactionRulesRead::GetRoleCell(RolesText, Index, "Role");
		FString Type = HOLFactionRulesRead::GetRoleCell(RolesText, Index, "Type");
		T.AssertEquals(Type, HOLFactionRulesRead::FindBuiltInRoleType(Faction, Role), "built-in " + HOLFactionRulesRead::GetRoleCell(RolesText, Index, "Name"));
	}
	int OverrideRows = HOLFactionRulesRead::CountOverrideRows(OverridesText);
	T.AssertEquals(OverrideRows, HOLFactionRulesRead::CountBuiltInOverrideRows(), "built-in overrides: the files'");
	for (int Index = 0; Index < OverrideRows; ++Index)
	{
		T.AssertTrue(HOLFactionRulesRead::BuiltInHasOverrideRowOf(OverridesText, Index),
			"built-in override " + HOLFactionRulesRead::GetOverrideCell(OverridesText, Index, "Name") + " is the file's (run Tools/Data/make_builtin_overrides.py)");
	}
}

// Section 3: the fixture, by the same parsers: the Legions differ from the Dawn, a faction with no rows reads as the Dawn.
void Test_FactionTablesFixture(FUnitTest& T)
{
	T.AssertEquals(0, HOLFactionRulesRead::CountRoleProblems(FixtureRolesText), "fixture roles: no problem");
	T.AssertEquals(0, HOLFactionRulesRead::CountOverrideProblems(FixtureOverridesText), "fixture overrides: no problem");
	T.AssertEquals(6, HOLFactionRulesRead::CountRoleRows(FixtureRolesText), "fixture roles: six rows");
	T.AssertEquals(2, HOLFactionRulesRead::CountOverrideRows(FixtureOverridesText), "fixture overrides: two rows");
	T.AssertEquals("Legions_Worker", HOLFactionRulesRead::GetRoleCell(FixtureRolesText, 3, "Name"), "cell: row 3 name");
	T.AssertEquals("Spearman", HOLFactionRulesRead::GetRoleCell(FixtureRolesText, 3, "Type"), "cell: row 3 type");
	T.AssertEquals("(Food=0,Wood=7,Stone=0,Gold=9)", HOLFactionRulesRead::GetOverrideCell(FixtureOverridesText, 1, "Value"), "cell: the quoted value keeps its commas");

	T.AssertEquals("Peasant", HOLFactionRulesRead::FindRoleType(FixtureRolesText, "Dawn", "Worker"), "fixture: Dawn Worker");
	T.AssertEquals("Spearman", HOLFactionRulesRead::FindRoleType(FixtureRolesText, "Legions", "Worker"), "fixture: Legions Worker");
	T.AssertEquals("Dawn", HOLFactionRulesRead::ResolveKey(FixtureRolesText, "Templars"), "a faction with no rows reads as the Dawn");
	T.AssertEquals("Legions", HOLFactionRulesRead::ResolveKey(FixtureRolesText, "Legions"), "a faction with rows reads as itself");
	T.AssertEquals("Peasant", HOLFactionRulesRead::FindRoleType(FixtureRolesText, "Templars", "Worker"), "... for its Worker too");
	T.AssertTrue(HOLFactionRulesRead::Allows(FixtureRolesText, "Common", "Legions"), "Common is for everyone");
	T.AssertFalse(HOLFactionRulesRead::Allows(FixtureRolesText, "Dawn", "Legions"), "a Dawn-only type is not the Legions'");
	T.AssertTrue(HOLFactionRulesRead::Allows(FixtureRolesText, "Dawn", "Dawn"), "a Dawn-only type is the Dawn's");
	T.AssertTrue(HOLFactionRulesRead::Allows(FixtureRolesText, "Legions;Dawn", "Legions"), "a list names several");
	T.AssertTrue(HOLFactionRulesRead::Allows(FixtureRolesText, "Dawn", "Templars"), "a faction with no rows reads as the Dawn: it has the Dawn-only type");
	T.AssertTrue(HOLFactionRulesRead::HasOverride(FixtureRolesText, FixtureOverridesText, "Legions", "Spearman"), "the Legions override the Spearman");
	T.AssertFalse(HOLFactionRulesRead::HasOverride(FixtureRolesText, FixtureOverridesText, "Dawn", "Spearman"), "the Dawn does not");
}

// Section 3, second half: the same through the roles of a match (FHOLStartRules on the config of the C++ test).
void Test_FactionTablesStartRoles(FUnitTest& T)
{
	T.AssertEquals("Peasant", HOLFactionRulesRead::GetSlotStartType(FixtureRolesText, FixtureOverridesText, 0, "Worker"), "slot 0 (Dawn) starts with Peasants");
	T.AssertEquals("Spearman", HOLFactionRulesRead::GetSlotStartType(FixtureRolesText, FixtureOverridesText, 1, "Worker"), "slot 1 (Legions) starts with the Legions' Worker");
	T.AssertEquals("ManAtArms", HOLFactionRulesRead::GetSlotStartType(FixtureRolesText, FixtureOverridesText, 1, "Hero"), "slot 1 (Legions) starts with the Legions' Hero");
	T.AssertEquals("Hero", HOLFactionRulesRead::GetSlotStartType(FixtureRolesText, FixtureOverridesText, 0, "Hero"), "slot 0 (Dawn) starts with its Hero");
	T.AssertEquals("MainCentre", HOLFactionRulesRead::GetSlotStartType(FixtureRolesText, FixtureOverridesText, 1, "MainCentre"), "slot 1 (Legions) starts with a MainCentre");
	T.AssertEquals("Peasant", HOLFactionRulesRead::GetSlotStartType(FixtureRolesText, FixtureOverridesText, 5, "Worker"), "slot 5 (closed, Dawn by default) reads as the Dawn");
}

// Section 4: the shape: a bad header, a repeated role and an unknown role are problems, not silently dropped; so is a field that does not exist.
void Test_FactionTablesShape(FUnitTest& T)
{
	T.AssertTrue(HOLFactionRulesRead::CountRoleProblems("Name,Faction,Kind,Type\nA,Dawn,Worker,Peasant\n") > 0, "a wrong header is a problem");

	FString Repeated = "Name,Faction,Role,Type\nA,Dawn,Worker,Peasant\nB,Dawn,Worker,Zombie\nC,Dawn,Cook,Peasant\n";
	T.AssertEquals(2, HOLFactionRulesRead::CountRoleProblems(Repeated), "a repeated role and an unknown role: two problems");
	T.AssertEquals(1, HOLFactionRulesRead::CountRoleRows(Repeated), "... and the good row is kept");

	// The C++ test builds the table row by row (X: NoSuchField, Y: MaxHealth 123); here the same two rows go through the CSV parser.
	FString Bad =
		"Name,Faction,Type,Field,Value\n" +
		"X,Legions,Spearman,NoSuchField,1\n" +
		"Y,Legions,Spearman,MaxHealth,123\n";
	T.AssertEquals(0, HOLFactionRulesRead::CountOverrideProblems(Bad), "the two override rows read");
	T.AssertEquals(1, HOLFactionRulesRead::ApplyOverridesToUnitRow(Bad, "Legions", "Spearman"), "one override applies, the unknown field does not");
	T.AssertEquals(1, HOLFactionRulesRead::CountApplyOverridesProblems(Bad, "Legions", "Spearman"), "... and is named");
	T.AssertEquals(123, HOLFactionRulesRead::ApplyOverridesToUnitRowReadInt(Bad, "Legions", "Spearman", "MaxHealth"), "the field is written");
	T.AssertEquals(0, HOLFactionRulesRead::ApplyOverridesToUnitRow(Bad, "Dawn", "Spearman"), "the Dawn has no override to apply");
}
