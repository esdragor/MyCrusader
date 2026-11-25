// D924 (AR2-a): the rules of HOLArtChecks.as, each asserted on the value the rule computes (pass / fail, path, line).

// Pivot on the ground: the bottom of the bounds at 0 within the tolerance passes; a mesh centred on its pivot fails.
void Test_ArtCheckPivotOnTheGround(FUnitTest& T)
{
	T.AssertTrue(HOLArtChecks::CheckPivot(0.f).bPass, "bottom at 0");
	T.AssertTrue(HOLArtChecks::CheckPivot(-1.5f).bPass, "within the tolerance");
	const FHOLArtCheck Centred = HOLArtChecks::CheckPivot(-90.f);
	T.AssertFalse(Centred.bPass, "a mesh centred on its pivot sinks 90 cm");
	T.AssertTrue(Centred.Line.Contains("-90"), "the line names the gap: " + Centred.Line);
}

// Material slots: at least one, at most the budget; triangles under the budget.
void Test_ArtCheckMaterialsAndTriangles(FUnitTest& T)
{
	T.AssertFalse(HOLArtChecks::CheckMaterialSlots(0).bPass, "no material");
	T.AssertTrue(HOLArtChecks::CheckMaterialSlots(2).bPass, "two materials");
	T.AssertFalse(HOLArtChecks::CheckMaterialSlots(HOLArtChecks::MaxMaterialSlots + 1).bPass, "over the budget");
	T.AssertTrue(HOLArtChecks::CheckTriangles(1200, HOLArtChecks::MaxUnitTriangles).bPass, "a light unit");
	const FHOLArtCheck Heavy = HOLArtChecks::CheckTriangles(HOLArtChecks::MaxUnitTriangles + 1, HOLArtChecks::MaxUnitTriangles);
	T.AssertFalse(Heavy.bPass, "one triangle over the budget");
	T.AssertTrue(Heavy.Line.Contains("" + HOLArtChecks::MaxUnitTriangles), "the line names the budget: " + Heavy.Line);
}

// Names and folder: SM_ for a static mesh, SKM_ for a skinned one, under /Game/Art/.
void Test_ArtCheckNameAndFolder(FUnitTest& T)
{
	T.AssertTrue(HOLArtChecks::CheckMeshPrefix("SM_Forum", false).bPass, "SM_ static");
	T.AssertFalse(HOLArtChecks::CheckMeshPrefix("Forum", false).bPass, "no prefix");
	T.AssertFalse(HOLArtChecks::CheckMeshPrefix("SM_Spearman", true).bPass, "SM_ on a skinned mesh");
	T.AssertTrue(HOLArtChecks::CheckMeshPrefix("SKM_Spearman", true).bPass, "SKM_ skinned");
	T.AssertTrue(HOLArtChecks::CheckArtFolder("/Game/Art/Units/Spearman").bPass, "under /Game/Art/");
	T.AssertFalse(HOLArtChecks::CheckArtFolder("/Game/Meshes/Units").bPass, "outside /Game/Art/");
	T.AssertFalse(HOLArtChecks::CheckArtFolder("/Game/Artwork/Spearman").bPass, "a look-alike folder");
}

// The art visual: its type in the tables, its set fields loaded, its sequences in the provider's list.
void Test_ArtCheckVisualFields(FUnitTest& T)
{
	TArray<FName> Known;
	Known.Add(n"Spearman");
	Known.Add(n"Forum");
	T.AssertTrue(HOLArtChecks::CheckTypeName(n"Spearman", Known).bPass, "a known type");
	T.AssertFalse(HOLArtChecks::CheckTypeName(n"Spearmen", Known).bPass, "a typo");
	T.AssertFalse(HOLArtChecks::CheckTypeName(NAME_None, Known).bPass, "no type");

	T.AssertTrue(HOLArtChecks::CheckLoaded("Mesh", false, false).bPass, "an empty field falls back: fine");
	T.AssertTrue(HOLArtChecks::CheckLoaded("Mesh", true, true).bPass, "set and loaded");
	T.AssertFalse(HOLArtChecks::CheckLoaded("Mesh", true, false).bPass, "set but broken");

	T.AssertTrue(HOLArtChecks::CheckSequence("Walk", false, 99, 3, false).bPass, "not overridden: the CSV's");
	T.AssertTrue(HOLArtChecks::CheckSequence("Walk", true, 2, 3, false).bPass, "the last of three");
	T.AssertFalse(HOLArtChecks::CheckSequence("Walk", true, 3, 3, false).bPass, "past the list");
	T.AssertTrue(HOLArtChecks::CheckSequence("Attack", true, -1, 3, true).bPass, "an optional state left out");
	T.AssertFalse(HOLArtChecks::CheckSequence("Idle", true, -1, 3, false).bPass, "idle cannot be left out");
}

// Create the DA_Art: the path built from the family and the type, the faction variant suffixed.
void Test_ArtAssetPathFromFamily(FUnitTest& T)
{
	const FString Units = HOLArtChecks::Family(true, false, false);
	T.AssertEquals("/Game/Art/Units/Spearman/DA_Art_Spearman", HOLArtChecks::ArtAssetPath(Units, n"Spearman", NAME_None), "common asset");
	T.AssertEquals("/Game/Art/Units/Spearman/DA_Art_Spearman_Legions", HOLArtChecks::ArtAssetPath(Units, n"Spearman", n"Legions"), "faction variant");
	T.AssertEquals("/Game/Art/Buildings/Forum/DA_Art_Forum", HOLArtChecks::ArtAssetPath(HOLArtChecks::Family(false, true, false), n"Forum", NAME_None), "a building");
	T.AssertEquals("", HOLArtChecks::ArtAssetPath(HOLArtChecks::Family(false, false, false), n"Nothing", NAME_None), "no family: no path");

	TArray<FHOLArtCheck> Checks;
	Checks.Add(HOLArtChecks::CheckPivot(0.f));
	Checks.Add(HOLArtChecks::CheckMeshPrefix("SM_Forum", false));
	T.AssertTrue(HOLArtChecks::AllPass(Checks), "all green");
	Checks.Add(HOLArtChecks::CheckArtFolder("/Game/Meshes"));
	T.AssertFalse(HOLArtChecks::AllPass(Checks), "one red line");
}
