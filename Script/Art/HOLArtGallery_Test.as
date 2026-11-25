// AR1-a (ArtistesInProgress.md): the gallery hol.Art.Gallery has ONE place per row of the real type tables (/Game/Units/UnitTypes,
// /Game/Buildings/BuildingTypes, /Game/Resources/SourceTypes), each row once, no two places on the same spot. The expected counts are
// read here from the tables themselves (GetRowNames), never written by hand: a type added to a table must appear on its own.

int ArtGalleryRows(const FString& Path, TArray<FName>& OutNames)
{
	OutNames.Reset();
	UDataTable Table = Cast<UDataTable>(LoadObject(nullptr, Path));
	if (Table != nullptr)
	{
		Table.GetRowNames(OutNames);
	}
	return OutNames.Num();
}

int ArtGalleryCount(const TArray<FHOLArtGalleryPlace>& Places, const FString& Family, FName TypeName)
{
	int Count = 0;
	for (int K = 0; K < Places.Num(); K++)
	{
		if (Places[K].Family == Family && Places[K].TypeName == TypeName)
		{
			Count += 1;
		}
	}
	return Count;
}

void ArtGalleryCheckFamily(FUnitTest& T, const TArray<FHOLArtGalleryPlace>& Places, const FString& Family, const TArray<FName>& Rows)
{
	for (int K = 0; K < Rows.Num(); K++)
	{
		const int Seen = ArtGalleryCount(Places, Family, Rows[K]);
		T.AssertEquals(1, Seen, f"{Family} row {Rows[K].ToString()} has exactly one place");
	}
}

void Test_ArtGalleryOnePlacePerTableRow(FUnitTest& T)
{
	TArray<FName> Units;
	TArray<FName> Buildings;
	TArray<FName> Sources;
	const int UnitRows = ArtGalleryRows("/Game/Units/UnitTypes.UnitTypes", Units);
	const int BuildingRows = ArtGalleryRows("/Game/Buildings/BuildingTypes.BuildingTypes", Buildings);
	const int SourceRows = ArtGalleryRows("/Game/Resources/SourceTypes.SourceTypes", Sources);
	T.AssertTrue(UnitRows > 0 && BuildingRows > 0 && SourceRows > 0, "the three type tables load and have rows");

	TArray<FHOLArtGalleryPlace> All;
	HOLArtGallery::PlanGrid("All", All);
	T.AssertEquals(UnitRows + BuildingRows + SourceRows, All.Num(), "All: one place per row of the three tables");
	ArtGalleryCheckFamily(T, All, "Units", Units);
	ArtGalleryCheckFamily(T, All, "Buildings", Buildings);
	ArtGalleryCheckFamily(T, All, "Sources", Sources);

	for (int A = 0; A < All.Num(); A++)
	{
		for (int B = A + 1; B < All.Num(); B++)
		{
			FVector Gap = All[A].Location - All[B].Location;
			Gap.Z = 0.0;
			const double Apart = Gap.Size();
			T.AssertTrue(Apart > 200.0, f"{All[A].TypeName.ToString()} and {All[B].TypeName.ToString()} are not on the same spot");
		}
	}
}

void Test_ArtGalleryFamilyFilter(FUnitTest& T)
{
	TArray<FName> Units;
	TArray<FName> Buildings;
	TArray<FName> Sources;
	const int UnitRows = ArtGalleryRows("/Game/Units/UnitTypes.UnitTypes", Units);
	const int BuildingRows = ArtGalleryRows("/Game/Buildings/BuildingTypes.BuildingTypes", Buildings);
	const int SourceRows = ArtGalleryRows("/Game/Resources/SourceTypes.SourceTypes", Sources);

	TArray<FHOLArtGalleryPlace> Only;
	HOLArtGallery::PlanGrid("Units", Only);
	T.AssertEquals(UnitRows, Only.Num(), "Units: one place per unit type, nothing else");
	ArtGalleryCheckFamily(T, Only, "Units", Units);
	HOLArtGallery::PlanGrid("buildings", Only);
	T.AssertEquals(BuildingRows, Only.Num(), "Buildings (any case): one place per building type");
	HOLArtGallery::PlanGrid("Sources", Only);
	T.AssertEquals(SourceRows, Only.Num(), "Sources: one place per source type");
	bool bNamed = true;
	for (int K = 0; K < Only.Num(); K++)
	{
		if (Only[K].DisplayName.IsEmpty())
		{
			bNamed = false;
		}
	}
	T.AssertTrue(Only.Num() > 0 && bNamed, "every source place carries its display name from the table");
}

// AR1-b: one capture frames a whole family (HOLArtGallery::FamilyFrame), within what the camera shows zoomed out.
void ArtGalleryCheckFrame(FUnitTest& T, const TArray<FHOLArtGalleryPlace>& Places, const FString& Family)
{
	FVector Centre;
	double Width = 0.0;
	HOLArtGallery::FamilyFrame(Places, Family, Centre, Width);
	int Seen = 0;
	int Outside = 0;
	for (const FHOLArtGalleryPlace& Place : Places)
	{
		if (Place.Family != Family)
			continue;
		Seen++;
		if (Math::Abs(Place.Location.X - Centre.X) > Width * 0.5 || Math::Abs(Place.Location.Y - Centre.Y) > Width * 0.5)
			Outside++;
	}
	T.AssertTrue(Seen > 0, f"{Family}: the family has places");
	T.AssertEquals(Outside, 0, f"{Family}: places outside the frame centred on {Centre.ToString()} of side {Width}");
	T.AssertTrue(Width > 0.0 && Width <= HOLArtGallery::MaxFrameWidth(), f"{Family}: frame side {Width} within one capture");
}

void Test_ArtGalleryFamilyFrameHoldsEveryPlace(FUnitTest& T)
{
	TArray<FHOLArtGalleryPlace> Places;
	HOLArtGallery::PlanGrid("All", Places);
	ArtGalleryCheckFrame(T, Places, "Units");
	ArtGalleryCheckFrame(T, Places, "Buildings");
	ArtGalleryCheckFrame(T, Places, "Sources");
}

// AR1-b: a player's Forum standing on the unit grid pushes the families away, each kept whole.
void Test_ArtGalleryGridKeepsAwayFromBase(FUnitTest& T)
{
	TArray<FHOLArtGalleryPlace> Places;
	HOLArtGallery::PlanGrid("All", Places);
	T.AssertTrue(Places.Num() > 1, "the gallery has places");
	if (Places.Num() < 2)
		return;
	const FVector Base = Places[0].Location;
	const FVector Gap = Places[1].Location - Places[0].Location;
	HOLArtGallery::KeepAwayFrom(Places, Base, HOLArtGallery::MinBaseDistance());
	int Near = 0;
	for (const FHOLArtGalleryPlace& Place : Places)
	{
		if (FVector(Place.Location.X - Base.X, Place.Location.Y - Base.Y, 0.0).Size() < HOLArtGallery::MinBaseDistance())
			Near++;
	}
	T.AssertEquals(Near, 0, "places within MinBaseDistance of the Forum");
	T.AssertTrue((Places[1].Location - Places[0].Location).Equals(Gap, 1.0), "the unit grid moved whole");
}
