// C9b (SAURON-1119ab): hol.Select.Stop / Hold / NextIdle / Own / Type and hol.Build.CancelSelected are now script (HOLControllerCommandsC9b.as); hol.Select.Order / OrderLine were removed.
// Each command is registered once; the `last` decision of hol.Select.Type, its Usage line and the journal texts are tested here.
// RED to see (once, then put back): in HOLControllerCommandsC9b.as KeepLast, keep Ids[0] instead of the last -> Test_ControllerCommandsC9bTypeLast fails on "last keeps the newest id".

TArray<FString> C9bArgs(const FString& A = "", const FString& B = "")
{
	TArray<FString> Result;
	if (!A.IsEmpty()) Result.Add(A);
	if (!B.IsEmpty()) Result.Add(B);
	return Result;
}

void Test_ControllerCommandsC9bEachNameOnceAndKnown(FUnitTest& T)
{
	TArray<FString> Names = HOLControllerCommandsC9b::Names();
	T.AssertEquals(6, Names.Num(), "6 console commands ported from the C++ controller by C9b");
	TArray<FString> Seen;
	for (int I = 0; I < Names.Num(); I++)
	{
		FString Name = Names[I];
		T.AssertFalse(Seen.Contains(Name), f"{Name}: registered once");
		Seen.Add(Name);
		T.AssertTrue(HOLControllerCommandsC9b::PlanOf(Name, C9bArgs("Monk")).bValid, f"{Name}: known to the planner");
		T.AssertFalse(HOLControllerCommandsC9a::Names().Contains(Name), f"{Name}: not also a C9a command");
		T.AssertFalse(HOLControllerCommandsC2b::Names().Contains(Name), f"{Name}: not also a C2b command");
	}
	T.AssertFalse(Names.Contains("hol.Select.Order"), "hol.Select.Order is removed");
	T.AssertFalse(Names.Contains("hol.Select.OrderLine"), "hol.Select.OrderLine is removed");
	FHOLConsolePlanC9b Unknown = HOLControllerCommandsC9b::PlanOf("hol.Select.Order", C9bArgs("0.5", "0.5"));
	T.AssertFalse(Unknown.bValid, "a removed name plans nothing");
	T.AssertTrue(Unknown.Usage.IsEmpty(), "a removed name logs nothing");
}

void Test_ControllerCommandsC9bTypeLast(FUnitTest& T)
{
	FHOLConsolePlanC9b NoArg = HOLControllerCommandsC9b::PlanOf("hol.Select.Type", C9bArgs());
	T.AssertFalse(NoArg.bValid, "Select.Type without a type is refused");
	T.AssertEquals("Usage: hol.Select.Type <Type>", NoArg.Usage, "Select.Type usage line");
	FHOLConsolePlanC9b Plain = HOLControllerCommandsC9b::PlanOf("hol.Select.Type", C9bArgs("Monk"));
	T.AssertEquals("Monk", Plain.Type, "the type is the first word");
	T.AssertFalse(Plain.bLast, "no `last` word");
	T.AssertTrue(HOLControllerCommandsC9b::PlanOf("hol.Select.Type", C9bArgs("Monk", "last")).bLast, "`last` word");
	// The C++ compared `Args[1] == TEXT("last")` on an FString, whose == is case INSENSITIVE in Unreal: LAST counted as last there too.
	T.AssertTrue(HOLControllerCommandsC9b::PlanOf("hol.Select.Type", C9bArgs("Monk", "LAST")).bLast, "`last` is case insensitive as in the C++ (FString ==)");
	T.AssertFalse(HOLControllerCommandsC9b::PlanOf("hol.Select.Type", C9bArgs("Monk", "lastly")).bLast, "another word is no `last`");

	TArray<int> Ids;
	Ids.Add(4); Ids.Add(9); Ids.Add(12);
	TArray<int> Newest = HOLControllerCommandsC9b::KeepLast(Ids, true);
	T.AssertEquals(1, Newest.Num(), "last keeps one id");
	T.AssertEquals(12, Newest[0], "last keeps the newest id");
	TArray<int> All = HOLControllerCommandsC9b::KeepLast(Ids, false);
	T.AssertEquals(3, All.Num(), "without last all ids stay");
	TArray<int> One;
	One.Add(7);
	T.AssertEquals(1, HOLControllerCommandsC9b::KeepLast(One, true).Num(), "last on one id keeps it");
	TArray<int> None;
	T.AssertEquals(0, HOLControllerCommandsC9b::KeepLast(None, true).Num(), "last on no id stays empty");
}

void Test_ControllerCommandsC9bTexts(FUnitTest& T)
{
	T.AssertEquals("Select own: 5 units", HOLControllerCommandsC9b::OwnText(5), "Select.Own line");
	T.AssertEquals("Select type Monk: 2 units", HOLControllerCommandsC9b::TypeText("Monk", 2), "Select.Type line");
}
