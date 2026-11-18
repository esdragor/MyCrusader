// Lot E3-13 of the migration, part "rule" (D830, D836, D839): the script port of the Map.Rules test of HOLMapRulesTests.cpp (the search part
// of the rule), run against the read library of the rule that STAYS in C++ (HOLMapRulesReadLibrary, S1D-1116c). Same vectors as the C++
// test; the Map.Csv part is already in python (Tools\Tests\map_table_test.py, 32b6154). The C++ file is removed at lot MIG-C++-FINAL.
// Not ported (the library does not expose it): the FTextInspector checks (text key Map_Plain / Map_Steppe, namespace HOLMaps, source
// string), FText::IdenticalTo, IsCultureInvariant, and the FSoftObjectPath form of the file (the library takes the path as text).

// HeroesOfLiberty.Map.Rules, part 1: the first map (the default of a new game), found by its key whatever the case; the second map.
void Test_MapRulesByKey(FUnitTest& T)
{
	TArray<FHOLMapInfoValue> Maps = HOLMapRulesRead::GetMaps();
	T.AssertTrue(Maps.Num() >= 2, "at least two maps");
	if (Maps.Num() < 2)
	{
		return;
	}
	T.AssertTrue(Maps[0].Key == "Plain", "Plain first");
	FHOLMapInfoValue Plain;
	T.AssertTrue(HOLMapRulesRead::FindByKey("Plain", Plain), "Plain by key");
	T.AssertTrue(Plain.Key == "Plain", "... the same map");
	FHOLMapInfoValue Lower;
	T.AssertTrue(HOLMapRulesRead::FindByKey("plain", Lower), "key case ignored");
	T.AssertTrue(Lower.Key == Maps[0].Key, "... lower case finds the first map");
	FHOLMapInfoValue Spaced;
	T.AssertTrue(HOLMapRulesRead::FindByKey(" PLAIN ", Spaced), "key case and blanks ignored");
	T.AssertTrue(Spaced.Key == Maps[0].Key, "... upper case with blanks finds the first map");
	FHOLMapInfoValue None;
	T.AssertFalse(HOLMapRulesRead::FindByKey("Desert", None), "unknown key");
	T.AssertFalse(HOLMapRulesRead::FindByKey("", None), "empty key");
	T.AssertTrue(Plain.DisplayName.ToString() == "Plaine", "Plain's display name");

	// La Steppe (C1), the second map: found by its key, its French name the source text of Map_Steppe.
	FHOLMapInfoValue Steppe;
	T.AssertTrue(HOLMapRulesRead::FindByKey("steppe", Steppe), "Steppe by key");
	T.AssertTrue(Steppe.Key == Maps[1].Key, "Steppe second");
	T.AssertTrue(Steppe.DisplayName.ToString() == "La Steppe", "Steppe's display name");
	FHOLMapInfoValue SteppeByFile;
	T.AssertTrue(HOLMapRulesRead::FindByAsset("/Game/Maps/Steppe.Steppe", SteppeByFile), "Steppe by file");
	T.AssertTrue(SteppeByFile.Key == Steppe.Key, "... the same map");
}

// HeroesOfLiberty.Map.Rules, part 2: the file of a map, the map of a file (the sized files, TC-Q3, and a test map are not maps of the table).
void Test_MapRulesByAsset(FUnitTest& T)
{
	const FString PlainPath = "/Game/Maps/Plain.Plain";
	T.AssertTrue(HOLMapRulesRead::GetAssetPath("Plain") == PlainPath, "Plain's file");
	FHOLMapInfoValue Plain;
	T.AssertTrue(HOLMapRulesRead::FindByAsset(PlainPath, Plain), "Plain by file");
	T.AssertTrue(Plain.Key == "Plain", "... the same map");
	FHOLMapInfoValue Lower;
	T.AssertTrue(HOLMapRulesRead::FindByAsset("/Game/Maps/plain.plain", Lower), "Plain by file, case ignored");
	T.AssertTrue(Lower.Key == Plain.Key, "... the same map");
	FHOLMapInfoValue None;
	T.AssertFalse(HOLMapRulesRead::FindByAsset("/Game/Maps/Plain_Large.Plain_Large", None), "a sized file is not a map");
	T.AssertFalse(HOLMapRulesRead::FindByAsset("/Game/Maps/Test.Test", None), "a test map is not a map");
	T.AssertFalse(HOLMapRulesRead::FindByAsset("", None), "no file, no map");
}

// HeroesOfLiberty.Map.Rules, part 3: the name shown, the table's or the asset name as it is.
void Test_MapRulesDisplayName(FUnitTest& T)
{
	const FString PlainPath = "/Game/Maps/Plain.Plain";
	T.AssertTrue(HOLMapRulesRead::GetDisplayName(PlainPath).ToString() == "Plaine", "Plain shown");
	T.AssertTrue(HOLMapRulesRead::GetDisplayName("/Game/Maps/Test.Test").ToString() == "Test", "an unknown map shown by its asset name");
	T.AssertTrue(HOLMapRulesRead::GetDisplayName("/Game/Maps/Plain_Large.Plain_Large").ToString() == "Plain_Large", "a sized file shown by its asset name");
}
