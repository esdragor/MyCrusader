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
	// Per repeating event (the volcano's bell): when the first ring was heard and how many rings so far (0 = none pending).
	TArray<int> RepeatStartMs;
	TArray<int> RepeatDone;
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
		RepeatStartMs.SetNum(HOLAudio::Count());
		RepeatDone.SetNum(HOLAudio::Count());
		for (int Index = 0; Index < LastMs.Num(); Index++)
		{
			LastMs[Index] = -1;
			LastFile[Index] = -1;
			RepeatStartMs[Index] = 0;
			RepeatDone[Index] = 0;
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
		if (!Start(Plan))
			return false;
		LastMs[Event] = NowMs;
		LastFile[Event] = Plan.FileIndex;
		// A repeating cue (the volcano's bell): the first ring is heard, the others come from PlayDueRepeats.
		RepeatStartMs[Event] = NowMs;
		RepeatDone[Event] = Plan.RepeatCount > 1 ? 1 : 0;
		return true;
	}

	// AR6-a / AR6-d: the sound of a type (Slot = Selection, Order, Construction; callers: BuildingSelected, RingUnitSelection, HandleOrderSound, RefreshMinorAlerts): its art asset's sound when one is set (HOLArtSound), played at 100 % within the
	// event's cooldown; else the table's cue of Event (-1 = no cue: silent). True when a sound was started.
	bool PlayTyped(int Event, FName TypeName, FName Faction, FName Slot)
	{
		const FString ArtPath = HOLArtSound::PathFor(this, TypeName, Faction, Slot);
		if (ArtPath.IsEmpty())
			return Play(Event);
		Prepare();
		const int NowMs = int(Gameplay::GetTimeSeconds() * 1000.0);
		if (HOLAudio::IsValid(Event))
		{
			if (LastMs[Event] >= 0 && NowMs - LastMs[Event] < HOLAudio::CueFor(Event).CooldownMs)
				return false;
		}
		USoundBase Sound = Cast<USoundBase>(LoadObject(nullptr, ArtPath));
		if (Sound == nullptr)
		{
			Log(n"LogHOLMatch", f"cue missing art {TypeName} {Slot} asset {ArtPath}");
			return Play(Event);
		}
		Gameplay::PlaySound2D(Sound, 1.0, 1.0);
		if (HOLAudio::IsValid(Event))
			LastMs[Event] = NowMs;
		PlayedCount++;
		LastLine = f"cue played art {TypeName} {Slot} asset {ArtPath}";
		Log(n"LogHOLMatch", LastLine);
		return true;
	}

	// AR6-b: end times of the typed combat sounds still counted (HOLArtSound::TryStartVoice, at most MaxCombatVoices at once).
	TArray<float> CombatVoiceEnds;

	// AR6-b: a typed combat sound (Slot = Attack or Death) of a unit of TypeName owned by Faction, played in 3D at Location. Only an art
	// asset's sound plays (the table has no attack or death cue: no art sound = silent, the game unchanged). The voice limit refuses the
	// sound when MaxCombatVoices are playing. True when a sound was started. Caller: HOLGameScreenScript.HandleCombatSound (UHOLCombatSoundSubsystem:
	// blows, shots, tower volleys, bodies laid; on a remote client its replayed blows and the volleys its towers show, AR6-c / AR6-d).
	bool PlayCombat(FName TypeName, FName Faction, FName Slot, FVector Location)
	{
		const FString ArtPath = HOLArtSound::PathFor(this, TypeName, Faction, Slot);
		if (ArtPath.IsEmpty())
			return false;
		if (!HOLArtSound::TryStartVoice(CombatVoiceEnds, Gameplay::GetTimeSeconds(), HOLArtSound::CombatVoiceSeconds, HOLArtSound::MaxCombatVoices))
			return false;
		USoundBase Sound = Cast<USoundBase>(LoadObject(nullptr, ArtPath));
		if (Sound == nullptr)
		{
			Log(n"LogHOLMatch", f"cue missing art {TypeName} {Slot} asset {ArtPath}");
			return false;
		}
		Gameplay::PlaySoundAtLocation(Sound, Location, FRotator(), 1.0, 1.0);
		PlayedCount++;
		LastLine = f"cue played art {TypeName} {Slot} asset {ArtPath} 3d";
		Log(n"LogHOLMatch", LastLine);
		return true;
	}

	// Loads and plays the plan's sound in 2D, logs it. False (and "cue missing" in the log) when the sound wave cannot be loaded.
	bool Start(FHOLAudioPlan Plan)
	{
		USoundBase Sound = Cast<USoundBase>(LoadObject(nullptr, Plan.Asset));
		if (Sound == nullptr)
		{
			Log(n"LogHOLMatch", f"cue missing {Plan.Name} asset {Plan.Asset}");
			return false;
		}
		Gameplay::PlaySound2D(Sound, float(Plan.VolumePercent) / 100.0, float(Plan.PitchPercent) / 100.0);
		PlayedCount++;
		LastLine = HOLAudio::PlayedLine(Plan);
		Log(n"LogHOLMatch", LastLine);
		return true;
	}

	// Called at every refresh of the screen: the next ring of a repeating cue when it is due (one per call, HOLAudio::RepeatsDue).
	void PlayDueRepeats()
	{
		Prepare();
		const int NowMs = int(Gameplay::GetTimeSeconds() * 1000.0);
		for (int Event = 0; Event < RepeatDone.Num(); Event++)
		{
			if (RepeatDone[Event] <= 0)
				continue;
			FHOLAudioCue Cue = HOLAudio::CueFor(Event);
			if (RepeatDone[Event] >= Cue.RepeatCount || NowMs < RepeatStartMs[Event])
			{
				RepeatDone[Event] = 0;
				continue;
			}
			if (HOLAudio::RepeatsDue(Cue, RepeatStartMs[Event], NowMs, RepeatDone[Event]) <= 0)
				continue;
			Counter++;
			// Past the cooldown on purpose (LastMs -1): the repeats belong to the same alert.
			FHOLAudioPlan Plan = HOLAudio::Plan(Event, NowMs, -1, Counter * 7919 + Event * 31, LastFile[Event]);
			RepeatDone[Event]++;
			if (Plan.bPlay)
				Start(Plan);
		}
	}
}
