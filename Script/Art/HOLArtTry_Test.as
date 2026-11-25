// hol.Art.Try (AR4-a, CELEBORN-1125ar2): the place of each copy is free and next to the Forum, computed by HOLArtTry::FreePlaceNearForum;
// an unknown type lists the close known types (HOLArtTry::NearTypes). The world chain (spawn, selection, camera) is played by the scenario art_try.

bool ArtTryIsFree(FVector Place, float ObjectRadius, const TArray<FVector>& Occupied, const TArray<float>& OccupiedRadii)
{
	for (int Index = 0; Index < Occupied.Num(); ++Index)
	{
		const FVector Delta = Place - Occupied[Index];
		const float Flat = FVector(Delta.X, Delta.Y, 0.0).Size();
		if (Flat < OccupiedRadii[Index] + ObjectRadius + HOLArtTry::Gap() - 1.0)
			return false;
	}
	return true;
}

// "Next to" the Forum: within four object widths (plus gaps) of its edge.
bool ArtTryIsNextTo(FVector Place, FVector ForumCenter, float ForumRadius, float ObjectRadius)
{
	const FVector Delta = Place - ForumCenter;
	const float Flat = FVector(Delta.X, Delta.Y, 0.0).Size();
	return Flat <= ForumRadius + 4.0 * (2.0 * ObjectRadius + HOLArtTry::Gap());
}

void Test_ArtTryPlaceIsFreeAndNextToForum(FUnitTest& T)
{
	const FVector Forum(1000.0, 2000.0, 50.0);
	const float ForumRadius = 600.0;
	const float ObjectRadius = 60.0;
	TArray<FVector> Occupied;
	TArray<float> Radii;
	Occupied.Add(Forum);
	Radii.Add(ForumRadius);
	// A house already standing right where the first ring starts (east of the Forum).
	Occupied.Add(Forum + FVector(ForumRadius + HOLArtTry::Gap() + ObjectRadius, 0.0, 0.0));
	Radii.Add(200.0);
	const FVector Place = HOLArtTry::FreePlaceNearForum(Forum, ForumRadius, ObjectRadius, Occupied, Radii);
	const bool bFree = ArtTryIsFree(Place, ObjectRadius, Occupied, Radii);
	const bool bNext = ArtTryIsNextTo(Place, Forum, ForumRadius, ObjectRadius);
	T.AssertTrue(bFree, f"the place {Place.ToString()} clears the Forum and the house");
	T.AssertTrue(bNext, f"the place {Place.ToString()} is next to the Forum");
}

void Test_ArtTryFiveCopiesEachFree(FUnitTest& T)
{
	const FVector Forum(0.0, 0.0, 0.0);
	const float ForumRadius = 500.0;
	const float ObjectRadius = 50.0;
	TArray<FVector> Occupied;
	TArray<float> Radii;
	Occupied.Add(Forum);
	Radii.Add(ForumRadius);
	int FreeCount = 0;
	for (int Copy = 0; Copy < 5; ++Copy)
	{
		const FVector Place = HOLArtTry::FreePlaceNearForum(Forum, ForumRadius, ObjectRadius, Occupied, Radii);
		if (ArtTryIsFree(Place, ObjectRadius, Occupied, Radii) && ArtTryIsNextTo(Place, Forum, ForumRadius, ObjectRadius))
			FreeCount++;
		Occupied.Add(Place);
		Radii.Add(ObjectRadius);
	}
	T.AssertEquals(5, FreeCount, "hol.Art.Try Arquebusier 5: five free places next to the Forum, none on another");
}

void Test_ArtTryUnknownTypeListsCloseTypes(FUnitTest& T)
{
	TArray<FString> Known;
	Known.Add("Arquebusier");
	Known.Add("Archer");
	Known.Add("Peasant");
	Known.Add("House");
	const TArray<FString> Near = HOLArtTry::NearTypes("Arquebusir", Known);
	const bool bHas = Near.Contains("Arquebusier");
	const bool bNoHouse = !Near.Contains("House");
	T.AssertTrue(bHas, "a typo of Arquebusier lists Arquebusier");
	T.AssertTrue(bNoHouse, "a far type is not listed");
	const TArray<FString> Exact = HOLArtTry::NearTypes("peasant", Known);
	T.AssertTrue(Exact.Num() >= 1 && Exact[0] == "Peasant", "the case-blind match comes first");
}
