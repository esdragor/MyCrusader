// C1 (SAURON-1119a, UNCOMPILED): the cheat and victory console commands in script (HOLControllerCommands.as). Each command is registered once under its `hol.*` name and asks the right RPC
// (ServerCheat<X> of Script\Net\HOLRpcPlayer.as) with the arguments the C++ command passed, defaults included; refused arguments give the C++ "Usage: ..." line and send nothing.

TArray<FString> CheatArgs(const FString& A = "", const FString& B = "", const FString& C = "", const FString& D = "")
{
	TArray<FString> Result;
	if (!A.IsEmpty()) Result.Add(A);
	if (!B.IsEmpty()) Result.Add(B);
	if (!C.IsEmpty()) Result.Add(C);
	if (!D.IsEmpty()) Result.Add(D);
	return Result;
}

void Test_ControllerCommandsEachNameOnceAndRouted(FUnitTest& T)
{
	TArray<FString> Names = HOLControllerCommands::Names();
	T.AssertEquals(16, Names.Num(), "16 cheat and victory commands ported");
	TArray<FString> Seen;
	for (int I = 0; I < Names.Num(); I++)
	{
		FString Name = Names[I];
		T.AssertTrue(Name.StartsWith("hol."), f"{Name}: a hol.* command");
		T.AssertFalse(Seen.Contains(Name), f"{Name}: registered once");
		Seen.Add(Name);
		// Every name has a plan that either routes to a ServerCheat function (with its default or a slot argument) or refuses with a usage line: none is unknown.
		FHOLCheatPlan Plan = HOLControllerCommands::PlanOf(Name, CheatArgs("1", "1", "1"));
		T.AssertTrue(Plan.bValid || !Plan.Usage.IsEmpty(), f"{Name}: known to the planner");
		if (Plan.bValid)
			T.AssertTrue(Plan.Route.StartsWith("ServerCheat"), f"{Name}: routes to a ServerCheat RPC");
	}
	FHOLCheatPlan Unknown = HOLControllerCommands::PlanOf("hol.Cheat.Nothing", CheatArgs());
	T.AssertFalse(Unknown.bValid, "an unknown name plans nothing");
	T.AssertTrue(Unknown.Usage.IsEmpty(), "an unknown name logs nothing");
}

void Test_ControllerCommandsResourcesDefaultsAndMasks(FUnitTest& T)
{
	FHOLCheatPlan All = HOLControllerCommands::PlanOf("hol.Cheat.Resources", CheatArgs());
	T.AssertEquals("ServerCheatResources", All.Route, "Resources routes to ServerCheatResources");
	T.AssertEquals(4, All.Ints.Num(), "food, wood, stone, gold");
	T.AssertEquals(1000, All.Ints[0], "default 1000 food");
	T.AssertEquals(1000, All.Ints[3], "default 1000 gold");
	FHOLCheatPlan Wood = HOLControllerCommands::PlanOf("hol.Cheat.Wood", CheatArgs("250"));
	T.AssertEquals(0, Wood.Ints[0], "wood: no food");
	T.AssertEquals(250, Wood.Ints[1], "wood: 250 wood");
	T.AssertEquals(0, Wood.Ints[2], "wood: no stone");
	T.AssertEquals(0, Wood.Ints[3], "wood: no gold");
	FHOLCheatPlan Gold = HOLControllerCommands::PlanOf("hol.Cheat.Gold", CheatArgs("7"));
	T.AssertEquals(7, Gold.Ints[3], "gold: 7 gold");
	T.AssertEquals(0, Gold.Ints[1], "gold: no wood");
	FHOLCheatPlan Food = HOLControllerCommands::PlanOf("hol.Cheat.Food", CheatArgs());
	T.AssertEquals(1000, Food.Ints[0], "food: default 1000");
	FHOLCheatPlan Stone = HOLControllerCommands::PlanOf("hol.Cheat.Stone", CheatArgs("-5"));
	T.AssertEquals(-5, Stone.Ints[2], "stone: a negative amount passes as typed");
}

void Test_ControllerCommandsScalarCheats(FUnitTest& T)
{
	FHOLCheatPlan Fast = HOLControllerCommands::PlanOf("hol.Cheat.FastBuild", CheatArgs());
	T.AssertEquals("ServerCheatFastBuild", Fast.Route, "FastBuild route");
	T.AssertEquals(1, Fast.Ints[0], "FastBuild with no argument is on");
	FHOLCheatPlan Off = HOLControllerCommands::PlanOf("hol.Cheat.FastBuild", CheatArgs("0"));
	T.AssertEquals(0, Off.Ints[0], "FastBuild 0 is off");
	FHOLCheatPlan Pop = HOLControllerCommands::PlanOf("hol.Cheat.Population", CheatArgs());
	T.AssertEquals("ServerCheatPopulation", Pop.Route, "Population route");
	T.AssertEquals(165, Pop.Ints[0], "Population default 165");
	FHOLCheatPlan Wound = HOLControllerCommands::PlanOf("hol.Cheat.Wound", CheatArgs());
	T.AssertEquals("ServerCheatWound", Wound.Route, "Wound route");
	T.AssertEquals(50, Wound.Ints[0], "Wound default 50");
	FHOLCheatPlan Speed = HOLControllerCommands::PlanOf("hol.Cheat.UnitSpeed", CheatArgs("2.5"));
	T.AssertEquals("ServerCheatUnitSpeed", Speed.Route, "UnitSpeed route");
	T.AssertTrue(Math::IsNearlyEqual(Speed.Real, 2.5), "UnitSpeed 2.5 parsed as a real");
	FHOLCheatPlan Gather = HOLControllerCommands::PlanOf("hol.Cheat.GatherSpeed", CheatArgs());
	T.AssertEquals("ServerCheatGatherSpeed", Gather.Route, "GatherSpeed route");
	T.AssertTrue(Math::IsNearlyEqual(Gather.Real, 1.0), "GatherSpeed default 1");
}

void Test_ControllerCommandsTechnologyAndTier(FUnitTest& T)
{
	FHOLCheatPlan None = HOLControllerCommands::PlanOf("hol.Cheat.Technology", CheatArgs());
	T.AssertFalse(None.bValid, "Technology with no name is refused");
	T.AssertEquals("Usage: hol.Cheat.Technology <Name>", None.Usage, "Technology usage line");
	FHOLCheatPlan Tech = HOLControllerCommands::PlanOf("hol.Cheat.Technology", CheatArgs("MeleeAttackI"));
	T.AssertEquals("ServerCheatTechnology", Tech.Route, "Technology route");
	T.AssertEquals("MeleeAttackI", Tech.Name, "Technology carries the name");

	FHOLCheatPlan NoTier = HOLControllerCommands::PlanOf("hol.Cheat.Tier", CheatArgs());
	T.AssertEquals("Usage: hol.Cheat.Tier <0..3|off> [Slot|all]", NoTier.Usage, "Tier usage line");
	FHOLCheatPlan Own = HOLControllerCommands::PlanOf("hol.Cheat.Tier", CheatArgs("1"));
	T.AssertEquals("ServerCheatTier", Own.Route, "Tier route");
	T.AssertEquals(1, Own.Ints[0], "Tier 1");
	T.AssertEquals(-1, Own.Ints[1], "no slot: -1 (the local player)");
	FHOLCheatPlan Big = HOLControllerCommands::PlanOf("hol.Cheat.Tier", CheatArgs("9", "all"));
	T.AssertEquals(3, Big.Ints[0], "Tier clamped to 3");
	T.AssertEquals(-2, Big.Ints[1], "all: -2");
	FHOLCheatPlan Lift = HOLControllerCommands::PlanOf("hol.Cheat.Tier", CheatArgs("OFF", "2"));
	T.AssertEquals(-1, Lift.Ints[0], "off lifts the force (-1), whatever the case");
	T.AssertEquals(2, Lift.Ints[1], "slot 2");
}

void Test_ControllerCommandsVictory(FUnitTest& T)
{
	FHOLCheatPlan NoSlot = HOLControllerCommands::PlanOf("hol.Victory.Defeat", CheatArgs());
	T.AssertFalse(NoSlot.bValid, "Defeat with no slot is refused");
	T.AssertEquals("Usage: hol.Victory.Defeat <Slot>", NoSlot.Usage, "Defeat usage line");
	FHOLCheatPlan Word = HOLControllerCommands::PlanOf("hol.Victory.FallForum", CheatArgs("abc"));
	T.AssertEquals("Usage: hol.Victory.FallForum <Slot>", Word.Usage, "a non-numeric slot is refused");
	FHOLCheatPlan Defeat = HOLControllerCommands::PlanOf("hol.Victory.Defeat", CheatArgs("2"));
	T.AssertEquals("ServerCheatDefeat", Defeat.Route, "Defeat route");
	T.AssertEquals(2, Defeat.Ints[0], "Defeat slot 2");
	FHOLCheatPlan Fall = HOLControllerCommands::PlanOf("hol.Victory.FallForum", CheatArgs("1"));
	T.AssertEquals("ServerCheatFallForum", Fall.Route, "FallForum route");
	FHOLCheatPlan Skip = HOLControllerCommands::PlanOf("hol.Victory.SkipRuin", CheatArgs("1"));
	T.AssertEquals("ServerCheatSkipForum", Skip.Route, "SkipRuin routes to ServerCheatSkipForum");
	T.AssertEquals("Usage: hol.Victory.SkipRuin <Slot>", HOLControllerCommands::PlanOf("hol.Victory.SkipRuin", CheatArgs()).Usage, "SkipRuin usage line");

	FHOLCheatPlan Hold = HOLControllerCommands::PlanOf("hol.Victory.HoldPresence", CheatArgs("1", "1", "0", "3"));
	T.AssertEquals("ServerCheatHoldPresence", Hold.Route, "HoldPresence route");
	T.AssertEquals(5, Hold.Ints.Num(), "slot, off, attackers, defenders, attacker slot");
	T.AssertEquals(1, Hold.Ints[0], "slot 1");
	T.AssertEquals(0, Hold.Ints[1], "not off");
	T.AssertEquals(1, Hold.Ints[2], "attackers present");
	T.AssertEquals(0, Hold.Ints[3], "no defenders");
	T.AssertEquals(3, Hold.Ints[4], "attacker slot 3");
	FHOLCheatPlan HoldNoAttacker = HOLControllerCommands::PlanOf("hol.Victory.HoldPresence", CheatArgs("1", "1", "1"));
	T.AssertEquals(-1, HoldNoAttacker.Ints[4], "no attacker slot: -1 (INDEX_NONE)");
	FHOLCheatPlan HoldOff = HOLControllerCommands::PlanOf("hol.Victory.HoldPresence", CheatArgs("2", "off"));
	T.AssertTrue(HoldOff.bValid, "<Slot> off is accepted");
	T.AssertEquals(1, HoldOff.Ints[1], "off flag");
	T.AssertEquals(0, HoldOff.Ints[2], "off: attackers 0");
	T.AssertEquals(-1, HoldOff.Ints[4], "off: no attacker slot");
	FHOLCheatPlan HoldShort = HOLControllerCommands::PlanOf("hol.Victory.HoldPresence", CheatArgs("1", "1"));
	T.AssertFalse(HoldShort.bValid, "HoldPresence with two arguments is refused");
	T.AssertTrue(HoldShort.Usage.StartsWith("Usage: hol.Victory.HoldPresence <Slot> <Attackers 0|1> <Defenders 0|1>"), "HoldPresence usage line");
}

void Test_ControllerCommandsParsing(FUnitTest& T)
{
	T.AssertEquals(42, HOLControllerCommands::ParseInt("42"), "42");
	T.AssertEquals(-3, HOLControllerCommands::ParseInt("-3"), "-3");
	T.AssertEquals(7, HOLControllerCommands::ParseInt("7.9"), "7.9 truncates to 7 (Atoi)");
	T.AssertEquals(12, HOLControllerCommands::ParseInt("12abc"), "the leading integer (Atoi)");
	T.AssertEquals(0, HOLControllerCommands::ParseInt("abc"), "no number is 0 (Atoi)");
	T.AssertTrue(HOLControllerCommands::IsNumber("3"), "3 is numeric");
	T.AssertTrue(HOLControllerCommands::IsNumber("-2.5"), "-2.5 is numeric");
	T.AssertFalse(HOLControllerCommands::IsNumber("off"), "off is not numeric");
	T.AssertFalse(HOLControllerCommands::IsNumber("1.2.3"), "two points are not numeric");
	T.AssertFalse(HOLControllerCommands::IsNumber("-"), "a sign alone is not numeric");
	T.AssertFalse(HOLControllerCommands::IsNumber(""), "empty is not numeric");
}
