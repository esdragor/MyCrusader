// B0 (D928): the trial command hol.Art.RuntimeLoad refuses what it cannot import before asking Interchange
// (HOLArtRuntimeLoad::Refusal), and names its package after the file.

void Test_RuntimeLoadRefusesAMissingFile(FUnitTest& T)
{
	T.AssertEquals("missing", HOLArtRuntimeLoad::Refusal(FPaths::ProjectSavedDir() + "B0/no_such_model_926.glb"), "a path with no file is refused");
}

void Test_RuntimeLoadRefusesEmptyAndOtherFormats(FUnitTest& T)
{
	T.AssertEquals("empty", HOLArtRuntimeLoad::Refusal("  "), "no path");
	T.AssertEquals("format", HOLArtRuntimeLoad::Refusal(FPaths::ProjectDir() + "HeroesOfLiberty.uproject"), "an existing file that is not a model");
}

void Test_RuntimeLoadAcceptsAnExistingModel(FUnitTest& T)
{
	T.AssertEquals("", HOLArtRuntimeLoad::Refusal(FPaths::ProjectDir() + "Content/Knight/Soi_Armour_A.fbx"), "the FBX of the repository can be tried");
}

void Test_RuntimeLoadPackageName(FUnitTest& T)
{
	T.AssertEquals("spearmanstatic", HOLArtRuntimeLoad::PackageNameOf("C:\B0\spearman_static.glb"), "letters and digits of the base name");
	T.AssertEquals("SoiArmourA", HOLArtRuntimeLoad::PackageNameOf("Content/Knight/Soi_Armour_A.fbx"), "slashes too");
}
