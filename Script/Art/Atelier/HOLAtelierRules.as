// AR8-b (D929, D930): pure rules of the light atelier (the "-Atelier" screen of the published game, HOLAtelierScreen.as).
// The game launched with -Atelier goes straight to the atelier (a Plain match turned into hol.Art.Lab) with its screen open; without
// the argument nothing changes. The screen takes FIXED types only (BuildingTypes, SourceTypes rows; units go through the packaged
// editor, D929), loads a .glb / .gltf from a known folder in the place of that type's model, and "Save" writes a send folder
// <Type>[_<Faction>]_<aaaammjj-hhmm> (manifest.json format 1, model.<ext>) that Tools\Art\apply_art_send.ps1 reads
// (ArtistesInProgress.md, "Conception AR8"). Tested by HOLAtelierRules_Test.as.

struct FHOLAtelierSend
{
	FString Type;
	FString Faction;
	FString Family;
	// The model file as loaded (full path); the send holds its copy as model.<ext> (SendModelName).
	FString ModelFile;
	double Yaw = 0.0;
	double Scale = 1.0;
	FString Status = "provisional";
	FString Author;
	// The licence: the columns of Docs\assets_art.csv.
	FString FileOrPack;
	FString Source;
	FString Licence;
	FString Url;
	FString DownloadedOn;
	FString LicenceProof;
	FString Notes;
	FString MadeWith;
}

namespace HOLAtelier
{
	// True when the command line carries the switch -<Name> (case ignored; -<Name>X is another switch).
	bool HasSwitch(const FString& CommandLine, const FString& Name)
	{
		TArray<FString> Tokens;
		TArray<FString> Switches;
		FCommandLine::Parse(CommandLine, Tokens, Switches);
		for (const FString& Switch : Switches)
		{
			if (Switch.ToLower() == Name.ToLower())
				return true;
		}
		return false;
	}

	// True when the command line carries the switch -Atelier.
	bool IsAsked(const FString& CommandLine)
	{
		return HasSwitch(CommandLine, "atelier");
	}

	// What the atelier does when a map starts: "launch" (main menu: launch the atelier's match), "lab" (the Plain map: hol.Art.Lab
	// then the screen), "" (nothing: no -Atelier, another map, or already done in this run).
	FString EntryStep(const FString& MapName, bool bAsked, bool bDone)
	{
		if (!bAsked || bDone)
			return "";
		if (MapName.Contains("MainMenu"))
			return "launch";
		if (MapName.StartsWith("Plain"))
			return "lab";
		return "";
	}

	// "Buildings" or "Sources" for a fixed type, "" for a unit or an unknown type.
	FString FamilyOf(const FString& Type, const TArray<FString>& Buildings, const TArray<FString>& Sources, const TArray<FString>& Units)
	{
		const FString Name = Type.TrimStartAndEnd();
		if (Units.Contains(Name))
			return "";
		if (Buildings.Contains(Name))
			return "Buildings";
		if (Sources.Contains(Name))
			return "Sources";
		return "";
	}

	// Why the type cannot be taken here: "type_unit" (units: by the editor, D929), "type_unknown", or "".
	FString TypeRefusal(const FString& Type, const TArray<FString>& Buildings, const TArray<FString>& Sources, const TArray<FString>& Units)
	{
		const FString Name = Type.TrimStartAndEnd();
		if (Units.Contains(Name))
			return "type_unit";
		if (FamilyOf(Name, Buildings, Sources, Units).IsEmpty())
			return "type_unknown";
		return "";
	}

	// The text shown for a refusal key (localised like the rest of the interface).
	FText RefusalText(const FString& Key)
	{
		if (Key == "type_unit") return NSLOCTEXT("HOLAtelier", "RefuseUnit", "unités : par l'éditeur");
		if (Key == "type_unknown") return NSLOCTEXT("HOLAtelier", "RefuseUnknown", "type inconnu (une ligne de BuildingTypes ou SourceTypes)");
		if (Key == "no_model") return NSLOCTEXT("HOLAtelier", "RefuseNoModel", "aucun modèle chargé");
		if (Key == "licence_file_or_pack") return NSLOCTEXT("HOLAtelier", "RefuseFileOrPack", "licence incomplète : fichier ou pack vide");
		if (Key == "licence_source") return NSLOCTEXT("HOLAtelier", "RefuseSource", "licence incomplète : source vide");
		if (Key == "licence_licence") return NSLOCTEXT("HOLAtelier", "RefuseLicence", "licence incomplète : licence vide");
		if (Key == "scale") return NSLOCTEXT("HOLAtelier", "RefuseScale", "échelle : un nombre plus grand que 0");
		if (Key == "status") return NSLOCTEXT("HOLAtelier", "RefuseStatus", "statut : final ou provisional");
		if (Key == "format") return NSLOCTEXT("HOLAtelier", "RefuseFormat", "fichier : .glb ou .gltf seulement");
		if (Key == "missing") return NSLOCTEXT("HOLAtelier", "RefuseMissing", "fichier introuvable");
		if (Key == "empty") return NSLOCTEXT("HOLAtelier", "RefuseEmpty", "aucun fichier choisi");
		if (Key == "editor_game") return NSLOCTEXT("HOLAtelier", "RefuseEditorGame", "import impossible dans l'éditeur lancé en -game : utilisez le jeu publié");
		if (Key == "import") return NSLOCTEXT("HOLAtelier", "RefuseImport", "import impossible (aucun maillage fixe dans le fichier)");
		return FText::FromString(Key);
	}

	// The row names of a game table (Units, Buildings, Sources).
	TArray<FString> TableNames(const FString& Family)
	{
		TArray<FString> Out;
		UDataTable Table = Cast<UDataTable>(LoadObject(nullptr, HOLArtGallery::TablePath(Family)));
		if (Table == nullptr)
			return Out;
		TArray<FName> Names;
		Table.GetRowNames(Names);
		for (const FName& Name : Names)
			Out.Add(Name.ToString());
		return Out;
	}

	// The types the screen offers, buildings then sources (never a unit).
	TArray<FString> FixedTypes(const TArray<FString>& Buildings, const TArray<FString>& Sources)
	{
		TArray<FString> Out;
		Out.Append(Buildings);
		Out.Append(Sources);
		return Out;
	}

	FString TwoDigits(int Value)
	{
		return Value < 10 ? f"0{Value}" : f"{Value}";
	}

	// aaaammjj-hhmm
	FString StampOf(const FDateTime& When)
	{
		return f"{When.GetYear()}{TwoDigits(When.GetMonth())}{TwoDigits(When.GetDay())}-{TwoDigits(When.GetHour())}{TwoDigits(When.GetMinute())}";
	}

	// <Type>[_<Faction>]_<aaaammjj-hhmm>
	FString SendFolderName(const FString& Type, const FString& Faction, const FDateTime& When)
	{
		const FString Middle = Faction.TrimStartAndEnd().IsEmpty() ? "" : "_" + Faction.TrimStartAndEnd();
		return Type.TrimStartAndEnd() + Middle + "_" + StampOf(When);
	}

	// The model the artist names: a bare name is looked for in the known folder, a full path is kept.
	FString ModelPathOf(const FString& Entrees, const FString& Name)
	{
		const FString Clean = Name.TrimStartAndEnd();
		if (Clean.Contains(":") || Clean.StartsWith("/") || Clean.StartsWith("\\"))
			return Clean;
		return Entrees + Clean;
	}

	// The name of the copy in the send: model.glb or model.gltf.
	FString SendModelName(const FString& ModelFile)
	{
		return "model." + FPaths::GetExtension(ModelFile, false).ToLower();
	}

	// AR8-c (D931): the extensions of the models the atelier lists and loads.
	TArray<FString> ModelExtensions()
	{
		TArray<FString> Out;
		Out.Add("glb");
		Out.Add("gltf");
		return Out;
	}

	// The models the artist put in the folder Entrees (names only, sorted): the screen's list, refreshed by its button.
	TArray<FString> ListModels(const FString& Entrees)
	{
		return UHOLArtVisualRegistry::ListArtFiles(Entrees, ModelExtensions());
	}

	// The model chosen after a step (+1 / -1) in a list of Count names, wrapping round; -1 for an empty list; from none (-1): the first.
	int StepIndex(int Index, int Count, int Step)
	{
		if (Count <= 0)
			return -1;
		if (Index < 0 || Index >= Count)
			return 0;
		return (((Index + Step) % Count) + Count) % Count;
	}

	// "Save": copies the model into the send folder as model.<ext> (a file already there is never replaced). Returns the name written
	// (model.glb / model.gltf), "" when nothing was copied.
	FString CopyModelToSend(const FString& ModelFile, const FString& Folder)
	{
		const FString Name = SendModelName(ModelFile);
		if (UHOLArtVisualRegistry::CopyArtFile(ModelFile, Folder + "/" + Name, false))
			return Name;
		return "";
	}

	// <Documents>/HeroesOfLiberty/Atelier/
	FString AtelierDir()
	{
		FString Documents = FPlatformProcess::UserDir();
		Documents = Documents.Replace("\\", "/");
		if (!Documents.EndsWith("/"))
			Documents += "/";
		return Documents + "HeroesOfLiberty/Atelier/";
	}

	FString EntreesDir()
	{
		return AtelierDir() + "Entrees/";
	}

	FString EnvoisDir()
	{
		return AtelierDir() + "Envois/";
	}

	// A number for the manifest: point, at most three decimals, no trailing zeros.
	FString NumberText(double Value)
	{
		const int Milli = Math::RoundToInt(Math::Abs(Value) * 1000.0);
		const int Whole = Milli / 1000;
		const int Frac = Milli % 1000;
		FString Out = (Value < 0.0 && Milli > 0) ? "-" : "";
		Out += f"{Whole}";
		if (Frac > 0)
		{
			FString Digits = f"{Frac}";
			while (Digits.Len() < 3)
				Digits = "0" + Digits;
			while (Digits.EndsWith("0"))
				Digits = Digits.Mid(0, Digits.Len() - 1);
			Out += "." + Digits;
		}
		return Out;
	}

	// A JSON string (quotes, backslash and control characters escaped).
	FString JsonText(const FString& Value)
	{
		FString Out = "\"";
		for (int K = 0; K < Value.Len(); K++)
		{
			const FString C = Value.Mid(K, 1);
			if (C == "\\")
				Out += "\\\\";
			else if (C == "\"")
				Out += "\\\"";
			else if (C == "\n")
				Out += "\\n";
			else if (C == "\r")
				Out += "\\r";
			else if (C == "\t")
				Out += "\\t";
			else
				Out += C;
		}
		return Out + "\"";
	}

	// manifest.json, format 1 (ArtistesInProgress.md "Conception AR8"; read by Tools\Art\apply_art_send.ps1).
	FString ManifestJson(const FHOLAtelierSend& Send)
	{
		FString Out = "{\n";
		Out += "  \"format\": 1,\n";
		Out += f"  \"type\": {JsonText(Send.Type.TrimStartAndEnd())},\n";
		Out += f"  \"faction\": {JsonText(Send.Faction.TrimStartAndEnd())},\n";
		Out += f"  \"family\": {JsonText(Send.Family)},\n";
		Out += f"  \"model\": {JsonText(SendModelName(Send.ModelFile))},\n";
		Out += f"  \"yaw\": {NumberText(Send.Yaw)},\n";
		Out += f"  \"scale\": {NumberText(Send.Scale)},\n";
		Out += f"  \"status\": {JsonText(Send.Status.TrimStartAndEnd())},\n";
		Out += f"  \"author\": {JsonText(Send.Author.TrimStartAndEnd())},\n";
		Out += "  \"licence\": {\n";
		Out += f"    \"file_or_pack\": {JsonText(Send.FileOrPack.TrimStartAndEnd())},\n";
		Out += f"    \"source\": {JsonText(Send.Source.TrimStartAndEnd())},\n";
		Out += f"    \"licence\": {JsonText(Send.Licence.TrimStartAndEnd())},\n";
		Out += f"    \"url\": {JsonText(Send.Url.TrimStartAndEnd())},\n";
		Out += f"    \"downloaded_on\": {JsonText(Send.DownloadedOn.TrimStartAndEnd())},\n";
		Out += f"    \"licence_proof\": {JsonText(Send.LicenceProof.TrimStartAndEnd())}\n";
		Out += "  },\n";
		Out += f"  \"notes\": {JsonText(Send.Notes)},\n";
		Out += f"  \"made_with\": {JsonText(Send.MadeWith)}\n";
		Out += "}\n";
		return Out;
	}

	// Every reason not to write the send (keys of RefusalText), all listed before anything is written; empty = written.
	// The same checks as apply_art_send.ps1, so a send the screen writes passes there.
	TArray<FString> SaveRefusals(const FHOLAtelierSend& Send, bool bModelLoaded, const TArray<FString>& Buildings, const TArray<FString>& Sources, const TArray<FString>& Units)
	{
		TArray<FString> Out;
		const FString TypeRefused = TypeRefusal(Send.Type, Buildings, Sources, Units);
		if (!TypeRefused.IsEmpty())
			Out.Add(TypeRefused);
		if (!bModelLoaded || Send.ModelFile.TrimStartAndEnd().IsEmpty())
			Out.Add("no_model");
		if (Send.FileOrPack.TrimStartAndEnd().IsEmpty())
			Out.Add("licence_file_or_pack");
		if (Send.Source.TrimStartAndEnd().IsEmpty())
			Out.Add("licence_source");
		if (Send.Licence.TrimStartAndEnd().IsEmpty())
			Out.Add("licence_licence");
		if (Send.Scale <= 0.0)
			Out.Add("scale");
		if (Send.Status != "final" && Send.Status != "provisional")
			Out.Add("status");
		return Out;
	}
}
