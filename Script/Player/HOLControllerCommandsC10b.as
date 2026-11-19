// C10b (SAURON-1119ca, UNCOMPILED): hol.Select.OrderSource / ClickSource / Heal / Attack / OrderSourceNear, hol.Units.Order / OrderNear, hol.Build.PlaceNear / PlaceAtSource of AHOLPlayerController.cpp, now script.
// Same names, same arguments, same journal lines at the same level (category LogHOLMatch; Display for the reports, Warning for the Usage and "no standing source" lines),
// for every LOCAL controller. The aim at a source, the unit lookup by slot, the requests and the placement log ("Build request: ...") stay ONE C++ rule
// (UHOLControllerCommandLibrary, section C10b). The %.3f of the C++ lines is HOLControllerCommandsC10a::Fixed3, the %.0f is Math::RoundToInt.
// Pattern of HOLControllerCommandsC10a.as; the numeric parser of C1 is reused (HOLControllerCommands::ParseReal).

struct FHOLConsolePlanC10b
{
	bool bValid = false;
	FString Usage;
	double X1 = 0.0;
	double Y1 = 0.0;
	bool bShift = false;
	bool bQueue = false;
	bool bAll = false;
	bool bEnemy = false;
	bool bHasOffset = false;
	bool bBySlot = false;
	int Slot = -1;
	int TargetId = 0;
	FString TypeName;
}

namespace HOLControllerCommandsC10b
{
	TArray<FString> Names()
	{
		TArray<FString> Result;
		Result.Add("hol.Select.OrderSource"); Result.Add("hol.Select.ClickSource"); Result.Add("hol.Select.Heal");
		Result.Add("hol.Select.Attack"); Result.Add("hol.Select.OrderSourceNear"); Result.Add("hol.Units.Order");
		Result.Add("hol.Units.OrderNear"); Result.Add("hol.Build.PlaceNear"); Result.Add("hol.Build.PlaceAtSource");
		return Result;
	}

	FHOLConsolePlanC10b PlanOf(const FString& Command, const TArray<FString>& Args)
	{
		FHOLConsolePlanC10b Plan;
		if (Command == "hol.Select.OrderSource" || Command == "hol.Select.ClickSource")
		{
			if (Args.Num() < 2)
			{
				Plan.Usage = Command == "hol.Select.OrderSource" ? "Usage: hol.Select.OrderSource <X> <Y> [shift]" : "Usage: hol.Select.ClickSource <X> <Y>";
				return Plan;
			}
			Plan.X1 = HOLControllerCommands::ParseReal(Args[0]);
			Plan.Y1 = HOLControllerCommands::ParseReal(Args[1]);
			Plan.bShift = Command == "hol.Select.OrderSource" && Args.Num() >= 3 && Args[2] == "shift";
			Plan.bValid = true;
		}
		else if (Command == "hol.Select.Heal" || Command == "hol.Select.Attack")
		{
			if (Args.Num() < 1)
			{
				Plan.Usage = Command == "hol.Select.Heal" ? "Usage: hol.Select.Heal <UnitId>" : "Usage: hol.Select.Attack <UnitId> [shift]";
				return Plan;
			}
			Plan.bBySlot = Args[0].StartsWith("slot=");
			if (Plan.bBySlot)
				Plan.Slot = int(HOLControllerCommands::ParseReal(Args[0].Mid(5)));
			else
				Plan.TargetId = int(HOLControllerCommands::ParseReal(Args[0]));
			// Only Attack reads "shift" (any case); Heal never queues.
			Plan.bQueue = Command == "hol.Select.Attack" && Args.Num() > 1 && Args[1].ToLower() == "shift";
			Plan.bValid = true;
		}
		else if (Command == "hol.Select.OrderSourceNear")
		{
			// No Usage line: without two numbers the offset is (0, 0).
			Plan.bHasOffset = Args.Num() >= 2;
			if (Plan.bHasOffset)
			{
				Plan.X1 = HOLControllerCommands::ParseReal(Args[0]);
				Plan.Y1 = HOLControllerCommands::ParseReal(Args[1]);
			}
			Plan.bValid = true;
		}
		else if (Command == "hol.Units.Order" || Command == "hol.Units.OrderNear")
		{
			// No Usage line: fewer than two numbers is silently ignored.
			if (Args.Num() < 2)
				return Plan;
			Plan.X1 = HOLControllerCommands::ParseReal(Args[0]);
			Plan.Y1 = HOLControllerCommands::ParseReal(Args[1]);
			Plan.bAll = Args.Num() >= 3 && Args[2] == "all";
			Plan.bValid = true;
		}
		else if (Command == "hol.Build.PlaceNear")
		{
			if (Args.Num() < 3)
			{
				Plan.Usage = "Usage: hol.Build.PlaceNear <Type> <Dx> <Dy> [enemy]";
				return Plan;
			}
			Plan.TypeName = Args[0];
			Plan.X1 = HOLControllerCommands::ParseReal(Args[1]);
			Plan.Y1 = HOLControllerCommands::ParseReal(Args[2]);
			Plan.bEnemy = Args.Num() >= 4 && Args[3] == "enemy";
			Plan.bValid = true;
		}
		else if (Command == "hol.Build.PlaceAtSource")
		{
			if (Args.Num() < 3)
			{
				Plan.Usage = "Usage: hol.Build.PlaceAtSource <Type> <X> <Y>";
				return Plan;
			}
			Plan.TypeName = Args[0];
			Plan.X1 = HOLControllerCommands::ParseReal(Args[1]);
			Plan.Y1 = HOLControllerCommands::ParseReal(Args[2]);
			Plan.bValid = true;
		}
		return Plan;
	}

	FString SourceClickText(bool bOrder, int Index, double Fx, double Fy, bool bShift)
	{
		FString What = bOrder ? "Right click" : "Click";
		FString Tail = bShift ? ", Shift" : "";
		return f"{What} on source {Index} at ({HOLControllerCommandsC10a::Fixed3(Fx)}, {HOLControllerCommandsC10a::Fixed3(Fy)}){Tail}";
	}

	FString NoSourceText(const FString& Command)
	{
		return f"{Command}: no standing source";
	}

	FString SourceOffScreenText(const FString& Command)
	{
		return f"{Command}: the source is not on the screen";
	}

	FString HealText(int Count, int TargetId)
	{
		return f"Order: {Count} units heal #{TargetId}";
	}

	FString AttackText(int Count, int TargetId, bool bQueue)
	{
		FString Tail = bQueue ? ", queued" : "";
		return f"Order: {Count} units attack #{TargetId}{Tail}";
	}

	FString UnitsOrderText(int Count, double X, double Y, bool bAll)
	{
		int Rx = Math::RoundToInt(X);
		int Ry = Math::RoundToInt(Y);
		FString Tail = bAll ? ", other players' too" : "";
		return f"Order: {Count} units to ({Rx}, {Ry}){Tail}";
	}

	FString NoCentreText(bool bEnemy)
	{
		FString Kind = bEnemy ? "enemy" : "own";
		return f"hol.Build.PlaceNear: no {Kind} main centre known here";
	}

	FString BuildOnSourceText(int Index, double X, double Y)
	{
		int Rx = Math::RoundToInt(X);
		int Ry = Math::RoundToInt(Y);
		return f"Build on source {Index} at ({Rx}, {Ry})";
	}

	// hol.Select.OrderSource / ClickSource on the ground point (X, Y): the aim is one C++ rule, the lines are written here.
	void AimAndClick(AHOLPlayerController Controller, const FString& Command, double X, double Y, bool bOrder, bool bShift)
	{
		int Index = -1;
		double Fx = 0.0;
		double Fy = 0.0;
		int Status = HOLControllerCommand::SourceScreenPoint(Controller, X, Y, Index, Fx, Fy);
		if (Status == 1)
		{
			HOLScriptLog::LogWarning(n"LogHOLMatch", NoSourceText(Command));
			return;
		}
		if (Status == 2)
		{
			HOLScriptLog::LogWarning(n"LogHOLMatch", SourceOffScreenText(Command));
			return;
		}
		HOLScriptLog::LogDisplay(n"LogHOLMatch", SourceClickText(bOrder, Index, Fx, Fy, bShift));
		if (bOrder)
			HOLControllerCommand::OrderAtFraction(Controller, Fx, Fy, bShift);
		else
			HOLControllerCommand::SelectAtFraction(Controller, Fx, Fy, false, false);
	}

	void MoveUnits(AHOLPlayerController Controller, double X, double Y, bool bAll)
	{
		TArray<int> Ids;
		if (bAll)
			HOLControllerCommand::AllUnitIds(Controller, Ids);
		else
			HOLControllerCommand::OwnUnitIds(Controller, Ids);
		HOLScriptLog::LogDisplay(n"LogHOLMatch", UnitsOrderText(Ids.Num(), X, Y, bAll));
		HOLControllerCommand::RequestMoveUnitIdsTo(Controller, Ids, X, Y);
	}

	void DoForController(AHOLPlayerController Controller, const FString& Command, const FHOLConsolePlanC10b& Plan)
	{
		if (Command == "hol.Select.OrderSource")
			AimAndClick(Controller, Command, Plan.X1, Plan.Y1, true, Plan.bShift);
		else if (Command == "hol.Select.ClickSource")
			AimAndClick(Controller, Command, Plan.X1, Plan.Y1, false, false);
		else if (Command == "hol.Select.OrderSourceNear")
		{
			double Cx = 0.0;
			double Cy = 0.0;
			if (!HOLControllerCommand::StartCentreOf(Controller, Cx, Cy))
				return;
			double Dx = Plan.bHasOffset ? Plan.X1 : 0.0;
			double Dy = Plan.bHasOffset ? Plan.Y1 : 0.0;
			// The C++ forwarded "hol.Select.OrderSource %.0f %.0f": the point is rounded to whole cm first.
			AimAndClick(Controller, "hol.Select.OrderSource", Math::RoundToInt(Cx + Dx), Math::RoundToInt(Cy + Dy), true, false);
		}
		else if (Command == "hol.Select.Heal" || Command == "hol.Select.Attack")
		{
			int TargetId = Plan.bBySlot ? HOLControllerCommand::FirstUnitIdOfSlot(Controller, Plan.Slot) : Plan.TargetId;
			TArray<int> Own;
			HOLControllerCommand::CommandUnitIds(Controller, Own);
			if (Command == "hol.Select.Heal")
			{
				HOLScriptLog::LogDisplay(n"LogHOLMatch", HealText(Own.Num(), TargetId));
				HOLControllerCommand::RequestHealUnitIds(Controller, Own, TargetId, false);
			}
			else
			{
				HOLScriptLog::LogDisplay(n"LogHOLMatch", AttackText(Own.Num(), TargetId, Plan.bQueue));
				HOLControllerCommand::RequestAttackUnitIds(Controller, Own, TargetId, Plan.bQueue);
			}
		}
		else if (Command == "hol.Units.Order")
			MoveUnits(Controller, Plan.X1, Plan.Y1, Plan.bAll);
		else if (Command == "hol.Units.OrderNear")
		{
			double Cx = 0.0;
			double Cy = 0.0;
			if (!HOLControllerCommand::StartCentreOf(Controller, Cx, Cy))
				return;
			// The C++ forwarded "hol.Units.Order %.0f %.0f": the target is rounded to whole cm first.
			MoveUnits(Controller, Math::RoundToInt(Cx + Plan.X1), Math::RoundToInt(Cy + Plan.Y1), Plan.bAll);
		}
		else if (Command == "hol.Build.PlaceNear")
		{
			if (!HOLControllerCommand::RequestPlacementNear(Controller, FName(Plan.TypeName), Plan.X1, Plan.Y1, Plan.bEnemy))
				HOLScriptLog::LogWarning(n"LogHOLMatch", NoCentreText(Plan.bEnemy));
		}
		else if (Command == "hol.Build.PlaceAtSource")
		{
			double Sx = 0.0;
			double Sy = 0.0;
			int Index = HOLControllerCommand::StandingSourceNear(Controller, Plan.X1, Plan.Y1, Sx, Sy);
			if (Index < 0)
			{
				HOLScriptLog::LogWarning(n"LogHOLMatch", "hol.Build.PlaceAtSource: no standing source");
				return;
			}
			HOLScriptLog::LogDisplay(n"LogHOLMatch", BuildOnSourceText(Index, Sx, Sy));
			HOLControllerCommand::RequestPlacementOnSource(Controller, FName(Plan.TypeName), Index);
		}
	}

	void Run(const FString& Command, const TArray<FString>& Args)
	{
		const FHOLConsolePlanC10b Plan = PlanOf(Command, Args);
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
			if (Controller.IsLocalController())
				DoForController(Controller, Command, Plan);
		}
	}
}

void HOLSelectOrderSourceC10bCommand(const TArray<FString>& Args) { HOLControllerCommandsC10b::Run("hol.Select.OrderSource", Args); }
void HOLSelectClickSourceC10bCommand(const TArray<FString>& Args) { HOLControllerCommandsC10b::Run("hol.Select.ClickSource", Args); }
void HOLSelectHealC10bCommand(const TArray<FString>& Args) { HOLControllerCommandsC10b::Run("hol.Select.Heal", Args); }
void HOLSelectAttackC10bCommand(const TArray<FString>& Args) { HOLControllerCommandsC10b::Run("hol.Select.Attack", Args); }
void HOLSelectOrderSourceNearC10bCommand(const TArray<FString>& Args) { HOLControllerCommandsC10b::Run("hol.Select.OrderSourceNear", Args); }
void HOLUnitsOrderC10bCommand(const TArray<FString>& Args) { HOLControllerCommandsC10b::Run("hol.Units.Order", Args); }
void HOLUnitsOrderNearC10bCommand(const TArray<FString>& Args) { HOLControllerCommandsC10b::Run("hol.Units.OrderNear", Args); }
void HOLBuildPlaceNearC10bCommand(const TArray<FString>& Args) { HOLControllerCommandsC10b::Run("hol.Build.PlaceNear", Args); }
void HOLBuildPlaceAtSourceC10bCommand(const TArray<FString>& Args) { HOLControllerCommandsC10b::Run("hol.Build.PlaceAtSource", Args); }

const FConsoleCommand HOLSelectOrderSourceC10bConsole("hol.Select.OrderSource", n"HOLSelectOrderSourceC10bCommand");
const FConsoleCommand HOLSelectClickSourceC10bConsole("hol.Select.ClickSource", n"HOLSelectClickSourceC10bCommand");
const FConsoleCommand HOLSelectHealC10bConsole("hol.Select.Heal", n"HOLSelectHealC10bCommand");
const FConsoleCommand HOLSelectAttackC10bConsole("hol.Select.Attack", n"HOLSelectAttackC10bCommand");
const FConsoleCommand HOLSelectOrderSourceNearC10bConsole("hol.Select.OrderSourceNear", n"HOLSelectOrderSourceNearC10bCommand");
const FConsoleCommand HOLUnitsOrderC10bConsole("hol.Units.Order", n"HOLUnitsOrderC10bCommand");
const FConsoleCommand HOLUnitsOrderNearC10bConsole("hol.Units.OrderNear", n"HOLUnitsOrderNearC10bCommand");
const FConsoleCommand HOLBuildPlaceNearC10bConsole("hol.Build.PlaceNear", n"HOLBuildPlaceNearC10bCommand");
const FConsoleCommand HOLBuildPlaceAtSourceC10bConsole("hol.Build.PlaceAtSource", n"HOLBuildPlaceAtSourceC10bCommand");
