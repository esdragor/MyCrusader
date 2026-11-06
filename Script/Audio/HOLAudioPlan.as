// What to play for an event (AudioInProgress.md A2b), from the table alone: pure, with the seed and the clock given by the caller
// (HOLAudioPlayer.as), so that it is tested without a world and never reaches the simulation or the network. Integers only (percent).

struct FHOLAudioPlan
{
	// False: nothing is played (unknown event, or the cue is still in its cooldown).
	bool bPlay = false;
	int Event = -1;
	FString Name = "";
	// The file of the cue (index in its Stems), -1 for none.
	int FileIndex = -1;
	// /Game/Audio/Kenney/<pack>/SW_<file>.SW_<file>, "" when nothing plays.
	FString Asset = "";
	int VolumePercent = 100;
	int PitchPercent = 100;
	// How many times the cue rings in a row and how far apart (from the cue).
	int RepeatCount = 1;
	int RepeatGapMs = 0;
}

namespace HOLAudio
{
	// The plan of an event at NowMs: LastMs = when this event last played (-1 never), LastFile = the file it played then (-1 none).
	FHOLAudioPlan Plan(int Event, int NowMs, int LastMs, int Seed, int LastFile)
	{
		FHOLAudioPlan Plan;
		FHOLAudioCue Cue = CueFor(Event);
		if (!MayPlay(Cue, NowMs, LastMs))
			return Plan;
		const int Index = PickNext(Cue, Seed, LastFile);
		if (Index < 0)
			return Plan;
		// The pitch is drawn from the seed too, away from the file's choice (Seed / 7).
		int Spread = Cue.PitchMaxPercent - Cue.PitchMinPercent + 1;
		int Drawn = (Seed / 7) % Spread;
		if (Drawn < 0)
			Drawn += Spread;
		Plan.bPlay = true;
		Plan.Event = Event;
		Plan.Name = Cue.Name;
		Plan.FileIndex = Index;
		Plan.Asset = SoundPath(Cue, Index);
		Plan.VolumePercent = Cue.VolumePercent;
		Plan.PitchPercent = Cue.PitchMinPercent + Drawn;
		Plan.RepeatCount = Cue.RepeatCount;
		Plan.RepeatGapMs = Cue.RepeatGapMs;
		return Plan;
	}

	// How many more rings of a repeating cue are due at NowMs: the first rang at StartMs, Done rings have been heard (the first included).
	// Ring number k (from 0) is due at StartMs + k * RepeatGapMs; never more than RepeatCount rings; 0 for a cue that rings once or a clock
	// that went back.
	int RepeatsDue(FHOLAudioCue Cue, int StartMs, int NowMs, int Done)
	{
		if (Cue.RepeatCount <= 1 || Cue.RepeatGapMs <= 0 || NowMs < StartMs)
			return 0;
		int Allowed = 1 + (NowMs - StartMs) / Cue.RepeatGapMs;
		if (Allowed > Cue.RepeatCount)
			Allowed = Cue.RepeatCount;
		return Math::Max(0, Allowed - Done);
	}

	// The log line of a cue played (a test of the game reads it): "cue played Ui.Click file 2 asset /Game/... volume 80 pitch 101".
	FString PlayedLine(FHOLAudioPlan Plan)
	{
		return f"cue played {Plan.Name} file {Plan.FileIndex} asset {Plan.Asset} volume {Plan.VolumePercent} pitch {Plan.PitchPercent}";
	}
}
