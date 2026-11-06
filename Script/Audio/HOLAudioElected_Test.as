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
	Events.Add(HOLAudioEvent::AlertUnderAttack); Packs.Add("impact-sounds"); Stems.Add("impactBell_heavy_000");
	Events.Add(HOLAudioEvent::AlertTrainingDone); Packs.Add("rpg-audio"); Stems.Add("bookPlace2");
	Events.Add(HOLAudioEvent::AlertTrade); Packs.Add("rpg-audio"); Stems.Add("handleCoins");
	Events.Add(HOLAudioEvent::AlertVolcano); Packs.Add("impact-sounds"); Stems.Add("impactBell_heavy_003");
	Events.Add(HOLAudioEvent::AlertDiplomacy); Packs.Add("vsco"); Stems.Add("v_harp_arp_up");
	for (int Index = 0; Index < Events.Num(); Index++)
	{
		FHOLAudioCue Cue = HOLAudio::CueFor(Events[Index]);
		FString Expected = f"/Game/Audio/Kenney/{Packs[Index]}/SW_{Stems[Index]}.SW_{Stems[Index]}";
		if (Packs[Index] == "vsco")
			Expected = f"/Game/Audio/Vsco/SW_{Stems[Index]}.SW_{Stems[Index]}";
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

// Hover, Toggle, Tick, Scroll, the five orders, Select.Building (every building will have its own sound later), BuildingDone and TechDone: the user wants NO sound. The cue stays in the table (same numbering) but nothing ever plays, at any
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
	Events.Add(HOLAudioEvent::AlertBuildingDone);
	Events.Add(HOLAudioEvent::AlertTechDone);
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

// Every cue is answered now: none is left on the spaceship console packs the user rejected.
void Test_AudioNoSpaceshipPackIsLeft(FUnitTest& T)
{
	for (int Event = 0; Event < HOLAudio::Count(); Event++)
	{
		FHOLAudioCue Cue = HOLAudio::CueFor(Event);
		T.AssertTrue(Cue.Pack != "interface-sounds" && Cue.Pack != "ui-audio", f"{Cue.Name}: not a spaceship console sound");
	}
}

// The volcano's bell rings several times (the user: "mais plusieurs fois"): 3 times, about 1 s apart; the other cues ring once.
void Test_AudioVolcanoRepeats(FUnitTest& T)
{
	FHOLAudioCue Volcano = HOLAudio::CueFor(HOLAudioEvent::AlertVolcano);
	T.AssertEquals(3, Volcano.RepeatCount, "the volcano rings 3 times");
	T.AssertEquals(1000, Volcano.RepeatGapMs, "about 1 s apart");
	FHOLAudioPlan Plan = HOLAudio::Plan(HOLAudioEvent::AlertVolcano, 20000, -1, 5, -1);
	T.AssertEquals(3, Plan.RepeatCount, "the plan carries the repeats");
	T.AssertEquals(1000, Plan.RepeatGapMs, "and their gap");
	// The clock: the first ring at 10000 (one done); nothing before 11000; one more at 11000; the third at 12000; never a fourth.
	T.AssertEquals(0, HOLAudio::RepeatsDue(Volcano, 10000, 10999, 1), "999 ms later: not yet");
	T.AssertEquals(1, HOLAudio::RepeatsDue(Volcano, 10000, 11000, 1), "1 s later: the second ring");
	T.AssertEquals(0, HOLAudio::RepeatsDue(Volcano, 10000, 11999, 2), "after the second: waits");
	T.AssertEquals(1, HOLAudio::RepeatsDue(Volcano, 10000, 12000, 2), "2 s later: the third ring");
	T.AssertEquals(0, HOLAudio::RepeatsDue(Volcano, 10000, 99999, 3), "never a fourth");
	T.AssertEquals(2, HOLAudio::RepeatsDue(Volcano, 10000, 99999, 1), "a long freeze: the two missing rings");
	T.AssertEquals(0, HOLAudio::RepeatsDue(Volcano, 10000, 5000, 1), "a clock that went back: nothing");
	FHOLAudioCue Attack = HOLAudio::CueFor(HOLAudioEvent::AlertUnderAttack);
	T.AssertEquals(1, Attack.RepeatCount, "the attack bell rings once");
	T.AssertEquals(0, HOLAudio::RepeatsDue(Attack, 10000, 50000, 1), "no repeat for a cue that rings once");
}
