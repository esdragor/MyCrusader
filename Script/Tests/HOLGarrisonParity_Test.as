// Garrison, parity of the two copies of the rule (F7): the script's HOLGarrison::AdmittedWith / Admitted (Script/Garrison/HOLGarrisonRules.as)
// and the C++ twin the server applies (Source/HeroesOfLiberty/Buildings/HOLGarrisonRules.h, reached through UHOLGarrisonRuleLibrary, namespace
// HOLGarrisonRule). Each list is computed by its own copy on the same table of cases, never written by hand; a case where they differ fails.

// A fixed pseudo-random sequence (the same table on every run).
int GarrisonParityNext(int& Seed, int Range)
{
	Seed = (Seed * 1103515245 + 12345) & 0x7fffffff;
	return (Seed >> 8) % Range;
}

FString GarrisonParityText(const TArray<int>& Values)
{
	FString Text = "[";
	for (int I = 0; I < Values.Num(); ++I)
	{
		Text += (I > 0 ? "," : "") + Values[I];
	}
	return Text + "]";
}

bool GarrisonParitySame(const TArray<int>& A, const TArray<int>& B)
{
	if (A.Num() != B.Num())
	{
		return false;
	}
	for (int I = 0; I < A.Num(); ++I)
	{
		if (A[I] != B[I])
		{
			return false;
		}
	}
	return true;
}

// 400 cases: 0 to 7 candidates of slots -1..2 and kinds -1..5 (outside 0..4 = a kind that never enters), building slot -1..1, places -1..6,
// occupants -1..6, the three kind settings in every combination. Both copies give the same list in every case, and the table is not empty of
// meaning: many cases admit someone, and some admit a rider only because the cavalry setting is on.
void Test_GarrisonScriptAndCppAdmitTheSame(FUnitTest& T)
{
	int Seed = 1793;
	int Differ = 0;
	int Admitting = 0;
	int RiderBySetting = 0;
	for (int Case = 0; Case < 400; ++Case)
	{
		TArray<int> Slots;
		TArray<int> Kinds;
		int Count = GarrisonParityNext(Seed, 8);
		for (int U = 0; U < Count; ++U)
		{
			Slots.Add(GarrisonParityNext(Seed, 4) - 1);
			Kinds.Add(GarrisonParityNext(Seed, 7) - 1);
		}
		int BuildingSlot = GarrisonParityNext(Seed, 3) - 1;
		int Places = GarrisonParityNext(Seed, 8) - 1;
		int Occupants = GarrisonParityNext(Seed, 8) - 1;
		bool bMelee = (Case & 1) != 0;
		bool bHero = (Case & 2) != 0;
		bool bCavalry = (Case & 4) != 0;
		TArray<int> Script = HOLGarrison::AdmittedWith(Slots, Kinds, BuildingSlot, Places, Occupants, bMelee, bHero, bCavalry);
		TArray<int> Cpp = HOLGarrisonRule::AdmittedWith(Slots, Kinds, BuildingSlot, Places, Occupants, bMelee, bHero, bCavalry);
		if (!GarrisonParitySame(Script, Cpp))
		{
			++Differ;
			T.AssertTrue(false, "case " + Case + ": script " + GarrisonParityText(Script) + " but C++ " + GarrisonParityText(Cpp));
		}
		if (Script.Num() > 0)
		{
			++Admitting;
		}
		if (bCavalry)
		{
			TArray<int> NoRider = HOLGarrison::AdmittedWith(Slots, Kinds, BuildingSlot, Places, Occupants, bMelee, bHero, false);
			if (!GarrisonParitySame(Script, NoRider))
			{
				++RiderBySetting;
			}
		}
	}
	T.AssertEquals(Differ, 0, "the script and the C++ twin admit the same units in all 400 cases");
	T.AssertTrue(Admitting >= 80, "at least 80 of the 400 cases admit someone (" + Admitting + ")");
	T.AssertTrue(RiderBySetting >= 10, "at least 10 cases change with the cavalry setting (" + RiderBySetting + ")");
}

// With the default settings, read by name on each side (the script's hol.Garrison.* variables; the server reads the same names): the same list.
void Test_GarrisonScriptAndCppDefaultsAgree(FUnitTest& T)
{
	TArray<int> Slots;
	TArray<int> Kinds;
	for (int Kind = 0; Kind <= 4; ++Kind)
	{
		Slots.Add(0);
		Kinds.Add(Kind);
	}
	TArray<int> Script = HOLGarrison::Admitted(Slots, Kinds, 0, 8, 0);
	TArray<int> Cpp = HOLGarrisonRule::AdmittedBySettings(Slots, Kinds, 0, 8, 0);
	T.AssertTrue(GarrisonParitySame(Script, Cpp), "defaults: script " + GarrisonParityText(Script) + ", C++ " + GarrisonParityText(Cpp));
	T.AssertEquals(Cpp.Num(), 4, "by default the server admits every kind but siege (D793 Q2 B)");
}
