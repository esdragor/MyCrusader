// Socle step 1, lot S1d (D830, D836, D839, S1D-1116b): one smoke test per read library of the rules that STAY in C++, each on ONE vector read in
// its C++ test. The full port of each C++ test is the next lot (E3-03, E3-04 Locations, E3-07, E3-08 of Auto_Journey_File.md).
// This file compiles only once the plugin is rebuilt with the five libraries (HOLStartRulesReadLibrary, HOLHUDLayoutReadLibrary,
// HOLVersionReadLibrary, HOLAccessKeyReadLibrary, HOLStartingPeasantsReadLibrary): until then the script compile fails, as expected.

// HOLStartRulesTests.cpp (HeroesOfLiberty.Match.StartRules): Bob here (trimmed, case ignored), Alice missing; the wait limit.
void Test_StartRulesReadLibrarySmoke(FUnitTest& T)
{
	TArray<FString> Expected;
	Expected.Add("Alice");
	Expected.Add("Bob");
	TArray<FString> Present;
	Present.Add(" bob ");
	TArray<FString> Missing = HOLStartRulesRead::GetMissing(Expected, Present);
	T.AssertEquals(1, Missing.Num(), "Bob here (trimmed, case ignored): one missing");
	if (Missing.Num() == 1)
	{
		T.AssertTrue(Missing[0] == "Alice", "Alice is the one missing");
	}
	T.AssertFalse(HOLStartRulesRead::ShouldStart(1, 59.9, 60.0), "one missing just before the limit: waits");
	T.AssertTrue(HOLStartRulesRead::ShouldStart(1, 60.0, 60.0), "one missing at the limit: starts");
}

// HOLHUDLayoutTests.cpp (HeroesOfLiberty.UI.Layout.Overlap, CheatsLowWindow): a line over the cheats panel overlaps it, touching edges do not.
void Test_HUDLayoutReadLibrarySmoke(FUnitTest& T)
{
	const FVector2D CheatsMin(16.0, 62.0);
	const FVector2D CheatsMax(220.0, 400.0);
	T.AssertTrue(HOLHUDLayoutRead::Overlaps(FVector2D(16.0, 72.0), FVector2D(300.0, 100.0), CheatsMin, CheatsMax, 1.0), "a line over the panel overlaps it");
	T.AssertFalse(HOLHUDLayoutRead::Overlaps(FVector2D(16.0, 400.0), FVector2D(300.0, 428.0), CheatsMin, CheatsMax, 1.0), "touching edges: no");
	T.AssertTrue(HOLHUDLayoutRead::IsLowWindow(600.0f), "600 px is low");
	T.AssertFalse(HOLHUDLayoutRead::IsLowWindow(700.0f), "700 px is not");
	T.AssertFalse(HOLHUDLayoutRead::CheatsStartOpen(true, 600.0f), "low: starts folded whatever was kept");

	FHOLHUDFoldState State;
	State = HOLHUDLayoutRead::ShouldFoldCheats(600.0f, false, true, State);
	T.AssertFalse(State.bFold, "low, hero block not there yet: nothing");
	State = HOLHUDLayoutRead::ShouldFoldCheats(600.0f, true, true, State);
	T.AssertTrue(State.bFold, "the hero block opens over an open panel: fold");
	State = HOLHUDLayoutRead::ShouldFoldCheats(600.0f, true, true, State);
	T.AssertFalse(State.bFold, "once: a click that opens it again stays");
}

// HOLVersionTests.cpp (HeroesOfLiberty.Version, VersionDisplay, VersionCheck): Parse, Display and the check at login on their first vectors.
void Test_VersionReadLibrarySmoke(FUnitTest& T)
{
	T.AssertTrue(HOLVersionRead::Parse(" 2026-10-23-f7269a8\r\nmore\n") == "2026-10-23-f7269a8", "first line, trimmed");
	T.AssertTrue(HOLVersionRead::Parse("\r\n") == HOLVersionRead::GetUnknown(), "empty file: unknown");
	T.AssertTrue(HOLVersionRead::Display("0.3.0") == "V 0.3.0", "numbered version shown with its V");
	T.AssertTrue(HOLVersionRead::Display("0.3") == "0.3", "two parts: as it is");

	const FString Packaged = "2026-10-23-b4a8c8c";
	const FString Other = "2026-10-24-0123abc";
	FString Error;
	T.AssertTrue(HOLVersionRead::IsAccepted(Packaged, Packaged, Error), "same packaged version");
	T.AssertFalse(HOLVersionRead::IsAccepted(Packaged, Other, Error), "other packaged version");
	T.AssertTrue(Error == "Version mismatch: server " + Packaged + ", client " + Other, "mismatch message");
}

// HOLVersionTests.cpp (HeroesOfLiberty.AccessKey): no key expected: everyone; a key expected: that key only, and the message never has it.
void Test_AccessKeyReadLibrarySmoke(FUnitTest& T)
{
	FString Error;
	T.AssertTrue(HOLAccessKeyRead::IsAccepted("", "", Error), "no key expected, none sent");
	T.AssertTrue(HOLAccessKeyRead::IsAccepted("Ab-_09", "Ab-_09", Error), "same key");
	T.AssertFalse(HOLAccessKeyRead::IsAccepted("Ab-_09", "ab-_09", Error), "case matters");
	T.AssertTrue(Error == "Access key mismatch", "mismatch message, without the key");
	T.AssertTrue(HOLAccessKeyRead::ParseServerKey("/Game/Maps/Plain -server -HOLAccessKey=Ab-_09 -Port=7778") == "Ab-_09", "server key from the command line");
}

// HOLStartingPeasantsTests.cpp (HeroesOfLiberty.Match.StartingPeasants.Locations): the last two vectors, and the King's place on one forum.
void Test_StartingPeasantsReadLibrarySmoke(FUnitTest& T)
{
	T.AssertEquals(0, HOLStartingPeasantsRead::GetStartingPeasantLocations(FVector(0.0, 0.0, 0.0), 600.0f, 0).Num(), "No Peasant, no place");
	T.AssertEquals(3, HOLStartingPeasantsRead::GetStartingPeasantLocations(FVector(0.0, 0.0, 0.0), 600.0f, 3).Num(), "Another number, as many places");

	const FVector Centre(15000.0, 0.0, 120.0);
	const FVector King = HOLStartingPeasantsRead::GetKingLocation(Centre, 600.0f);
	TArray<FVector> Peasants = HOLStartingPeasantsRead::GetStartingPeasantLocations(Centre, 600.0f, 5);
	T.AssertEquals(5, Peasants.Num(), "5 places");
	for (int Index = 0; Index < Peasants.Num(); Index++)
	{
		T.AssertTrue(Peasants[Index].X <= Centre.X - 600.0 - 100.0, f"Peasant {Index} in front of the forum, off its body");
		T.AssertTrue(Peasants[Index].X >= Centre.X - 600.0 - 500.0, f"Peasant {Index} by the forum (within 5 m of its edge)");
		T.AssertTrue(Peasants[Index].X > King.X, f"Peasant {Index} between the body's edge and the King");
		T.AssertTrue(Peasants[Index].Z == Centre.Z, f"Peasant {Index} at the forum's height");
	}
}
