// hol.Art.Try <Type> [N] (AR4-a, CELEBORN-1125ar2, ArtistesInProgress.md lot AR4 "Jouer avec"): pure rules of the try command.
// The place of each copy is computed here (free, next to the Forum) so a script test drives it without a world.
namespace HOLArtTry
{
	// Gap kept between the Forum's edge and a placed object, and between two placed objects (cm).
	float Gap()
	{
		return 150.0;
	}

	// The Index-th free place next to a Forum centred on ForumCenter with radius ForumRadius: on rings around the Forum, the first point
	// whose disc of radius ObjectRadius stays clear (Gap) of every Occupied disc (centre in Occupied, radius in OccupiedRadii).
	// STUB (red first): returns the Forum's centre.
	FVector FreePlaceNearForum(FVector ForumCenter, float ForumRadius, float ObjectRadius, const TArray<FVector>& Occupied, const TArray<float>& OccupiedRadii)
	{
		return ForumCenter;
	}

	// The known types close to Typed (case-blind prefix, contains, or a few letters apart), the exact match first.
	// STUB (red first): returns none.
	TArray<FString> NearTypes(const FString& Typed, const TArray<FString>& Known)
	{
		TArray<FString> Near;
		return Near;
	}
}
