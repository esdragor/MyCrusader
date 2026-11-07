// AudioBuildingsInProgress.md, step 1: the table "building -> sound" (one sound per type of building, the user's idea of 2026-11-06). For each type of
// building of Tools/Data/BuildingTypes.csv, the audio event it rings for each kind of moment (selected, built, produced, destroyed). STEP 1 changes
// NOTHING that is heard: every building points at the event the game rings today, so the generic Select.Building and Alert.BuildingDone stay SILENT
// (the user's answer of 2026-11-06: a sound he wanted silent stays silent until he asks again), and the production moment rings what it rings today.
// Checked on the real table (HOLAudio::CueFor) and on the real plan the game plays (HOLAudio::Plan), and on the real type table for the names.

// The buildings of the type table the audio table must know (names as BuildingTypes.csv writes them).
// Every building of the audio table is a REAL type of the table (a typo would silently never ring).
void Test_AudioBuildingsAreRealTypes(FUnitTest& T)
{
	T.AssertTrue(HOLAudioBuildings::Count() > 0, "the audio table lists buildings");
	for (int Index = 0; Index < HOLAudioBuildings::Count(); Index++)
	{
		FString Name = HOLAudioBuildings::NameAt(Index);
		FHOLAITypeFigures Figures;
		T.AssertTrue(HOLAIOrder::ReadAIBuildingFigures(FName(Name), Figures), f"{Name} is a row of BuildingTypes.csv");
	}
	T.AssertTrue(HOLAudioBuildings::Has("MainCentre"), "the main centre is there");
	T.AssertTrue(HOLAudioBuildings::Has("House"), "the house is there");
	T.AssertTrue(HOLAudioBuildings::Has("Barracks"), "the barracks are there");
	T.AssertTrue(HOLAudioBuildings::Has("Forge"), "the forge is there");
	T.AssertFalse(HOLAudioBuildings::Has("Spaceship"), "an unknown building is not there");
}

// Selected and built: the generic cues are SILENT (the user's choice); no building rings either, whatever the seed, on the plan the game plays.
void Test_AudioBuildingsSelectAndBuiltStaySilent(FUnitTest& T)
{
	for (int Index = 0; Index < HOLAudioBuildings::Count(); Index++)
	{
		FString Name = HOLAudioBuildings::NameAt(Index);
		T.AssertEquals(HOLAudioEvent::SelectBuilding, HOLAudioBuildings::EventFor(Name, HOLBuildingSound::Selected), f"{Name}: selected rings the generic Select.Building today");
		T.AssertEquals(HOLAudioEvent::AlertBuildingDone, HOLAudioBuildings::EventFor(Name, HOLBuildingSound::Built), f"{Name}: built rings the generic Alert.BuildingDone today");
		T.AssertFalse(HOLAudioBuildings::Rings(Name, HOLBuildingSound::Selected), f"{Name}: selected is silent");
		T.AssertFalse(HOLAudioBuildings::Rings(Name, HOLBuildingSound::Built), f"{Name}: built is silent");
		for (int Seed = -3; Seed < 12; Seed++)
		{
			FHOLAudioPlan Selected = HOLAudio::Plan(HOLAudioBuildings::EventFor(Name, HOLBuildingSound::Selected), 5000, -1, Seed, -1);
			FHOLAudioPlan Built = HOLAudio::Plan(HOLAudioBuildings::EventFor(Name, HOLBuildingSound::Built), 5000, -1, Seed, -1);
			T.AssertFalse(Selected.bPlay, f"{Name}, seed {Seed}: selecting plays nothing");
			T.AssertFalse(Built.bPlay, f"{Name}, seed {Seed}: finishing the build plays nothing");
		}
	}
}

// Production finished: the buildings that train units ring what the game rings today for a trained unit (Alert.TrainingDone, its elected file);
// the Forge and the Sanctuary research (Alert.TechDone, silent); the Market trades (Alert.Trade); the others produce nothing and have no cue.
void Test_AudioBuildingsProductionKeepsTodaysCues(FUnitTest& T)
{
	TArray<FString> Trainers;
	Trainers.Add("MainCentre");
	Trainers.Add("Barracks");
	Trainers.Add("ArcheryRange");
	Trainers.Add("Stable");
	Trainers.Add("SiegeWorkshop");
	Trainers.Add("Chapel");
	FHOLAudioPlan Training = HOLAudio::Plan(HOLAudioEvent::AlertTrainingDone, 5000, -1, 0, -1);
	T.AssertTrue(Training.bPlay, "a trained unit rings today");
	for (int Index = 0; Index < Trainers.Num(); Index++)
	{
		FString Name = Trainers[Index];
		T.AssertEquals(HOLAudioEvent::AlertTrainingDone, HOLAudioBuildings::EventFor(Name, HOLBuildingSound::Produced), f"{Name}: a trained unit rings Alert.TrainingDone");
		FHOLAudioPlan Plan = HOLAudio::Plan(HOLAudioBuildings::EventFor(Name, HOLBuildingSound::Produced), 5000, -1, 0, -1);
		T.AssertTrue(Plan.bPlay, f"{Name}: the unit's sound plays");
		T.AssertEquals(Training.Asset, Plan.Asset, f"{Name}: the same file as today");
	}
	T.AssertEquals(HOLAudioEvent::AlertTechDone, HOLAudioBuildings::EventFor("Forge", HOLBuildingSound::Produced), "the forge researches: Alert.TechDone");
	T.AssertEquals(HOLAudioEvent::AlertTechDone, HOLAudioBuildings::EventFor("Sanctuary", HOLBuildingSound::Produced), "the sanctuary researches: Alert.TechDone");
	T.AssertFalse(HOLAudioBuildings::Rings("Forge", HOLBuildingSound::Produced), "Alert.TechDone is silent (the user's choice)");
	T.AssertEquals(HOLAudioEvent::AlertTrade, HOLAudioBuildings::EventFor("Market", HOLBuildingSound::Produced), "the market trades: Alert.Trade");
	T.AssertTrue(HOLAudioBuildings::Rings("Market", HOLBuildingSound::Produced), "a trade rings today");
	T.AssertEquals(-1, HOLAudioBuildings::EventFor("House", HOLBuildingSound::Produced), "a house produces nothing");
	T.AssertEquals(-1, HOLAudioBuildings::EventFor("Farm", HOLBuildingSound::Produced), "a farm produces nothing");
	T.AssertEquals(-1, HOLAudioBuildings::EventFor("LumberCamp", HOLBuildingSound::Produced), "a lumber camp produces nothing");
	T.AssertEquals(-1, HOLAudioBuildings::EventFor("MiningCamp", HOLBuildingSound::Produced), "a mining camp produces nothing");
	T.AssertEquals(-1, HOLAudioBuildings::EventFor("Mill", HOLBuildingSound::Produced), "a mill produces nothing");
}

// Destroyed: the game has no such event today, so no building rings it; an unknown building or an unknown kind has no event and no cue.
void Test_AudioBuildingsDestroyedAndUnknownHaveNoCue(FUnitTest& T)
{
	for (int Index = 0; Index < HOLAudioBuildings::Count(); Index++)
	{
		FString Name = HOLAudioBuildings::NameAt(Index);
		T.AssertEquals(-1, HOLAudioBuildings::EventFor(Name, HOLBuildingSound::Destroyed), f"{Name}: no destruction event today");
		T.AssertFalse(HOLAudioBuildings::Rings(Name, HOLBuildingSound::Destroyed), f"{Name}: destruction is silent");
	}
	T.AssertEquals(-1, HOLAudioBuildings::EventFor("Spaceship", HOLBuildingSound::Selected), "an unknown building has no event");
	T.AssertEquals(-1, HOLAudioBuildings::EventFor("House", 99), "an unknown kind has no event");
	T.AssertEquals(-1, HOLAudioBuildings::EventFor("House", -1), "a negative kind has no event");
	T.AssertTrue(HOLAudioBuildings::CueFor("Spaceship", HOLBuildingSound::Selected).Name.IsEmpty(), "an unknown building has an empty cue");
	T.AssertFalse(HOLAudioBuildings::Rings("Spaceship", HOLBuildingSound::Selected), "an unknown building never rings");
}

// The cue of a building and a kind is the table's own cue of its event (same name, same files): this table only chooses WHICH event.
void Test_AudioBuildingsCueIsTheTablesCue(FUnitTest& T)
{
	FHOLAudioCue Cue = HOLAudioBuildings::CueFor("Barracks", HOLBuildingSound::Produced);
	FHOLAudioCue Expected = HOLAudio::CueFor(HOLAudioEvent::AlertTrainingDone);
	T.AssertEquals(Expected.Name, Cue.Name, "the barracks' production cue is Alert.TrainingDone");
	T.AssertEquals(Expected.Stems.Num(), Cue.Stems.Num(), "with the same files");
	T.AssertEquals(Expected.VolumePercent, Cue.VolumePercent, "with the same volume");
}
