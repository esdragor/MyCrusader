// Socle step 1, lot S1d part 2 (D830, D836, D839, S1D-1116c): one smoke test per read library of the rules that STAY in C++, each on ONE vector
// read in its C++ test. The full port of each C++ test is the next lot (E3-02, E3-09, E3-12, E3-13 of Auto_Journey_File.md).
// This file compiles only once the plugin is rebuilt with the four libraries (HOLFactionChoiceReadLibrary, HOLManagerErrorTextReadLibrary,
// HOLMapRulesReadLibrary, HOLGameNamesReadLibrary): until then the script compile fails, as expected.

// HOLFactionChoiceTests.cpp (HeroesOfLiberty.Match.FactionChoice.List, Names, TripleClick).
void Test_FactionChoiceReadLibrarySmoke(FUnitTest& T)
{
	T.AssertTrue(HOLFactionChoiceRead::IsChoosable(EHOLFaction::DawnOrder, false), "the Dawn is chosen");
	T.AssertTrue(HOLFactionChoiceRead::IsListed(EHOLFaction::FireHeirs, false), "the Fire is listed...");
	T.AssertFalse(HOLFactionChoiceRead::IsChoosable(EHOLFaction::FireHeirs, false), "... but greyed");
	T.AssertFalse(HOLFactionChoiceRead::IsListed(EHOLFaction::Templars, false), "without the cheat the Templars are not listed");
	T.AssertEquals(5, HOLFactionChoiceRead::GetListed(false).Num(), "five entries without the cheat");
	T.AssertEquals(6, HOLFactionChoiceRead::GetListed(true).Num(), "six with it");
	T.AssertTrue(HOLFactionChoiceRead::Sanitize(EHOLFaction::Templars, false) == EHOLFaction::DawnOrder, "a Templar slot without the cheat plays the Dawn");
	T.AssertTrue(HOLFactionChoiceRead::GetName(EHOLFaction::DawnOrder) == "DawnOrder", "the manager's name of the Dawn");
	EHOLFaction Back = EHOLFaction::Templars;
	T.AssertTrue(HOLFactionChoiceRead::TryFindByName("BlackLegions", Back), "a name reads back");
	T.AssertTrue(Back == EHOLFaction::BlackLegions, "... to the same faction");
	EHOLFaction Unknown = EHOLFaction::BlackLegions;
	T.AssertFalse(HOLFactionChoiceRead::TryFindByName("Pirates", Unknown), "an unknown name is refused");

	FHOLClickCounter Clicks;
	T.AssertFalse(HOLFactionChoiceRead::Click(Clicks, 0.0), "one click");
	T.AssertFalse(HOLFactionChoiceRead::Click(Clicks, 0.3), "two clicks");
	T.AssertTrue(HOLFactionChoiceRead::Click(Clicks, 0.6), "three quick clicks are a triple");
	T.AssertFalse(HOLFactionChoiceRead::Click(Clicks, 0.7), "a fourth does not make another");
}

// HOLManagerErrorTextTests.cpp (HeroesOfLiberty.UI.ManagerErrorText.Codes): lobby_full, and the version_mismatch of /status.
void Test_ManagerErrorTextReadLibrarySmoke(FUnitTest& T)
{
	FHOLManagerErrorValue Full;
	Full.Kind = EHOLManagerErrorKind::Protocol;
	Full.Code = "lobby_full";
	Full.Message = "Raw message.";
	T.AssertTrue(HOLManagerErrorTextRead::GetManagerErrorText(Full).ToString() == "Ce salon est plein.", "lobby_full text");

	FHOLManagerErrorValue FromStatus = HOLManagerErrorTextRead::MakeVersionMismatchError("other-version", "2026-10-24-aaaaaaa");
	T.AssertTrue(FromStatus.Kind == EHOLManagerErrorKind::Protocol, "status mismatch is a protocol error");
	T.AssertTrue(FromStatus.Code == "version_mismatch", "status mismatch code");

	FHOLManagerErrorValue BadKey;
	BadKey.Kind = EHOLManagerErrorKind::Protocol;
	BadKey.Code = "bad_key";
	T.AssertFalse(HOLManagerErrorTextRead::CanCreateLobbyAfter(BadKey), "a refused access key greys the create button");
	T.AssertTrue(HOLManagerErrorTextRead::CanCreateLobbyAfter(Full), "any other failure keeps it");
}

// HOLMapRulesTests.cpp (HeroesOfLiberty.Map.Rules): the search part, the first map (the default of a new game) read from the table itself.
void Test_MapRulesReadLibrarySmoke(FUnitTest& T)
{
	TArray<FHOLMapInfoValue> Maps = HOLMapRulesRead::GetMaps();
	T.AssertTrue(Maps.Num() >= 1, "at least one map");
	if (Maps.Num() < 1)
	{
		return;
	}
	FHOLMapInfoValue ByKey;
	T.AssertTrue(HOLMapRulesRead::FindByKey(Maps[0].Key.ToUpper(), ByKey), "the key's case is ignored");
	T.AssertTrue(ByKey.Key == Maps[0].Key, "... and finds the first map");
	FHOLMapInfoValue ByAsset;
	T.AssertTrue(HOLMapRulesRead::FindByAsset(HOLMapRulesRead::GetAssetPath(Maps[0].Key), ByAsset), "the asset path of a map finds it back");
	T.AssertTrue(ByAsset.Key == Maps[0].Key, "... the same map");
	FHOLMapInfoValue None;
	T.AssertFalse(HOLMapRulesRead::FindByKey("NoSuchMap", None), "an unknown key is refused");
	T.AssertFalse(HOLMapRulesRead::FindByAsset("/Game/Maps/" + Maps[0].Key + "_Large." + Maps[0].Key + "_Large", None), "a sized file is not a table map");
}

// HOLGameNamesTests.cpp (HeroesOfLiberty.UI.GameNames.FactionNames): the Legions' Spearman has a name of its own, the Dawn's keeps the base name.
void Test_GameNamesReadLibrarySmoke(FUnitTest& T)
{
	FString Base = HOLGameNamesRead::GetUnitName(n"Spearman").ToString();
	FString Legions = HOLGameNamesRead::GetUnitNameForFaction(n"Spearman", n"Legions").ToString();
	FString Dawn = HOLGameNamesRead::GetUnitNameForFaction(n"Spearman", n"Dawn").ToString();
	T.AssertTrue(Dawn == Base, "the Dawn's Spearman keeps the base name");
	T.AssertTrue(HOLGameNamesRead::GetUnitNameForFaction(n"Spearman", NAME_None).ToString() == Base, "no faction keeps the base name");
	T.AssertFalse(Legions == Dawn, "the Legions' name is not the Dawn's");
	T.AssertFalse(HOLGameNamesRead::GetBuildingName(n"Barracks").IsEmpty(), "a building has a name");
}
