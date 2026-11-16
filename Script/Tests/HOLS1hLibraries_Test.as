// Socle step 1, lot S1h (D830, D836, D839, GLORFINDEL-1117b): one smoke test per function of the minimap read library (HOLMinimapReadLibrary,
// the rules that STAY in HOLMinimapView.cpp), each on ONE vector read in its C++ test. The full port of the two C++ tests
// (L17-05: AlertRing, TerrainSources) comes after the build. This file compiles only once the plugin is rebuilt with the library:
// until then the script compile fails, as expected.

// HOLMinimapAlertRingTests.cpp (HeroesOfLiberty.UI.Minimap.AlertRing): age 0 the full size, half a period halfway, a period later again; the console defaults.
void Test_MinimapAlertRingReadLibrarySmoke(FUnitTest& T)
{
	T.AssertTrue(Math::Abs(24.0f - HOLMinimapRead::AlertDiameter(0.0f, 1.0f, 24.0f)) < 0.001f, "age 0: the full size");
	T.AssertTrue(Math::Abs(16.0f - HOLMinimapRead::AlertDiameter(0.5f, 1.0f, 24.0f)) < 0.001f, "half a period: halfway");
	T.AssertTrue(Math::Abs(24.0f - HOLMinimapRead::AlertDiameter(3.0f, 1.0f, 24.0f)) < 0.001f, "a period later: again the full size");
	T.AssertTrue(Math::Abs(0.75f - HOLMinimapRead::GetConsoleFloat("hol.Minimap.AlertShrinkSeconds")) < 0.001f, "default pass of the shrinking");
	T.AssertTrue(Math::Abs(24.0f - HOLMinimapRead::GetConsoleFloat("hol.Minimap.AlertSize")) < 0.001f, "default size");
	T.AssertTrue(Math::Abs(6.0f - HOLMinimapRead::GetConsoleFloat("hol.Minimap.AlertSeconds")) < 0.001f, "default life");
}

// HOLMinimapTerrainTests.cpp (HeroesOfLiberty.UI.Minimap.TerrainSources): gathered from, still standing: no change; used up: changed.
void Test_MinimapTerrainSignatureReadLibrarySmoke(FUnitTest& T)
{
	TArray<FVector> Places;
	Places.Add(FVector(0.0, 0.0, 0.0));
	Places.Add(FVector(500.0, 0.0, 0.0));
	Places.Add(FVector(0.0, 500.0, 0.0));
	TArray<int32> Left;
	Left.Add(100);
	Left.Add(100);
	Left.Add(100);
	const int64 Whole = HOLMinimapRead::GetSourcesSignature(Places, Left);
	T.AssertTrue(Whole == HOLMinimapRead::GetSourcesSignature(Places, Left), "read twice: the same");
	Left[1] = 40;
	T.AssertTrue(Whole == HOLMinimapRead::GetSourcesSignature(Places, Left), "gathered from, still standing: no change");
	Left[1] = 0;
	T.AssertTrue(HOLMinimapRead::GetSourcesSignature(Places, Left) != Whole, "used up: changed");
}