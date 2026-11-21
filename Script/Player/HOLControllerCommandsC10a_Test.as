// C10a (SAURON-1119ba): hol.Select.Click / ClickUnit / Box / BoxGround / OrderGround / OrderLineGround / ClickGround / OrderNear are now script (HOLControllerCommandsC10a.as).
// Each command is registered once; the arguments, the Usage / Warning / Display texts (the scenarios and quick_check read them) and the box of four corners are tested here.
// RED to see (once, then put back): in HOLControllerCommandsC10a.as Fixed3, pad to 2 digits instead of 3 -> Test_ControllerCommandsC10aTexts fails on "0.050 keeps its zeros".

TArray<FString> C10aArgs(const FString& A = "", const FString& B = "", const FString& C = "", const FString& D = "", const FString& E = "", const FString& F = "")
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

void Test_ControllerCommandsC10aEachNameOnceAndKnown(FUnitTest& T)
{
	TArray<FString> Names = HOLControllerCommandsC10a::Names();
	T.AssertEquals(8, Names.Num(), "8 console commands ported from the C++ controller by C10a");
	TArray<FString> Seen;
	for (int I = 0; I < Names.Num(); I++)
	{
		FString Name = Names[I];
		T.AssertFalse(Seen.Contains(Name), f"{Name}: registered once");
		Seen.Add(Name);
		T.AssertTrue(HOLControllerCommandsC10a::PlanOf(Name, C10aArgs("1", "2", "3", "4")).bValid, f"{Name}: known to the planner");
		T.AssertFalse(HOLControllerCommandsC9a::Names().Contains(Name), f"{Name}: not also a C9a command");
		T.AssertFalse(HOLControllerCommandsC9b::Names().Contains(Name), f"{Name}: not also a C9b command");
		T.AssertFalse(HOLControllerCommandsC2b::Names().Contains(Name), f"{Name}: not also a C2b command");
	}
	FHOLConsolePlanC10a Unknown = HOLControllerCommandsC10a::PlanOf("hol.Select.Nothing", C10aArgs("1", "2", "3", "4"));
	T.AssertFalse(Unknown.bValid, "an unknown name plans nothing");
	T.AssertTrue(Unknown.Usage.IsEmpty(), "an unknown name logs nothing");
}

void Test_ControllerCommandsC10aArguments(FUnitTest& T)
{
	// Usage lines (Warning), exactly the C++ ones.
	T.AssertEquals("Usage: hol.Select.Click <X> <Y> [add]", HOLControllerCommandsC10a::PlanOf("hol.Select.Click", C10aArgs("0.5")).Usage, "Click usage");
	T.AssertEquals("Usage: hol.Select.ClickGround <X> <Y> [add]", HOLControllerCommandsC10a::PlanOf("hol.Select.ClickGround", C10aArgs()).Usage, "ClickGround usage");
	T.AssertEquals("Usage: hol.Select.Box <X1> <Y1> <X2> <Y2> [add]", HOLControllerCommandsC10a::PlanOf("hol.Select.Box", C10aArgs("1", "2", "3")).Usage, "Box usage");
	T.AssertEquals("Usage: hol.Select.BoxGround <X1> <Y1> <X2> <Y2> [add]", HOLControllerCommandsC10a::PlanOf("hol.Select.BoxGround", C10aArgs("1", "2", "3")).Usage, "BoxGround usage");
	T.AssertEquals("Usage: hol.Select.OrderGround <X> <Y> [shift]", HOLControllerCommandsC10a::PlanOf("hol.Select.OrderGround", C10aArgs("1")).Usage, "OrderGround usage");
	T.AssertEquals("Usage: hol.Select.OrderLineGround <X1> <Y1> <X2> <Y2>", HOLControllerCommandsC10a::PlanOf("hol.Select.OrderLineGround", C10aArgs("1", "2", "3")).Usage, "OrderLineGround usage");
	T.AssertFalse(HOLControllerCommandsC10a::PlanOf("hol.Select.Click", C10aArgs("0.5")).bValid, "Click without Y is refused");

	// Click: add / double anywhere after the point.
	FHOLConsolePlanC10a Click = HOLControllerCommandsC10a::PlanOf("hol.Select.Click", C10aArgs("0.25", "0.75", "double", "add"));
	T.AssertTrue(Click.bValid, "Click is valid");
	T.AssertTrue(Click.bAdd && Click.bDouble, "Click add and double are read in any order");
	T.AssertEquals(0.25, Click.X1, "Click X");
	T.AssertEquals(0.75, Click.Y1, "Click Y");
	// Box / ClickGround / OrderGround take their flag at a fixed place.
	T.AssertTrue(HOLControllerCommandsC10a::PlanOf("hol.Select.Box", C10aArgs("0", "0", "1", "1", "add")).bAdd, "Box add is the fifth word");
	// The C++ compared `Args[4] == TEXT("add")` on an FString, whose == is case INSENSITIVE in Unreal (the script == is the same operator): Add counted there too.
	T.AssertTrue(HOLControllerCommandsC10a::PlanOf("hol.Select.Box", C10aArgs("0", "0", "1", "1", "Add")).bAdd, "Box `add` is case insensitive as in the C++ (FString ==)");
	T.AssertTrue(HOLControllerCommandsC10a::PlanOf("hol.Select.ClickGround", C10aArgs("100", "200", "add")).bAdd, "ClickGround add is the third word");
	T.AssertTrue(HOLControllerCommandsC10a::PlanOf("hol.Select.OrderGround", C10aArgs("100", "200", "shift")).bShift, "OrderGround shift is the third word");
	// OrderLineGround flags only count after the four numbers, any case.
	FHOLConsolePlanC10a Line = HOLControllerCommandsC10a::PlanOf("hol.Select.OrderLineGround", C10aArgs("1", "2", "3", "4", "CTRL", "Shift"));
	T.AssertTrue(Line.bCtrl && Line.bShift, "OrderLineGround ctrl and shift, any case");
	FHOLConsolePlanC10a EarlyFlag = HOLControllerCommandsC10a::PlanOf("hol.Select.OrderLineGround", C10aArgs("ctrl", "2", "3", "4"));
	T.AssertFalse(EarlyFlag.bCtrl, "a flag among the four numbers is not a flag");
	// ClickUnit: unit number (negative from the last), flags, slot=.
	FHOLConsolePlanC10a Unit = HOLControllerCommandsC10a::PlanOf("hol.Select.ClickUnit", C10aArgs("-1", "order", "enemy", "slot=3"));
	T.AssertTrue(Unit.bValid, "ClickUnit is always valid");
	T.AssertEquals(-1, Unit.Wanted, "ClickUnit counts from the last");
	T.AssertTrue(Unit.bOrder && Unit.bEnemy && !Unit.bAlly, "ClickUnit order and enemy");
	T.AssertEquals(3, Unit.Slot, "ClickUnit slot=3");
	FHOLConsolePlanC10a NoArg = HOLControllerCommandsC10a::PlanOf("hol.Select.ClickUnit", C10aArgs());
	T.AssertEquals(0, NoArg.Wanted, "ClickUnit without argument is unit 0");
	T.AssertEquals(-1, NoArg.Slot, "ClickUnit without slot= is no slot");
	// OrderNear: the point is Dx Dy [enemy]; without two numbers there is no point.
	FHOLConsolePlanC10a Near = HOLControllerCommandsC10a::PlanOf("hol.Select.OrderNear", C10aArgs("-300", "1200", "enemy"));
	T.AssertTrue(Near.bHasPoint && Near.bEnemy, "OrderNear Dx Dy enemy");
	T.AssertEquals(-300.0, Near.X1, "OrderNear Dx");
	T.AssertFalse(HOLControllerCommandsC10a::PlanOf("hol.Select.OrderNear", C10aArgs("5")).bHasPoint, "OrderNear with one number has no point");
	T.AssertTrue(HOLControllerCommandsC10a::PlanOf("hol.Select.OrderNear", C10aArgs("5", "6", "Enemy")).bEnemy, "OrderNear `enemy` is case insensitive as in the C++ (FString ==)");
}

void Test_ControllerCommandsC10aTexts(FUnitTest& T)
{
	T.AssertEquals("0.500", HOLControllerCommandsC10a::Fixed3(0.5), "0.5 prints three decimals");
	T.AssertEquals("0.050", HOLControllerCommandsC10a::Fixed3(0.05), "0.050 keeps its zeros");
	T.AssertEquals("0.123", HOLControllerCommandsC10a::Fixed3(0.1234), "rounded down");
	T.AssertEquals("0.124", HOLControllerCommandsC10a::Fixed3(0.1236), "rounded up");
	T.AssertEquals("1.000", HOLControllerCommandsC10a::Fixed3(1.0), "one");
	T.AssertEquals("-0.250", HOLControllerCommandsC10a::Fixed3(-0.25), "a fraction outside the screen is negative");
	T.AssertEquals("Click on the ground at (0.250, 0.750)", HOLControllerCommandsC10a::GroundClickText(0.25, 0.75), "ClickGround line");
	T.AssertEquals("Right click on the ground at (0.250, 0.750)", HOLControllerCommandsC10a::RightClickGroundText(0.25, 0.75, false), "OrderGround line");
	T.AssertEquals("Right click on the ground at (0.250, 0.750), Shift", HOLControllerCommandsC10a::RightClickGroundText(0.25, 0.75, true), "OrderGround line with Shift");
	T.AssertEquals("Right drag on the ground from (0.100, 0.200) to (0.300, 0.400)", HOLControllerCommandsC10a::RightDragText(0.1, 0.2, 0.3, 0.4), "OrderLineGround line");
	T.AssertEquals("Click on unit #12 at (0.500, 0.625)", HOLControllerCommandsC10a::UnitClickText(false, 12, 0.5, 0.625), "ClickUnit line");
	T.AssertEquals("Right click on unit #12 at (0.500, 0.625)", HOLControllerCommandsC10a::UnitClickText(true, 12, 0.5, 0.625), "ClickUnit order line");
	T.AssertEquals("hol.Select.ClickUnit: no own unit 4 on the screen", HOLControllerCommandsC10a::UnitMissText(-1, false, false, 4), "ClickUnit miss, own");
	T.AssertEquals("hol.Select.ClickUnit: no enemy unit -2 on the screen", HOLControllerCommandsC10a::UnitMissText(-1, false, true, -2), "ClickUnit miss, enemy");
	T.AssertEquals("hol.Select.ClickUnit: no ally unit 0 on the screen", HOLControllerCommandsC10a::UnitMissText(-1, true, false, 0), "ClickUnit miss, ally");
	T.AssertEquals("hol.Select.ClickUnit: no such unit 1 on the screen", HOLControllerCommandsC10a::UnitMissText(2, true, true, 1), "ClickUnit miss, slot wins");
	T.AssertEquals("Right click near the centre at (-300, 1200)", HOLControllerCommandsC10a::NearText(-300.0, 1200.0), "OrderNear line");
}

// A right click on a unit (D818 Q1 C): the path of hol.Select.ClickUnit ... order on an ALLY starts Follow for the other selected units; on an ENEMY it stays the attack
// (no follow, the ordinary order goes out); with slot= the camp is unknown, so the existing path is kept.
void Test_ControllerCommandsC10aRightClickOnAllyStartsFollow(FUnitTest& T)
{
	FHOLFollowBook Book;
	TArray<int> Selected;
	Selected.Add(1); Selected.Add(2); Selected.Add(9);
	const EHOLRightClickOrder Order = HOLControllerCommandsC10a::RightClickUnitOrder(Book, Selected, 9, false, -1);
	T.AssertTrue(Order == EHOLRightClickOrder::Follow, "an ally under the right click is Follow");
	T.AssertEquals(2, Book.UnitIds.Num(), "the two other selected units follow (not the target itself)");
	const int FirstIndex = HOLFollowOrders::IndexOf(Book, 1);
	T.AssertTrue(FirstIndex >= 0, "unit 1 is in the book");
	if (FirstIndex >= 0)
		T.AssertEquals(9, Book.TargetIds[FirstIndex], "unit 1 follows unit 9");
	T.AssertEquals(-1, HOLFollowOrders::IndexOf(Book, 9), "the target does not follow itself");
}

void Test_ControllerCommandsC10aRightClickOnEnemyKeepsAttack(FUnitTest& T)
{
	FHOLFollowBook Book;
	TArray<int> Selected;
	Selected.Add(1); Selected.Add(2);
	const EHOLRightClickOrder Order = HOLControllerCommandsC10a::RightClickUnitOrder(Book, Selected, 30, true, -1);
	T.AssertTrue(Order == EHOLRightClickOrder::Attack, "an enemy under the right click stays Attack");
	T.AssertEquals(0, Book.UnitIds.Num(), "no follow is started on an enemy");
}

void Test_ControllerCommandsC10aRightClickSlotKeepsOrdinaryPath(FUnitTest& T)
{
	FHOLFollowBook Book;
	TArray<int> Selected;
	Selected.Add(1); Selected.Add(2);
	const EHOLRightClickOrder Order = HOLControllerCommandsC10a::RightClickUnitOrder(Book, Selected, 30, false, 2);
	T.AssertTrue(Order == EHOLRightClickOrder::Move, "with slot= the camp is unknown: ordinary path");
	T.AssertEquals(0, Book.UnitIds.Num(), "no follow with slot=");
}

void Test_ControllerCommandsC10aBoxOfCorners(FUnitTest& T)
{
	// Projected corners of a ground rectangle come in any order (a camera looking along a diagonal swaps min and max).
	TArray<double> Xs;
	TArray<double> Ys;
	Xs.Add(0.6); Ys.Add(0.2);
	Xs.Add(0.2); Ys.Add(0.3);
	Xs.Add(0.7); Ys.Add(0.9);
	Xs.Add(0.3); Ys.Add(0.8);
	FHOLFractionBoxC10a Box = HOLControllerCommandsC10a::BoxOfCorners(Xs, Ys);
	T.AssertEquals(0.2, Box.MinX, "box min X");
	T.AssertEquals(0.7, Box.MaxX, "box max X");
	T.AssertEquals(0.2, Box.MinY, "box min Y");
	T.AssertEquals(0.9, Box.MaxY, "box max Y");
	T.AssertEquals("Box on the ground from (0.200, 0.200) to (0.700, 0.900)", HOLControllerCommandsC10a::BoxGroundText(Box), "BoxGround line");
}
