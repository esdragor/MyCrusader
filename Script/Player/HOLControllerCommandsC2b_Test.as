// C2b (SAURON-1119v): the console commands that stayed in C++ after C2 (hero rage, market, tribute, production, reports, forced diplomacy, tribute notice) are now script (HOLControllerCommandsC2b.as).
// Each command is registered once under its `hol.*` name; its plan (what it asks, with which numbers) and its refusal lines (the C++ "Usage" / "unknown resource" lines) are tested here.
// RED to see (once, then put back): in HOLControllerCommandsC2b.as ResourceOf, return 2 for "wood" instead of 1 -> Test_ControllerCommandsC2bMarketAndTribute fails on "wood is resource 1".

TArray<FString> C2bArgs(const FString& A = "", const FString& B = "", const FString& C = "", const FString& D = "")
{
	TArray<FString> Result;
	if (!A.IsEmpty()) Result.Add(A);
	if (!B.IsEmpty()) Result.Add(B);
	if (!C.IsEmpty()) Result.Add(C);
	if (!D.IsEmpty()) Result.Add(D);
	return Result;
}

void Test_ControllerCommandsC2bEachNameOnceAndKnown(FUnitTest& T)
{
	TArray<FString> Names = HOLControllerCommandsC2b::Names();
	T.AssertEquals(15, Names.Num(), "15 console commands ported from the C++ controller by C2b");
	TArray<FString> Seen;
	for (int I = 0; I < Names.Num(); I++)
	{
		FString Name = Names[I];
		T.AssertTrue(Name.StartsWith("hol."), f"{Name}: a hol.* command");
		T.AssertFalse(Seen.Contains(Name), f"{Name}: registered once");
		Seen.Add(Name);
		FHOLConsolePlanC2b Plan = HOLControllerCommandsC2b::PlanOf(Name, C2bArgs("food", "2", "buy", "7"));
		T.AssertTrue(Plan.bValid || !Plan.Usage.IsEmpty(), f"{Name}: known to the planner");
		T.AssertFalse(HOLControllerCommandsC2::Names().Contains(Name), f"{Name}: not also a C2 command");
	}
	FHOLConsolePlanC2b Unknown = HOLControllerCommandsC2b::PlanOf("hol.Nothing.Here", C2bArgs());
	T.AssertFalse(Unknown.bValid, "an unknown name plans nothing");
	T.AssertTrue(Unknown.Usage.IsEmpty(), "an unknown name logs nothing");
}

void Test_ControllerCommandsC2bMarketAndTribute(FUnitTest& T)
{
	T.AssertEquals(0, HOLControllerCommandsC2b::ResourceOf("FOOD"), "food is resource 0 (any case)");
	T.AssertEquals(1, HOLControllerCommandsC2b::ResourceOf("wood"), "wood is resource 1");
	T.AssertEquals(2, HOLControllerCommandsC2b::ResourceOf("Stone"), "stone is resource 2");
	T.AssertEquals(3, HOLControllerCommandsC2b::ResourceOf("gold"), "gold is resource 3");
	T.AssertEquals(-1, HOLControllerCommandsC2b::ResourceOf("iron"), "an unknown name is -1");
	FHOLConsolePlanC2b Buy = HOLControllerCommandsC2b::PlanOf("hol.Market.Trade", C2bArgs("wood", "3", "BUY"));
	T.AssertEquals("MarketTrade", Buy.Route, "Market.Trade route");
	T.AssertEquals(1, Buy.Ints[0], "trade resource wood");
	T.AssertEquals(3, Buy.Ints[1], "trade lots 3");
	T.AssertEquals(1, Buy.Ints[2], "buy is 1");
	FHOLConsolePlanC2b Sell = HOLControllerCommandsC2b::PlanOf("hol.Market.Trade", C2bArgs("stone", "2", "sell"));
	T.AssertEquals(0, Sell.Ints[2], "anything else than buy is a sale");
	FHOLConsolePlanC2b TradeShort = HOLControllerCommandsC2b::PlanOf("hol.Market.Trade", C2bArgs("wood", "3"));
	T.AssertFalse(TradeShort.bValid, "Market.Trade with two words is refused");
	T.AssertEquals("Usage: hol.Market.Trade <food|wood|stone> <Lots> <buy|sell>", TradeShort.Usage, "Market.Trade usage line");
	FHOLConsolePlanC2b TradeGold = HOLControllerCommandsC2b::PlanOf("hol.Market.Trade", C2bArgs("gold", "3", "buy"));
	T.AssertEquals("hol.Market.Trade: unknown resource gold", TradeGold.Usage, "gold is not tradable: the C++ unknown-resource line");
	FHOLConsolePlanC2b Tribute = HOLControllerCommandsC2b::PlanOf("hol.Tribute", C2bArgs("2", "gold", "500"));
	T.AssertEquals("Tribute", Tribute.Route, "Tribute route");
	T.AssertEquals(2, Tribute.Ints[0], "tribute receiver slot 2");
	T.AssertEquals(3, Tribute.Ints[1], "tribute resource gold");
	T.AssertEquals(500, Tribute.Ints[2], "tribute amount 500");
	FHOLConsolePlanC2b TributeShort = HOLControllerCommandsC2b::PlanOf("hol.Tribute", C2bArgs("2", "gold"));
	T.AssertEquals("Usage: hol.Tribute <Slot> <food|wood|stone|gold> <Amount>", TributeShort.Usage, "Tribute usage line");
	FHOLConsolePlanC2b TributeBad = HOLControllerCommandsC2b::PlanOf("hol.Tribute", C2bArgs("2", "iron", "5"));
	T.AssertEquals("hol.Tribute: unknown resource iron", TributeBad.Usage, "Tribute unknown-resource line");
}

void Test_ControllerCommandsC2bProduction(FUnitTest& T)
{
	FHOLConsolePlanC2b Rally = HOLControllerCommandsC2b::PlanOf("hol.Prod.Rally", C2bArgs("1500", "-250.5"));
	T.AssertEquals("Rally", Rally.Route, "Rally route");
	T.AssertTrue(Math::IsNearlyEqual(Rally.X, 1500.0), "Rally X 1500");
	T.AssertTrue(Math::IsNearlyEqual(Rally.Y, -250.5), "Rally Y -250.5");
	FHOLConsolePlanC2b RallyShort = HOLControllerCommandsC2b::PlanOf("hol.Prod.Rally", C2bArgs("1500"));
	T.AssertFalse(RallyShort.bValid, "Rally with one number is refused");
	T.AssertTrue(RallyShort.Usage.IsEmpty(), "Rally refused silently, as the C++");
	FHOLConsolePlanC2b Default = HOLControllerCommandsC2b::PlanOf("hol.Prod.Queue", C2bArgs());
	T.AssertEquals("Queue", Default.Route, "Queue route");
	T.AssertEquals("Peasant", Default.Name, "Queue produces a Peasant by default");
	T.AssertEquals("", Default.Building, "Queue selects no building first by default");
	FHOLConsolePlanC2b Barracks = HOLControllerCommandsC2b::PlanOf("hol.Prod.Queue", C2bArgs("Hoplite", "Barracks"));
	T.AssertEquals("Hoplite", Barracks.Name, "Queue unit type");
	T.AssertEquals("Barracks", Barracks.Building, "Queue building selected first");
	FHOLConsolePlanC2b Cancel = HOLControllerCommandsC2b::PlanOf("hol.Prod.Cancel", C2bArgs("2"));
	T.AssertEquals("Cancel", Cancel.Route, "Cancel route");
	T.AssertEquals(2, Cancel.Ints[0], "Cancel index 2");
	FHOLConsolePlanC2b CancelFirst = HOLControllerCommandsC2b::PlanOf("hol.Prod.Cancel", C2bArgs());
	T.AssertEquals(0, CancelFirst.Ints[0], "Cancel without a number is the first item");
}

void Test_ControllerCommandsC2bReportsAndDiplomacy(FUnitTest& T)
{
	T.AssertEquals("Rage", HOLControllerCommandsC2b::PlanOf("hol.Hero.Rage", C2bArgs()).Route, "Hero.Rage route");
	T.AssertEquals("SignalReport", HOLControllerCommandsC2b::PlanOf("hol.Signal.Report", C2bArgs()).Route, "Signal.Report route");
	T.AssertEquals("ChatReport", HOLControllerCommandsC2b::PlanOf("hol.Chat.Report", C2bArgs()).Route, "Chat.Report route");
	T.AssertEquals("SpecReport", HOLControllerCommandsC2b::PlanOf("hol.Spec.Report", C2bArgs()).Route, "Spec.Report route");
	T.AssertEquals("ProdReport", HOLControllerCommandsC2b::PlanOf("hol.Prod.Report", C2bArgs()).Route, "Prod.Report route");
	T.AssertEquals("MarketReport", HOLControllerCommandsC2b::PlanOf("hol.Market.Report", C2bArgs()).Route, "Market.Report route");
	T.AssertEquals("DiplomacyReport", HOLControllerCommandsC2b::PlanOf("hol.Diplomacy.Report", C2bArgs()).Route, "Diplomacy.Report route");
	FHOLConsolePlanC2b Force = HOLControllerCommandsC2b::PlanOf("hol.Diplomacy.Force", C2bArgs("1", "2", "Ally"));
	T.AssertEquals("ForceStance", Force.Route, "Force route");
	T.AssertEquals(1, Force.Ints[0], "Force from slot 1");
	T.AssertEquals(2, Force.Ints[1], "Force to slot 2");
	T.AssertEquals(2, Force.Ints[2], "Force ally is stance 2");
	T.AssertEquals(1, HOLControllerCommandsC2b::PlanOf("hol.Diplomacy.Force", C2bArgs("1", "2", "neutral")).Ints[2], "Force neutral is stance 1");
	T.AssertEquals(0, HOLControllerCommandsC2b::PlanOf("hol.Diplomacy.Force", C2bArgs("1", "2", "friend")).Ints[2], "an unknown word is enemy, as the C++");
	FHOLConsolePlanC2b ForceShort = HOLControllerCommandsC2b::PlanOf("hol.Diplomacy.Force", C2bArgs("1", "2"));
	T.AssertEquals("Usage: hol.Diplomacy.Force <FromSlot> <ToSlot> <ally|neutral|enemy> (in a match, on the server)", ForceShort.Usage, "Force usage line");
	FHOLConsolePlanC2b Vision = HOLControllerCommandsC2b::PlanOf("hol.Diplomacy.ForceVision", C2bArgs("1", "2", "1"));
	T.AssertEquals("ForceVision", Vision.Route, "ForceVision route");
	T.AssertEquals(1, Vision.Ints[2], "ForceVision 1 shares");
	T.AssertEquals(0, HOLControllerCommandsC2b::PlanOf("hol.Diplomacy.ForceVision", C2bArgs("1", "2", "0")).Ints[2], "ForceVision 0 stops");
	FHOLConsolePlanC2b VisionShort = HOLControllerCommandsC2b::PlanOf("hol.Diplomacy.ForceVision", C2bArgs("1", "2"));
	T.AssertEquals("Usage: hol.Diplomacy.ForceVision <FromSlot> <ToSlot> <0|1> (in a match, on the server)", VisionShort.Usage, "ForceVision usage line");
	FHOLConsolePlanC2b Notice = HOLControllerCommandsC2b::PlanOf("hol.Tribute.Notice", C2bArgs("3", "wood", "70", "30"));
	T.AssertEquals("TributeNotice", Notice.Route, "Tribute.Notice route");
	T.AssertEquals(3, Notice.Ints[0], "notice giver slot 3");
	T.AssertEquals(1, Notice.Ints[1], "notice resource wood");
	T.AssertEquals(70, Notice.Ints[2], "notice arrives 70");
	T.AssertEquals(30, Notice.Ints[3], "notice tax 30");
	T.AssertEquals(3, HOLControllerCommandsC2b::PlanOf("hol.Tribute.Notice", C2bArgs("3", "iron", "70", "30")).Ints[1], "an unknown resource is gold (3) for the notice, as the C++");
	FHOLConsolePlanC2b NoticeShort = HOLControllerCommandsC2b::PlanOf("hol.Tribute.Notice", C2bArgs("3", "wood", "70"));
	T.AssertEquals("Usage: hol.Tribute.Notice <GiverSlot> <food|wood|stone|gold> <Arrives> <Tax> (in a match)", NoticeShort.Usage, "Tribute.Notice usage line");
}
