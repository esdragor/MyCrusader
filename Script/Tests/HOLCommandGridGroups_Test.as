// MIG-C-CommandGrid, lot G2b (HALDIR-1119y): the AGREEMENT test of the command grid's cycle, subgroups, merge and distribution. NOT COMPILED when written.
// Two copies of the rule exist (the script one, Script/Player/HOLCommandGridGroups.as = namespace HOLCommandGridGroups, and the C++ one read through
// UHOLCommandGridReadLibrary = namespace HOLCommandGridRead): for every input of the sweeps the script rule must give what the C++ library gives.
// Sweeps: 4096 member lists (Split, Priority, Merge), cycles of 1 to 5 buildings on every last key (keys negative too: the uint32 order), 216 queue
// choices x 5 last keys, the repeat presses, the faces and categories. Plus hard values copied from the C++ tests' vectors (the numbers do not come from
// the C++ copy): Script/Tests/HOLCommandGrid_Test.as:246-263 (Cycle), :343-425 (Subgroups, MergeCommands), :428-453 (Distribution), themselves the port of
// Source/HeroesOfLiberty/Tests/HOLCommandGridTests.cpp (Cycle, Subgroups, Distribution).
// To SEE RED (rouge a voir), in HOLCommandGridGroups.as: in PickEvenly change `(Last + Step) % Choices.Num()` to `(Last + Step + 1) % Choices.Num()`
// (the sweep and Test_CGGHardValues both go red), or in SortsBefore change `Buildings[A].Rank < Buildings[B].Rank` to `>`, or the alone name n"MainCentre".

bool CGGSameCommand(const FHOLGridCommandView& A, const FHOLGridCommandView& B)
{
	return A.Command == B.Command && A.TypeName == B.TypeName && A.Age == B.Age && A.bGreyed == B.bGreyed && A.Resource == B.Resource
		&& A.bBuy == B.bBuy && A.Ability == B.Ability && A.CooldownSeconds == B.CooldownSeconds && A.bPack == B.bPack && A.Tier == B.Tier
		&& A.Option == B.Option && A.RemainingMs == B.RemainingMs;
}

bool CGGSameInts(const TArray<int>& A, const TArray<int>& B)
{
	if (A.Num() != B.Num())
	{
		return false;
	}
	for (int I = 0; I < A.Num(); I++)
	{
		if (A[I] != B[I])
		{
			return false;
		}
	}
	return true;
}

FHOLCycleBuildingView CGGCycle(int Rank, int Key)
{
	FHOLCycleBuildingView Building;
	Building.Rank = Rank;
	Building.Key = Key;
	return Building;
}

FHOLQueueChoiceView CGGChoice(int Queue, int Max, int Key)
{
	FHOLQueueChoiceView Choice;
	Choice.QueueLength = Queue;
	Choice.MaxQueue = Max;
	Choice.Key = Key;
	return Choice;
}

FHOLGroupMemberView CGGMember(FName TypeName, bool bAlone)
{
	FHOLGroupMemberView Member;
	Member.TypeName = TypeName;
	Member.bAlone = bAlone;
	return Member;
}

// Member list number Mask: 6 members, each of one of 4 types (the main centre is alone), base 4.
TArray<FHOLGroupMemberView> CGGMakeMembers(int Mask, int Count)
{
	TArray<FHOLGroupMemberView> Members;
	int Rest = Mask;
	for (int I = 0; I < Count; I++)
	{
		int Kind = Rest % 4;
		Rest = Rest / 4;
		if (Kind == 0)
		{
			Members.Add(CGGMember(n"MainCentre", true));
		}
		else if (Kind == 1)
		{
			Members.Add(CGGMember(n"House", false));
		}
		else if (Kind == 2)
		{
			Members.Add(CGGMember(n"LumberCamp", false));
		}
		else
		{
			Members.Add(CGGMember(n"Barracks", false));
		}
	}
	return Members;
}

// HeroesOfLiberty.CommandGridGroups.Subgroups (sweep: SplitSubgroups, GetCommandPriority, IsAloneInGroup)
void Test_CGGSubgroupsSweep(FUnitTest& T)
{
	int Mismatches = 0;
	FString First = "";
	for (int Count = 0; Count <= 6; Count++)
	{
		int Total = 1;
		for (int I = 0; I < Count; I++)
		{
			Total = Total * 4;
		}
		for (int Mask = 0; Mask < Total; Mask++)
		{
			TArray<FHOLGroupMemberView> Members = CGGMakeMembers(Mask, Count);
			TArray<FHOLSubgroupView> Script = HOLCommandGridGroups::SplitSubgroups(Members);
			TArray<FHOLSubgroupView> Cpp = HOLCommandGridRead::SplitSubgroups(Members);
			bool bSame = Script.Num() == Cpp.Num();
			for (int I = 0; bSame && I < Script.Num(); I++)
			{
				bSame = Script[I].bAll == Cpp[I].bAll && Script[I].TypeName == Cpp[I].TypeName && CGGSameInts(Script[I].Members, Cpp[I].Members);
			}
			if (!CGGSameInts(HOLCommandGridGroups::GetCommandPriority(Members), HOLCommandGridRead::GetCommandPriority(Members)))
			{
				bSame = false;
			}
			if (!bSame)
			{
				Mismatches++;
				if (First == "")
				{
					First = f"count {Count} mask {Mask}";
				}
			}
		}
	}
	T.AssertEquals(0, Mismatches, f"script and C++ split and order the groups alike, first difference: {First}");
	T.AssertTrue(HOLCommandGridGroups::IsAloneInGroup(n"MainCentre") == HOLCommandGridRead::IsAloneInGroup(n"MainCentre"), "the centre: alone alike");
	T.AssertTrue(HOLCommandGridGroups::IsAloneInGroup(n"House") == HOLCommandGridRead::IsAloneInGroup(n"House"), "a house: not alone alike");
}

// HeroesOfLiberty.CommandGridGroups.Merge (sweep: MergeCommands over member lists and priorities)
void Test_CGGMergeSweep(FUnitTest& T)
{
	int Cells = HOLCommandGridGroups::GetNumCells();
	T.AssertEquals(HOLCommandGridRead::GetNumCells(), Cells, "same number of cells");
	int Mismatches = 0;
	FString First = "";
	for (int Seed = 0; Seed < 400; Seed++)
	{
		int Count = 1 + Seed % 4;
		TArray<FHOLGroupMemberView> Members = CGGMakeMembers(Seed, Count);
		TArray<FHOLGridCommandView> PerBuilding;
		for (int B = 0; B < Count; B++)
		{
			for (int C = 0; C < Cells; C++)
			{
				FHOLGridCommandView Command;
				int Pick = (Seed * 7 + B * 3 + C * 5) % 3;
				if (Pick == 1)
				{
					Command.Command = EHOLGridCommandKind::Produce;
					Command.TypeName = B == 0 ? n"Peasant" : (B == 1 ? n"Militia" : n"Technology");
				}
				else if (Pick == 2)
				{
					Command.Command = EHOLGridCommandKind::Build;
					Command.Age = B;
				}
				PerBuilding.Add(Command);
			}
		}
		TArray<int> Priority = HOLCommandGridRead::GetCommandPriority(Members);
		if (Seed % 5 == 0)
		{
			Priority.Add(-1);
			Priority.Add(Count + 2);
			Priority.Add(0);
		}
		TArray<FHOLGridCommandView> Script = HOLCommandGridGroups::MergeCommands(PerBuilding, Priority);
		TArray<FHOLGridCommandView> Cpp = HOLCommandGridRead::MergeCommands(PerBuilding, Priority);
		bool bSame = Script.Num() == Cpp.Num();
		for (int I = 0; bSame && I < Script.Num(); I++)
		{
			bSame = CGGSameCommand(Script[I], Cpp[I]);
		}
		if (!bSame)
		{
			Mismatches++;
			if (First == "")
			{
				First = f"seed {Seed}";
			}
		}
	}
	T.AssertEquals(0, Mismatches, f"script and C++ merge the cells alike, first difference: {First}");
}

// HeroesOfLiberty.CommandGridGroups.Cycle (sweep: NextInCycle on every last key, IsRepeatPress, GetCellFace, categories)
void Test_CGGCycleSweep(FUnitTest& T)
{
	int Mismatches = 0;
	FString First = "";
	for (int Seed = 0; Seed < 300; Seed++)
	{
		int Count = 1 + Seed % 5;
		TArray<FHOLCycleBuildingView> Buildings;
		TArray<int> Keys;
		for (int I = 0; I < Count; I++)
		{
			int Key = ((Seed + I * 3) % 4 == 0) ? -(10 + I * 7 + Seed % 5) : (10 + I * 11 + Seed % 7);
			Keys.Add(Key);
			Buildings.Add(CGGCycle((Seed / 2 + I * 5) % 3, Key));
		}
		Keys.Add(0);
		Keys.Add(55);
		for (int K = 0; K < Keys.Num(); K++)
		{
			int Script = HOLCommandGridGroups::NextInCycle(Buildings, Keys[K]);
			int Cpp = HOLCommandGridRead::NextInCycle(Buildings, Keys[K]);
			if (Script != Cpp)
			{
				Mismatches++;
				if (First == "")
				{
					First = f"seed {Seed} key {Keys[K]}: script {Script} C++ {Cpp}";
				}
			}
		}
	}
	TArray<FHOLCycleBuildingView> NoBuilding;
	T.AssertEquals(HOLCommandGridRead::NextInCycle(NoBuilding, 0), HOLCommandGridGroups::NextInCycle(NoBuilding, 0), "no building: none alike");
	T.AssertEquals(0, Mismatches, f"script and C++ cycle alike, first difference: {First}");

	int Repeat = 0;
	for (int A = 0; A < 64; A++)
	{
		TArray<int> Selected;
		TArray<int> Last;
		for (int Bit = 0; Bit < 3; Bit++)
		{
			if ((A & (1 << Bit)) != 0)
			{
				Selected.Add(Bit + 1);
			}
			if ((A & (8 << Bit)) != 0)
			{
				Last.Add(Bit + 1);
			}
		}
		for (int Same = 0; Same < 2; Same++)
		{
			if (HOLCommandGridGroups::IsRepeatPress(Selected, Last, Same == 1) != HOLCommandGridRead::IsRepeatPress(Selected, Last, Same == 1))
			{
				Repeat++;
			}
		}
	}
	T.AssertEquals(0, Repeat, "script and C++ tell a repeat press alike (64 pairs of selections x 2)");

	for (int Icon = 0; Icon < 2; Icon++)
	{
		FHOLGridCellFaceView ScriptFace = HOLCommandGridGroups::GetCellFace(Icon == 1);
		FHOLGridCellFaceView CppFace = HOLCommandGridRead::GetCellFace(Icon == 1);
		T.AssertTrue(ScriptFace.bIcon == CppFace.bIcon && ScriptFace.bName == CppFace.bName && ScriptFace.bCost == CppFace.bCost, "the cell face alike");
	}
	TArray<EHOLBuildingCategory> Categories;
	Categories.Add(EHOLBuildingCategory::Economic);
	Categories.Add(EHOLBuildingCategory::Military);
	Categories.Add(EHOLBuildingCategory::Technological);
	for (int A = 0; A < Categories.Num(); A++)
	{
		for (int B = 0; B < Categories.Num(); B++)
		{
			T.AssertTrue(HOLCommandGridGroups::IsCategorySelectedByCategory(Categories[A], Categories[B]) == HOLCommandGridRead::IsCategorySelectedByCategory(Categories[A], Categories[B]), "the F1 / F2 / F3 category alike");
		}
	}
}

// HeroesOfLiberty.CommandGridGroups.Distribution (sweep: PickEvenly)
void Test_CGGDistributionSweep(FUnitTest& T)
{
	int Mismatches = 0;
	FString First = "";
	for (int Mask = 0; Mask < 216; Mask++)
	{
		TArray<FHOLQueueChoiceView> Choices;
		int Rest = Mask;
		for (int I = 0; I < 3; I++)
		{
			int Code = Rest % 6;
			Rest = Rest / 6;
			Choices.Add(CGGChoice(Code % 3, 1 + Code / 3, I + 1));
		}
		for (int Last = 0; Last <= 3; Last++)
		{
			int LastKey = Last == 3 ? 99 : Last;
			int Script = HOLCommandGridGroups::PickEvenly(Choices, LastKey);
			int Cpp = HOLCommandGridRead::PickEvenly(Choices, LastKey);
			if (Script != Cpp)
			{
				Mismatches++;
				if (First == "")
				{
					First = f"mask {Mask} last {LastKey}: script {Script} C++ {Cpp}";
				}
			}
		}
	}
	T.AssertEquals(0, Mismatches, f"script and C++ distribute alike, first difference: {First}");
}

// HeroesOfLiberty.CommandGridGroups.HardValues (copied from the C++ tests' vectors, not read from the C++ library)
void Test_CGGHardValues(FUnitTest& T)
{
	// Cycle (HOLCommandGridTests.cpp "Cycle"; ported in HOLCommandGrid_Test.as:246-263).
	TArray<FHOLCycleBuildingView> Buildings;
	Buildings.Add(CGGCycle(1, 40));
	Buildings.Add(CGGCycle(0, 90));
	Buildings.Add(CGGCycle(2, 10));
	Buildings.Add(CGGCycle(1, 20));
	T.AssertEquals(1, HOLCommandGridGroups::NextInCycle(Buildings, 0), "first press: the centre");
	T.AssertEquals(3, HOLCommandGridGroups::NextInCycle(Buildings, 90), "then the house of the lowest key");
	T.AssertEquals(0, HOLCommandGridGroups::NextInCycle(Buildings, 20), "then the other house");
	T.AssertEquals(2, HOLCommandGridGroups::NextInCycle(Buildings, 40), "then the lumber camp");
	T.AssertEquals(1, HOLCommandGridGroups::NextInCycle(Buildings, 10), "after the last one: the centre again");
	T.AssertEquals(1, HOLCommandGridGroups::NextInCycle(Buildings, 55), "a key gone: the first one");
	T.AssertEquals(4, HOLCommandGridGroups::GetCycleRank(true, 3), "a buildable type: its slot + 1");
	T.AssertEquals(0, HOLCommandGridGroups::GetCycleRank(false, 3), "a type that cannot be built: 0");

	// Subgroups (HOLCommandGrid_Test.as:343-376).
	TArray<FHOLGroupMemberView> Members;
	Members.Add(CGGMember(n"MainCentre", true));
	Members.Add(CGGMember(n"House", false));
	Members.Add(CGGMember(n"LumberCamp", false));
	Members.Add(CGGMember(n"House", false));
	Members.Add(CGGMember(n"MainCentre", true));
	Members.Add(CGGMember(n"House", false));
	TArray<FHOLSubgroupView> Subgroups = HOLCommandGridGroups::SplitSubgroups(Members);
	T.AssertEquals(5, Subgroups.Num(), "All, centre, houses, camp, other centre");
	if (Subgroups.Num() == 5)
	{
		T.AssertEquals(6, Subgroups[0].Members.Num(), "All: every building");
		T.AssertEquals(3, Subgroups[2].Members.Num(), "the houses, all three");
		T.AssertEquals(1, Subgroups[4].Members.Num(), "the second centre alone");
	}
	TArray<int> Priority = HOLCommandGridGroups::GetCommandPriority(Members);
	T.AssertEquals(0, Priority[0], "priority: the first centre");
	T.AssertEquals(4, Priority[1], "priority: the second centre");
	T.AssertEquals(1, Priority[2], "priority: then the group's order");

	// Merge (HOLCommandGrid_Test.as:393-418): mill, forum, barracks.
	int Cells = HOLCommandGridGroups::GetNumCells();
	TArray<FHOLGroupMemberView> Mixed;
	Mixed.Add(CGGMember(n"Mill", false));
	Mixed.Add(CGGMember(n"MainCentre", true));
	Mixed.Add(CGGMember(n"Barracks", false));
	TArray<FHOLGridCommandView> PerBuilding;
	for (int I = 0; I < 3 * Cells; I++)
	{
		PerBuilding.Add(FHOLGridCommandView());
	}
	PerBuilding[0].Command = EHOLGridCommandKind::Produce;
	PerBuilding[0].TypeName = n"Technology";
	PerBuilding[1].Command = EHOLGridCommandKind::Produce;
	PerBuilding[1].TypeName = n"Technology";
	PerBuilding[Cells].Command = EHOLGridCommandKind::Produce;
	PerBuilding[Cells].TypeName = n"Peasant";
	PerBuilding[2 * Cells].Command = EHOLGridCommandKind::Produce;
	PerBuilding[2 * Cells].TypeName = n"Militia";
	PerBuilding[2 * Cells + 2].Command = EHOLGridCommandKind::Produce;
	PerBuilding[2 * Cells + 2].TypeName = n"Militia";
	TArray<FHOLGridCommandView> Merged = HOLCommandGridGroups::MergeCommands(PerBuilding, HOLCommandGridGroups::GetCommandPriority(Mixed));
	T.AssertEquals(Cells, Merged.Num(), "twelve cells");
	if (Merged.Num() == Cells)
	{
		T.AssertTrue(Merged[0].TypeName == n"Peasant", "A: the forum's Peasant");
		T.AssertTrue(Merged[1].TypeName == n"Technology", "Z: the mill's");
		T.AssertTrue(Merged[2].TypeName == n"Militia", "E: the barracks'");
		T.AssertTrue(Merged[3].Command == EHOLGridCommandKind::None, "R: empty");
	}

	// Distribution (HOLCommandGrid_Test.as:430-453).
	TArray<FHOLQueueChoiceView> A;
	A.Add(CGGChoice(3, 5, 1));
	A.Add(CGGChoice(1, 5, 2));
	A.Add(CGGChoice(2, 5, 3));
	T.AssertEquals(1, HOLCommandGridGroups::PickEvenly(A, 0), "the shortest queue");
	TArray<FHOLQueueChoiceView> B;
	B.Add(CGGChoice(2, 5, 9));
	B.Add(CGGChoice(2, 5, 4));
	T.AssertEquals(0, HOLCommandGridGroups::PickEvenly(B, 0), "equal queues, none picked yet: the first");
	T.AssertEquals(1, HOLCommandGridGroups::PickEvenly(B, 9), "equal queues: the one after the last picked");
	TArray<FHOLQueueChoiceView> C;
	C.Add(CGGChoice(5, 5, 1));
	C.Add(CGGChoice(5, 5, 2));
	T.AssertEquals(-1, HOLCommandGridGroups::PickEvenly(C, 0), "every queue full: none");
}
