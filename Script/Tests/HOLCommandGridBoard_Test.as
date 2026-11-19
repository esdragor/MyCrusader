// MIG-CPP, step D.1 (GALADRIEL-1119ca): the script board's answers to the 6 small rules the C++ bridge now asks first (IsAloneInGroup, IsRepeatPress,
// GetCommandPriority, IsSelectedByCategory, Blocks, GetCycleRank). NOT COMPILED when written.
// There is no world in a unit test, so the C++ Ask* (UHOLCommandGridBoard::FindAny) cannot be reached from here: the tests make a board of the script
// class (UHOLCommandGridBoardScript, Script/Player/HOLCommandGridBoard.as) and call its Answer functions, the very ones the C++ Ask* call. Each test
// gives the values in clear AND compares with the C++ twin (HOLCommandGridRead = the read library on the HOLCommandGrid::*Native bodies).
// To SEE RED (rouge a voir): in Script/Player/HOLCommandGridGroups.as, IsAloneInGroup, change n"MainCentre" to n"House": Test_CGBIsAloneInGroup,
// Test_CGBGetCommandPriority (hard values) and the Test_CGG* agreement sweeps (script vs C++) go red. Or Keys.as Blocks: drop `QueueFull`: Test_CGBBlocks
// and the Test_CGKYAgreementFeedback sweep go red. Or Groups.as GetCycleRank: `BuildGridSlot + 1` to `BuildGridSlot`: Test_CGBGetCycleRank and Test_CGGHardValues.

UHOLCommandGridBoardScript CGBMakeBoard()
{
	return NewObject(GetTransientPackage(), UHOLCommandGridBoardScript);
}

FHOLGroupMemberView CGBMember(FName TypeName, bool bAlone)
{
	FHOLGroupMemberView Member;
	Member.TypeName = TypeName;
	Member.bAlone = bAlone;
	return Member;
}

FHOLFeedbackView CGBFeedback(EHOLCommandFeedbackKind Kind)
{
	FHOLFeedbackView Feedback;
	Feedback.Kind = Kind;
	return Feedback;
}

void Test_CGBIsAloneInGroup(FUnitTest& T)
{
	UHOLCommandGridBoardScript Board = CGBMakeBoard();
	T.AssertTrue(Board != nullptr, "the script board is born");
	bool bCentre = Board.IsAloneInGroupAnswer(n"MainCentre");
	bool bHouse = Board.IsAloneInGroupAnswer(n"House");
	bool bNone = Board.IsAloneInGroupAnswer(NAME_None);
	T.AssertTrue(bCentre, "the main centre is alone in its group");
	T.AssertTrue(!bHouse, "a house joins the others");
	T.AssertTrue(!bNone, "no name: not alone");
	T.AssertTrue(bCentre == HOLCommandGridRead::IsAloneInGroup(n"MainCentre"), "centre: the board and the C++ agree");
	T.AssertTrue(bHouse == HOLCommandGridRead::IsAloneInGroup(n"House"), "house: the board and the C++ agree");
}

void Test_CGBIsRepeatPress(FUnitTest& T)
{
	UHOLCommandGridBoardScript Board = CGBMakeBoard();
	TArray<int> Selected;
	Selected.Add(7);
	Selected.Add(3);
	Selected.Add(-5);
	TArray<int> SameAnyOrder;
	SameAnyOrder.Add(-5);
	SameAnyOrder.Add(7);
	SameAnyOrder.Add(3);
	TArray<int> OtherOne;
	OtherOne.Add(7);
	OtherOne.Add(3);
	OtherOne.Add(9);
	TArray<int> Shorter;
	Shorter.Add(7);
	Shorter.Add(3);
	TArray<int> Empty;
	bool bSame = Board.IsRepeatPressAnswer(Selected, SameAnyOrder, true);
	bool bOtherKey = Board.IsRepeatPressAnswer(Selected, SameAnyOrder, false);
	bool bOtherBuilding = Board.IsRepeatPressAnswer(Selected, OtherOne, true);
	bool bShorter = Board.IsRepeatPressAnswer(Selected, Shorter, true);
	bool bEmpty = Board.IsRepeatPressAnswer(Empty, Empty, true);
	T.AssertTrue(bSame, "same key, same buildings in another order: a repeat press");
	T.AssertTrue(!bOtherKey, "another key: not a repeat");
	T.AssertTrue(!bOtherBuilding, "one building replaced: not a repeat");
	T.AssertTrue(!bShorter, "a shorter selection: not a repeat");
	T.AssertTrue(!bEmpty, "an empty selection: not a repeat");
	T.AssertTrue(bSame == HOLCommandGridRead::IsRepeatPress(Selected, SameAnyOrder, true), "same: the board and the C++ agree");
	T.AssertTrue(bOtherBuilding == HOLCommandGridRead::IsRepeatPress(Selected, OtherOne, true), "other building: the board and the C++ agree");
}

void Test_CGBGetCommandPriority(FUnitTest& T)
{
	UHOLCommandGridBoardScript Board = CGBMakeBoard();
	TArray<FHOLGroupMemberView> Members;
	Members.Add(CGBMember(n"Mill", false));
	Members.Add(CGBMember(n"MainCentre", true));
	Members.Add(CGBMember(n"Barracks", false));
	Members.Add(CGBMember(n"MainCentre", true));
	TArray<int> Order = Board.GetCommandPriorityAnswer(Members);
	T.AssertEquals(4, Order.Num(), "every building has a place");
	if (Order.Num() == 4)
	{
		T.AssertEquals(1, Order[0], "first: the first centre");
		T.AssertEquals(3, Order[1], "then: the second centre");
		T.AssertEquals(0, Order[2], "then the others in the group's order: the mill");
		T.AssertEquals(2, Order[3], "then the barracks");
	}
	TArray<int> Native = HOLCommandGridRead::GetCommandPriority(Members);
	T.AssertEquals(Native.Num(), Order.Num(), "the board and the C++ give as many places");
	if (Native.Num() == Order.Num())
	{
		for (int I = 0; I < Order.Num(); I++)
		{
			T.AssertEquals(Native[I], Order[I], "the board and the C++ agree at place " + I);
		}
	}
	TArray<FHOLGroupMemberView> NoMember;
	TArray<int> OfNone = Board.GetCommandPriorityAnswer(NoMember);
	T.AssertEquals(0, OfNone.Num(), "no building: no order");
}

void Test_CGBIsSelectedByCategory(FUnitTest& T)
{
	UHOLCommandGridBoardScript Board = CGBMakeBoard();
	bool bSame = Board.IsSelectedByCategoryAnswer(EHOLBuildingCategory::Economic, EHOLBuildingCategory::Economic);
	bool bOther = Board.IsSelectedByCategoryAnswer(EHOLBuildingCategory::Military, EHOLBuildingCategory::Economic);
	bool bOtherWay = Board.IsSelectedByCategoryAnswer(EHOLBuildingCategory::Economic, EHOLBuildingCategory::Military);
	T.AssertTrue(bSame, "an Economic building is selected by the Economic key");
	T.AssertTrue(!bOther, "a Military building is not selected by the Economic key");
	T.AssertTrue(!bOtherWay, "an Economic building is not selected by the Military key");
	T.AssertTrue(bSame == HOLCommandGridRead::IsCategorySelectedByCategory(EHOLBuildingCategory::Economic, EHOLBuildingCategory::Economic), "same: the board and the C++ agree");
	T.AssertTrue(bOther == HOLCommandGridRead::IsCategorySelectedByCategory(EHOLBuildingCategory::Military, EHOLBuildingCategory::Economic), "other: the board and the C++ agree");
}

void Test_CGBBlocks(FUnitTest& T)
{
	UHOLCommandGridBoardScript Board = CGBMakeBoard();
	bool bNoResource = Board.BlocksAnswer(CGBFeedback(EHOLCommandFeedbackKind::NotEnoughResources));
	bool bQueueFull = Board.BlocksAnswer(CGBFeedback(EHOLCommandFeedbackKind::QueueFull));
	bool bPlacement = Board.BlocksAnswer(CGBFeedback(EHOLCommandFeedbackKind::PlacementRefused));
	bool bResearch = Board.BlocksAnswer(CGBFeedback(EHOLCommandFeedbackKind::ResearchRefused));
	bool bPopulation = Board.BlocksAnswer(CGBFeedback(EHOLCommandFeedbackKind::PopulationFull));
	bool bNone = Board.BlocksAnswer(CGBFeedback(EHOLCommandFeedbackKind::None));
	T.AssertTrue(bNoResource, "not enough resources stops the command");
	T.AssertTrue(bQueueFull, "a full queue stops the command");
	T.AssertTrue(bPlacement, "a refused placement stops the command");
	T.AssertTrue(bResearch, "a refused research stops the command");
	T.AssertTrue(!bPopulation, "a full population only waits: it does not block");
	T.AssertTrue(!bNone, "no feedback: nothing blocks");
	T.AssertTrue(bQueueFull == HOLCommandGridRead::Blocks(CGBFeedback(EHOLCommandFeedbackKind::QueueFull)), "queue full: the board and the C++ agree");
	T.AssertTrue(bPopulation == HOLCommandGridRead::Blocks(CGBFeedback(EHOLCommandFeedbackKind::PopulationFull)), "population: the board and the C++ agree");
}

void Test_CGBGetCycleRank(FUnitTest& T)
{
	UHOLCommandGridBoardScript Board = CGBMakeBoard();
	int Slot3 = Board.GetCycleRankAnswer(true, 3);
	int Slot0 = Board.GetCycleRankAnswer(true, 0);
	int NotBuildable = Board.GetCycleRankAnswer(false, 3);
	T.AssertEquals(4, Slot3, "a buildable type at slot 3: rank 4");
	T.AssertEquals(1, Slot0, "a buildable type at slot 0: rank 1 (after the centre, rank 0)");
	T.AssertEquals(0, NotBuildable, "a type that cannot be built (the main centre): rank 0");
}
