// AR8-c (D931): the two file bindings the atelier needs (UHOLArtVisualRegistry, HOLArtVisual.h): the copy of a file (the .glb into the
// send folder, which "Save" makes) and the list of a folder's files by extension (the models of Entrees). Each test works in its own
// new folder under Saved/Tests/ArtFiles/ (never in the user's Documents).

FString ArtFilesTestDir(const FString& Leaf)
{
	const FDateTime Now = FDateTime::Now();
	const FString Stamp = HOLAtelier::StampOf(Now);
	const int Draw = Math::RandRange(0, 999999);
	return FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() + f"Tests/ArtFiles/{Leaf}_{Stamp}_{Now.GetSecond()}_{Draw}/");
}

void ArtFilesTestWrite(const FString& Path, const FString& Text)
{
	FFileHelper::SaveStringToFile(Text, Path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}

FString ArtFilesTestRead(const FString& Path)
{
	FString Text;
	FFileHelper::LoadFileToString(Text, Path);
	return Text;
}

void Test_ArtFilesListByExtension(FUnitTest& T)
{
	const FString Dir = ArtFilesTestDir("list");
	ArtFilesTestWrite(Dir + "b.glb", "b");
	ArtFilesTestWrite(Dir + "A.GLTF", "a");
	ArtFilesTestWrite(Dir + "c.txt", "c");
	ArtFilesTestWrite(Dir + "d.glb.bak", "d");
	ArtFilesTestWrite(Dir + "Sub/e.glb", "e");
	TArray<FString> Models;
	Models.Add("glb");
	Models.Add(".gltf");
	const TArray<FString> Names = UHOLArtVisualRegistry::ListArtFiles(Dir, Models);
	T.AssertEquals(2, Names.Num(), "the .glb and the .gltf of the folder (case ignored), not the .txt, the .bak or the subfolder's file");
	if (Names.Num() == 2)
	{
		T.AssertEquals("A.GLTF", Names[0], "names only, sorted with the case ignored: A before b");
		T.AssertEquals("b.glb", Names[1], "names only, sorted");
	}
	TArray<FString> Text;
	Text.Add("TXT");
	const TArray<FString> Notes = UHOLArtVisualRegistry::ListArtFiles(Dir, Text);
	T.AssertEquals(1, Notes.Num(), "another extension: the .txt only");
	const TArray<FString> Absent = UHOLArtVisualRegistry::ListArtFiles(Dir + "Absent/", Models);
	T.AssertEquals(0, Absent.Num(), "a missing folder: nothing");
}

void Test_ArtFilesCopy(FUnitTest& T)
{
	const FString Dir = ArtFilesTestDir("copy");
	const FString From = Dir + "Entrees/maison.glb";
	const FString To = Dir + "Envois/House_x/model.glb";
	ArtFilesTestWrite(From, "first");
	T.AssertFalse(FPaths::DirectoryExists(Dir + "Envois/"), "the send folder does not exist before the copy");
	const bool bCopied = UHOLArtVisualRegistry::CopyArtFile(From, To, false);
	T.AssertTrue(bCopied, "copied, its folder made");
	T.AssertEquals("first", ArtFilesTestRead(To), "the same content");

	ArtFilesTestWrite(From, "second");
	const bool bAgain = UHOLArtVisualRegistry::CopyArtFile(From, To, false);
	T.AssertFalse(bAgain, "an existing file is not replaced without the flag");
	T.AssertEquals("first", ArtFilesTestRead(To), "the existing file kept");
	const bool bReplaced = UHOLArtVisualRegistry::CopyArtFile(From, To, true);
	T.AssertTrue(bReplaced, "replaced with the flag");
	T.AssertEquals("second", ArtFilesTestRead(To), "the new content");

	const FString Other = Dir + "Envois/Other/model.glb";
	const bool bMissing = UHOLArtVisualRegistry::CopyArtFile(Dir + "Entrees/absent.glb", Other, false);
	T.AssertFalse(bMissing, "a missing source: refused");
	T.AssertFalse(FPaths::FileExists(Other), "nothing written for a missing source");
	const bool bSame = UHOLArtVisualRegistry::CopyArtFile(From, From, true);
	T.AssertFalse(bSame, "a file onto itself: refused");
	T.AssertEquals("second", ArtFilesTestRead(From), "the source untouched");
	const bool bEmpty = UHOLArtVisualRegistry::CopyArtFile("", To, true);
	T.AssertFalse(bEmpty, "an empty path: refused");
}

void Test_ArtFilesCopyBinaryGlb(FUnitTest& T)
{
	// The scenario's test cube, a real binary glTF (0 bytes in its header): the copy reads back as the original does (the script has
	// no byte array reader; two files read the same way give the same text only when their bytes match).
	const FString Cube = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() + "Tools/Scenarios/art_atelier_cube.glb");
	T.AssertTrue(FPaths::FileExists(Cube), "the test cube is in the project");
	const FString To = ArtFilesTestDir("binary") + "Envois/House_y/model.glb";
	const bool bCopied = UHOLArtVisualRegistry::CopyArtFile(Cube, To, false);
	T.AssertTrue(bCopied, "the binary glb copied");
	const FString Original = ArtFilesTestRead(Cube);
	const FString Copy = ArtFilesTestRead(To);
	T.AssertTrue(Original.Len() > 0, "the cube reads");
	T.AssertEquals(Original, Copy, "the copy reads as the original");
	const FString Refused = HOLArtRuntimeLoad::Refusal(To);
	T.AssertEquals("", Refused, "the copy is a model the atelier's load takes");
}
