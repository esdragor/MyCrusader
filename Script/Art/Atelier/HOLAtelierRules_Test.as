// AR8-b (D929, D930): the pure rules of the light atelier (HOLAtelierRules.as): the -Atelier entry, the refusal of the unit types,
// the folder name of a send, the manifest.json (format 1, the fields Tools\Art\apply_art_send.ps1 reads) and the refusals before writing.

FHOLAtelierSend AtelierTestSend()
{
	FHOLAtelierSend Send;
	Send.Type = "House";
	Send.Faction = "";
	Send.Family = "Buildings";
	Send.ModelFile = "C:/Users/Artist/Documents/HeroesOfLiberty/Atelier/Entrees/Maison V2.glb";
	Send.Yaw = 90.0;
	Send.Scale = 1.25;
	Send.Status = "provisional";
	Send.Author = "Ana";
	Send.FileOrPack = "Maison V2.glb";
	Send.Source = "Ana (commande)";
	Send.Licence = "CC0";
	Send.Url = "https://example.org/maison";
	Send.Notes = "toit \"rouge\" C:\\temp";
	Send.MadeWith = "2026-11-27-db61f64";
	return Send;
}

void Test_AtelierEntryAskedByTheArgument(FUnitTest& T)
{
	T.AssertTrue(HOLAtelier::IsAsked("-Atelier"), "the argument alone");
	T.AssertTrue(HOLAtelier::IsAsked("../../../HeroesOfLiberty/HeroesOfLiberty.uproject -game -atelier -nosound"), "among others, case ignored");
	T.AssertFalse(HOLAtelier::IsAsked(""), "no argument: the game as for a player");
	T.AssertFalse(HOLAtelier::IsAsked("-game -AtelierX -HOLCheats"), "another word that starts the same");
}

void Test_AtelierEntryStepByMap(FUnitTest& T)
{
	T.AssertEquals("launch", HOLAtelier::EntryStep("MainMenu", true, false), "main menu with -Atelier: the match of the atelier is launched");
	T.AssertEquals("lab", HOLAtelier::EntryStep("Plain_Small", true, false), "the Plain map with -Atelier: the lab and the screen");
	T.AssertEquals("", HOLAtelier::EntryStep("Plain_Small", true, true), "once per run");
	T.AssertEquals("", HOLAtelier::EntryStep("MainMenu", false, false), "without the argument nothing changes");
	T.AssertEquals("", HOLAtelier::EntryStep("Plain_Small", false, false), "a player's match without the argument");
}

void Test_AtelierFamilyRefusesUnits(FUnitTest& T)
{
	TArray<FString> Buildings;
	Buildings.Add("House");
	Buildings.Add("Mill");
	TArray<FString> Sources;
	Sources.Add("Tree");
	TArray<FString> Units;
	Units.Add("Spearman");
	T.AssertEquals("Buildings", HOLAtelier::FamilyOf("House", Buildings, Sources, Units), "a building");
	T.AssertEquals("Sources", HOLAtelier::FamilyOf("Tree", Buildings, Sources, Units), "a source");
	T.AssertEquals("", HOLAtelier::FamilyOf("Spearman", Buildings, Sources, Units), "a unit has no family here");
	T.AssertEquals("type_unit", HOLAtelier::TypeRefusal("Spearman", Buildings, Sources, Units), "a unit is refused");
	T.AssertEquals("type_unknown", HOLAtelier::TypeRefusal("Castle", Buildings, Sources, Units), "a type of no table");
	T.AssertEquals("", HOLAtelier::TypeRefusal("House", Buildings, Sources, Units), "a fixed type is accepted");
	const FString UnitText = HOLAtelier::RefusalText("type_unit").ToString();
	T.AssertEquals("unités : par l'éditeur", UnitText, "the message the artist reads");
}

void Test_AtelierFamilyFromTheGameTables(FUnitTest& T)
{
	TArray<FString> Buildings = HOLAtelier::TableNames("Buildings");
	TArray<FString> Sources = HOLAtelier::TableNames("Sources");
	TArray<FString> Units = HOLAtelier::TableNames("Units");
	T.AssertEquals("Buildings", HOLAtelier::FamilyOf("House", Buildings, Sources, Units), "House is a row of BuildingTypes");
	T.AssertEquals("Sources", HOLAtelier::FamilyOf("BerryBush", Buildings, Sources, Units), "BerryBush is a row of SourceTypes");
	T.AssertEquals("type_unit", HOLAtelier::TypeRefusal("ManAtArms", Buildings, Sources, Units), "ManAtArms is a row of UnitTypes");
	TArray<FString> Fixed = HOLAtelier::FixedTypes(Buildings, Sources);
	T.AssertTrue(Fixed.Num() == Buildings.Num() + Sources.Num() && Fixed.Num() > 20, "the types the screen offers: buildings then sources");
	T.AssertFalse(Fixed.Contains("ManAtArms"), "no unit offered");
}

void Test_AtelierSendFolderName(FUnitTest& T)
{
	const FDateTime When(2026, 11, 27, 9, 5, 0);
	T.AssertEquals("20261127-0905", HOLAtelier::StampOf(When), "aaaammjj-hhmm, two digits each");
	T.AssertEquals("House_20261127-0905", HOLAtelier::SendFolderName("House", "", When), "no faction");
	T.AssertEquals("House_Templars_20261127-0905", HOLAtelier::SendFolderName("House", "Templars", When), "a faction variant");
}

// HALDIR-1127ar4: the send's folder in the panel's message (it overflowed the panel): from the Documents folder, cut in its middle.
void Test_AtelierShownPath(FUnitTest& T)
{
	const FString Documents = "C:/Users/Artist/Documents/";
	const FString Send = "C:/Users/Artist/Documents/HeroesOfLiberty/Atelier/Envois/House_Templars_20261127-0905_2";
	FString Shown = HOLAtelier::ShownPath(Send, Documents, 80);
	T.AssertEquals("Documents/HeroesOfLiberty/Atelier/Envois/House_Templars_20261127-0905_2", Shown, "under Documents: from the Documents folder's own name");
	Shown = HOLAtelier::ShownPath("C:\\Users\\Artist\\Documents\\HeroesOfLiberty\\Atelier\\Envois\\House", "C:\\Users\\Artist\\Documents", 80);
	T.AssertEquals("Documents/HeroesOfLiberty/Atelier/Envois/House", Shown, "back slashes, Documents without its last slash");
	Shown = HOLAtelier::ShownPath(Send, Documents, 48);
	T.AssertEquals("Documents/Heroes…/House_Templars_20261127-0905_2", Shown, "longer than 48: its start, …, its last folder");
	const int Length = Shown.Len();
	T.AssertEquals(48, Length, "cut to MaxChars");
	Shown = HOLAtelier::ShownPath("D:/Art/Envois/House", Documents, 80);
	T.AssertEquals("D:/Art/Envois/House", Shown, "elsewhere and short: as it is");
	Shown = HOLAtelier::ShownPath("D:/A_very_long_folder_name_without_any_slash_at_all_x", Documents, 20);
	T.AssertEquals("D:/A_very…h_at_all_x", Shown, "a last folder longer than the room: cut in its middle too");
}

void Test_AtelierModelPaths(FUnitTest& T)
{
	const FString Entrees = "C:/Users/Artist/Documents/HeroesOfLiberty/Atelier/Entrees/";
	T.AssertEquals(Entrees + "maison.glb", HOLAtelier::ModelPathOf(Entrees, " maison.glb "), "a name: in the known folder");
	T.AssertEquals("D:/Art/maison.gltf", HOLAtelier::ModelPathOf(Entrees, "D:/Art/maison.gltf"), "a full path as it is");
	T.AssertEquals("model.glb", HOLAtelier::SendModelName("C:/x/Maison V2.GLB"), "the copy in the send is model.<ext>");
	T.AssertEquals("model.gltf", HOLAtelier::SendModelName("C:/x/maison.gltf"), "gltf");
	T.AssertTrue(HOLAtelier::EntreesDir().EndsWith("HeroesOfLiberty/Atelier/Entrees/"), "the known folder of the models");
	T.AssertTrue(HOLAtelier::EnvoisDir().EndsWith("HeroesOfLiberty/Atelier/Envois/"), "the folder of the sends");
}

void Test_AtelierNumbers(FUnitTest& T)
{
	T.AssertEquals("90", HOLAtelier::NumberText(90.0), "whole");
	T.AssertEquals("1.25", HOLAtelier::NumberText(1.25), "decimals, point");
	T.AssertEquals("-22.5", HOLAtelier::NumberText(-22.5), "negative");
	T.AssertEquals("0.1", HOLAtelier::NumberText(0.1000001), "rounded to the thousandth");
}

void Test_AtelierManifest(FUnitTest& T)
{
	const FString Json = HOLAtelier::ManifestJson(AtelierTestSend());
	T.AssertTrue(Json.Contains("\"format\": 1,"), "format 1");
	T.AssertTrue(Json.Contains("\"type\": \"House\","), "type");
	T.AssertTrue(Json.Contains("\"faction\": \"\","), "no faction = every faction");
	T.AssertTrue(Json.Contains("\"family\": \"Buildings\","), "family");
	T.AssertTrue(Json.Contains("\"model\": \"model.glb\","), "the copy's name, not the artist's file");
	T.AssertTrue(Json.Contains("\"yaw\": 90,"), "yaw as a number");
	T.AssertTrue(Json.Contains("\"scale\": 1.25,"), "scale as a number");
	T.AssertTrue(Json.Contains("\"status\": \"provisional\","), "status");
	T.AssertTrue(Json.Contains("\"author\": \"Ana\","), "author");
	T.AssertTrue(Json.Contains("\"file_or_pack\": \"Maison V2.glb\","), "licence: file_or_pack");
	T.AssertTrue(Json.Contains("\"source\": \"Ana (commande)\","), "licence: source");
	T.AssertTrue(Json.Contains("\"licence\": \"CC0\","), "licence: licence");
	T.AssertTrue(Json.Contains("\"url\": \"https://example.org/maison\","), "licence: url");
	T.AssertTrue(Json.Contains("\"downloaded_on\": \"\","), "licence: downloaded_on (column of assets_art.csv)");
	T.AssertTrue(Json.Contains("\"licence_proof\": \"\""), "licence: licence_proof");
	T.AssertTrue(Json.Contains("\"notes\": \"toit \\\"rouge\\\" C:\\\\temp\","), "notes escaped for JSON");
	T.AssertTrue(Json.Contains("\"made_with\": \"2026-11-27-db61f64\""), "made_with = the version of the game");
}

void Test_AtelierSaveRefusals(FUnitTest& T)
{
	TArray<FString> Buildings;
	Buildings.Add("House");
	TArray<FString> Sources;
	TArray<FString> Units;
	Units.Add("Spearman");
	FHOLAtelierSend Good = AtelierTestSend();
	TArray<FString> None = HOLAtelier::SaveRefusals(Good, true, Buildings, Sources, Units);
	T.AssertEquals(0, None.Num(), "a complete send is written");

	FHOLAtelierSend Bad = AtelierTestSend();
	Bad.FileOrPack = " ";
	Bad.Licence = "";
	Bad.Scale = 0.0;
	TArray<FString> Three = HOLAtelier::SaveRefusals(Bad, true, Buildings, Sources, Units);
	T.AssertTrue(Three.Contains("licence_file_or_pack") && Three.Contains("licence_licence") && Three.Contains("scale"), "incomplete licence and scale, all listed");
	T.AssertFalse(Three.Contains("licence_source"), "the filled source is not refused");

	TArray<FString> NoModel = HOLAtelier::SaveRefusals(Good, false, Buildings, Sources, Units);
	T.AssertTrue(NoModel.Contains("no_model"), "no model loaded");

	FHOLAtelierSend Unit = AtelierTestSend();
	Unit.Type = "Spearman";
	Unit.Status = "final ";
	TArray<FString> UnitRefused = HOLAtelier::SaveRefusals(Unit, true, Buildings, Sources, Units);
	T.AssertTrue(UnitRefused.Contains("type_unit"), "a unit type is never written");
	T.AssertTrue(UnitRefused.Contains("status"), "status: final or provisional exactly");
}

// AR8-c (D931): the list of the models of Entrees and the copy of the chosen one into the send (no copy by hand any more).
FString AtelierTestDir(const FString& Leaf)
{
	const FDateTime Now = FDateTime::Now();
	const FString Stamp = HOLAtelier::StampOf(Now);
	const int Draw = Math::RandRange(0, 999999);
	return FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() + f"Tests/Atelier/{Leaf}_{Stamp}_{Now.GetSecond()}_{Draw}/");
}

void Test_AtelierModelList(FUnitTest& T)
{
	const FString Entrees = AtelierTestDir("entrees") + "Entrees/";
	FFileHelper::SaveStringToFile("m", Entrees + "maison.glb", FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	FFileHelper::SaveStringToFile("a", Entrees + "Arbre.gltf", FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	FFileHelper::SaveStringToFile("b", Entrees + "Arbre.bin", FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	FFileHelper::SaveStringToFile("l", Entrees + "LISEZMOI.txt", FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	const TArray<FString> Names = HOLAtelier::ListModels(Entrees);
	T.AssertEquals(2, Names.Num(), "the .glb and the .gltf of Entrees, not the note nor the .bin");
	if (Names.Num() == 2)
	{
		T.AssertEquals("Arbre.gltf", Names[0], "sorted by name");
		T.AssertEquals("maison.glb", Names[1], "sorted by name");
	}
}

void Test_AtelierModelStep(FUnitTest& T)
{
	T.AssertEquals(1, HOLAtelier::StepIndex(0, 3, 1), "next");
	T.AssertEquals(0, HOLAtelier::StepIndex(2, 3, 1), "after the last: the first");
	T.AssertEquals(2, HOLAtelier::StepIndex(0, 3, -1), "before the first: the last");
	T.AssertEquals(0, HOLAtelier::StepIndex(-1, 3, 1), "none chosen yet: the first");
	T.AssertEquals(-1, HOLAtelier::StepIndex(0, 0, 1), "an empty list: none");
}

void Test_AtelierCopyModelIntoTheSend(FUnitTest& T)
{
	const FString Dir = AtelierTestDir("send");
	const FString Model = Dir + "Entrees/Ma Maison.glb";
	FFileHelper::SaveStringToFile("glb bytes", Model, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	const FString Folder = Dir + "Envois/House_20261127-1200";
	const FString Written = HOLAtelier::CopyModelToSend(Model, Folder);
	T.AssertEquals("model.glb", Written, "the model copied under the name the manifest gives");
	FString Copy;
	FFileHelper::LoadFileToString(Copy, Folder + "/model.glb");
	T.AssertEquals("glb bytes", Copy, "the same content in the send");
	const FString Again = HOLAtelier::CopyModelToSend(Model, Folder);
	T.AssertEquals("", Again, "a model already in the send is not replaced");

	const FString Gltf = Dir + "Entrees/arbre.GLTF";
	FFileHelper::SaveStringToFile("{ \"asset\": {} }", Gltf, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	const FString GltfWritten = HOLAtelier::CopyModelToSend(Gltf, Dir + "Envois/Tree_20261127-1201");
	T.AssertEquals("model.gltf", GltfWritten, "a .gltf: model.gltf");
	const FString Missing = HOLAtelier::CopyModelToSend(Dir + "Entrees/absent.glb", Dir + "Envois/House_20261127-1202");
	T.AssertEquals("", Missing, "a missing model: nothing copied");
}
