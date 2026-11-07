// The candidates for the interface sounds (AudioInProgress.md, user's REGLAGE of 2026-11-05: the Kenney interface-sounds pack
// sounds like a spaceship console; the interface must sound medieval fantasy: wood, leather, parchment, dull metal, bell,
// stone, strings). For EACH cue of the table, ten variants A to J that the user listens to one after the other (scenario
// audio_compare) and picks by ear. Nothing here plays a sound and nothing is chosen for the user.

// Every cue of the table has its ten candidates, and only those (no cue unknown to the table).
void Test_AudioCandidatesEveryCueHasTen(FUnitTest& T)
{
	T.AssertEquals(HOLAudio::Count(), HOLAudioCandidates::CueCount(), "one candidate list per cue of the table");
	for (int Event = 0; Event < HOLAudio::Count(); Event++)
	{
		FString Name = HOLAudio::CueFor(Event).Name;
		FHOLAudioCandidates List = HOLAudioCandidates::ForCue(Name);
		T.AssertEquals(Name, List.Cue, f"{Name}: the list is found by the cue's name");
		T.AssertEquals(10, List.Stems.Num(), f"{Name}: ten variants");
	}
	T.AssertTrue(HOLAudioCandidates::ForCue("Nothing.Here").Stems.Num() == 0, "an unknown cue has no candidate");
}

// Labels A to J, assets under /Game/Audio/Candidates, kenney or vsco by the stem's name, no stem twice in a cue.
void Test_AudioCandidatesLabelsAndAssets(FUnitTest& T)
{
	T.AssertEquals("A", HOLAudioCandidates::Label(0), "first label");
	T.AssertEquals("J", HOLAudioCandidates::Label(9), "tenth label");
	T.AssertEquals("vsco", HOLAudioCandidates::Folder("v_harp_C5"), "v_ stems come from the VSCO samples");
	T.AssertEquals("kenney", HOLAudioCandidates::Folder("bookOpen"), "the others from the Kenney packs");
	T.AssertEquals("/Game/Audio/Candidates/kenney/SW_bookOpen.SW_bookOpen", HOLAudioCandidates::AssetPath("bookOpen"), "asset path, Kenney");
	T.AssertEquals("/Game/Audio/Candidates/vsco/SW_v_harp_C5.SW_v_harp_C5", HOLAudioCandidates::AssetPath("v_harp_C5"), "asset path, VSCO");
	for (int Event = 0; Event < HOLAudio::Count(); Event++)
	{
		FHOLAudioCandidates List = HOLAudioCandidates::ForCue(HOLAudio::CueFor(Event).Name);
		TArray<FString> Seen;
		for (int Index = 0; Index < List.Stems.Num(); Index++)
		{
			T.AssertFalse(Seen.Contains(List.Stems[Index]), f"{List.Cue}: {List.Stems[Index]} appears once");
			Seen.Add(List.Stems[Index]);
		}
	}
}

// The spaceship sounds are out: no candidate is a file of the Kenney interface-sounds pack (nor its names: select_, tick_,
// pluck_, maximize_, minimize_, bong_, question_, confirmation_, glitch_, scratch_, toggle_, switch_, drop_, back_, open_, close_,
// error_, scroll_, click_00x).
void Test_AudioCandidatesAreNotTheSpaceshipPack(FUnitTest& T)
{
	TArray<FString> Banned;
	Banned.Add("select_");
	Banned.Add("tick_");
	Banned.Add("pluck_");
	Banned.Add("maximize_");
	Banned.Add("minimize_");
	Banned.Add("bong_");
	Banned.Add("question_");
	Banned.Add("confirmation_");
	Banned.Add("glitch_");
	Banned.Add("scratch_");
	Banned.Add("toggle_");
	Banned.Add("switch");
	Banned.Add("drop_");
	Banned.Add("back_");
	Banned.Add("open_");
	Banned.Add("close_");
	Banned.Add("error_");
	Banned.Add("scroll_");
	Banned.Add("click");
	Banned.Add("rollover");
	for (int Event = 0; Event < HOLAudio::Count(); Event++)
	{
		FHOLAudioCandidates List = HOLAudioCandidates::ForCue(HOLAudio::CueFor(Event).Name);
		for (int Index = 0; Index < List.Stems.Num(); Index++)
			for (int Word = 0; Word < Banned.Num(); Word++)
				T.AssertFalse(List.Stems[Index].StartsWith(Banned[Word]), f"{List.Cue}: {List.Stems[Index]} is not a sci-fi interface sound");
	}
}

// Alert.TrainingDone is a MARTIAL cue (user, 2026-11-06: "un truc martial"; the first round gave harp, bells and a book): at least
// eight of its ten variants are war sounds (horn, trumpet, timpani, trombone, sword, shield, metal), none a harp, a glockenspiel,
// a bell of the Nepalese set or a book.
void Test_AudioCandidatesTrainingDoneIsMartial(FUnitTest& T)
{
	FHOLAudioCandidates List = HOLAudioCandidates::ForCue("Alert.TrainingDone");
	T.AssertEquals(10, List.Stems.Num(), "ten variants");
	TArray<FString> Martial;
	Martial.Add("v_horn");
	Martial.Add("v_trumpet");
	Martial.Add("v_timpani");
	Martial.Add("v_trombone");
	Martial.Add("drawKnife");
	Martial.Add("knifeSlice");
	Martial.Add("impactPlate");
	Martial.Add("impactMetal");
	TArray<FString> Soft;
	Soft.Add("v_harp");
	Soft.Add("v_glock");
	Soft.Add("v_nbell");
	Soft.Add("v_cello");
	Soft.Add("v_vln");
	Soft.Add("book");
	int Count = 0;
	for (int Index = 0; Index < List.Stems.Num(); Index++)
	{
		for (int Word = 0; Word < Martial.Num(); Word++)
			if (List.Stems[Index].StartsWith(Martial[Word])) { Count++; break; }
		for (int Word = 0; Word < Soft.Num(); Word++)
			T.AssertFalse(List.Stems[Index].StartsWith(Soft[Word]), f"{List.Stems[Index]} is not martial (a harp, a bell or a book)");
	}
	T.AssertTrue(Count >= 8, f"at least 8 martial variants for the training alert, got {Count}");
}

// The line the player writes when a candidate plays (the scenario waits for it): cue, label, stem.
void Test_AudioCandidatesPlayedLine(FUnitTest& T)
{
	T.AssertEquals("candidate played Ui.Open B bookOpen", HOLAudioCandidates::PlayedLine("Ui.Open", 1, "bookOpen"), "the log line");
	T.AssertEquals("candidate missing Ui.Open B asset /Game/x", HOLAudioCandidates::MissingLine("Ui.Open", 1, "/Game/x"), "the missing line");
}
