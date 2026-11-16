// HOLGameNamesTests.cpp ported to script (E3-09, socle step 1, D830, D836, D839): the rule stays in C++ (HOLGameNames.h, read by
// HOLGameNamesReadLibrary, namespace HOLGameNamesRead), the vectors are those of the C++ tests. The C++ test stays until MIG-C++-FINAL.
// Not ported: FactionNames (its last half walks HOLFactionRules::GetTable().Overrides: E3-06, tests that build a world; the Legions-versus-Dawn
// half is already in Test_GameNamesReadLibrarySmoke), the language switch (FScopedLanguage) and the .locres read of LegionsLanguages (en, ja),
// the FTextInspector key and namespace checks of ForgeKey (none of these is exposed to script). The French text is checked only when the
// machine reads French (the script cannot switch the language), and ASCII names only (UTF-8 of the script not verified).

// LegionsLanguages, the French and Dawn halves: in any language the Legions' name differs from the Dawn's for each of the seven types; when
// the language is French (the source text) the names are those of the table.
// C++: HeroesOfLiberty.UI.GameNames.LegionsLanguages.
void Test_GameNamesLegionsNames(FUnitTest& T)
{
	TArray<FName> Types;
	Types.Add(n"Spearman");
	Types.Add(n"ManAtArms");
	Types.Add(n"Archer");
	Types.Add(n"Crossbowman");
	Types.Add(n"LightCavalry");
	Types.Add(n"Ram");
	Types.Add(n"Mangonel");
	for (FName Type : Types)
	{
		FString Own = HOLGameNamesRead::GetUnitNameForFaction(Type, n"Legions").ToString();
		T.AssertFalse(Own.IsEmpty(), "the Legions' name is not empty");
		T.AssertFalse(Own == HOLGameNamesRead::GetUnitNameForFaction(Type, n"Dawn").ToString(), "the Legions' name is not the Dawn's");
	}

	// The Dawn keeps its names (French); read only when the machine's language is French.
	if (HOLGameNamesRead::GetUnitNameForFaction(n"Spearman", n"Dawn").ToString() == "Lancier")
	{
		T.AssertTrue(HOLGameNamesRead::GetUnitNameForFaction(n"ManAtArms", n"Dawn").ToString() == "Homme d'armes", "ManAtArms of the Dawn keeps its name");
		T.AssertTrue(HOLGameNamesRead::GetUnitNameForFaction(n"Archer", n"Dawn").ToString() == "Archer", "Archer of the Dawn keeps its name");
		T.AssertTrue(HOLGameNamesRead::GetUnitNameForFaction(n"Spearman", n"Legions").ToString() == "Lancier squelette", "Spearman of the Legions in fr");
		T.AssertTrue(HOLGameNamesRead::GetUnitNameForFaction(n"ManAtArms", n"Legions").ToString() == "Garde noire", "ManAtArms of the Legions in fr");
		T.AssertTrue(HOLGameNamesRead::GetUnitNameForFaction(n"Archer", n"Legions").ToString() == "Archer squelette", "Archer of the Legions in fr");
		T.AssertTrue(HOLGameNamesRead::GetUnitNameForFaction(n"LightCavalry", n"Legions").ToString() == "Cavalier funeste", "LightCavalry of the Legions in fr");
		T.AssertTrue(HOLGameNamesRead::GetUnitNameForFaction(n"Mangonel", n"Legions").ToString() == "Mangonneau d'os", "Mangonel of the Legions in fr");
	}
}

// F1: the Forge's name (the localization key and namespace of the C++ test are not exposed): "Forge" in French and in English.
// C++: HeroesOfLiberty.UI.GameNames.ForgeKey.
void Test_GameNamesForgeName(FUnitTest& T)
{
	T.AssertTrue(HOLGameNamesRead::GetBuildingName(n"Forge").ToString() == "Forge", "the Forge's name");
}