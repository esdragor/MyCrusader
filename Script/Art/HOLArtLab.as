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
		return "Herbe fraiche";
	}

	// The biome named by the words of the command (the shown names have spaces), or the default one.
	FString BiomeOf(const TArray<FString>& Args)
	{
		FString Name = "";
		for (int K = 0; K < Args.Num(); K++)
		{
			if (K > 0)
				Name += " ";
			Name += Args[K];
		}
		return Name.TrimStartAndEnd().IsEmpty() ? DefaultBiome() : Name;
	}

	// The console lines run in order by hol.Art.Lab; empty when the biome is not one the game knows.
	// Combat is off and construction fast before anything is put; the gallery clears the forest of its frames itself (hol.Decor.ClearNear)
	// before it puts the objects; the light comes after, so it also lights the gallery; the camera last.
	TArray<FString> Steps(const FString& Biome)
	{
		TArray<FString> Out;
		if (HOLArtGallery::BiomeIndexFromName(Biome) == -2)
			return Out;
		Out.Add("hol.Combat.Enabled 0");
		Out.Add("hol.Cheat.FastBuild 1");
		Out.Add("hol.Cheat.Resources 1000");
		Out.Add("hol.Art.Gallery All");
		Out.Add("hol.Art.Light " + Biome);
		Out.Add("hol.Art.GalleryFrame Units");
		return Out;
	}

	// The on-screen help: one line per hol.Art.* command.
	FString HelpText()
	{
		FString Out = "Atelier : commandes hol.Art.*\n";
		Out += "hol.Art.Lab [Biome] : tout l'atelier en une commande (galerie, eclairage, construction rapide, combat coupe)\n";
		Out += "hol.Art.Gallery [Units|Buildings|Sources|All] : un exemplaire de chaque type, etiquete, avec reperes d'echelle\n";
		Out += "hol.Art.GalleryFrame <Units|Buildings|Sources> [North|South] : la camera sur une famille de la galerie\n";
		Out += "hol.Art.Focus <Type> [Zoom] : vue rapprochee d'un objet et de sa barre de 1,80 m\n";
		Out += "hol.Art.Light <Biome|default> : eclairage d'un biome (Herbe fraiche, Steppe seche, Automne, Foret sombre, Neige)\n";
		Out += "hol.Art.Try <Type> [N] : poser N exemplaires d'un type pres du Forum\n";
		Out += "hol.Art.Icon [Type|All] : exporter l'icone 256 px d'un type (Saved\\ArtIcons)";
		return Out;
	}

	AHOLPlayerController LocalController()
	{
		TArray<AHOLPlayerController> Controllers;
		GetAllActorsOfClass(Controllers);
		for (AHOLPlayerController Controller : Controllers)
		{
			if (Controller.IsLocalController())
				return Controller;
		}
		return nullptr;
	}
}

void HOLArtLabCommand(const TArray<FString>& Args)
{
	const FString Biome = HOLArtLab::BiomeOf(Args);
	const TArray<FString> Lines = HOLArtLab::Steps(Biome);
	if (Lines.Num() == 0)
	{
		const FString Refused = f"hol.Art.Lab: unknown biome {Biome} (Herbe fraiche, Steppe seche, Automne, Foret sombre, Neige)";
		Print(Refused, 8.0);
		HOLScriptLog::LogWarning(n"LogHOLMatch", Refused);
		return;
	}
	AHOLPlayerController Controller = HOLArtLab::LocalController();
	if (Controller == nullptr)
	{
		const FString NoMatch = "hol.Art.Lab: start a Plain match first (no local player)";
		Print(NoMatch, 8.0);
		HOLScriptLog::LogWarning(n"LogHOLMatch", NoMatch);
		return;
	}
	for (const FString& Line : Lines)
	{
		System::ExecuteConsoleCommand(Line, Controller);
		Log(n"LogHOLMatch", f"Art lab: step {Line}");
	}
	Print(HOLArtLab::HelpText(), 30.0);
	Log(n"LogHOLMatch", f"Art lab: done biome={Biome} steps={Lines.Num()}");
}

const FConsoleCommand HOLArtLabConsole("hol.Art.Lab", n"HOLArtLabCommand");
