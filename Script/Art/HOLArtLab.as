// hol.Art.Lab [Biome] (ArtistesInProgress.md, lot AR1-e): one command that turns a Plain match in progress into the artists' lab:
// combat off, fast construction and resources, the whole gallery (hol.Art.Gallery All, which clears the forest of the zone first),
// the light of a biome (hol.Art.Light, "Herbe fraiche" by default), then the camera on the units' frame (hol.Art.GalleryFrame Units);
// the help of the hol.Art.* commands is shown on screen once per call. Developer tool only (not shown to the player).
// The order of the steps and the default biome are the pure rules below (tested by HOLArtLab_Test.as).
namespace HOLArtLab
{
	// The biome used when the command is given none (a name the game shows, HOLBiome::At(I).Name).
	FString DefaultBiome()
	{
		return "";
	}

	// The biome named by the words of the command (the shown names have spaces), or the default one.
	FString BiomeOf(const TArray<FString>& Args)
	{
		return "";
	}

	// The console lines run in order by hol.Art.Lab; empty when the biome is not one the game knows.
	TArray<FString> Steps(const FString& Biome)
	{
		TArray<FString> Out;
		return Out;
	}

	// The on-screen help: one line per hol.Art.* command.
	FString HelpText()
	{
		return "";
	}
}
