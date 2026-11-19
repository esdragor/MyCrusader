// C1 (SAURON-1119a, D851 step 5, UNCOMPILED): the cheat and victory console commands of AHOLPlayerController.cpp, now script. Same names `hol.*`, same arguments, same effect: each one calls the
// script RPC actor of every local controller (AHOLRpcPlayer.ServerCheat<X>, Script\Net\HOLRpcPlayer.as), as the C++ RequestCheat<X> did through CallFull; same "Usage: ..." lines.
// NOT in Script\Dev: the plugin skips Dev / Developers / Examples / Editor outside the interactive editor (-game and -server would never see these commands).
// Guard: the C++ ones were under #if !UE_BUILD_SHIPPING; the script has no such guard. The server still refuses every cheat unless the cheats are active (HOLRpcBridge::CheatsAllowed in each ServerCheat<X>, R3.5),
// and the C++ Do<X> bodies stay #if !UE_BUILD_SHIPPING: in Shipping the command exists but does nothing.
// Step 5 follow-up (SAURON-1119p, UNCOMPILED): hol.Cheat.DamageNear, hol.Cheat.SpawnBuilding, hol.Cheat.DamageSelected and hol.Units.SpawnHere too, reading the C++ state they need (main centre point, selection,
// camera focus) through HOLControllerRead:: (UHOLControllerReadLibrary); SpawnHere has no RPC (it spawns through the unit subsystem on the server or solo, as the C++ did).

// What one command asks: the RPC function, its integer parameters in order (a bool as 0 / 1), its real parameter, its name parameter; or a usage line when the arguments are refused.
struct FHOLCheatPlan
{
	bool bValid = false;
	FString Route;
	TArray<int> Ints;
	double Real = 0.0;
	FString Name;
	FString Usage;
	// The point the RPC needs, resolved per local controller: "" none, "near" the main centre (or an enemy's) plus (Dx, Dy) in its frame, "selection" the selected building, "camera" the camera focus (SpawnHere).
	FString PointSource;
	double Dx = 0.0;
	double Dy = 0.0;
	bool bEnemy = false;
}

namespace HOLControllerCommands
{
	// The command names (the exact `hol.*` of the C++ ones) and the RPC function each calls; one row per registered command.
	TArray<FString> Names()
	{
		TArray<FString> Result;
		Result.Add("hol.Cheat.Resources"); Result.Add("hol.Cheat.Food"); Result.Add("hol.Cheat.Wood"); Result.Add("hol.Cheat.Stone"); Result.Add("hol.Cheat.Gold");
		Result.Add("hol.Cheat.FastBuild"); Result.Add("hol.Cheat.Population"); Result.Add("hol.Cheat.Wound"); Result.Add("hol.Cheat.Technology"); Result.Add("hol.Cheat.Tier");
		Result.Add("hol.Cheat.UnitSpeed"); Result.Add("hol.Cheat.GatherSpeed");
		Result.Add("hol.Cheat.DamageNear"); Result.Add("hol.Cheat.SpawnBuilding"); Result.Add("hol.Cheat.DamageSelected"); Result.Add("hol.Units.SpawnHere");
		Result.Add("hol.Victory.Defeat"); Result.Add("hol.Victory.FallForum"); Result.Add("hol.Victory.SkipRuin"); Result.Add("hol.Victory.HoldPresence");
		return Result;
	}

	int DigitOf(const FString& Character)
	{
		const FString Digits = "0123456789";
		for (int I = 0; I < 10; I++)
		{
			if (Digits.Mid(I, 1) == Character)
				return I;
		}
		return -1;
	}

	// FString::IsNumeric of the C++ command: an optional sign, digits with at most one point, at least one digit (no exponent).
	bool IsNumber(const FString& Text)
	{
		int Start = 0;
		if (Text.Len() > 0 && (Text.Mid(0, 1) == "-" || Text.Mid(0, 1) == "+"))
			Start = 1;
		int DigitCount = 0;
		int PointCount = 0;
		for (int I = Start; I < Text.Len(); I++)
		{
			const FString Character = Text.Mid(I, 1);
			if (Character == ".")
				PointCount++;
			else if (DigitOf(Character) >= 0)
				DigitCount++;
			else
				return false;
		}
		return DigitCount > 0 && PointCount <= 1;
	}

	// FCString::Atof: the leading number of the text, 0 when there is none.
	double ParseReal(const FString& Text)
	{
		int Start = 0;
		double Sign = 1.0;
		if (Text.Len() > 0 && (Text.Mid(0, 1) == "-" || Text.Mid(0, 1) == "+"))
		{
			if (Text.Mid(0, 1) == "-")
				Sign = -1.0;
			Start = 1;
		}
		// The digits are gathered as one whole number and divided once by the power of ten: 0.75 is then exactly the 0.75 of the C++ Atod
		// (adding 7 * 0.1 and 5 * 0.01 gave 0.7500000000000001, seen red by Test_ControllerCommandsC10aArguments on 2026-11-19).
		double Value = 0.0;
		double Divisor = 1.0;
		bool bFraction = false;
		for (int I = Start; I < Text.Len(); I++)
		{
			const FString Character = Text.Mid(I, 1);
			if (Character == "." && !bFraction)
			{
				bFraction = true;
				continue;
			}
			const int Digit = DigitOf(Character);
			if (Digit < 0)
				break;
			Value = Value * 10.0 + Digit;
			if (bFraction)
				Divisor *= 10.0;
		}
		return Sign * Value / Divisor;
	}

	// FCString::Atoi: the leading integer (the part after a point is dropped).
	int ParseInt(const FString& Text)
	{
		return int(ParseReal(Text));
	}

	FHOLCheatPlan Refused(const FString& Usage)
	{
		FHOLCheatPlan Plan;
		Plan.Usage = Usage;
		return Plan;
	}

	// ParseAttackType of the C++: ranged 1, siege 2, hero 3, melee 0 by default (EHOLAttackType order), the name compared in lowercase; the argument at Index.
	int AttackTypeOf(const TArray<FString>& Args, int Index)
	{
		const FString Name = Args.IsValidIndex(Index) ? Args[Index].ToLower() : "";
		if (Name == "ranged")
			return 1;
		if (Name == "siege")
			return 2;
		if (Name == "hero")
			return 3;
		return 0;
	}

	FHOLCheatPlan Accepted(const FString& Route)
	{
		FHOLCheatPlan Plan;
		Plan.bValid = true;
		Plan.Route = Route;
		return Plan;
	}

	// hol.Cheat.Resources / Food / Wood / Stone / Gold [N=1000]: N times the per-unit stock (Food, Wood, Stone, Gold).
	FHOLCheatPlan ResourcesPlan(const TArray<FString>& Args, int Food, int Wood, int Stone, int Gold)
	{
		const int Amount = Args.Num() == 0 ? 1000 : ParseInt(Args[0]);
		FHOLCheatPlan Plan = Accepted("ServerCheatResources");
		Plan.Ints.Add(Food * Amount);
		Plan.Ints.Add(Wood * Amount);
		Plan.Ints.Add(Stone * Amount);
		Plan.Ints.Add(Gold * Amount);
		return Plan;
	}

	FHOLCheatPlan SlotPlan(const TArray<FString>& Args, const FString& Route, const FString& Usage)
	{
		if (Args.Num() == 0 || !IsNumber(Args[0]))
			return Refused(Usage);
		FHOLCheatPlan Plan = Accepted(Route);
		Plan.Ints.Add(ParseInt(Args[0]));
		return Plan;
	}

	// The plan of one command by its name; Plan.bValid false with Plan.Usage empty is an unknown command, with a Usage the refused arguments (the line the C++ command logged).
	FHOLCheatPlan PlanOf(const FString& Command, const TArray<FString>& Args)
	{
		if (Command == "hol.Cheat.Resources")
			return ResourcesPlan(Args, 1, 1, 1, 1);
		if (Command == "hol.Cheat.Food")
			return ResourcesPlan(Args, 1, 0, 0, 0);
		if (Command == "hol.Cheat.Wood")
			return ResourcesPlan(Args, 0, 1, 0, 0);
		if (Command == "hol.Cheat.Stone")
			return ResourcesPlan(Args, 0, 0, 1, 0);
		if (Command == "hol.Cheat.Gold")
			return ResourcesPlan(Args, 0, 0, 0, 1);
		if (Command == "hol.Cheat.FastBuild")
		{
			FHOLCheatPlan Plan = Accepted("ServerCheatFastBuild");
			Plan.Ints.Add((Args.Num() == 0 || ParseInt(Args[0]) != 0) ? 1 : 0);
			return Plan;
		}
		if (Command == "hol.Cheat.Population")
		{
			FHOLCheatPlan Plan = Accepted("ServerCheatPopulation");
			Plan.Ints.Add(Args.Num() == 0 ? 165 : ParseInt(Args[0]));
			return Plan;
		}
		if (Command == "hol.Cheat.Wound")
		{
			FHOLCheatPlan Plan = Accepted("ServerCheatWound");
			Plan.Ints.Add(Args.Num() == 0 ? 50 : ParseInt(Args[0]));
			return Plan;
		}
		if (Command == "hol.Cheat.Technology")
		{
			if (Args.Num() == 0)
				return Refused("Usage: hol.Cheat.Technology <Name>");
			FHOLCheatPlan Plan = Accepted("ServerCheatTechnology");
			Plan.Name = Args[0];
			return Plan;
		}
		if (Command == "hol.Cheat.Tier")
		{
			if (Args.Num() == 0)
				return Refused("Usage: hol.Cheat.Tier <0..3|off> [Slot|all]");
			FHOLCheatPlan Plan = Accepted("ServerCheatTier");
			Plan.Ints.Add(Args[0].ToLower() == "off" ? -1 : Math::Clamp(ParseInt(Args[0]), 0, 3));
			Plan.Ints.Add(Args.Num() < 2 ? -1 : (Args[1].ToLower() == "all" ? -2 : ParseInt(Args[1])));
			return Plan;
		}
		if (Command == "hol.Cheat.UnitSpeed" || Command == "hol.Cheat.GatherSpeed")
		{
			FHOLCheatPlan Plan = Accepted(Command == "hol.Cheat.UnitSpeed" ? "ServerCheatUnitSpeed" : "ServerCheatGatherSpeed");
			Plan.Real = Args.Num() == 0 ? 1.0 : ParseReal(Args[0]);
			return Plan;
		}
		if (Command == "hol.Cheat.DamageNear")
		{
			if (Args.Num() < 3)
				return Refused("Usage: hol.Cheat.DamageNear <Amount> <Dx> <Dy> [enemy|own] [melee|ranged|siege|hero]");
			FHOLCheatPlan Plan = Accepted("ServerCheatDamageBuilding");
			Plan.Ints.Add(ParseInt(Args[0]));
			Plan.Ints.Add(AttackTypeOf(Args, 4));
			Plan.PointSource = "near";
			Plan.Dx = ParseReal(Args[1]);
			Plan.Dy = ParseReal(Args[2]);
			Plan.bEnemy = Args.Num() >= 4 && Args[3] == "enemy";
			return Plan;
		}
		if (Command == "hol.Cheat.SpawnBuilding")
		{
			if (Args.Num() < 4)
				return Refused("Usage: hol.Cheat.SpawnBuilding <Type> <Slot> <Dx> <Dy> [enemy]");
			FHOLCheatPlan Plan = Accepted("ServerCheatSpawnBuilding");
			Plan.Name = Args[0];
			Plan.Ints.Add(ParseInt(Args[1]));
			Plan.PointSource = "near";
			Plan.Dx = ParseReal(Args[2]);
			Plan.Dy = ParseReal(Args[3]);
			Plan.bEnemy = Args.Num() >= 5 && Args[4] == "enemy";
			return Plan;
		}
		if (Command == "hol.Cheat.DamageSelected")
		{
			FHOLCheatPlan Plan = Accepted("ServerCheatDamageBuilding");
			Plan.Ints.Add(Args.Num() == 0 ? 100 : ParseInt(Args[0]));
			Plan.Ints.Add(AttackTypeOf(Args, 1));
			Plan.PointSource = "selection";
			return Plan;
		}
		if (Command == "hol.Units.SpawnHere")
		{
			// No RPC: the units are made through the unit subsystem on the server or in solo (Route "SpawnHere"); Ints = Count, Slot; Name = the type; Real = the radius.
			FHOLCheatPlan Plan = Accepted("SpawnHere");
			Plan.Ints.Add(Args.Num() >= 1 ? ParseInt(Args[0]) : 5);
			Plan.Ints.Add(Args.Num() >= 3 ? ParseInt(Args[2]) : 1);
			Plan.Name = Args.Num() >= 2 ? Args[1] : "Spearman";
			Plan.Real = Args.Num() >= 4 ? ParseReal(Args[3]) : 600.0;
			Plan.PointSource = "camera";
			return Plan;
		}
		if (Command == "hol.Victory.Defeat")
			return SlotPlan(Args, "ServerCheatDefeat", "Usage: hol.Victory.Defeat <Slot>");
		if (Command == "hol.Victory.FallForum")
			return SlotPlan(Args, "ServerCheatFallForum", "Usage: hol.Victory.FallForum <Slot>");
		if (Command == "hol.Victory.SkipRuin")
			return SlotPlan(Args, "ServerCheatSkipForum", "Usage: hol.Victory.SkipRuin <Slot>");
		if (Command == "hol.Victory.HoldPresence")
		{
			const bool bOff = Args.Num() >= 2 && Args[1].ToLower() == "off";
			if (Args.Num() == 0 || !IsNumber(Args[0]) || (!bOff && (Args.Num() < 3 || !IsNumber(Args[1]) || !IsNumber(Args[2]))))
				return Refused("Usage: hol.Victory.HoldPresence <Slot> <Attackers 0|1> <Defenders 0|1> [AttackerSlot] | <Slot> off");
			FHOLCheatPlan Plan = Accepted("ServerCheatHoldPresence");
			Plan.Ints.Add(ParseInt(Args[0]));
			Plan.Ints.Add(bOff ? 1 : 0);
			Plan.Ints.Add((!bOff && ParseInt(Args[1]) != 0) ? 1 : 0);
			Plan.Ints.Add((!bOff && ParseInt(Args[2]) != 0) ? 1 : 0);
			Plan.Ints.Add((!bOff && Args.Num() >= 4 && IsNumber(Args[3])) ? ParseInt(Args[3]) : -1);
			return Plan;
		}
		return FHOLCheatPlan();
	}

	// Sends one plan through the script RPC actor of a controller (the only route, D845); the journal line of a call not taken is the C++ LogNotSent's.
	void Send(AHOLPlayerController Controller, const FHOLCheatPlan& Plan, const FVector& Point)
	{
		if (Plan.Route == "SpawnHere")
		{
			// The C++ made the units here, on the server or in solo only (SpawnUnitsAround is false on a client or without the unit subsystem); the camera focus is Point.
			if (!HOLControllerRead::SpawnUnitsAround(Controller, Plan.Ints[0], FName(Plan.Name), Plan.Ints[1], Plan.Real, Point))
			{
				HOLScriptLog::LogWarning(n"LogHOLMatch", "hol.Units.SpawnHere: on the server or in solo, once the match runs");
				return;
			}
			Log(n"LogHOLMatch", f"SpawnHere: {Plan.Ints[0]} {Plan.Name} of slot {Plan.Ints[1]} around ({Math::RoundToInt(Point.X)}, {Math::RoundToInt(Point.Y)})");
			return;
		}
		AHOLRpcPlayer Rpc = Cast<AHOLRpcPlayer>(HOLRpcBridge::FindPlayerActor(Controller));
		if (Rpc == nullptr)
		{
			HOLScriptLog::LogError(n"LogHOLMatch", f"RpcRoute: {Plan.Route} not sent: no script RPC actor / function");
			return;
		}
		if (Plan.Route == "ServerCheatResources")
			Rpc.ServerCheatResources(Plan.Ints[0], Plan.Ints[1], Plan.Ints[2], Plan.Ints[3]);
		else if (Plan.Route == "ServerCheatFastBuild")
			Rpc.ServerCheatFastBuild(Plan.Ints[0] != 0);
		else if (Plan.Route == "ServerCheatPopulation")
			Rpc.ServerCheatPopulation(Plan.Ints[0]);
		else if (Plan.Route == "ServerCheatWound")
			Rpc.ServerCheatWound(Plan.Ints[0]);
		else if (Plan.Route == "ServerCheatTechnology")
			Rpc.ServerCheatTechnology(FName(Plan.Name));
		else if (Plan.Route == "ServerCheatTier")
			Rpc.ServerCheatTier(Plan.Ints[0], Plan.Ints[1]);
		else if (Plan.Route == "ServerCheatUnitSpeed")
			Rpc.ServerCheatUnitSpeed(float(Plan.Real));
		else if (Plan.Route == "ServerCheatGatherSpeed")
			Rpc.ServerCheatGatherSpeed(float(Plan.Real));
		else if (Plan.Route == "ServerCheatDamageBuilding")
			Rpc.ServerCheatDamageBuilding(Plan.Ints[0], Plan.Ints[1], Point);
		else if (Plan.Route == "ServerCheatSpawnBuilding")
			Rpc.ServerCheatSpawnBuilding(Plan.Name, Plan.Ints[0], Point);
		else if (Plan.Route == "ServerCheatDefeat")
			Rpc.ServerCheatDefeat(Plan.Ints[0]);
		else if (Plan.Route == "ServerCheatFallForum")
			Rpc.ServerCheatFallForum(Plan.Ints[0]);
		else if (Plan.Route == "ServerCheatSkipForum")
			Rpc.ServerCheatSkipForum(Plan.Ints[0]);
		else if (Plan.Route == "ServerCheatHoldPresence")
			Rpc.ServerCheatHoldPresence(Plan.Ints[0], Plan.Ints[1] != 0, Plan.Ints[2] != 0, Plan.Ints[3] != 0, Plan.Ints[4]);
		else
			HOLScriptLog::LogError(n"LogHOLMatch", f"RpcRoute: {Plan.Route} not sent: no script RPC actor / function");
	}

	// The command as typed: refused arguments log their usage line, else every local controller sends the plan (GetLocalControllers of the C++).
	void Run(const FString& Command, const TArray<FString>& Args)
	{
		const FHOLCheatPlan Plan = PlanOf(Command, Args);
		if (!Plan.bValid)
		{
			if (!Plan.Usage.IsEmpty())
				HOLScriptLog::LogWarning(n"LogHOLMatch", Plan.Usage);
			return;
		}
		TArray<AHOLPlayerController> Controllers;
		GetAllActorsOfClass(Controllers);
		for (AHOLPlayerController Controller : Controllers)
		{
			if (!Controller.IsLocalController())
				continue;
			// The point the plan needs, read from the controller's C++ state; nothing there (no main centre, no selection) skips this controller silently, as the C++ did.
			FVector Point;
			bool bFound = true;
			if (Plan.PointSource == "near")
				bFound = HOLControllerRead::PointNearMainCentre(Controller, Plan.Dx, Plan.Dy, Plan.bEnemy, Point);
			else if (Plan.PointSource == "selection")
				bFound = HOLControllerRead::SelectedBuildingLocation(Controller, Point);
			else if (Plan.PointSource == "camera")
			{
				// No camera: the C++ logged the same warning as without the unit subsystem.
				if (!HOLControllerRead::CameraFocus(Controller, Point))
				{
					HOLScriptLog::LogWarning(n"LogHOLMatch", "hol.Units.SpawnHere: on the server or in solo, once the match runs");
					continue;
				}
			}
			if (bFound)
				Send(Controller, Plan, Point);
		}
	}
}

void HOLCheatResourcesCommand(const TArray<FString>& Args) { HOLControllerCommands::Run("hol.Cheat.Resources", Args); }
void HOLCheatFoodCommand(const TArray<FString>& Args) { HOLControllerCommands::Run("hol.Cheat.Food", Args); }
void HOLCheatWoodCommand(const TArray<FString>& Args) { HOLControllerCommands::Run("hol.Cheat.Wood", Args); }
void HOLCheatStoneCommand(const TArray<FString>& Args) { HOLControllerCommands::Run("hol.Cheat.Stone", Args); }
void HOLCheatGoldCommand(const TArray<FString>& Args) { HOLControllerCommands::Run("hol.Cheat.Gold", Args); }
void HOLCheatFastBuildCommand(const TArray<FString>& Args) { HOLControllerCommands::Run("hol.Cheat.FastBuild", Args); }
void HOLCheatPopulationCommand(const TArray<FString>& Args) { HOLControllerCommands::Run("hol.Cheat.Population", Args); }
void HOLCheatWoundCommand(const TArray<FString>& Args) { HOLControllerCommands::Run("hol.Cheat.Wound", Args); }
void HOLCheatTechnologyCommand(const TArray<FString>& Args) { HOLControllerCommands::Run("hol.Cheat.Technology", Args); }
void HOLCheatTierCommand(const TArray<FString>& Args) { HOLControllerCommands::Run("hol.Cheat.Tier", Args); }
void HOLCheatUnitSpeedCommand(const TArray<FString>& Args) { HOLControllerCommands::Run("hol.Cheat.UnitSpeed", Args); }
void HOLCheatGatherSpeedCommand(const TArray<FString>& Args) { HOLControllerCommands::Run("hol.Cheat.GatherSpeed", Args); }
void HOLCheatDamageNearCommand(const TArray<FString>& Args) { HOLControllerCommands::Run("hol.Cheat.DamageNear", Args); }
void HOLCheatSpawnBuildingCommand(const TArray<FString>& Args) { HOLControllerCommands::Run("hol.Cheat.SpawnBuilding", Args); }
void HOLCheatDamageSelectedCommand(const TArray<FString>& Args) { HOLControllerCommands::Run("hol.Cheat.DamageSelected", Args); }
void HOLUnitsSpawnHereCommand(const TArray<FString>& Args) { HOLControllerCommands::Run("hol.Units.SpawnHere", Args); }
void HOLVictoryDefeatCommand(const TArray<FString>& Args) { HOLControllerCommands::Run("hol.Victory.Defeat", Args); }
void HOLVictoryFallForumCommand(const TArray<FString>& Args) { HOLControllerCommands::Run("hol.Victory.FallForum", Args); }
void HOLVictorySkipRuinCommand(const TArray<FString>& Args) { HOLControllerCommands::Run("hol.Victory.SkipRuin", Args); }
void HOLVictoryHoldPresenceCommand(const TArray<FString>& Args) { HOLControllerCommands::Run("hol.Victory.HoldPresence", Args); }

const FConsoleCommand HOLCheatResourcesConsole("hol.Cheat.Resources", n"HOLCheatResourcesCommand");
const FConsoleCommand HOLCheatFoodConsole("hol.Cheat.Food", n"HOLCheatFoodCommand");
const FConsoleCommand HOLCheatWoodConsole("hol.Cheat.Wood", n"HOLCheatWoodCommand");
const FConsoleCommand HOLCheatStoneConsole("hol.Cheat.Stone", n"HOLCheatStoneCommand");
const FConsoleCommand HOLCheatGoldConsole("hol.Cheat.Gold", n"HOLCheatGoldCommand");
const FConsoleCommand HOLCheatFastBuildConsole("hol.Cheat.FastBuild", n"HOLCheatFastBuildCommand");
const FConsoleCommand HOLCheatPopulationConsole("hol.Cheat.Population", n"HOLCheatPopulationCommand");
const FConsoleCommand HOLCheatWoundConsole("hol.Cheat.Wound", n"HOLCheatWoundCommand");
const FConsoleCommand HOLCheatTechnologyConsole("hol.Cheat.Technology", n"HOLCheatTechnologyCommand");
const FConsoleCommand HOLCheatTierConsole("hol.Cheat.Tier", n"HOLCheatTierCommand");
const FConsoleCommand HOLCheatUnitSpeedConsole("hol.Cheat.UnitSpeed", n"HOLCheatUnitSpeedCommand");
const FConsoleCommand HOLCheatGatherSpeedConsole("hol.Cheat.GatherSpeed", n"HOLCheatGatherSpeedCommand");
const FConsoleCommand HOLCheatDamageNearConsole("hol.Cheat.DamageNear", n"HOLCheatDamageNearCommand");
const FConsoleCommand HOLCheatSpawnBuildingConsole("hol.Cheat.SpawnBuilding", n"HOLCheatSpawnBuildingCommand");
const FConsoleCommand HOLCheatDamageSelectedConsole("hol.Cheat.DamageSelected", n"HOLCheatDamageSelectedCommand");
const FConsoleCommand HOLUnitsSpawnHereConsole("hol.Units.SpawnHere", n"HOLUnitsSpawnHereCommand");
const FConsoleCommand HOLVictoryDefeatConsole("hol.Victory.Defeat", n"HOLVictoryDefeatCommand");
const FConsoleCommand HOLVictoryFallForumConsole("hol.Victory.FallForum", n"HOLVictoryFallForumCommand");
const FConsoleCommand HOLVictorySkipRuinConsole("hol.Victory.SkipRuin", n"HOLVictorySkipRuinCommand");
const FConsoleCommand HOLVictoryHoldPresenceConsole("hol.Victory.HoldPresence", n"HOLVictoryHoldPresenceCommand");
