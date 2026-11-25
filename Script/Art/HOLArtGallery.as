// hol.Art.Gallery [Units|Buildings|Sources|All] (ArtistesInProgress.md, lot AR1-a, niveau Atelier):
// in a Plain match, puts ONE of each type of the game tables (UnitTypes, BuildingTypes, SourceTypes rows, read from the tables,
// never a hand-written list) on a grid far from the Forum, combat off (hol.Combat.Enabled 0), each object labelled on the ground
// with its type name and its display name. A type added to a table shows up on its own.
// STUB (red commit): the grid is empty; the correction fills it.

struct FHOLArtGalleryPlace
{
	FName TypeName;
	FString DisplayName;
	FString Family;
	FVector Location;
}

namespace HOLArtGallery
{
	// The places of the gallery for one family (Units, Buildings, Sources) or All, one per row of the type tables.
	void PlanGrid(const FString& Family, TArray<FHOLArtGalleryPlace>& OutPlaces)
	{
		OutPlaces.Reset();
	}
}
