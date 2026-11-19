// MIG-CPP, step D.1 (GALADRIEL-1119ca): the script board's answers to the 6 small rules the C++ bridge now asks first (IsAloneInGroup, IsRepeatPress,
// GetCommandPriority, IsSelectedByCategory, Blocks, GetCycleRank). NOT COMPILED when written.
// There is no world in a unit test, so the C++ Ask* (UHOLCommandGridBoard::FindAny) cannot be reached from here, and a script cannot call the board's Answer
// overrides either (BlueprintNativeEvent, not ScriptCallable: compile error seen 2026-11-19). The tests call the rule each Answer forwards to in ONE line
// (Script/Player/HOLCommandGridBoard.as, family 4), with the values in clear (no C++ twin any more: one source, D836).
// The crossing C++ Ask* -> board itself is proved by the game checks (quick_check build / production / selection), not here.
// To SEE RED (rouge a voir): in Script/Player/HOLCommandGridGroups.as, IsAloneInGroup, change n"MainCentre" to n"House": Test_CGBIsAloneInGroup,
// Test_CGBGetCommandPriority (hard values) go red. Or Keys.as Blocks: drop `QueueFull`: Test_CGBBlocks and Test_CGKYHardValues go red. Or Groups.as GetCycleRank: `BuildGridSlot + 1` to `BuildGridSlot`: Test_CGBGetCycleRank and Test_CGGHardValues.

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
	bool bCentre = HOLCommandGridGroups::IsAloneInGroup(n"MainCentre");
	bool bHouse = HOLCommandGridGroups::IsAloneInGroup(n"House");
	bool bNone = HOLCommandGridGroups::IsAloneInGroup(NAME_None);
	T.AssertTrue(bCentre, "the main centre is alone in its group");
	T.AssertTrue(!bHouse, "a house joins the others");
	T.AssertTrue(!bNone, "no name: not alone");
}

void Test_CGBIsRepeatPress(FUnitTest& T)
{
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
	bool bSame = HOLCommandGridGroups::IsRepeatPress(Selected, SameAnyOrder, true);
	bool bOtherKey = HOLCommandGridGroups::IsRepeatPress(Selected, SameAnyOrder, false);
	bool bOtherBuilding = HOLCommandGridGroups::IsRepeatPress(Selected, OtherOne, true);
	bool bShorter = HOLCommandGridGroups::IsRepeatPress(Selected, Shorter, true);
	bool bEmpty = HOLCommandGridGroups::IsRepeatPress(Empty, Empty, true);
	T.AssertTrue(bSame, "same key, same buildings in another order: a repeat press");
	T.AssertTrue(!bOtherKey, "another key: not a repeat");
	T.AssertTrue(!bOtherBuilding, "one building replaced: not a repeat");
	T.AssertTrue(!bShorter, "a shorter selection: not a repeat");
	T.AssertTrue(!bEmpty, "an empty selection: not a repeat");
}

void Test_CGBGetCommandPriority(FUnitTest& T)
{
	TArray<FHOLGroupMemberView> Members;
	Members.Add(CGBMember(n"Mill", false));
	Members.Add(CGBMember(n"MainCentre", true));
	Members.Add(CGBMember(n"Barracks", false));
	Members.Add(CGBMember(n"MainCentre", true));
	TArray<int> Order = HOLCommandGridGroups::GetCommandPriority(Members);
	T.AssertEquals(4, Order.Num(), "every building has a place");
	if (Order.Num() == 4)
	{
		T.AssertEquals(1, Order[0], "first: the first centre");
		T.AssertEquals(3, Order[1], "then: the second centre");
		T.AssertEquals(0, Order[2], "then the others in the group's order: the mill");
		T.AssertEquals(2, Order[3], "then the barracks");
	}
	TArray<FHOLGroupMemberView> NoMember;
	TArray<int> OfNone = HOLCommandGridGroups::GetCommandPriority(NoMember);
	T.AssertEquals(0, OfNone.Num(), "no building: no order");
}

void Test_CGBIsSelectedByCategory(FUnitTest& T)
{
	bool bSame = HOLCommandGridGroups::IsCategorySelectedByCategory(EHOLBuildingCategory::Economic, EHOLBuildingCategory::Economic);
	bool bOther = HOLCommandGridGroups::IsCategorySelectedByCategory(EHOLBuildingCategory::Military, EHOLBuildingCategory::Economic);
	bool bOtherWay = HOLCommandGridGroups::IsCategorySelectedByCategory(EHOLBuildingCategory::Economic, EHOLBuildingCategory::Military);
	T.AssertTrue(bSame, "an Economic building is selected by the Economic key");
	T.AssertTrue(!bOther, "a Military building is not selected by the Economic key");
	T.AssertTrue(!bOtherWay, "an Economic building is not selected by the Military key");
}

void Test_CGBBlocks(FUnitTest& T)
{
	bool bNoResource = HOLCommandGridKeys::Blocks(CGBFeedback(EHOLCommandFeedbackKind::NotEnoughResources));
	bool bQueueFull = HOLCommandGridKeys::Blocks(CGBFeedback(EHOLCommandFeedbackKind::QueueFull));
	bool bPlacement = HOLCommandGridKeys::Blocks(CGBFeedback(EHOLCommandFeedbackKind::PlacementRefused));
	bool bResearch = HOLCommandGridKeys::Blocks(CGBFeedback(EHOLCommandFeedbackKind::ResearchRefused));
	bool bPopulation = HOLCommandGridKeys::Blocks(CGBFeedback(EHOLCommandFeedbackKind::PopulationFull));
	bool bNone = HOLCommandGridKeys::Blocks(CGBFeedback(EHOLCommandFeedbackKind::None));
	T.AssertTrue(bNoResource, "not enough resources stops the command");
	T.AssertTrue(bQueueFull, "a full queue stops the command");
	T.AssertTrue(bPlacement, "a refused placement stops the command");
	T.AssertTrue(bResearch, "a refused research stops the command");
	T.AssertTrue(!bPopulation, "a full population only waits: it does not block");
	T.AssertTrue(!bNone, "no feedback: nothing blocks");
}

void Test_CGBGetCycleRank(FUnitTest& T)
{
	int Slot3 = HOLCommandGridGroups::GetCycleRank(true, 3);
	int Slot0 = HOLCommandGridGroups::GetCycleRank(true, 0);
	int NotBuildable = HOLCommandGridGroups::GetCycleRank(false, 3);
	T.AssertEquals(4, Slot3, "a buildable type at slot 3: rank 4");
	T.AssertEquals(1, Slot0, "a buildable type at slot 0: rank 1 (after the centre, rank 0)");
	T.AssertEquals(0, NotBuildable, "a type that cannot be built (the main centre): rank 0");
}
