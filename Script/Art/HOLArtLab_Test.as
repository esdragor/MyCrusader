// AR1-e (ArtistesInProgress.md): hol.Art.Lab, the artists' lab in one command. The order of its steps, its default biome and its help are
// checked through the real rules: the biome is resolved by the gallery's own HOLArtGallery::BiomeIndexFromName against the game's
// HOLBiome table, never compared with a copied constant.

int ArtLabIndexOf(const TArray<FString>& Lines, const FString& Start)
{
	for (int K = 0; K < Lines.Num(); K++)
	{
		if (Lines[K].StartsWith(Start))
			return K;
	}
	return -1;
}

int ArtLabBiomeKeyIndex(const FString& Key)
{
	for (int I = 0; I < HOLBiome::Count; I++)
	{
		if (HOLBiome::At(I).Key == Key)
			return I;
	}
	return -3;
}

void Test_ArtLabStepsInOrder(FUnitTest& T)
{
	TArray<FString> NoArgs;
	const TArray<FString> Lines = HOLArtLab::Steps(HOLArtLab::BiomeOf(NoArgs));
	const int Combat = ArtLabIndexOf(Lines, "hol.Combat.Enabled 0");
	const int Fast = ArtLabIndexOf(Lines, "hol.Cheat.FastBuild 1");
	const int Money = ArtLabIndexOf(Lines, "hol.Cheat.Resources ");
	const int Gallery = ArtLabIndexOf(Lines, "hol.Art.Gallery All");
	const int Light = ArtLabIndexOf(Lines, "hol.Art.Light ");
	const int Camera = ArtLabIndexOf(Lines, "hol.Art.GalleryFrame Units");
	T.AssertTrue(Combat >= 0 && Fast >= 0 && Money >= 0, "combat off, fast construction and resources are steps");
	T.AssertTrue(Gallery >= 0 && Light >= 0 && Camera >= 0, "gallery, light and camera are steps");
	T.AssertTrue(Combat < Gallery, "combat is off before the gallery is put");
	T.AssertTrue(Fast < Gallery && Money < Gallery, "fast construction and resources before the gallery");
	T.AssertTrue(Gallery < Light, "the light after the gallery");
	T.AssertEquals(Lines.Num() - 1, Camera, "the camera on the units' frame is the last step");
}

void Test_ArtLabDefaultBiomeIsFreshGrass(FUnitTest& T)
{
	TArray<FString> NoArgs;
	const FString Default = HOLArtLab::BiomeOf(NoArgs);
	const int Resolved = HOLArtGallery::BiomeIndexFromName(Default);
	T.AssertEquals(ArtLabBiomeKeyIndex("fresh_grass"), Resolved, f"default biome '{Default}' is the game's fresh grass");
	const TArray<FString> Lines = HOLArtLab::Steps(Default);
	const int Light = ArtLabIndexOf(Lines, "hol.Art.Light ");
	T.AssertTrue(Light >= 0, "a light step");
	if (Light >= 0)
	{
		const FString Named = Lines[Light].Replace("hol.Art.Light ", "");
		const int FromLine = HOLArtGallery::BiomeIndexFromName(Named);
		T.AssertEquals(Resolved, FromLine, f"the light step '{Lines[Light]}' sets the default biome");
	}
	TArray<FString> Snow;
	Snow.Add("Neige");
	const int SnowIndex = HOLArtGallery::BiomeIndexFromName(HOLArtLab::BiomeOf(Snow));
	T.AssertEquals(ArtLabBiomeKeyIndex("snow"), SnowIndex, "hol.Art.Lab Neige lights the snow");
	TArray<FString> TwoWords;
	TwoWords.Add("Steppe");
	TwoWords.Add("seche");
	const int SteppeIndex = HOLArtGallery::BiomeIndexFromName(HOLArtLab::BiomeOf(TwoWords));
	T.AssertEquals(ArtLabBiomeKeyIndex("dry_steppe"), SteppeIndex, "hol.Art.Lab Steppe seche: every word is the name");
}

void Test_ArtLabUnknownBiomeRunsNothing(FUnitTest& T)
{
	const TArray<FString> Lines = HOLArtLab::Steps("Lune");
	T.AssertEquals(0, Lines.Num(), "an unknown biome runs no step");
	const TArray<FString> Known = HOLArtLab::Steps("Neige");
	T.AssertTrue(Known.Num() >= 6, "a known biome runs the six steps");
}

void Test_ArtLabHelpListsEveryArtCommand(FUnitTest& T)
{
	const FString Help = HOLArtLab::HelpText();
	TArray<FString> Names;
	Names.Add("hol.Art.Lab");
	Names.Add("hol.Art.Gallery ");
	Names.Add("hol.Art.GalleryFrame");
	Names.Add("hol.Art.Focus");
	Names.Add("hol.Art.Light");
	Names.Add("hol.Art.Try");
	Names.Add("hol.Art.Icon");
	for (const FString& Name : Names)
		T.AssertTrue(Help.Contains(Name), f"the help names {Name}");
	TArray<FString> NoArgs;
	const TArray<FString> Lines = HOLArtLab::Steps(HOLArtLab::BiomeOf(NoArgs));
	for (const FString& Line : Lines)
	{
		if (Line.StartsWith("hol.Art."))
		{
			TArray<FString> Words;
			Line.ParseIntoArray(Words, " ", true);
			const FString Command = Words.Num() > 0 ? Words[0] : Line;
			T.AssertTrue(Help.Contains(Command), f"the help names the step {Command}");
		}
	}
}
