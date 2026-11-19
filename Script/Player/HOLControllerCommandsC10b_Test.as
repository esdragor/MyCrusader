// C10b (SAURON-1119ca): hol.Select.OrderSource / ClickSource / Heal / Attack / OrderSourceNear, hol.Units.Order / OrderNear, hol.Build.PlaceNear / PlaceAtSource are now script (HOLControllerCommandsC10b.as).
// Each command is registered once; the arguments (slot=<S>, enemy, shift, all), the Usage / Warning / Display texts (the scenarios and quick_check read "Order: N units to (", "Order: N units attack #", "Right click on source N") are tested here.
// RED to see (once, then put back): in HOLControllerCommandsC10b.as UnitsOrderText, write "units at (" instead of "units to (" -> Test_ControllerCommandsC10bTexts fails on "Order: 3 units to (100, -200)".

TArray<FString> C10bArgs(const FString& A = "", const FString& B = "", const FString& C = "", const FString& D = "")
{
	TArray<FString> Result;
	if (!A.IsEmpty()) Result.Add(A);
	if (!B.IsEmpty()) Result.Add(B);
	if (!C.IsEmpty()) Result.Add(C);
	if (!D.IsEmpty()) Result.Add(D);
	return Result;
}

void Test_ControllerCommandsC10bEachNameOnceAndKnown(FUnitTest& T)
{
	TArray<FString> Names = HOLControllerCommandsC10b::Names();
	T.AssertEquals(9, Names.Num(), "9 console commands ported from the C++ controller by C10b");
	TArray<FString> Seen;
	for (int I = 0; I < Names.Num(); I++)
	{
		FString Name = Names[I];
		T.AssertFalse(Seen.Contains(Name), f"{Name}: registered once");
		Seen.Add(Name);
		T.AssertTrue(HOLControllerCommandsC10b::PlanOf(Name, C10bArgs("1", "2", "3", "4")).bValid, f"{Name}: known to the planner");
		T.AssertFalse(HOLControllerCommandsC9a::Names().Contains(Name), f"{Name}: not also a C9a command");
		T.AssertFalse(HOLControllerCommandsC9b::Names().Contains(Name), f"{Name}: not also a C9b command");
		T.AssertFalse(HOLControllerCommandsC10a::Names().Contains(Name), f"{Name}: not also a C10a command");
		T.AssertFalse(HOLControllerCommandsC2b::Names().Contains(Name), f"{Name}: not also a C2b command");
	}
	FHOLConsolePlanC10b Unknown = HOLControllerCommandsC10b::PlanOf("hol.Select.Nothing", C10bArgs("1", "2", "3", "4"));
	T.AssertFalse(Unknown.bValid, "an unknown name plans nothing");
	T.AssertTrue(Unknown.Usage.IsEmpty(), "an unknown name logs nothing");
}

void Test_ControllerCommandsC10bArguments(FUnitTest& T)
{
	// Usage lines (Warning), exactly the C++ ones; Units.Order(Near) and OrderSourceNear have none.
	T.AssertEquals("Usage: hol.Select.OrderSource <X> <Y> [shift]", HOLControllerCommandsC10b::PlanOf("hol.Select.OrderSource", C10bArgs("5")).Usage, "OrderSource usage");
	T.AssertEquals("Usage: hol.Select.ClickSource <X> <Y>", HOLControllerCommandsC10b::PlanOf("hol.Select.ClickSource", C10bArgs()).Usage, "ClickSource usage");
	T.AssertEquals("Usage: hol.Select.Heal <UnitId>", HOLControllerCommandsC10b::PlanOf("hol.Select.Heal", C10bArgs()).Usage, "Heal usage");
	T.AssertEquals("Usage: hol.Select.Attack <UnitId> [shift]", HOLControllerCommandsC10b::PlanOf("hol.Select.Attack", C10bArgs()).Usage, "Attack usage");
	T.AssertEquals("Usage: hol.Build.PlaceNear <Type> <Dx> <Dy> [enemy]", HOLControllerCommandsC10b::PlanOf("hol.Build.PlaceNear", C10bArgs("House", "1")).Usage, "PlaceNear usage");
	T.AssertEquals("Usage: hol.Build.PlaceAtSource <Type> <X> <Y>", HOLControllerCommandsC10b::PlanOf("hol.Build.PlaceAtSource", C10bArgs("LumberCamp", "1")).Usage, "PlaceAtSource usage");
	FHOLConsolePlanC10b ShortOrder = HOLControllerCommandsC10b::PlanOf("hol.Units.Order", C10bArgs("1"));
	T.AssertFalse(ShortOrder.bValid, "Units.Order with one number is ignored");
	T.AssertTrue(ShortOrder.Usage.IsEmpty(), "Units.Order with one number logs nothing");
	T.AssertTrue(HOLControllerCommandsC10b::PlanOf("hol.Units.OrderNear", C10bArgs("1")).Usage.IsEmpty(), "Units.OrderNear with one number logs nothing");
	T.AssertTrue(HOLControllerCommandsC10b::PlanOf("hol.Select.OrderSourceNear", C10bArgs()).bValid, "OrderSourceNear without offset is valid");
	T.AssertFalse(HOLControllerCommandsC10b::PlanOf("hol.Select.OrderSourceNear", C10bArgs()).bHasOffset, "OrderSourceNear without offset has none");

	// Sources: shift is the third word of OrderSource only, case sensitive.
	FHOLConsolePlanC10b Source = HOLControllerCommandsC10b::PlanOf("hol.Select.OrderSource", C10bArgs("-300.5", "1200", "shift"));
	T.AssertTrue(Source.bShift, "OrderSource shift");
	T.AssertEquals(-300.5, Source.X1, "OrderSource X");
	T.AssertEquals(1200.0, Source.Y1, "OrderSource Y");
	// The C++ compared `Args[2] == TEXT("shift")` on an FString, whose == is case INSENSITIVE in Unreal (the script == is the same operator): Shift counted there too.
	T.AssertTrue(HOLControllerCommandsC10b::PlanOf("hol.Select.OrderSource", C10bArgs("1", "2", "Shift")).bShift, "OrderSource `shift` is case insensitive as in the C++ (FString ==)");
	T.AssertFalse(HOLControllerCommandsC10b::PlanOf("hol.Select.ClickSource", C10bArgs("1", "2", "shift")).bShift, "ClickSource has no shift");

	// Heal / Attack: an identifier, or slot=<S>; only Attack queues, and any case.
	FHOLConsolePlanC10b BySlot = HOLControllerCommandsC10b::PlanOf("hol.Select.Attack", C10bArgs("slot=1", "SHIFT"));
	T.AssertTrue(BySlot.bBySlot, "Attack slot=1 is by slot");
	T.AssertEquals(1, BySlot.Slot, "Attack slot number");
	T.AssertTrue(BySlot.bQueue, "Attack queues on SHIFT");
	FHOLConsolePlanC10b ById = HOLControllerCommandsC10b::PlanOf("hol.Select.Heal", C10bArgs("42", "shift"));
	T.AssertFalse(ById.bBySlot, "Heal 42 is by identifier");
	T.AssertEquals(42, ById.TargetId, "Heal target");
	T.AssertFalse(ById.bQueue, "Heal never queues");

	// Units.Order(Near): all is the third word, exact.
	FHOLConsolePlanC10b All = HOLControllerCommandsC10b::PlanOf("hol.Units.OrderNear", C10bArgs("-500", "250", "all"));
	T.AssertTrue(All.bValid && All.bAll, "OrderNear all");
	T.AssertEquals(-500.0, All.X1, "OrderNear Dx");
	T.AssertEquals(250.0, All.Y1, "OrderNear Dy");
	T.AssertTrue(HOLControllerCommandsC10b::PlanOf("hol.Units.Order", C10bArgs("1", "2", "All")).bAll, "Order `all` is case insensitive as in the C++ (FString ==)");

	// Placement: type then two numbers; enemy is the fourth word of PlaceNear.
	FHOLConsolePlanC10b Near = HOLControllerCommandsC10b::PlanOf("hol.Build.PlaceNear", C10bArgs("House", "800", "-400", "enemy"));
	T.AssertEquals("House", Near.TypeName, "PlaceNear type");
	T.AssertEquals(800.0, Near.X1, "PlaceNear Dx");
	T.AssertEquals(-400.0, Near.Y1, "PlaceNear Dy");
	T.AssertTrue(Near.bEnemy, "PlaceNear enemy");
	T.AssertFalse(HOLControllerCommandsC10b::PlanOf("hol.Build.PlaceNear", C10bArgs("House", "800", "-400")).bEnemy, "PlaceNear without enemy");
	FHOLConsolePlanC10b AtSource = HOLControllerCommandsC10b::PlanOf("hol.Build.PlaceAtSource", C10bArgs("LumberCamp", "-9000", "300"));
	T.AssertEquals("LumberCamp", AtSource.TypeName, "PlaceAtSource type");
	T.AssertEquals(-9000.0, AtSource.X1, "PlaceAtSource X");
}

void Test_ControllerCommandsC10bTexts(FUnitTest& T)
{
	// The lines the scenarios and quick_check read (waitlog "Order: ", "Right click on source (\d+)", "Order: N units attack #").
	T.AssertEquals("Order: 3 units to (100, -200)", HOLControllerCommandsC10b::UnitsOrderText(3, 100.0, -200.0, false), "Units.Order line");
	T.AssertEquals("Order: 6 units to (-13300, 0), other players' too", HOLControllerCommandsC10b::UnitsOrderText(6, -13300.2, 0.0, true), "Units.Order all line, rounded to whole cm");
	T.AssertEquals("Order: 4 units attack #7", HOLControllerCommandsC10b::AttackText(4, 7, false), "Attack line");
	T.AssertEquals("Order: 3 units attack #12, queued", HOLControllerCommandsC10b::AttackText(3, 12, true), "Attack queued line");
	T.AssertEquals("Order: 1 units heal #9", HOLControllerCommandsC10b::HealText(1, 9), "Heal line");
	T.AssertEquals("Right click on source 5 at (0.500, 0.050), Shift", HOLControllerCommandsC10b::SourceClickText(true, 5, 0.5, 0.05, true), "OrderSource line with Shift");
	T.AssertEquals("Right click on source 5 at (0.500, 0.050)", HOLControllerCommandsC10b::SourceClickText(true, 5, 0.5, 0.05, false), "OrderSource line");
	T.AssertEquals("Click on source 12 at (0.123, 0.987)", HOLControllerCommandsC10b::SourceClickText(false, 12, 0.1234, 0.9871, false), "ClickSource line");
	T.AssertEquals("hol.Select.OrderSource: no standing source", HOLControllerCommandsC10b::NoSourceText("hol.Select.OrderSource"), "no standing source Warning");
	T.AssertEquals("hol.Select.ClickSource: the source is not on the screen", HOLControllerCommandsC10b::SourceOffScreenText("hol.Select.ClickSource"), "source off screen Warning");
	T.AssertEquals("hol.Build.PlaceNear: no own main centre known here", HOLControllerCommandsC10b::NoCentreText(false), "PlaceNear own Warning");
	T.AssertEquals("hol.Build.PlaceNear: no enemy main centre known here", HOLControllerCommandsC10b::NoCentreText(true), "PlaceNear enemy Warning");
	T.AssertEquals("Build on source 3 at (-8100, 4200)", HOLControllerCommandsC10b::BuildOnSourceText(3, -8100.0, 4200.4), "PlaceAtSource line");
}
