// C9a (SAURON-1119ab): hol.Match.Pause / Resume, hol.Victory.Surrender, hol.Units.* (8), hol.Prod.RallySite are now script (HOLControllerCommandsC9a.as).
// Each command is registered once under its `hol.*` name; the arguments' plan, the "Usage" lines and the "Order: ..." journal texts (what the rpc_orders_* scenarios read) are tested here.
// RED to see (once, then put back): in HOLControllerCommandsC9a.as Queued(), return "; queued" instead of ", queued" -> Test_ControllerCommandsC9aOrderTexts fails on "attack-move queued suffix".

TArray<FString> C9aArgs(const FString& A = "", const FString& B = "", const FString& C = "", const FString& D = "", const FString& E = "", const FString& F = "")
{
	TArray<FString> Result;
	if (!A.IsEmpty()) Result.Add(A);
	if (!B.IsEmpty()) Result.Add(B);
	if (!C.IsEmpty()) Result.Add(C);
	if (!D.IsEmpty()) Result.Add(D);
	if (!E.IsEmpty()) Result.Add(E);
	if (!F.IsEmpty()) Result.Add(F);
	return Result;
}

void Test_ControllerCommandsC9aEachNameOnceAndKnown(FUnitTest& T)
{
	TArray<FString> Names = HOLControllerCommandsC9a::Names();
	T.AssertEquals(12, Names.Num(), "12 console commands ported from the C++ controller by C9a");
	TArray<FString> Seen;
	for (int I = 0; I < Names.Num(); I++)
	{
		FString Name = Names[I];
		T.AssertTrue(Name.StartsWith("hol."), f"{Name}: a hol.* command");
		T.AssertFalse(Seen.Contains(Name), f"{Name}: registered once");
		Seen.Add(Name);
		FHOLConsolePlanC9a Plan = HOLControllerCommandsC9a::PlanOf(Name, C9aArgs("1", "2", "3", "4"));
		T.AssertTrue(Plan.bValid, f"{Name}: known to the planner");
		T.AssertFalse(HOLControllerCommandsC2::Names().Contains(Name), f"{Name}: not also a C2 command");
		T.AssertFalse(HOLControllerCommandsC2b::Names().Contains(Name), f"{Name}: not also a C2b command");
	}
	FHOLConsolePlanC9a Unknown = HOLControllerCommandsC9a::PlanOf("hol.Nothing.Here", C9aArgs());
	T.AssertFalse(Unknown.bValid, "an unknown name plans nothing");
	T.AssertTrue(Unknown.Usage.IsEmpty(), "an unknown name logs nothing");
}

void Test_ControllerCommandsC9aArguments(FUnitTest& T)
{
	FHOLConsolePlanC9a Move = HOLControllerCommandsC9a::PlanOf("hol.Units.AttackMove", C9aArgs("1500", "-250.5", "SHIFT"));
	T.AssertTrue(Move.bValid, "AttackMove with a target is valid");
	T.AssertTrue(Math::IsNearlyEqual(Move.X1, 1500.0), "AttackMove X 1500");
	T.AssertTrue(Math::IsNearlyEqual(Move.Y1, -250.5), "AttackMove Y -250.5");
	T.AssertTrue(Move.bShift, "shift is read in any case");
	// The C++ refused on Args.Num() < 2 only: ONE word is refused, but two words (1 shift) are accepted (Y = Atod("shift") = 0, shift is looked for from index 2, so none).
	FHOLConsolePlanC9a MoveShortShift = HOLControllerCommandsC9a::PlanOf("hol.Units.AttackMove", C9aArgs("1"));
	T.AssertFalse(MoveShortShift.bValid, "AttackMove with one number is refused");
	T.AssertEquals("Usage: hol.Units.AttackMove <X> <Y> [shift]", MoveShortShift.Usage, "AttackMove usage line");
	FHOLConsolePlanC9a MoveTwoWords = HOLControllerCommandsC9a::PlanOf("hol.Units.AttackMove", C9aArgs("1", "shift"));
	T.AssertTrue(MoveTwoWords.bValid, "AttackMove with two words is accepted, as the C++ (it only counted the words)");
	T.AssertFalse(MoveTwoWords.bShift, "AttackMove: the word shift in the place of Y is no flag");
	// The flag is looked for from index 2: a word 'shift' in the place of a number does not count.
	T.AssertFalse(HOLControllerCommandsC9a::PlanOf("hol.Units.AttackMove", C9aArgs("shift", "2")).bShift, "AttackMove: shift before index 2 is no flag");

	FHOLConsolePlanC9a Line = HOLControllerCommandsC9a::PlanOf("hol.Units.OrderLine", C9aArgs("0", "0", "300", "400", "ctrl", "shift"));
	T.AssertTrue(Line.bWidth, "OrderLine ctrl is the width");
	T.AssertTrue(Line.bShift, "OrderLine shift is queued");
	T.AssertTrue(Math::IsNearlyEqual(Line.Y2, 400.0), "OrderLine Y2 400");
	FHOLConsolePlanC9a LineOnlyCtrl = HOLControllerCommandsC9a::PlanOf("hol.Units.OrderLine", C9aArgs("0", "0", "300", "400", "ctrl"));
	T.AssertFalse(LineOnlyCtrl.bShift, "OrderLine without shift is not queued");
	T.AssertEquals("Usage: hol.Units.OrderLine <X1> <Y1> <X2> <Y2> [ctrl] [shift]", HOLControllerCommandsC9a::PlanOf("hol.Units.OrderLine", C9aArgs("0", "0", "300")).Usage, "OrderLine usage line");

	// GatherOrder [Dx Dy] [shift]: two words none of which is shift are the offset.
	T.AssertTrue(HOLControllerCommandsC9a::GatherHasOffset(C9aArgs("100", "-50")), "100 -50 is an offset");
	T.AssertFalse(HOLControllerCommandsC9a::GatherHasOffset(C9aArgs("shift")), "shift alone is no offset");
	T.AssertFalse(HOLControllerCommandsC9a::GatherHasOffset(C9aArgs("100", "shift")), "100 shift is no offset");
	T.AssertFalse(HOLControllerCommandsC9a::GatherHasOffset(C9aArgs()), "no argument is no offset");
	FHOLConsolePlanC9a Gather = HOLControllerCommandsC9a::PlanOf("hol.Units.GatherOrder", C9aArgs("100", "-50", "shift"));
	T.AssertTrue(Math::IsNearlyEqual(Gather.X1, 100.0), "Gather Dx 100");
	T.AssertTrue(Math::IsNearlyEqual(Gather.Y1, -50.0), "Gather Dy -50");
	T.AssertTrue(Gather.bShift, "Gather shift");

	T.AssertTrue(HOLControllerCommandsC9a::PlanOf("hol.Units.DropOff", C9aArgs("shift")).bShift, "DropOff shift");
	T.AssertFalse(HOLControllerCommandsC9a::PlanOf("hol.Units.Build", C9aArgs()).bShift, "Build without shift");
	T.AssertTrue(HOLControllerCommandsC9a::PlanOf("hol.Prod.RallySite", C9aArgs("Shift")).bShift, "RallySite shift");
	T.AssertEquals("Barracks", HOLControllerCommandsC9a::PlanOf("hol.Units.Garrison", C9aArgs("Barracks")).Type, "Garrison building type");
	T.AssertEquals("", HOLControllerCommandsC9a::PlanOf("hol.Units.UngarrisonAll", C9aArgs()).Type, "UngarrisonAll any type");
}

void Test_ControllerCommandsC9aOrderTexts(FUnitTest& T)
{
	T.AssertEquals("Order: 3 units attack-move to (1500, -250)", HOLControllerCommandsC9a::AttackMoveText(3, 1500.0, -250.0, false), "attack-move line");
	T.AssertEquals("Order: 0 units attack-move to (10, 20), queued", HOLControllerCommandsC9a::AttackMoveText(0, 10.0, 20.0, true), "attack-move queued suffix");
	T.AssertEquals("Order: 5 units in line from (0, 0) to (300, 400), its width, queued", HOLControllerCommandsC9a::OrderLineText(5, 0.0, 0.0, 300.0, 400.0, true, true), "line with width and queue");
	T.AssertEquals("Order: 5 units in line from (0, 0) to (300, 400)", HOLControllerCommandsC9a::OrderLineText(5, 0.0, 0.0, 300.0, 400.0, false, false), "plain line");
	T.AssertEquals("Order: 2 units gather source 7 (Tree) at (120, -80), queued", HOLControllerCommandsC9a::GatherText(2, 7, "Tree", 120.0, -80.0, true), "gather line");
	T.AssertEquals("Order: rally point of MainCentre at (5, 6) on site House, a step added", HOLControllerCommandsC9a::RallySiteText("MainCentre", 5.0, 6.0, "House", true), "rally site line");
	T.AssertEquals("Order: rally point of MainCentre at (5, 6) on site House", HOLControllerCommandsC9a::RallySiteText("MainCentre", 5.0, 6.0, "House", false), "rally site line without shift");
}
