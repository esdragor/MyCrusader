// What to play for an event (AudioInProgress.md A2b): HOLAudio::Plan turns an event, a clock in milliseconds, the time the event
// last played, a seed and the file played last into a plan (play or not, which file, which asset, volume and pitch in percent), from
// the table alone. No sound is played here (the player does that, with the plan); the plan is what the effect depends on.

// The first play of a cue: it plays, with a file of the cue, the asset path of that file, and the cue's volume and a pitch in its range.
void Test_AudioPlanFirstPlayUsesTheTable(FUnitTest& T)
{
	for (int Event = 0; Event < HOLAudio::Count(); Event++)
	{
		FHOLAudioCue Cue = HOLAudio::CueFor(Event);
		FHOLAudioPlan Plan = HOLAudio::Plan(Event, 5000, -1, 7 + Event, -1);
		if (Cue.Stems.Num() == 0)
		{
			T.AssertFalse(Plan.bPlay, f"{Cue.Name}: a silent cue does not play");
			continue;
		}
		T.AssertTrue(Plan.bPlay, f"{Cue.Name}: the first play goes");
		T.AssertEquals(Event, Plan.Event, f"{Cue.Name}: the plan says which event");
		T.AssertEquals(Cue.Name, Plan.Name, f"{Cue.Name}: the plan carries the cue's name");
		T.AssertTrue(Plan.FileIndex >= 0 && Plan.FileIndex < Cue.Stems.Num(), f"{Cue.Name}: a file of the cue");
		T.AssertEquals(HOLAudio::SoundPath(Cue, Plan.FileIndex), Plan.Asset, f"{Cue.Name}: the asset of that file");
		T.AssertEquals(Cue.VolumePercent, Plan.VolumePercent, f"{Cue.Name}: the cue's volume");
		T.AssertTrue(Plan.PitchPercent >= Cue.PitchMinPercent && Plan.PitchPercent <= Cue.PitchMaxPercent, f"{Cue.Name}: pitch in the cue's range");
	}
}

// The cooldown: inside it nothing plays (an empty plan with no asset), at its end it plays again.
void Test_AudioPlanCooldownBlocksThenReleases(FUnitTest& T)
{
	FHOLAudioCue Cue = HOLAudio::CueFor(HOLAudioEvent::UiClick);
	T.AssertTrue(Cue.CooldownMs > 1, "the click has a cooldown");
	FHOLAudioPlan Early = HOLAudio::Plan(HOLAudioEvent::UiClick, 1000 + Cue.CooldownMs - 1, 1000, 3, -1);
	T.AssertFalse(Early.bPlay, "inside the cooldown: not played");
	T.AssertTrue(Early.Asset.IsEmpty(), "inside the cooldown: no asset");
	FHOLAudioPlan Late = HOLAudio::Plan(HOLAudioEvent::UiClick, 1000 + Cue.CooldownMs, 1000, 3, -1);
	T.AssertTrue(Late.bPlay, "at the end of the cooldown: played");
}

// An unknown event never plays.
void Test_AudioPlanUnknownEventDoesNotPlay(FUnitTest& T)
{
	FHOLAudioPlan Plan = HOLAudio::Plan(-1, 5000, -1, 1, -1);
	T.AssertFalse(Plan.bPlay, "-1 does not play");
	T.AssertFalse(HOLAudio::Plan(HOLAudio::Count(), 5000, -1, 1, -1).bPlay, "the count does not play");
}

// The same file is not heard twice in a row when the cue has another, whatever the seed.
void Test_AudioPlanNeverRepeatsTheLastFile(FUnitTest& T)
{
	for (int Event = 0; Event < HOLAudio::Count(); Event++)
	{
	FHOLAudioCue Cue = HOLAudio::CueFor(Event);
	if (Cue.Stems.Num() < 2)
		continue;
	for (int Last = 0; Last < Cue.Stems.Num(); Last++)
	{
		for (int Seed = -7; Seed < 20; Seed++)
		{
			FHOLAudioPlan Plan = HOLAudio::Plan(Event, 5000, -1, Seed, Last);
			T.AssertTrue(Plan.FileIndex != Last, f"{Cue.Name}: last {Last}, seed {Seed}: another file");
		}
	}
	}
}

// The same inputs give the same plan (presentation only: no hidden state, no clock).
void Test_AudioPlanIsDeterministic(FUnitTest& T)
{
	FHOLAudioPlan A = HOLAudio::Plan(HOLAudioEvent::AlertUnderAttack, 90000, 100, 42, 1);
	FHOLAudioPlan B = HOLAudio::Plan(HOLAudioEvent::AlertUnderAttack, 90000, 100, 42, 1);
	T.AssertEquals(A.Asset, B.Asset, "same asset");
	T.AssertEquals(A.PitchPercent, B.PitchPercent, "same pitch");
	T.AssertEquals(A.VolumePercent, B.VolumePercent, "same volume");
}

// The log line the player writes for a played cue (what a test of the game reads): the name, the file, the asset.
void Test_AudioPlanLineNamesTheCueAndTheFile(FUnitTest& T)
{
	FHOLAudioPlan Plan = HOLAudio::Plan(HOLAudioEvent::AlertTechDone, 5000, -1, 0, -1);
	FString Line = HOLAudio::PlayedLine(Plan);
	T.AssertTrue(Line.StartsWith("cue played "), "starts with 'cue played '");
	T.AssertTrue(Line.Contains("Alert.TechDone"), "carries the cue's name");
	T.AssertTrue(Line.Contains(Plan.Asset), "carries the asset");
}
