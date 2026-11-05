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
		return Plan;
	}

	// The log line of a cue played (a test of the game reads it): "cue played Ui.Click file 2 asset /Game/... volume 80 pitch 101".
	FString PlayedLine(FHOLAudioPlan Plan)
	{
		return f"cue played {Plan.Name} file {Plan.FileIndex} asset {Plan.Asset} volume {Plan.VolumePercent} pitch {Plan.PitchPercent}";
	}
}
