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

// AR2-b: the triangle budget from the folder (a skinned unit is held to the unit budget too).
void Test_ArtCheckTriangleBudgetFromFolder(FUnitTest& T)
{
	const int Unit = HOLArtChecks::TriangleBudget("/Game/Art/Units/Spearman/SKM_Spearman.SKM_Spearman");
	const int Building = HOLArtChecks::TriangleBudget("/Game/Art/Buildings/Forum/SM_Forum.SM_Forum");
	T.AssertTrue(Unit == HOLArtChecks::MaxUnitTriangles, "a unit's mesh: " + Unit);
	T.AssertTrue(Building == HOLArtChecks::MaxBuildingTriangles, "a building's mesh: " + Building);
	T.AssertFalse(HOLArtChecks::CheckTriangles(9000, Unit).bPass, "a 9000-triangle unit is over");
	T.AssertTrue(HOLArtChecks::CheckTriangles(9000, Building).bPass, "a 9000-triangle building is under");
}

// AR2-b: Projectile / MuzzleSmoke: empty is fine, set must load and be a StaticMesh, its pivot inside its bounds.
void Test_ArtCheckShotAssets(FUnitTest& T)
{
	T.AssertTrue(HOLArtChecks::CheckShotField("Projectile", false, false, false).bPass, "a melee unit: no projectile");
	T.AssertTrue(HOLArtChecks::CheckShotField("Projectile", true, true, true).bPass, "an arrow mesh");
	const FHOLArtCheck Broken = HOLArtChecks::CheckShotField("Projectile", true, false, false);
	T.AssertFalse(Broken.bPass, "set but does not load");
	T.AssertTrue(Broken.Line.Contains("Projectile"), "the line names the field: " + Broken.Line);
	T.AssertFalse(HOLArtChecks::CheckShotField("MuzzleSmoke", true, true, false).bPass, "loads but is not a StaticMesh");

	T.AssertTrue(HOLArtChecks::CheckShotPivot("Projectile", FVector(-50.f, -1.f, -1.f), FVector(50.f, 1.f, 1.f)).bPass, "an arrow around its pivot");
	const FHOLArtCheck Offset = HOLArtChecks::CheckShotPivot("MuzzleSmoke", FVector(100.f, -10.f, -10.f), FVector(200.f, 10.f, 10.f));
	T.AssertFalse(Offset.bPass, "a smoke one metre off its pivot");
	T.AssertTrue(Offset.Line.Contains("MuzzleSmoke"), "the line names the field: " + Offset.Line);
}

// AR2-b: an AnimSequence under /Game/Art/<Family>/<Type>/ is checked against its type's SkinnedMesh.
void Test_ArtCheckAnimSequence(FUnitTest& T)
{
	T.AssertTrue(HOLArtChecks::TypeFromArtPath("/Game/Art/Units/Spearman/Anims") == n"Spearman", "type from a sub-folder");
	T.AssertTrue(HOLArtChecks::TypeFromArtPath("/Game/Art/Buildings/Forum") == n"Forum", "type from its folder");
	T.AssertTrue(HOLArtChecks::TypeFromArtPath("/Game/Art/Units") == NAME_None, "a family folder has no type");
	T.AssertTrue(HOLArtChecks::TypeFromArtPath("/Game/Meshes/Spearman") == NAME_None, "outside /Game/Art/");

	const FString Skel = "/Game/Art/Units/Spearman/SK_Spearman.SK_Spearman";
	T.AssertTrue(HOLArtChecks::CheckSkeleton(Skel, Skel).bPass, "same skeleton");
	T.AssertFalse(HOLArtChecks::CheckSkeleton(Skel, "/Game/Art/Units/Archer/SK_Archer.SK_Archer").bPass, "another type's skeleton");
	T.AssertFalse(HOLArtChecks::CheckSkeleton(Skel, "").bPass, "no SkinnedMesh to compare with");

	T.AssertTrue(HOLArtChecks::CheckAnimLength(1.2f).bPass, "a 1.2 s walk");
	T.AssertFalse(HOLArtChecks::CheckAnimLength(0.f).bPass, "an empty clip");
	T.AssertFalse(HOLArtChecks::CheckAnimLength(HOLArtChecks::MaxAnimSeconds + 1.f).bPass, "over the length bound");
	T.AssertTrue(HOLArtChecks::CheckAnimFrames(30).bPass, "30 frames");
	T.AssertFalse(HOLArtChecks::CheckAnimFrames(1).bPass, "a single pose");
}

// AR2-b: "See in the Workshop" gives the full console line, ready to paste.
void Test_ArtTryCommandLine(FUnitTest& T)
{
	T.AssertEquals("hol.Art.Try Arquebusier 1", HOLArtChecks::TryCommand(n"Arquebusier"), "one Arquebusier");
}
