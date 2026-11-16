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

// 400 cases: building slot -1..2, 0 to 7 candidates (half of the building's slot, half of any slot -1..2) of kinds -1..5 (outside 0..4 = a
// kind that never enters), places -1..6, occupants -1..2, the three kind settings in every combination. Both copies give the same list in every case, and the table is not empty of
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
		int BuildingSlot = GarrisonParityNext(Seed, 4) - 1;
		int Count = GarrisonParityNext(Seed, 8);
		for (int U = 0; U < Count; ++U)
		{
			// Half the candidates are of the building's own slot (often admitted), the others of any slot -1..2.
			Slots.Add(GarrisonParityNext(Seed, 2) == 0 ? BuildingSlot : GarrisonParityNext(Seed, 4) - 1);
			Kinds.Add(GarrisonParityNext(Seed, 7) - 1);
		}
		int Places = GarrisonParityNext(Seed, 8) - 1;
		int Occupants = GarrisonParityNext(Seed, 4) - 1;
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

	// MIG-J-Garrison, extended to each kind of unit: every kind -1..5 (0..4 the rule's, the others never enter) from each slot -1..2 into a
	// building of slot -1 or 0, with -1..5 places, -1..2 occupants and the three settings in every combination; the candidates are two of that
	// kind and slot around an own archer and one of that kind from the building's slot (8960 cases, none drawn by chance).
	int Cases = 0;
	int Exhaustive = 0;
	int AdmittingKind = 0;
	for (int Kind = -1; Kind <= 5; ++Kind)
	{
		for (int UnitSlot = -1; UnitSlot <= 2; ++UnitSlot)
		{
			for (int Settings = 0; Settings < 8; ++Settings)
			{
				for (int Building = -1; Building <= 0; ++Building)
				{
					for (int P = 0; P < 5; ++P)
					{
						const int ExPlaces = P == 4 ? 5 : P - 1;
						for (int ExOccupants = -1; ExOccupants <= 2; ++ExOccupants)
						{
							TArray<int> ExSlots;
							TArray<int> ExKinds;
							ExSlots.Add(UnitSlot); ExKinds.Add(Kind);
							ExSlots.Add(0); ExKinds.Add(HOLGarrison::KindRangedInfantry);
							ExSlots.Add(Building); ExKinds.Add(Kind);
							ExSlots.Add(UnitSlot); ExKinds.Add(Kind);
							const bool bM = (Settings & 1) != 0;
							const bool bH = (Settings & 2) != 0;
							const bool bC = (Settings & 4) != 0;
							TArray<int> ExScript = HOLGarrison::AdmittedWith(ExSlots, ExKinds, Building, ExPlaces, ExOccupants, bM, bH, bC);
							TArray<int> ExCpp = HOLGarrisonRule::AdmittedWith(ExSlots, ExKinds, Building, ExPlaces, ExOccupants, bM, bH, bC);
							++Cases;
							if (!GarrisonParitySame(ExScript, ExCpp))
							{
								++Exhaustive;
								if (Exhaustive <= 5)
									T.AssertTrue(false, "kind " + Kind + " slot " + UnitSlot + " settings " + Settings + ": script " + GarrisonParityText(ExScript) + " but C++ " + GarrisonParityText(ExCpp));
							}
							if (ExScript.Contains(2))
								++AdmittingKind;
						}
					}
				}
			}
		}
	}
	T.AssertEquals(Cases, 7 * 4 * 8 * 2 * 5 * 4, "every kind, slot, setting, building, place and occupant case compared");
	T.AssertEquals(Exhaustive, 0, "the script and the C++ twin admit the same units for every kind of unit");
	T.AssertTrue(AdmittingKind > 0, "the table admits units of the kind tried (" + AdmittingKind + ")");
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
