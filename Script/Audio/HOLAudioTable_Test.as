// The table "event -> sound" alone (AudioInProgress.md A2, D483, D484): every event of the interface, the orders, the
// selection and the alerts has one cue (the pack and the file names of the Kenney CC0 sounds, volume, pitch range,
// cooldown, priority); the choice of a variant and the cooldown are pure functions of a seed and a clock in milliseconds,
// so the sounds are presentation and never reach the simulation or the network. Nothing here plays a sound.

// The cues the user wants SILENT (answer of the audio_compare trial): Hover, Toggle, Tick, Scroll. They stay in the table with no file.
bool AudioTestIsSilentEvent(int Event)
{
	return Event == HOLAudioEvent::UiHover || Event == HOLAudioEvent::UiToggle || Event == HOLAudioEvent::UiTick || Event == HOLAudioEvent::UiScroll;
}

// A cue that is not in the table gives an empty name.
void Test_AudioUnknownEventHasNoCue(FUnitTest& T)
{
	T.AssertFalse(HOLAudio::IsValid(-1), "-1 is not an event");
	T.AssertFalse(HOLAudio::IsValid(HOLAudio::Count()), "the count is not an event");
	FHOLAudioCue Cue = HOLAudio::CueFor(-1);
	T.AssertTrue(Cue.Name.IsEmpty(), "no name for an unknown event");
	T.AssertEquals(0, Cue.Stems.Num(), "no file for an unknown event");
	T.AssertTrue(HOLAudio::Count() > 0, "the table is not empty");
}

// Every event has its cue, with a name "Group.Name", at least one file, values in range.
void Test_AudioEveryEventHasACue(FUnitTest& T)
{
	for (int Event = 0; Event < HOLAudio::Count(); Event++)
	{
		FHOLAudioCue Cue = HOLAudio::CueFor(Event);
		T.AssertTrue(HOLAudio::IsValid(Event), f"event {Event} is valid");
		T.AssertEquals(Event, Cue.Event, f"event {Event}: the cue says which event it is");
		T.AssertTrue(Cue.Name.Contains("."), f"event {Event}: name '{Cue.Name}' is Group.Name");
		if (!AudioTestIsSilentEvent(Event))
		{
			T.AssertFalse(Cue.Pack.IsEmpty(), f"event {Event}: a pack");
			T.AssertTrue(Cue.Stems.Num() >= 1, f"event {Event}: at least one file");
		}
		T.AssertTrue(Cue.VolumePercent >= 1 && Cue.VolumePercent <= 100, f"event {Event}: volume 1 to 100");
		T.AssertTrue(Cue.PitchMinPercent >= 50 && Cue.PitchMinPercent <= Cue.PitchMaxPercent, f"event {Event}: pitch range ordered");
		T.AssertTrue(Cue.PitchMaxPercent <= 200, f"event {Event}: pitch at most 200 %");
		T.AssertTrue(Cue.CooldownMs >= 0, f"event {Event}: cooldown not negative");
		T.AssertTrue(Cue.Priority >= 0 && Cue.Priority <= HOLAudioPriority::Alert, f"event {Event}: priority in range");
	}
}

// One name per event, one file once per cue, and the path of a file is the asset convention.
void Test_AudioNamesAndFilesAreUnique(FUnitTest& T)
{
	TArray<FString> Names;
	for (int Event = 0; Event < HOLAudio::Count(); Event++)
	{
		FHOLAudioCue Cue = HOLAudio::CueFor(Event);
		T.AssertFalse(Names.Contains(Cue.Name), f"the name '{Cue.Name}' is used once");
		Names.Add(Cue.Name);
		TArray<FString> Seen;
		for (int Index = 0; Index < Cue.Stems.Num(); Index++)
		{
			T.AssertFalse(Seen.Contains(Cue.Stems[Index]), f"{Cue.Name}: file '{Cue.Stems[Index]}' listed once");
			Seen.Add(Cue.Stems[Index]);
			FString Path = HOLAudio::SoundPath(Cue, Index);
			FString Expected = f"/Game/Audio/Kenney/{Cue.Pack}/SW_{Cue.Stems[Index]}.SW_{Cue.Stems[Index]}";
			T.AssertEquals(Expected, Path, f"{Cue.Name}: asset path of file {Index}");
		}
	}
	T.AssertEquals("", HOLAudio::SoundPath(HOLAudio::CueFor(-1), 0), "no path for an empty cue");
	FHOLAudioCue Click = HOLAudio::CueFor(HOLAudioEvent::UiClick);
	T.AssertEquals("", HOLAudio::SoundPath(Click, Click.Stems.Num()), "no path past the last file");
	T.AssertEquals("", HOLAudio::SoundPath(Click, -1), "no path before the first file");
}

// The groups the player hears: each has at least one event, and the group name is the start of the cue's name.
void Test_AudioGroups(FUnitTest& T)
{
	int Ui = 0;
	int Orders = 0;
	int Selection = 0;
	int Alerts = 0;
	for (int Event = 0; Event < HOLAudio::Count(); Event++)
	{
		FHOLAudioCue Cue = HOLAudio::CueFor(Event);
		FString Group = HOLAudio::GroupOf(Event);
		T.AssertTrue(Cue.Name.StartsWith(Group + "."), f"{Cue.Name}: the group is '{Group}'");
		if (Group == "Ui") Ui++;
		else if (Group == "Order") Orders++;
		else if (Group == "Select") Selection++;
		else if (Group == "Alert") Alerts++;
		else T.AssertTrue(false, f"{Cue.Name}: unknown group '{Group}'");
	}
	T.AssertTrue(Ui >= 8, "the interface has its sounds (click, hover, open, close, back, error, confirm, toggle)");
	T.AssertTrue(Orders >= 4, "the orders have their sounds (move, attack, gather, build)");
	T.AssertTrue(Selection >= 2, "the selection has its sounds (unit, building)");
	T.AssertTrue(Alerts >= 5, "the alerts have their sounds");
	T.AssertEquals("", HOLAudio::GroupOf(-1), "no group for an unknown event");
}

// The events the GDD and the player's grid name must be there, by name.
void Test_AudioNamedEvents(FUnitTest& T)
{
	T.AssertEquals("Ui.Click", HOLAudio::CueFor(HOLAudioEvent::UiClick).Name, "click");
	T.AssertEquals("Ui.Error", HOLAudio::CueFor(HOLAudioEvent::UiError).Name, "error (missing resource)");
	T.AssertEquals("Order.Attack", HOLAudio::CueFor(HOLAudioEvent::OrderAttack).Name, "attack order");
	T.AssertEquals("Order.Gather", HOLAudio::CueFor(HOLAudioEvent::OrderGather).Name, "gather order");
	T.AssertEquals("Alert.UnderAttack", HOLAudio::CueFor(HOLAudioEvent::AlertUnderAttack).Name, "under attack");
	T.AssertEquals("Alert.Trade", HOLAudio::CueFor(HOLAudioEvent::AlertTrade).Name, "market trade (coins)");
	T.AssertEquals("Alert.Volcano", HOLAudio::CueFor(HOLAudioEvent::AlertVolcano).Name, "volcano alert");
}

// An alert outranks the interface and is not repeated as fast as a click (the player is under attack, not under a drum).
void Test_AudioAlertsOutrankTheInterface(FUnitTest& T)
{
	FHOLAudioCue Click = HOLAudio::CueFor(HOLAudioEvent::UiClick);
	FHOLAudioCue Hover = HOLAudio::CueFor(HOLAudioEvent::UiHover);
	FHOLAudioCue Attack = HOLAudio::CueFor(HOLAudioEvent::AlertUnderAttack);
	T.AssertTrue(Attack.Priority > Click.Priority, "under attack outranks a click");
	T.AssertTrue(Attack.CooldownMs > Click.CooldownMs, "under attack repeats slower than a click");
	T.AssertTrue(Hover.CooldownMs > 0, "hovering does not machine-gun");
	T.AssertTrue(Hover.VolumePercent <= Click.VolumePercent, "hovering is not louder than a click");
	for (int Event = 0; Event < HOLAudio::Count(); Event++)
	{
		if (HOLAudio::GroupOf(Event) == "Alert")
		{
			T.AssertEquals(HOLAudioPriority::Alert, HOLAudio::CueFor(Event).Priority, f"{HOLAudio::CueFor(Event).Name}: an alert has the alert priority");
		}
	}
}

// The variant is a function of the seed: same seed, same file; every file comes up; a negative seed is fine.
void Test_AudioPickIsDeterministicAndCoversEveryFile(FUnitTest& T)
{
	FHOLAudioCue Cue = HOLAudio::CueFor(HOLAudioEvent::OrderMove);
	T.AssertTrue(Cue.Stems.Num() >= 2, "the move order has variants");
	TArray<bool> Heard;
	for (int Index = 0; Index < Cue.Stems.Num(); Index++) Heard.Add(false);
	for (int Seed = -7; Seed < 40; Seed++)
	{
		int First = HOLAudio::Pick(Cue, Seed);
		int Second = HOLAudio::Pick(Cue, Seed);
		T.AssertEquals(First, Second, f"seed {Seed}: same seed, same file");
		T.AssertTrue(First >= 0 && First < Cue.Stems.Num(), f"seed {Seed}: the file is in range");
		if (First >= 0 && First < Cue.Stems.Num()) Heard[First] = true;
	}
	for (int Index = 0; Index < Heard.Num(); Index++) T.AssertTrue(Heard[Index], f"file {Index} of the click comes up");
	T.AssertEquals(-1, HOLAudio::Pick(HOLAudio::CueFor(-1), 3), "nothing to pick in an empty cue");
}

// The same file is never played twice in a row when the cue has another one.
void Test_AudioPickNextAvoidsTheLastFile(FUnitTest& T)
{
	FHOLAudioCue Cue = HOLAudio::CueFor(HOLAudioEvent::OrderMove);
	int Last = -1;
	for (int Seed = 0; Seed < 60; Seed++)
	{
		int Next = HOLAudio::PickNext(Cue, Seed, Last);
		T.AssertTrue(Next >= 0 && Next < Cue.Stems.Num(), f"seed {Seed}: in range");
		if (Last >= 0) T.AssertTrue(Next != Last, f"seed {Seed}: not the file just played ({Last})");
		Last = Next;
	}
	// A cue with a single file repeats it: there is nothing else.
	for (int Event = 0; Event < HOLAudio::Count(); Event++)
	{
		FHOLAudioCue One = HOLAudio::CueFor(Event);
		if (One.Stems.Num() == 1) T.AssertEquals(0, HOLAudio::PickNext(One, 5, 0), f"{One.Name}: a single file is repeated");
	}
}

// The cooldown: never played, plays; inside the window, no; at the end of the window, yes; a clock that went back (new
// match) plays.
void Test_AudioCooldown(FUnitTest& T)
{
	FHOLAudioCue Cue = HOLAudio::CueFor(HOLAudioEvent::OrderMove);
	T.AssertTrue(Cue.CooldownMs > 0, "the move order has a cooldown to test");
	T.AssertTrue(HOLAudio::MayPlay(Cue, 1000, -1), "never played: plays");
	T.AssertFalse(HOLAudio::MayPlay(Cue, 1000 + Cue.CooldownMs - 1, 1000), "one millisecond before the end: waits");
	T.AssertTrue(HOLAudio::MayPlay(Cue, 1000 + Cue.CooldownMs, 1000), "at the end of the window: plays");
	T.AssertFalse(HOLAudio::MayPlay(Cue, 1000, 1000), "at the same instant: waits");
	T.AssertTrue(HOLAudio::MayPlay(Cue, 100, 5000), "the clock went back: plays");
	T.AssertFalse(HOLAudio::MayPlay(HOLAudio::CueFor(-1), 1000, -1), "an empty cue never plays");
	FHOLAudioCue Free;
	Free.Name = "Test.Free";
	Free.Stems.Add("x");
	Free.CooldownMs = 0;
	T.AssertTrue(HOLAudio::MayPlay(Free, 500, 500), "no cooldown: plays at the same instant");
}
