// AR1-c (ArtistesInProgress.md): the scale marks of the gallery hol.Art.Gallery and the biome lights of hol.Art.Light.
// The buildings and the biomes are read from the game (BuildingTypes through HOLArtGallery::PlanGrid, HOLBiome::At / HOLBiome::Count),
// never written by hand here.

int ArtMarksCount(const TArray<int>& Kinds, int Kind)
{
	int Count = 0;
	for (int K = 0; K < Kinds.Num(); K++)
	{
		if (Kinds[K] == Kind)
			Count += 1;
	}
	return Count;
}

void Test_ArtGalleryMarksFollowEachBuildingFootprint(FUnitTest& T)
{
	TArray<FHOLArtGalleryPlace> Places;
	HOLArtGallery::PlanGrid("Buildings", Places);
	T.AssertTrue(Places.Num() > 0, "the gallery plans the buildings of BuildingTypes");
	const double Tile = 400.0;
	const int Sub = 4;
	for (int P = 0; P < Places.Num(); P++)
	{
		const FHOLArtGalleryPlace Place = Places[P];
		const FString Name = Place.TypeName.ToString();
		T.AssertTrue(Place.FootprintX >= Tile && Place.FootprintY >= Tile, f"{Name}: footprint read from the table ({Place.FootprintX} x {Place.FootprintY})");
		TArray<FVector> Starts;
		TArray<FVector> Ends;
		TArray<int> Kinds;
		HOLArtGallery::MarkLines(Place, Tile, Sub, Starts, Ends, Kinds);
		const int TilesX = Math::RoundToInt(Place.FootprintX / Tile);
		const int TilesY = Math::RoundToInt(Place.FootprintY / Tile);
		T.AssertEquals(4, ArtMarksCount(Kinds, HOLArtGallery::MarkOutline), f"{Name}: four outline sides");
		T.AssertEquals(TilesX - 1 + TilesY - 1, ArtMarksCount(Kinds, HOLArtGallery::MarkTile), f"{Name}: one line between two 4 m tiles");
		T.AssertEquals((TilesX + TilesY) * (Sub - 1), ArtMarksCount(Kinds, HOLArtGallery::MarkSub), f"{Name}: the sub-cell lines inside each tile");
		T.AssertEquals(Place.Height > 0.0 ? 1 : 0, ArtMarksCount(Kinds, HOLArtGallery::MarkHeight), f"{Name}: one Height pole");
		for (int K = 0; K < Kinds.Num(); K++)
		{
			const FVector A = Starts[K];
			const FVector B = Ends[K];
			const bool bInside = Math::Abs(A.X - Place.Location.X) <= Place.FootprintX * 0.5 + 1.0 && Math::Abs(B.Y - Place.Location.Y) <= Place.FootprintY * 0.5 + 1.0;
			T.AssertTrue(bInside, f"{Name}: mark {K} on the footprint");
			if (Kinds[K] == HOLArtGallery::MarkHeight)
			{
				const double Pole = B.Z - A.Z;
				T.AssertTrue(Math::Abs(Pole - Place.Height) < 1.0, f"{Name}: the pole is the table's Height ({Pole} vs {Place.Height})");
			}
		}
		const FVector Bar = HOLArtGallery::ScaleBarBase(Place);
		T.AssertTrue(Bar.Y - Place.Location.Y > Place.FootprintY * 0.5, f"{Name}: the 1,80 m figure stands beside the footprint, not in it");
	}
}

void Test_ArtGalleryScaleFigureBesideUnitsAndSources(FUnitTest& T)
{
	TArray<FHOLArtGalleryPlace> Places;
	HOLArtGallery::PlanGrid("All", Places);
	int Units = 0;
	for (int P = 0; P < Places.Num(); P++)
	{
		const FHOLArtGalleryPlace Place = Places[P];
		if (Place.Family == "Buildings")
			continue;
		Units += 1;
		const FVector Bar = HOLArtGallery::ScaleBarBase(Place);
		const double Away = Bar.Y - Place.Location.Y;
		T.AssertTrue(Away > Place.Radius && Away < Place.Radius + 400.0, f"{Place.TypeName.ToString()}: the figure beside it ({Away} cm, radius {Place.Radius})");

		TArray<FVector> Starts;
		TArray<FVector> Ends;
		TArray<int> Kinds;
		HOLArtGallery::MarkLines(Place, 400.0, 4, Starts, Ends, Kinds);
		T.AssertEquals(0, Kinds.Num(), f"{Place.TypeName.ToString()}: no footprint lines for a unit or a source");
	}
	T.AssertTrue(Units > 0, "units and sources planned");
	T.AssertTrue(Math::Abs(HOLArtGallery::ReferenceHeightCm() - 180.0) < 0.5, "the reference figure is 1,80 m");
}

void Test_ArtLightBiomeNamesReadFromTheGame(FUnitTest& T)
{
	T.AssertTrue(HOLBiome::Count > 1, "the game has biomes");
	for (int I = 0; I < HOLBiome::Count; I++)
	{
		const FString Key = HOLBiome::At(I).Key;
		const int Found = HOLArtGallery::BiomeIndexFromName(Key);
		T.AssertEquals(I, Found, f"biome {Key} by its key");
		const int Upper = HOLArtGallery::BiomeIndexFromName(Key.ToUpper());
		T.AssertEquals(I, Upper, f"biome {Key} in capitals");
		const int ByIndex = HOLArtGallery::BiomeIndexFromName(f"{I}");
		T.AssertEquals(I, ByIndex, f"biome {Key} by its index");
		// The chain the applier follows: the index found gives the look of that biome (HOLBiomeApply::LookOf).
		const FString LookKey = HOLBiomeApply::LookOf(Found).Key;
		T.AssertEquals(Key, LookKey, f"the look applied for {Key}");
	}
	const int Own = HOLArtGallery::BiomeIndexFromName("default");
	T.AssertEquals(-1, Own, "default = the map's own look");
	const int Unknown = HOLArtGallery::BiomeIndexFromName("moon");
	T.AssertEquals(-2, Unknown, "a biome the game does not know");
	const int Beyond = HOLArtGallery::BiomeIndexFromName(f"{HOLBiome::Count}");
	T.AssertEquals(-2, Beyond, "an index out of the table");
}
