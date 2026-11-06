// The sounds the user ELECTED by ear in the audio_compare trial (Retours/E-audio_compare.md, answer of 2026-11-06; AudioInProgress.md,
// D736: only the elected sounds are imported and committed, never the 145 candidates). Checked on the real table and on the real plan
// that the game plays (HOLAudio::Plan): the elected cue plays its elected file whatever the seed, the cues the user wants silent
// never play, and the cues he has not answered about keep the table they had.

// Click H, Open F, Close A, Back B, Error A, Confirm G, Select.Unit D: one file each, in the pack it comes from.
void Test_AudioElectedCuesPointAtTheElectedSound(FUnitTest& T)
{
	TArray<int> Events;
	TArray<FString> Packs;
	TArray<FString> Stems;
	Events.Add(HOLAudioEvent::UiClick);   Packs.Add("rpg-audio");     Stems.Add("beltHandle1");
	Events.Add(HOLAudioEvent::UiOpen);    Packs.Add("rpg-audio");     Stems.Add("bookFlip2");
	Events.Add(HOLAudioEvent::UiClose);   Packs.Add("rpg-audio");     Stems.Add("bookClose");
	Events.Add(HOLAudioEvent::SelectUnit); Packs.Add("rpg-audio");    Stems.Add("handleSmallLeather");
	Events.Add(HOLAudioEvent::UiBack);    Packs.Add("rpg-audio");     Stems.Add("cloth2");
	Events.Add(HOLAudioEvent::UiError);   Packs.Add("impact-sounds"); Stems.Add("impactMetal_heavy_000");
	Events.Add(HOLAudioEvent::UiConfirm); Packs.Add("rpg-audio");     Stems.Add("metalClick");
	for (int Index = 0; Index < Events.Num(); Index++)
	{
		FHOLAudioCue Cue = HOLAudio::CueFor(Events[Index]);
		const FString Expected = f"/Game/Audio/Kenney/{Packs[Index]}/SW_{Stems[Index]}.SW_{Stems[Index]}";
		T.AssertEquals(Packs[Index], Cue.Pack, f"{Cue.Name}: the elected pack");
		T.AssertEquals(1, Cue.Stems.Num(), f"{Cue.Name}: only the elected file");
		for (int Seed = -7; Seed < 40; Seed++)
		{
			FHOLAudioPlan Plan = HOLAudio::Plan(Events[Index], 5000, -1, Seed, -1);
			T.AssertTrue(Plan.bPlay, f"{Cue.Name}, seed {Seed}: plays");
			T.AssertEquals(Expected, Plan.Asset, f"{Cue.Name}, seed {Seed}: the elected asset");
		}
	}
}

// Hover, Toggle, Tick, Scroll, the five orders and Select.Building (every building will have its own sound later): the user wants NO sound. The cue stays in the table (same numbering) but nothing ever plays, at any
// time, with any seed, and there is no asset to load.
void Test_AudioSilentCuesNeverPlay(FUnitTest& T)
{
	TArray<int> Events;
	Events.Add(HOLAudioEvent::UiHover);
	Events.Add(HOLAudioEvent::UiToggle);
	Events.Add(HOLAudioEvent::UiTick);
	Events.Add(HOLAudioEvent::UiScroll);
	Events.Add(HOLAudioEvent::OrderMove);
	Events.Add(HOLAudioEvent::OrderAttack);
	Events.Add(HOLAudioEvent::OrderGather);
	Events.Add(HOLAudioEvent::OrderBuild);
	Events.Add(HOLAudioEvent::OrderGarrison);
	Events.Add(HOLAudioEvent::SelectBuilding);
	T.AssertEquals(24, HOLAudio::Count(), "the table still has its 24 events");
	for (int Index = 0; Index < Events.Num(); Index++)
	{
		FHOLAudioCue Cue = HOLAudio::CueFor(Events[Index]);
		T.AssertEquals(Events[Index], Cue.Event, f"{Cue.Name}: still in the table, same event number");
		T.AssertTrue(Cue.Name.Contains("."), f"{Cue.Name}: still named");
		T.AssertEquals(0, Cue.Stems.Num(), f"{Cue.Name}: no file");
		T.AssertFalse(HOLAudio::MayPlay(Cue, 100000, -1), f"{Cue.Name}: may not play");
		for (int Seed = -7; Seed < 40; Seed++)
		{
			FHOLAudioPlan Plan = HOLAudio::Plan(Events[Index], 100000 + Seed * 1000, -1, Seed, -1);
			T.AssertFalse(Plan.bPlay, f"{Cue.Name}, seed {Seed}: nothing plays");
			T.AssertTrue(Plan.Asset.IsEmpty(), f"{Cue.Name}, seed {Seed}: no asset");
		}
	}
}

// The user has not answered about the alerts (audio_compare_alerts, not yet listened to): they keep the table they had.
void Test_AudioUnansweredCuesAreUnchanged(FUnitTest& T)
{
	T.AssertEquals("impact-sounds", HOLAudio::CueFor(HOLAudioEvent::AlertUnderAttack).Pack, "Alert.UnderAttack: unchanged");
	T.AssertEquals(3, HOLAudio::CueFor(HOLAudioEvent::AlertUnderAttack).Stems.Num(), "Alert.UnderAttack: unchanged files");
	T.AssertEquals("question_001", HOLAudio::CueFor(HOLAudioEvent::AlertDiplomacy).Stems[0], "Alert.Diplomacy: unchanged");
}
