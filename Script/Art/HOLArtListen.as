// AR6-e (ArtistesInProgress.md): the listening table of a type's sounds. hol.Art.Listen <Type> [Faction] writes, for each slot of the art
// asset, what the game plays there (the asset's sound, the table's cue, silence, or nothing because the game never rings that slot for
// this family) and plays the audible ones one after the other; hol.Art.TrySound <Type> <Slot> <SoundAssetPath> [Faction] sets a slot's
// sound IN MEMORY only (UHOLArtVisualRegistry::TryArtVisualSound: a transient copy of the asset, never saved) to hear it in a match.
// The rules below are pure (tested by HOLArtListen_Test.as); the two commands are at the end of the file.
namespace HOLArtListen
{
	// Seconds between two sounds of hol.Art.Listen.
	const float GapSeconds = 1.2;

	// The five sound slots of UHOLArtVisual, in the order the table is written.
	TArray<FName> Slots()
	{
		TArray<FName> Out;
		Out.Add(n"Selection");
		Out.Add(n"Order");
		Out.Add(n"Attack");
		Out.Add(n"Death");
		Out.Add(n"Construction");
		return Out;
	}

	// Does the game ring Slot for a type of Family? Where the code raises it: a building's selection (HOLGameScreenScript.BuildingSelected),
	// a unit's order (UHOLCombatSoundSubsystem::RaiseOrder), a unit's blow and a tower's volley (Attack), a unit's body (Death), a foundation
	// laid and a building finished (Construction), a new selection of units (HOLGameScreenScript.RingUnitSelection, SAURON-1127ar1: the
	// asset's sound only, TableCueFor has none, so "silent" without one); a source rings nothing.
	bool SlotPlayedFor(const FString& Family, FName Slot)
	{
		if (Family == "Units")
			return Slot == n"Selection" || Slot == n"Order" || Slot == n"Attack" || Slot == n"Death";
		if (Family == "Buildings")
			return Slot == n"Selection" || Slot == n"Attack" || Slot == n"Construction";
		return false;
	}

	// The name of an event's cue when the table would play it (a cue with a file), else empty.
	FString AudibleCue(int Event)
	{
		if (!HOLAudio::IsValid(Event))
			return "";
		FHOLAudioCue Cue = HOLAudio::CueFor(Event);
		return Cue.Name.IsEmpty() || Cue.Stems.Num() == 0 ? "" : Cue.Name;
	}

	// The table's cue the game plays in Slot when the art asset has no sound there (the same events as the callers), empty when silent.
	// Order: the five order cues are silent by the user's choice (Order.Move stands for them); Construction: the foundation laid has no
	// table cue and Alert.BuildingDone is silent; Attack and Death: the table has none.
	FString TableCueFor(FName TypeName, const FString& Family, FName Slot)
	{
		if (Family == "Buildings" && Slot == n"Selection")
			return AudibleCue(HOLAudioBuildings::SelectEventFor(TypeName.ToString()));
		if (Family == "Units" && Slot == n"Order")
			return AudibleCue(HOLAudioEvent::OrderMove);
		if (Family == "Buildings" && Slot == n"Construction")
			return AudibleCue(HOLAudioEvent::AlertBuildingDone);
		return "";
	}

	// The line of a slot: "art <path>", "table <cue>", "silent", or "never played ..." when the game never rings it for this family.
	FString Diagnostic(bool bPlayed, const FString& ArtPath, const FString& TableCue)
	{
		if (!bPlayed)
			return "never played for this family";
		if (!ArtPath.IsEmpty())
			return "art " + ArtPath;
		if (!TableCue.IsEmpty())
			return "table " + TableCue;
		return "silent";
	}

	// hol.Art.TrySound: 0 accepted, 1 unknown slot, 2 empty path.
	int TrySoundRefusal(FName Slot, const FString& SoundPath)
	{
		if (!Slots().Contains(Slot))
			return 1;
		if (SoundPath.TrimStartAndEnd().IsEmpty())
			return 2;
		return 0;
	}

	// What hol.Art.Listen plays: the asset's path or the table's event, per slot, in order.
	struct FListenStep
	{
		FName Slot;
		FString ArtPath = "";
		int Event = -1;
	}

	void Say(const FString& Line, float Seconds)
	{
		Log(n"LogHOLMatch", Line);
		Print(Line, Seconds);
	}

	// hol.Art.Listen <Type> [Faction]
	void Run(const TArray<FString>& Args)
	{
		if (Args.Num() < 1)
		{
			Say("hol.Art.Listen <Type> [Faction] : ecoute les 5 sons de la fiche d'un type", 8.0);
			return;
		}
		const FName TypeName = FName(Args[0]);
		const FName Faction = Args.Num() > 1 ? FName(Args[1]) : NAME_None;
		const FString Family = HOLArtIconRules::FamilyOf(TypeName);
		if (Family.IsEmpty())
		{
			Say(f"Art listen: unknown type {TypeName}", 8.0);
			return;
		}
		TArray<FListenStep> Steps;
		for (FName Slot : Slots())
		{
			const bool bPlayed = SlotPlayedFor(Family, Slot);
			const FString ArtPath = HOLArtSound::PathFor(nullptr, TypeName, Faction, Slot);
			const FString TableCue = TableCueFor(TypeName, Family, Slot);
			Say(f"Art listen: {TypeName} {Slot} {Diagnostic(bPlayed, ArtPath, TableCue)}", 12.0);
			if (!bPlayed || (ArtPath.IsEmpty() && TableCue.IsEmpty()))
				continue;
			FListenStep Step;
			Step.Slot = Slot;
			Step.ArtPath = ArtPath;
			Step.Event = ArtPath.IsEmpty() ? TableEventFor(TypeName, Family, Slot) : -1;
			Steps.Add(Step);
		}
		if (Steps.Num() == 0)
			return;
		AHOLArtListenActor Player = Cast<AHOLArtListenActor>(SpawnActor(AHOLArtListenActor::StaticClass(), FVector::ZeroVector));
		if (Player != nullptr)
			Player.Start(TypeName, Steps);
	}

	// The event behind TableCueFor (same cases).
	int TableEventFor(FName TypeName, const FString& Family, FName Slot)
	{
		if (Family == "Buildings" && Slot == n"Selection")
			return HOLAudioBuildings::SelectEventFor(TypeName.ToString());
		if (Family == "Units" && Slot == n"Order")
			return HOLAudioEvent::OrderMove;
		if (Family == "Buildings" && Slot == n"Construction")
			return HOLAudioEvent::AlertBuildingDone;
		return -1;
	}

	// hol.Art.TrySound <Type> <Slot> <SoundAssetPath> [Faction]
	void Try(const TArray<FString>& Args)
	{
		if (Args.Num() < 3)
		{
			Say("hol.Art.TrySound <Type> <Selection|Order|Attack|Death|Construction> <chemin du son> [Faction] : essai en memoire, jamais enregistre", 8.0);
			return;
		}
		const FName TypeName = FName(Args[0]);
		const FName Slot = FName(Args[1]);
		const FString Path = Args[2];
		const FName Faction = Args.Num() > 3 ? FName(Args[3]) : NAME_None;
		const int Refusal = TrySoundRefusal(Slot, Path);
		if (Refusal == 1)
		{
			Say(f"Art try sound refused: unknown slot {Slot} (Selection, Order, Attack, Death, Construction)", 8.0);
			return;
		}
		if (Refusal == 2)
		{
			Say("Art try sound refused: empty sound path", 8.0);
			return;
		}
		if (Cast<USoundBase>(LoadObject(nullptr, Path)) == nullptr)
		{
			Say(f"Art try sound refused: no sound at {Path}", 8.0);
			return;
		}
		if (!UHOLArtVisualRegistry::TryArtVisualSound(TypeName, Faction, Slot, Path))
		{
			Say(f"Art try sound refused: no art asset for {TypeName} {Faction}", 8.0);
			return;
		}
		Say(f"Art try sound: {TypeName} {Slot} {Path} (en memoire seulement : la fiche n'est pas enregistree)", 10.0);
	}
}

// Plays the steps of hol.Art.Listen one after the other (HOLArtListen::GapSeconds apart), in 2D, then goes.
class AHOLArtListenActor : AActor
{
	FName TypeName;
	TArray<HOLArtListen::FListenStep> Steps;
	int Next = 0;
	float Wait = 0.0;
	UHOLAudioPlayer Table;

	void Start(FName InType, const TArray<HOLArtListen::FListenStep>& InSteps)
	{
		TypeName = InType;
		Steps = InSteps;
		Next = 0;
		Wait = 0.0;
		Table = Cast<UHOLAudioPlayer>(NewObject(this, UHOLAudioPlayer));
	}

	UFUNCTION(BlueprintOverride)
	void Tick(float DeltaTime)
	{
		Wait -= DeltaTime;
		if (Wait > 0.0)
			return;
		if (Next >= Steps.Num())
		{
			DestroyActor();
			return;
		}
		HOLArtListen::FListenStep Step = Steps[Next];
		Next++;
		Wait = HOLArtListen::GapSeconds;
		if (!Step.ArtPath.IsEmpty())
		{
			USoundBase Sound = Cast<USoundBase>(LoadObject(nullptr, Step.ArtPath));
			if (Sound != nullptr)
			{
				Gameplay::PlaySound2D(Sound, 1.0, 1.0);
				Log(n"LogHOLMatch", f"Art listen: played {TypeName} {Step.Slot} art {Step.ArtPath}");
			}
			else
				Log(n"LogHOLMatch", f"Art listen: missing {TypeName} {Step.Slot} art {Step.ArtPath}");
		}
		else if (Table != nullptr && Table.Play(Step.Event))
			Log(n"LogHOLMatch", f"Art listen: played {TypeName} {Step.Slot} table");
	}
}

void HOLArtListenCommand(const TArray<FString>& Args) { HOLArtListen::Run(Args); }
const FConsoleCommand HOLArtListenConsole("hol.Art.Listen", n"HOLArtListenCommand");
void HOLArtTrySoundCommand(const TArray<FString>& Args) { HOLArtListen::Try(Args); }
const FConsoleCommand HOLArtTrySoundConsole("hol.Art.TrySound", n"HOLArtTrySoundCommand");
