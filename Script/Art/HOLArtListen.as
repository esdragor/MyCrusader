// AR6-e (ArtistesInProgress.md): the listening table of a type's sounds. hol.Art.Listen <Type> [Faction] writes, for each slot of the art
// asset, what the game plays there (the asset's sound, the table's cue, silence, or nothing because the game never rings that slot for
// this family) and plays the audible ones one after the other; hol.Art.TrySound <Type> <Slot> <SoundAssetPath> [Faction] sets a slot's
// sound IN MEMORY only (UHOLArtVisualRegistry::TryArtVisualSound: a transient copy of the asset, never saved) to hear it in a match.
// The rules below are pure (tested by HOLArtListen_Test.as).
namespace HOLArtListen
{
	// RED STUB (AR6-e).
	TArray<FName> Slots()
	{
		TArray<FName> Out;
		return Out;
	}

	// RED STUB (AR6-e).
	bool SlotPlayedFor(const FString& Family, FName Slot)
	{
		return false;
	}

	// RED STUB (AR6-e).
	FString TableCueFor(FName TypeName, const FString& Family, FName Slot)
	{
		return "";
	}

	// RED STUB (AR6-e).
	FString Diagnostic(bool bPlayed, const FString& ArtPath, const FString& TableCue)
	{
		return "";
	}

	// RED STUB (AR6-e).
	int TrySoundRefusal(FName Slot, const FString& SoundPath)
	{
		return 0;
	}
}
