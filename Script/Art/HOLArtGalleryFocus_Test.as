// AR1-d (ArtistesInProgress.md): the gallery reads its grid from the game, hol.Art.Light takes the biome names the game shows, the
// ground labels stay readable on the snow, hol.Art.Focus frames one object and its 1,80 m figure. Every value checked is computed by the
// gallery from the game's tables and settings (UHOLBuildingSettings, hol.Wall.SubDivision, HOLBiome, BuildingTypes, UnitTypes).

double ArtFocusLuminance(FLinearColor C)
{
	return 0.2126 * C.R + 0.7152 * C.G + 0.0722 * C.B;
}

void Test_ArtGalleryGridReadFromTheGame(FUnitTest& T)
{
	const double Tile = HOLArtGallery::GalleryTileCm();
	T.AssertTrue(Math::Abs(Tile - HOLConstructionRead::GetSettingTileSize()) < 0.5, f"the gallery tile ({Tile}) is the construction grid's (UHOLBuildingSettings)");
	const int Game = HOLWallLine::SubDivision();
	const int Sub = HOLArtGallery::GallerySubDivision();
	T.AssertEquals(Game < 1 ? 1 : Game, Sub, "the gallery sub-cells are the construction grid's (hol.Wall.SubDivision)");
}

void Test_ArtLightAcceptsShownBiomeNames(FUnitTest& T)
{
	for (int I = 0; I < HOLBiome::Count; I++)
	{
		const FString Shown = HOLBiome::At(I).Name;
		T.AssertFalse(Shown.IsEmpty(), f"biome {HOLBiome::At(I).Key}: a shown name");
		const int ByShown = HOLArtGallery::BiomeIndexFromName(Shown);
		T.AssertEquals(I, ByShown, f"hol.Art.Light {Shown}");
		const int ByUpper = HOLArtGallery::BiomeIndexFromName(Shown.ToUpper());
		T.AssertEquals(I, ByUpper, f"hol.Art.Light {Shown.ToUpper()}");
		const int ByKey = HOLArtGallery::BiomeIndexFromName(HOLBiome::At(I).Key);
		T.AssertEquals(I, ByKey, f"hol.Art.Light {HOLBiome::At(I).Key} (key still accepted)");
	}
	// Typed at the console without accents, as the user writes them.
	const int Forest = HOLArtGallery::BiomeIndexFromName("Foret sombre");
	T.AssertEquals(HOLBiome::IndexOfKey("dark_forest"), Forest, "Foret sombre");
	const int Grass = HOLArtGallery::BiomeIndexFromName("Herbe fraiche");
	T.AssertEquals(HOLBiome::IndexOfKey("fresh_grass"), Grass, "Herbe fraiche");
	const int Steppe = HOLArtGallery::BiomeIndexFromName("steppe seche");
	T.AssertEquals(HOLBiome::IndexOfKey("dry_steppe"), Steppe, "steppe seche");
	const int Unknown = HOLArtGallery::BiomeIndexFromName("Lune");
	T.AssertEquals(-2, Unknown, "an unknown name stays unknown");
}

void Test_ArtGalleryLabelsReadableOnSnow(FUnitTest& T)
{
	const int SnowIndex = HOLBiome::IndexOfKey("snow");
	T.AssertTrue(SnowIndex >= 0, "the snow biome is in the table");
	const FHOLBiome Snow = HOLBiome::At(SnowIndex);
	const FLinearColor Ground = FLinearColor(Snow.GroundAR / 1000.0f, Snow.GroundAG / 1000.0f, Snow.GroundAB / 1000.0f, 1.0f);
	const FLinearColor Text = FLinearColor(FColor::Yellow);
	const FLinearColor Outline = HOLArtGallery::LabelOutlineColour();
	const double OnGround = Math::Abs(ArtFocusLuminance(Outline) - ArtFocusLuminance(Ground));
	const double OnText = Math::Abs(ArtFocusLuminance(Outline) - ArtFocusLuminance(Text));
	T.AssertTrue(OnGround >= 0.5, f"the outline stands out of the snow ground ({OnGround})");
	T.AssertTrue(OnText >= 0.5, f"the outline stands out of the yellow text ({OnText})");
	TArray<FVector2D> Offsets;
	HOLArtGallery::LabelOutlineOffsets(60.0, Offsets);
	T.AssertTrue(Offsets.Num() >= 4, f"the outline goes around the text ({Offsets.Num()} copies)");
	bool bLeft = false;
	bool bRight = false;
	bool bUp = false;
	bool bDown = false;
	for (FVector2D Each : Offsets)
	{
		bLeft = bLeft || Each.X < 0.0;
		bRight = bRight || Each.X > 0.0;
		bUp = bUp || Each.Y > 0.0;
		bDown = bDown || Each.Y < 0.0;
	}
	T.AssertTrue(bLeft && bRight && bUp && bDown, "the outline is on the four sides");
}

void Test_ArtFocusFramesObjectAndFigure(FUnitTest& T)
{
	TArray<FHOLArtGalleryPlace> Places;
	HOLArtGallery::PlanGrid("All", Places);
	T.AssertTrue(Places.Num() > 0, "the gallery plans the tables");
	TArray<FString> Seen;
	for (const FHOLArtGalleryPlace& Place : Places)
	{
		if (Seen.Contains(Place.Family))
			continue;
		Seen.Add(Place.Family);
		const FString Name = Place.TypeName.ToString();
		FVector Centre;
		double Side = 0.0;
		const bool bFound = HOLArtGallery::FocusOn(Places, Name, Centre, Side);
		T.AssertTrue(bFound, f"hol.Art.Focus {Name} finds it");
		const FVector Bar = HOLArtGallery::ScaleBarBase(Place);
		const double ToObject = FVector(Centre.X - Place.Location.X, Centre.Y - Place.Location.Y, 0.0).Size();
		const double ToBar = FVector(Centre.X - Bar.X, Centre.Y - Bar.Y, 0.0).Size();
		T.AssertTrue(ToObject * 2.0 < Side && ToBar * 2.0 < Side, f"{Name}: object ({ToObject}) and figure ({ToBar}) inside the view ({Side})");
		const double Wide = Place.Family == "Buildings" ? Math::Max(Place.FootprintX, Place.FootprintY) : 2.0 * Math::Max(Place.Radius, 60.0);
		T.AssertTrue(Side >= Wide && Side <= Wide + 1200.0, f"{Name}: a close view ({Side} for {Wide})");
	}
	FVector None;
	double NoSide = 0.0;
	const bool bUnknown = HOLArtGallery::FocusOn(Places, "NoSuchType", None, NoSide);
	T.AssertFalse(bUnknown, "an unknown type is not focused");
}
