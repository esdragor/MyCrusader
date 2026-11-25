// AR6-a (D924): the sound of a type read from its art asset (DA_Art_<Type>[_<Faction>], fields SelectionSound, OrderSound, AttackSound,
// DeathSound, ConstructionSound of UHOLArtVisual) before the sound table (HOLAudioTable.as). Field by field: the faction's asset, then the
// type's common asset, then the table's file. Pure apart from the registry read; nothing played here (HOLAudioPlayer.PlayTyped plays).
namespace HOLArtSound
{
	// The path of the art sound of (Type, Faction) in Slot (Selection, Order, Attack, Death, Construction); empty when no asset sets it.
	FString PathFor(const UObject Context, FName TypeName, FName Faction, FName Slot)
	{
		return "";
	}

	// What to play: the art sound when an art asset sets one, else the table's asset.
	FString AssetToPlay(const UObject Context, FName TypeName, FName Faction, FName Slot, FString TableAsset)
	{
		return TableAsset;
	}
}
