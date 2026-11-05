// Plays the table's sounds (AudioInProgress.md A2b): one per screen. Play(Event) asks HOLAudio::Plan what to play (cooldown, file,
// pitch), loads the sound wave imported under /Game/Audio/Kenney (Tools/Editor/import_kenney_sounds.py) and plays it in 2D, then
// writes "cue played ..." to the log (HOLAudio::PlayedLine); a sound that cannot be loaded writes "cue missing ..." and plays nothing.
// Presentation only: it reads the clock of the world for the cooldown and touches neither the simulation nor the network.
// The sounds are provisional (chosen by file name, not yet listened to): the user settles them by ear.
class UHOLAudioPlayer : UObject
{
	// Per event: when it last played (ms, -1 never) and which file.
	TArray<int> LastMs;
	TArray<int> LastFile;
	// Mixed into the seed so that two plays of the same cue do not draw the same file or pitch.
	int Counter = 0;
	// What was played since the start (a test reads it through the log; this is for the screen's report).
	int PlayedCount = 0;
	FString LastLine = "";

	void Prepare()
	{
		if (LastMs.Num() == HOLAudio::Count())
			return;
		LastMs.SetNum(HOLAudio::Count());
		LastFile.SetNum(HOLAudio::Count());
		for (int Index = 0; Index < LastMs.Num(); Index++)
		{
			LastMs[Index] = -1;
			LastFile[Index] = -1;
		}
	}

	// True when a sound was started.
	bool Play(int Event)
	{
		if (!HOLAudio::IsValid(Event))
			return false;
		Prepare();
		const int NowMs = int(Gameplay::GetTimeSeconds() * 1000.0);
		Counter++;
		FHOLAudioPlan Plan = HOLAudio::Plan(Event, NowMs, LastMs[Event], Counter * 7919 + Event * 31, LastFile[Event]);
		if (!Plan.bPlay)
			return false;
		USoundBase Sound = Cast<USoundBase>(LoadObject(nullptr, Plan.Asset));
		if (Sound == nullptr)
		{
			Log(n"LogHOLMatch", f"cue missing {Plan.Name} asset {Plan.Asset}");
			return false;
		}
		Gameplay::PlaySound2D(Sound, float(Plan.VolumePercent) / 100.0, float(Plan.PitchPercent) / 100.0);
		LastMs[Event] = NowMs;
		LastFile[Event] = Plan.FileIndex;
		PlayedCount++;
		LastLine = HOLAudio::PlayedLine(Plan);
		Log(n"LogHOLMatch", LastLine);
		return true;
	}
}
