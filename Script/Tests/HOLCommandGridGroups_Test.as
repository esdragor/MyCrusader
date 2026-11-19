// MIG-C-CommandGrid, lot G2b (HALDIR-1119y), trimmed by L3b (HALDIR-1119cd): the command grid's cycle, subgroups, merge and distribution (script rule HOLCommandGridGroups).
// The sweeps against the *Native bodies are gone (one source, D836); what stays is Test_CGGHardValues (vectors copied from the C++ tests) and the cell face against the C++ one.
// To SEE RED (rouge a voir), in HOLCommandGridGroups.as: in PickEvenly change `(Last + Step) % Choices.Num()` to `(Last + Step + 1) % Choices.Num()` (Test_CGGHardValues goes red),
// or in SortsBefore change `Buildings[A].Rank < Buildings[B].Rank` to `>`, or the alone name n"MainCentre".

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

// HeroesOfLiberty.CommandGridGroups.FaceAndCategory (the cell face against the C++ one that stays; F1 / F2 / F3 in clear)
void Test_CGGFaceAndCategory(FUnitTest& T)
{
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
			bool bSelected = HOLCommandGridGroups::IsCategorySelectedByCategory(Categories[A], Categories[B]);
			T.AssertTrue(bSelected == (A == B), "a category key (F1 / F2 / F3) takes only its own category");
		}
	}
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
