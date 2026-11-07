// AudioBuildingsInProgress.md B3/B4 (user's answer Q3 = A of 2026-11-07 10:36): the selection of a building gets ONE sound per FAMILY of buildings,
// about five sounds: wood (House, LumberCamp, MiningCamp, Mill, Farm), stone (MainCentre, Barracks, ArcheryRange, Stable, SiegeWorkshop),
// sacred (Chapel, Sanctuary), metal (Forge), coins (Market). Construction done, production and destruction stay silent.
// This step is DATA only: the tables "family -> cue name" and "building -> family" (HOLAudioBuildings.as) and the ten candidates of each family cue
// (HOLAudioCandidates.as, listened to in the scenario audio_compare_buildings). NOTHING is chosen and NOTHING new is heard: Select.Building stays
// silent until the user picks a sound by ear and the game (C++, step B5) rings it.

// Every building of the audio table belongs to a family that exists; the families are the user's five.
void Test_AudioBuildingFamiliesEveryBuildingHasOne(FUnitTest& T)
{
	T.AssertEquals(5, HOLAudioBuildings::FamilyCount(), "five families (the user's answer Q3 = A)");
	for (int Index = 0; Index < HOLAudioBuildings::Count(); Index++)
	{
		FString Name = HOLAudioBuildings::NameAt(Index);
		FString Family = HOLAudioBuildings::FamilyOf(Name);
		T.AssertFalse(Family.IsEmpty(), f"{Name} belongs to a family");
		T.AssertTrue(HOLAudioBuildings::HasFamily(Family), f"{Name}: the family {Family} exists");
		T.AssertFalse(HOLAudioBuildings::CueNameFor(Name).IsEmpty(), f"{Name} has a family cue name");
	}
	T.AssertEquals("", HOLAudioBuildings::FamilyOf("Spaceship"), "an unknown building has no family");
	T.AssertEquals("", HOLAudioBuildings::CueNameFor("Spaceship"), "an unknown building has no family cue");
	T.AssertEquals("", HOLAudioBuildings::CueNameOfFamily("Plastic"), "an unknown family has no cue");
	T.AssertFalse(HOLAudioBuildings::HasFamily("Plastic"), "an unknown family does not exist");
}

// The user's grouping, building by building (names as BuildingTypes.csv writes them).
void Test_AudioBuildingFamiliesFollowTheUsersGrouping(FUnitTest& T)
{
	TArray<FString> Wood;
	Wood.Add("House");
	Wood.Add("LumberCamp");
	Wood.Add("MiningCamp");
	Wood.Add("Mill");
	Wood.Add("Farm");
	for (int Index = 0; Index < Wood.Num(); Index++)
		T.AssertEquals("Wood", HOLAudioBuildings::FamilyOf(Wood[Index]), f"{Wood[Index]} is wood");
	TArray<FString> Stone;
	Stone.Add("MainCentre");
	Stone.Add("Barracks");
	Stone.Add("ArcheryRange");
	Stone.Add("Stable");
	Stone.Add("SiegeWorkshop");
	for (int Index = 0; Index < Stone.Num(); Index++)
		T.AssertEquals("Stone", HOLAudioBuildings::FamilyOf(Stone[Index]), f"{Stone[Index]} is stone");
	T.AssertEquals("Sacred", HOLAudioBuildings::FamilyOf("Chapel"), "the chapel is sacred");
	T.AssertEquals("Sacred", HOLAudioBuildings::FamilyOf("Sanctuary"), "the sanctuary is sacred");
	T.AssertEquals("Metal", HOLAudioBuildings::FamilyOf("Forge"), "the forge is metal");
	T.AssertEquals("Coins", HOLAudioBuildings::FamilyOf("Market"), "the market is coins");
}

// The cue name of each family ("Building.<Family>"); not an event of the sound table, so not heard by the game.
void Test_AudioBuildingFamiliesCueNames(FUnitTest& T)
{
	T.AssertEquals("Building.Wood", HOLAudioBuildings::CueNameOfFamily("Wood"), "wood cue");
	T.AssertEquals("Building.Stone", HOLAudioBuildings::CueNameOfFamily("Stone"), "stone cue");
	T.AssertEquals("Building.Sacred", HOLAudioBuildings::CueNameOfFamily("Sacred"), "sacred cue");
	T.AssertEquals("Building.Metal", HOLAudioBuildings::CueNameOfFamily("Metal"), "metal cue");
	T.AssertEquals("Building.Coins", HOLAudioBuildings::CueNameOfFamily("Coins"), "coins cue");
	T.AssertEquals("Building.Metal", HOLAudioBuildings::CueNameFor("Forge"), "the forge's cue is the metal one");
	T.AssertEquals("Building.Wood", HOLAudioBuildings::CueNameFor("Farm"), "the farm's cue is the wood one");
	for (int Event = 0; Event < HOLAudio::Count(); Event++)
	{
		FString Taken = HOLAudio::CueFor(Event).Name;
		for (int Index = 0; Index < HOLAudioBuildings::FamilyCount(); Index++)
			T.AssertTrue(Taken != HOLAudioBuildings::CueNameOfFamily(HOLAudioBuildings::FamilyNameAt(Index)), f"{Taken} is not a family cue (family cues are candidates only)");
	}
}

// Each family cue has its ten candidates A to J, none twice, each found by the cue name (the console command hol.Audio.Candidate reads them).
void Test_AudioBuildingFamiliesHaveTenCandidates(FUnitTest& T)
{
	for (int Index = 0; Index < HOLAudioBuildings::FamilyCount(); Index++)
	{
		FString Cue = HOLAudioBuildings::CueNameOfFamily(HOLAudioBuildings::FamilyNameAt(Index));
		FHOLAudioCandidates List = HOLAudioCandidates::ForCue(Cue);
		T.AssertEquals(Cue, List.Cue, f"{Cue}: the list is found by the cue's name");
		T.AssertEquals(10, List.Stems.Num(), f"{Cue}: ten variants A to J");
		TArray<FString> Seen;
		for (int Stem = 0; Stem < List.Stems.Num(); Stem++)
		{
			T.AssertFalse(Seen.Contains(List.Stems[Stem]), f"{Cue}: {List.Stems[Stem]} appears once");
			Seen.Add(List.Stems[Stem]);
		}
	}
}

// The EFFECT: a family cue name exists for every building, yet selecting a building still rings NOTHING on the plan the game plays
// (Select.Building stays silent until a sound is elected and wired in C++).
void Test_AudioBuildingFamiliesSelectionStaysSilent(FUnitTest& T)
{
	for (int Index = 0; Index < HOLAudioBuildings::Count(); Index++)
	{
		FString Name = HOLAudioBuildings::NameAt(Index);
		T.AssertFalse(HOLAudioBuildings::CueNameFor(Name).IsEmpty(), f"{Name}: the family cue exists");
		T.AssertFalse(HOLAudioBuildings::Rings(Name, HOLBuildingSound::Selected), f"{Name}: selecting is still silent");
		for (int Seed = -2; Seed < 8; Seed++)
		{
			FHOLAudioPlan Plan = HOLAudio::Plan(HOLAudioBuildings::EventFor(Name, HOLBuildingSound::Selected), 5000, -1, Seed, -1);
			T.AssertFalse(Plan.bPlay, f"{Name}, seed {Seed}: selecting plays nothing");
		}
	}
}
