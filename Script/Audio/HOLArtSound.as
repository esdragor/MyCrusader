// AR6-a (D924): the sound of a type read from its art asset (DA_Art_<Type>[_<Faction>], fields SelectionSound, OrderSound, AttackSound,
// DeathSound, ConstructionSound of UHOLArtVisual) before the sound table (HOLAudioTable.as). Field by field: the faction's asset, then the
// type's common asset, then the table's file. Pure apart from the registry read; nothing played here (HOLAudioPlayer.PlayTyped plays).
namespace HOLArtSound
{
	// The path of the art sound of (Type, Faction) in Slot (Selection, Order, Attack, Death, Construction); empty when no asset sets it.
	FString PathFor(const UObject Context, FName TypeName, FName Faction, FName Slot)
	{
		if (TypeName == NAME_None)
			return "";
		// (Context: kept for the callers; the registry takes its world context implicitly.) FindArtVisual(Type, Faction) gives the faction's asset, or the common one when the faction has none.
		UHOLArtVisual Own = UHOLArtVisualRegistry::FindArtVisual(TypeName, Faction);
		const FString FromOwn = Own != nullptr ? Own.GetSoundPath(Slot) : "";
		if (!FromOwn.IsEmpty() || Faction == NAME_None)
			return FromOwn;
		// The faction's asset leaves this slot empty: the common asset's (field by field).
		UHOLArtVisual Common = UHOLArtVisualRegistry::FindArtVisual(TypeName, NAME_None);
		return Common != nullptr ? Common.GetSoundPath(Slot) : "";
	}

	// What to play: the art sound when an art asset sets one, else the table's asset.
	FString AssetToPlay(const UObject Context, FName TypeName, FName Faction, FName Slot, FString TableAsset)
	{
		const FString ArtPath = PathFor(Context, TypeName, Faction, Slot);
		return ArtPath.IsEmpty() ? TableAsset : ArtPath;
	}

	// AR6-b: at most this many typed combat sounds (attack, death) playing at once, so 200 units striking do not start 200 sounds.
	const int MaxCombatVoices = 8;
	// A typed combat sound is counted as playing for this long (seconds) after it starts.
	const float CombatVoiceSeconds = 1.0;

	// Voice limit (pure): VoiceEnds holds the end times of the voices still counted. Drops the ended ones (<= Now), then starts a voice
	// (appends Now + Length, returns true) only while fewer than Limit are playing; returns false (nothing appended) when full.
	bool TryStartVoice(TArray<float>& VoiceEnds, float Now, float Length, int Limit)
	{
		for (int Index = VoiceEnds.Num() - 1; Index >= 0; Index--)
		{
			if (VoiceEnds[Index] <= Now)
				VoiceEnds.RemoveAt(Index);
		}
		if (VoiceEnds.Num() >= Limit)
			return false;
		VoiceEnds.Add(Now + Length);
		return true;
	}
}
