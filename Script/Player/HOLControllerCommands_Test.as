// C1 (SAURON-1119a, UNCOMPILED): the cheat and victory console commands in script (HOLControllerCommands.as). Each command is registered once under its `hol.*` name and asks the right RPC
// (ServerCheat<X> of Script\Net\HOLRpcPlayer.as) with the arguments the C++ command passed, defaults included; refused arguments give the C++ "Usage: ..." line and send nothing.

TArray<FString> CheatArgs(const FString& A = "", const FString& B = "", const FString& C = "", const FString& D = "", const FString& E = "")
{
	TArray<FString> Result;
	if (!A.IsEmpty()) Result.Add(A);
	if (!B.IsEmpty()) Result.Add(B);
	if (!C.IsEmpty()) Result.Add(C);
	if (!D.IsEmpty()) Result.Add(D);
	if (!E.IsEmpty()) Result.Add(E);
	return Result;
}

void Test_ControllerCommandsEachNameOnceAndRouted(FUnitTest& T)
{
	TArray<FString> Names = HOLControllerCommands::Names();
	T.AssertEquals(20, Names.Num(), "20 cheat, victory and spawn commands ported");
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
		// SpawnHere is the one without an RPC (it spawns through the unit subsystem, server or solo).
		if (Plan.bValid && Name != "hol.Units.SpawnHere")
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

void Test_ControllerCommandsDamageNear(FUnitTest& T)
{
	FHOLCheatPlan Short = HOLControllerCommands::PlanOf("hol.Cheat.DamageNear", CheatArgs("50", "100"));
	T.AssertFalse(Short.bValid, "DamageNear with two arguments is refused");
	T.AssertEquals("Usage: hol.Cheat.DamageNear <Amount> <Dx> <Dy> [enemy|own] [melee|ranged|siege|hero]", Short.Usage, "DamageNear usage line");
	FHOLCheatPlan Plan = HOLControllerCommands::PlanOf("hol.Cheat.DamageNear", CheatArgs("50", "100", "-250.5", "enemy", "siege"));
	T.AssertEquals("ServerCheatDamageBuilding", Plan.Route, "DamageNear route");
	T.AssertEquals(50, Plan.Ints[0], "amount 50");
	T.AssertEquals(2, Plan.Ints[1], "siege is attack type 2");
	T.AssertEquals("near", Plan.PointSource, "the point is near the main centre");
	T.AssertTrue(Math::IsNearlyEqual(Plan.Dx, 100.0) && Math::IsNearlyEqual(Plan.Dy, -250.5), "Dx and Dy parsed as reals");
	T.AssertTrue(Plan.bEnemy, "enemy flag");
	FHOLCheatPlan Own = HOLControllerCommands::PlanOf("hol.Cheat.DamageNear", CheatArgs("50", "0", "0", "ENEMY", "hero"));
	T.AssertTrue(Own.bEnemy, "enemy is compared case-insensitive, as the C++ FString ==");
	T.AssertEquals(3, Own.Ints[1], "hero is attack type 3");
	T.AssertEquals(0, HOLControllerCommands::PlanOf("hol.Cheat.DamageNear", CheatArgs("1", "0", "0")).Ints[1], "no attack type is melee 0");
}

void Test_ControllerCommandsSpawnBuilding(FUnitTest& T)
{
	FHOLCheatPlan Short = HOLControllerCommands::PlanOf("hol.Cheat.SpawnBuilding", CheatArgs("Barracks", "1", "300"));
	T.AssertFalse(Short.bValid, "SpawnBuilding with three arguments is refused");
	T.AssertEquals("Usage: hol.Cheat.SpawnBuilding <Type> <Slot> <Dx> <Dy> [enemy]", Short.Usage, "SpawnBuilding usage line");
	FHOLCheatPlan Plan = HOLControllerCommands::PlanOf("hol.Cheat.SpawnBuilding", CheatArgs("Barracks", "2", "300", "-400", "enemy"));
	T.AssertEquals("ServerCheatSpawnBuilding", Plan.Route, "SpawnBuilding route");
	T.AssertEquals("Barracks", Plan.Name, "the type name");
	T.AssertEquals(2, Plan.Ints[0], "slot 2");
	T.AssertEquals("near", Plan.PointSource, "the point is near the main centre");
	T.AssertTrue(Math::IsNearlyEqual(Plan.Dx, 300.0) && Math::IsNearlyEqual(Plan.Dy, -400.0), "Dx and Dy");
	T.AssertTrue(Plan.bEnemy, "enemy is the fifth argument");
	T.AssertFalse(HOLControllerCommands::PlanOf("hol.Cheat.SpawnBuilding", CheatArgs("Barracks", "1", "0", "0")).bEnemy, "no fifth argument: own");
}

void Test_ControllerCommandsDamageSelected(FUnitTest& T)
{
	FHOLCheatPlan Default = HOLControllerCommands::PlanOf("hol.Cheat.DamageSelected", CheatArgs());
	T.AssertTrue(Default.bValid, "DamageSelected has no refusal");
	T.AssertEquals("ServerCheatDamageBuilding", Default.Route, "DamageSelected route");
	T.AssertEquals(100, Default.Ints[0], "default amount 100");
	T.AssertEquals(0, Default.Ints[1], "default melee");
	T.AssertEquals("selection", Default.PointSource, "the point is the selected building");
	FHOLCheatPlan Typed = HOLControllerCommands::PlanOf("hol.Cheat.DamageSelected", CheatArgs("30", "Ranged"));
	T.AssertEquals(30, Typed.Ints[0], "amount 30");
	T.AssertEquals(1, Typed.Ints[1], "ranged is attack type 1, whatever the case");
}

void Test_ControllerCommandsSpawnHere(FUnitTest& T)
{
	FHOLCheatPlan Default = HOLControllerCommands::PlanOf("hol.Units.SpawnHere", CheatArgs());
	T.AssertTrue(Default.bValid, "SpawnHere has no refusal");
	T.AssertEquals("SpawnHere", Default.Route, "SpawnHere has no RPC: its own route");
	T.AssertEquals(5, Default.Ints[0], "default count 5");
	T.AssertEquals(1, Default.Ints[1], "default slot 1");
	T.AssertEquals("Spearman", Default.Name, "default type Spearman");
	T.AssertTrue(Math::IsNearlyEqual(Default.Real, 600.0), "default radius 600");
	T.AssertEquals("camera", Default.PointSource, "around the camera focus");
	FHOLCheatPlan Typed = HOLControllerCommands::PlanOf("hol.Units.SpawnHere", CheatArgs("12", "Arquebusier", "2", "850.5"));
	T.AssertEquals(12, Typed.Ints[0], "count 12");
	T.AssertEquals(2, Typed.Ints[1], "slot 2 (third argument)");
	T.AssertEquals("Arquebusier", Typed.Name, "type is the second argument");
	T.AssertTrue(Math::IsNearlyEqual(Typed.Real, 850.5), "radius 850.5");
}
