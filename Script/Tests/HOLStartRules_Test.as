// Lot E3-03 and E3-04 (Locations) of the migration (D830, D836, D839): the script port of two C++ tests, run against the read libraries
// of the rules that STAY in C++ (HOLStartRulesReadLibrary, S1D-1116b). Same vectors as the C++ tests; the C++ tests are removed at lot E3-FIN.

// HOLStartRulesTests.cpp (HeroesOfLiberty.Match.StartRules): R3.9, a match launched by the lobby manager waits for its named players,
// 60 s at most, then starts without the absent.
void Test_StartRulesVectors(FUnitTest& T)
{
	TArray<FString> Expected;
	Expected.Add("Alice");
	Expected.Add("Bob");

	TArray<FString> NobodyHere;
	TArray<FString> Missing = HOLStartRulesRead::GetMissing(Expected, NobodyHere);
	T.AssertEquals(2, Missing.Num(), "nobody here: two missing");
	if (Missing.Num() == 2)
	{
		T.AssertTrue(Missing[0] == "Alice", "nobody here: Alice first, in order");
		T.AssertTrue(Missing[1] == "Bob", "nobody here: then Bob");
	}

	TArray<FString> BobHere;
	BobHere.Add(" bob ");
	Missing = HOLStartRulesRead::GetMissing(Expected, BobHere);
	T.AssertEquals(1, Missing.Num(), "Bob here (trimmed, case ignored): one missing");
	if (Missing.Num() == 1)
	{
		T.AssertTrue(Missing[0] == "Alice", "Bob here (trimmed, case ignored): Alice missing");
	}

	TArray<FString> AllHere;
	AllHere.Add("Bob");
	AllHere.Add("Alice");
	AllHere.Add("Chloe");
	T.AssertEquals(0, HOLStartRulesRead::GetMissing(Expected, AllHere).Num(), "all here (and a stranger): nobody missing");

	T.AssertFalse(HOLStartRulesRead::ShouldStart(1, 10.0, 60.0), "one missing at 10 s of 60: waits");
	T.AssertFalse(HOLStartRulesRead::ShouldStart(1, 59.9, 60.0), "one missing just before the limit: waits");
	T.AssertTrue(HOLStartRulesRead::ShouldStart(1, 60.0, 60.0), "one missing at the limit: starts");
	T.AssertTrue(HOLStartRulesRead::ShouldStart(0, 3.0, 60.0), "all here at 3 s: starts");
	T.AssertTrue(HOLStartRulesRead::ShouldStart(0, 0.0, 60.0), "all here at once: starts");
	T.AssertTrue(HOLStartRulesRead::ShouldStart(2, 0.0, 0.0), "no wait limit (0): starts at once");
}

// HOLStartingPeasantsTests.cpp (HeroesOfLiberty.Match.StartingPeasants.Locations): eight forums around the middle of a map, as Plain's
// eight starts (about 150 m out), two body sizes; the Peasants stand in front of the forum, off the body, off the King, apart.
void Test_StartingPeasantsLocations(FUnitTest& T)
{
	const double Pi = 3.14159265358979323846;
	for (int Body = 0; Body < 2; Body++)
	{
		const float HalfBody = (Body == 0) ? 600.0f : 800.0f;
		for (int Slot = 0; Slot < 8; Slot++)
		{
			const double Angle = Slot * Pi / 4.0;
			const FVector Centre(15000.0 * Math::Cos(Angle), 15000.0 * Math::Sin(Angle), 120.0);
			const FVector King = HOLStartingPeasantsRead::GetKingLocation(Centre, HalfBody);
			TArray<FVector> Peasants = HOLStartingPeasantsRead::GetStartingPeasantLocations(Centre, HalfBody, 5);
			const FString Where = f"slot {Slot}, half body {HalfBody}";
			T.AssertEquals(5, Peasants.Num(), f"{Where}: 5 places");
			if (Peasants.Num() != 5)
			{
				continue;
			}
			for (int Index = 0; Index < Peasants.Num(); Index++)
			{
				const FVector Peasant = Peasants[Index];
				T.AssertTrue(Peasant.X <= Centre.X - HalfBody - 100.0, f"{Where}: Peasant {Index} in front of the forum, off its body");
				T.AssertTrue(Peasant.X >= Centre.X - HalfBody - 500.0 && Math::Abs(Peasant.Y - Centre.Y) <= HalfBody, f"{Where}: Peasant {Index} by the forum (within 5 m of its edge)");
				T.AssertTrue(Dist2D(Peasant, King) >= 200.0, f"{Where}: Peasant {Index} off the King (2 m)");
				T.AssertTrue(Peasant.Z == Centre.Z, f"{Where}: Peasant {Index} at the forum's height");
				for (int Other = Index + 1; Other < Peasants.Num(); Other++)
				{
					T.AssertTrue(Dist2D(Peasant, Peasants[Other]) >= 150.0, f"{Where}: Peasants {Index} and {Other} apart (1.5 m)");
				}
			}
		}
	}
	T.AssertEquals(0, HOLStartingPeasantsRead::GetStartingPeasantLocations(FVector(0.0, 0.0, 0.0), 600.0f, 0).Num(), "No Peasant, no place");
	T.AssertEquals(3, HOLStartingPeasantsRead::GetStartingPeasantLocations(FVector(0.0, 0.0, 0.0), 600.0f, 3).Num(), "Another number, as many places");
}

double Dist2D(const FVector& A, const FVector& B)
{
	const double DX = A.X - B.X;
	const double DY = A.Y - B.Y;
	return Math::Sqrt(DX * DX + DY * DY);
}