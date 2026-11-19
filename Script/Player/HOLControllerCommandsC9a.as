// C9a (SAURON-1119ab, D851 step 5, UNCOMPILED): the "test pilot" console commands hol.Match.Pause / Resume, hol.Victory.Surrender, hol.Units.* (8) and hol.Prod.RallySite of AHOLPlayerController.cpp, now script.
// Same names `hol.*`, same arguments, same effect: each one does, for every LOCAL controller (GetLocalControllers of the C++), what the C++ lambda did (the controller's own Request<X> through HOLControllerCommand::, UHOLControllerCommandLibrary C9 section;
// their guards, the spectator check included, stay the C++ ones). Same journal lines (text, level, category LogHOLMatch): "Order: N units ..." (Display, what the rpc_orders_* scenarios wait for), "Usage: ..." and the "no ... known here" lines (Warning).
// The C++ `#if !UE_BUILD_SHIPPING` around hol.Units.* and hol.Prod.RallySite has no script twin (scripts are not loaded in a Shipping build, as the hol.Cheat.* of C1).
// Pattern of HOLControllerCommandsC2b.as; the numeric parser of C1 is reused (HOLControllerCommands::ParseReal).

struct FHOLConsolePlanC9a
{
	bool bValid = false;
	FString Usage;
	double X1 = 0.0;
	double Y1 = 0.0;
	double X2 = 0.0;
	double Y2 = 0.0;
	bool bShift = false;
	bool bWidth = false;
	FString Type;
}

namespace HOLControllerCommandsC9a
{
	TArray<FString> Names()
	{
		TArray<FString> Result;
		Result.Add("hol.Match.Pause"); Result.Add("hol.Match.Resume"); Result.Add("hol.Victory.Surrender");
		Result.Add("hol.Units.AttackMove"); Result.Add("hol.Units.OrderLine"); Result.Add("hol.Units.AttackBuilding");
		Result.Add("hol.Units.GatherOrder"); Result.Add("hol.Units.DropOff"); Result.Add("hol.Units.Build");
		Result.Add("hol.Units.Garrison"); Result.Add("hol.Units.UngarrisonAll"); Result.Add("hol.Prod.RallySite");
		return Result;
	}

	// HasProofFlag: a word equal to Flag (any case) at index From or after.
	bool HasFlag(const TArray<FString>& Args, int From, const FString& Flag)
	{
		for (int Index = From; Index < Args.Num(); Index++)
		{
			if (Args[Index].ToLower() == Flag)
				return true;
		}
		return false;
	}

	// "[Dx Dy] [shift]" of hol.Units.GatherOrder: the offset is given when two words come and neither is "shift".
	bool GatherHasOffset(const TArray<FString>& Args)
	{
		return Args.Num() >= 2 && Args[0].ToLower() != "shift" && Args[1].ToLower() != "shift";
	}

	FString Queued(bool bShift)
	{
		return bShift ? ", queued" : "";
	}

	// %.0f of the C++ lines: the cm rounded to the unit.
	int Cm(double Value)
	{
		return Math::RoundToInt(Value);
	}

	FString AttackMoveText(int Count, double X, double Y, bool bShift)
	{
		int Xi = Cm(X);
		int Yi = Cm(Y);
		return f"Order: {Count} units attack-move to ({Xi}, {Yi}){Queued(bShift)}";
	}

	FString OrderLineText(int Count, double X1, double Y1, double X2, double Y2, bool bWidth, bool bShift)
	{
		int X1i = Cm(X1);
		int Y1i = Cm(Y1);
		int X2i = Cm(X2);
		int Y2i = Cm(Y2);
		FString Width = bWidth ? ", its width" : "";
		return f"Order: {Count} units in line from ({X1i}, {Y1i}) to ({X2i}, {Y2i}){Width}{Queued(bShift)}";
	}

	FString GatherText(int Count, int Index, const FString& TypeName, double X, double Y, bool bShift)
	{
		int Xi = Cm(X);
		int Yi = Cm(Y);
		return f"Order: {Count} units gather source {Index} ({TypeName}) at ({Xi}, {Yi}){Queued(bShift)}";
	}

	FString RallySiteText(const FString& Producer, double X, double Y, const FString& Site, bool bShift)
	{
		int Xi = Cm(X);
		int Yi = Cm(Y);
		FString Step = bShift ? ", a step added" : "";
		return f"Order: rally point of {Producer} at ({Xi}, {Yi}) on site {Site}{Step}";
	}

	// The plan of one command by its name; bValid false with a Usage is a refusal (the line the C++ logged at Warning level), without it an unknown name.
	FHOLConsolePlanC9a PlanOf(const FString& Command, const TArray<FString>& Args)
	{
		FHOLConsolePlanC9a Plan;
		if (Command == "hol.Match.Pause" || Command == "hol.Match.Resume" || Command == "hol.Victory.Surrender")
		{
			Plan.bValid = true;
		}
		else if (Command == "hol.Units.AttackMove")
		{
			if (Args.Num() < 2)
			{
				Plan.Usage = "Usage: hol.Units.AttackMove <X> <Y> [shift]";
				return Plan;
			}
			Plan.bValid = true;
			Plan.X1 = HOLControllerCommands::ParseReal(Args[0]);
			Plan.Y1 = HOLControllerCommands::ParseReal(Args[1]);
			Plan.bShift = HasFlag(Args, 2, "shift");
		}
		else if (Command == "hol.Units.OrderLine")
		{
			if (Args.Num() < 4)
			{
				Plan.Usage = "Usage: hol.Units.OrderLine <X1> <Y1> <X2> <Y2> [ctrl] [shift]";
				return Plan;
			}
			Plan.bValid = true;
			Plan.X1 = HOLControllerCommands::ParseReal(Args[0]);
			Plan.Y1 = HOLControllerCommands::ParseReal(Args[1]);
			Plan.X2 = HOLControllerCommands::ParseReal(Args[2]);
			Plan.Y2 = HOLControllerCommands::ParseReal(Args[3]);
			Plan.bWidth = HasFlag(Args, 4, "ctrl");
			Plan.bShift = HasFlag(Args, 4, "shift");
		}
		else if (Command == "hol.Units.GatherOrder")
		{
			Plan.bValid = true;
			if (GatherHasOffset(Args))
			{
				Plan.X1 = HOLControllerCommands::ParseReal(Args[0]);
				Plan.Y1 = HOLControllerCommands::ParseReal(Args[1]);
			}
			Plan.bShift = HasFlag(Args, 0, "shift");
		}
		else if (Command == "hol.Units.AttackBuilding" || Command == "hol.Units.DropOff" || Command == "hol.Units.Build" || Command == "hol.Prod.RallySite")
		{
			Plan.bValid = true;
			Plan.bShift = HasFlag(Args, 0, "shift");
		}
		else if (Command == "hol.Units.Garrison" || Command == "hol.Units.UngarrisonAll")
		{
			Plan.bValid = true;
			Plan.Type = Args.Num() == 0 ? "" : Args[0];
		}
		return Plan;
	}

	// One controller does the plan (the body of the C++ lambda, per local controller).
	void DoForController(AHOLPlayerController Controller, const FString& Command, const FHOLConsolePlanC9a& Plan)
	{
		if (Command == "hol.Match.Pause")
		{
			HOLControllerCommand::RequestMatchPause(Controller);
			return;
		}
		if (Command == "hol.Match.Resume")
		{
			HOLControllerCommand::RequestMatchResume(Controller);
			return;
		}
		if (Command == "hol.Victory.Surrender")
		{
			HOLControllerCommand::RequestMatchSurrender(Controller);
			return;
		}
		// hol.Prod.RallySite: the producer and the site first (both lines are Warning when one is missing).
		if (Command == "hol.Prod.RallySite")
		{
			AHOLBuilding Producer = HOLControllerCommand::FindOwnBuildingByType(Controller, n"MainCentre");
			AHOLBuilding Site = HOLControllerCommand::FindOwnSite(Controller);
			if (Producer == nullptr || Site == nullptr)
			{
				FString Why = Producer == nullptr ? "no own main centre known here" : "no own site (building not finished) known here";
				HOLScriptLog::LogWarning(n"LogHOLMatch", f"hol.Prod.RallySite: {Why}");
				return;
			}
			FVector Point = Site.GetActorLocation();
			HOLScriptLog::LogDisplay(n"LogHOLMatch", RallySiteText(HOLControllerCommand::DescribeBuildingOf(Controller, Producer), Point.X, Point.Y, HOLControllerCommand::DescribeBuildingOf(Controller, Site), Plan.bShift));
			HOLControllerCommand::RequestRallyOnSite(Controller, Producer, Site, Plan.bShift);
			return;
		}

		// hol.Units.*: the target first (a Warning and no order when it is missing), then the line, then the request.
		TArray<int> Ids;
		if (Command == "hol.Units.AttackMove")
		{
			HOLControllerCommand::OwnUnitIds(Controller, Ids);
			HOLScriptLog::LogDisplay(n"LogHOLMatch", AttackMoveText(Ids.Num(), Plan.X1, Plan.Y1, Plan.bShift));
			HOLControllerCommand::RequestAttackMoveUnits(Controller, Ids, Plan.X1, Plan.Y1, Plan.bShift);
		}
		else if (Command == "hol.Units.OrderLine")
		{
			HOLControllerCommand::OwnUnitIds(Controller, Ids);
			HOLScriptLog::LogDisplay(n"LogHOLMatch", OrderLineText(Ids.Num(), Plan.X1, Plan.Y1, Plan.X2, Plan.Y2, Plan.bWidth, Plan.bShift));
			HOLControllerCommand::RequestMoveUnitsInLineOf(Controller, Ids, Plan.X1, Plan.Y1, Plan.X2, Plan.Y2, Plan.bWidth, Plan.bShift);
		}
		else if (Command == "hol.Units.AttackBuilding")
		{
			AHOLBuilding Enemy = HOLControllerCommand::FindMainCentreOf(Controller, true);
			if (Enemy == nullptr)
			{
				HOLScriptLog::LogWarning(n"LogHOLMatch", "hol.Units.AttackBuilding: no enemy main centre known here");
				return;
			}
			HOLControllerCommand::OwnUnitIds(Controller, Ids);
			HOLScriptLog::LogDisplay(n"LogHOLMatch", f"Order: {Ids.Num()} units attack {HOLControllerCommand::DescribeBuildingOf(Controller, Enemy)}{Queued(Plan.bShift)}");
			HOLControllerCommand::RequestAttackBuildingUnits(Controller, Ids, Enemy, Plan.bShift);
		}
		else if (Command == "hol.Units.GatherOrder")
		{
			int Index = -1;
			FString TypeName;
			double X = 0.0;
			double Y = 0.0;
			int Found = HOLControllerCommand::FindGatherSource(Controller, Plan.X1, Plan.Y1, Index, TypeName, X, Y);
			if (Found == 1)
			{
				HOLScriptLog::LogWarning(n"LogHOLMatch", "hol.Units.GatherOrder: no start slot or no source subsystem here");
				return;
			}
			if (Found == 2)
			{
				HOLScriptLog::LogWarning(n"LogHOLMatch", "hol.Units.GatherOrder: no standing source");
				return;
			}
			HOLControllerCommand::OwnUnitIds(Controller, Ids);
			HOLScriptLog::LogDisplay(n"LogHOLMatch", GatherText(Ids.Num(), Index, TypeName, X, Y, Plan.bShift));
			HOLControllerCommand::RequestGatherSourceUnits(Controller, Ids, Index, Plan.bShift);
		}
		else if (Command == "hol.Units.DropOff")
		{
			AHOLBuilding Centre = HOLControllerCommand::FindMainCentreOf(Controller, false);
			if (Centre == nullptr)
			{
				HOLScriptLog::LogWarning(n"LogHOLMatch", "hol.Units.DropOff: no own main centre known here");
				return;
			}
			HOLControllerCommand::OwnUnitIds(Controller, Ids);
			HOLScriptLog::LogDisplay(n"LogHOLMatch", f"Order: {Ids.Num()} units drop off at {HOLControllerCommand::DescribeBuildingOf(Controller, Centre)}{Queued(Plan.bShift)}");
			HOLControllerCommand::RequestDropOffUnitsAt(Controller, Ids, Centre, Plan.bShift);
		}
		else if (Command == "hol.Units.Build")
		{
			AHOLBuilding Site = HOLControllerCommand::FindOwnSite(Controller);
			if (Site == nullptr)
			{
				HOLScriptLog::LogWarning(n"LogHOLMatch", "hol.Units.Build: no own building waiting for builders known here");
				return;
			}
			HOLControllerCommand::OwnUnitIds(Controller, Ids);
			HOLScriptLog::LogDisplay(n"LogHOLMatch", f"Order: {Ids.Num()} units build {HOLControllerCommand::DescribeBuildingOf(Controller, Site)}{Queued(Plan.bShift)}");
			HOLControllerCommand::RequestBuildSiteUnits(Controller, Ids, Site, Plan.bShift);
		}
		else if (Command == "hol.Units.Garrison" || Command == "hol.Units.UngarrisonAll")
		{
			AHOLBuilding Garrison = HOLControllerCommand::FindOwnGarrison(Controller, FName(Plan.Type));
			if (Garrison == nullptr)
			{
				HOLScriptLog::LogWarning(n"LogHOLMatch", f"{Command}: no own finished building with places known here");
				return;
			}
			if (Command == "hol.Units.Garrison")
			{
				HOLControllerCommand::OwnUnitIds(Controller, Ids);
				HOLScriptLog::LogDisplay(n"LogHOLMatch", f"Order: {Ids.Num()} units garrison {HOLControllerCommand::DescribeBuildingOf(Controller, Garrison)}");
				HOLControllerCommand::RequestGarrisonUnitsIn(Controller, Ids, Garrison);
			}
			else
			{
				HOLScriptLog::LogDisplay(n"LogHOLMatch", f"Order: ungarrison all of {HOLControllerCommand::DescribeBuildingOf(Controller, Garrison)}");
				HOLControllerCommand::RequestUngarrisonAllOf(Controller, Garrison);
			}
		}
	}

	// The command as typed: refused arguments log their usage line (Warning), else it runs for every local controller (GetLocalControllers of the C++).
	void Run(const FString& Command, const TArray<FString>& Args)
	{
		const FHOLConsolePlanC9a Plan = PlanOf(Command, Args);
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

void HOLMatchPauseCommand(const TArray<FString>& Args) { HOLControllerCommandsC9a::Run("hol.Match.Pause", Args); }
void HOLMatchResumeCommand(const TArray<FString>& Args) { HOLControllerCommandsC9a::Run("hol.Match.Resume", Args); }
void HOLVictorySurrenderCommand(const TArray<FString>& Args) { HOLControllerCommandsC9a::Run("hol.Victory.Surrender", Args); }
void HOLUnitsAttackMoveCommand(const TArray<FString>& Args) { HOLControllerCommandsC9a::Run("hol.Units.AttackMove", Args); }
void HOLUnitsOrderLineCommand(const TArray<FString>& Args) { HOLControllerCommandsC9a::Run("hol.Units.OrderLine", Args); }
void HOLUnitsAttackBuildingCommand(const TArray<FString>& Args) { HOLControllerCommandsC9a::Run("hol.Units.AttackBuilding", Args); }
void HOLUnitsGatherOrderCommand(const TArray<FString>& Args) { HOLControllerCommandsC9a::Run("hol.Units.GatherOrder", Args); }
void HOLUnitsDropOffCommand(const TArray<FString>& Args) { HOLControllerCommandsC9a::Run("hol.Units.DropOff", Args); }
void HOLUnitsBuildCommand(const TArray<FString>& Args) { HOLControllerCommandsC9a::Run("hol.Units.Build", Args); }
void HOLUnitsGarrisonCommand(const TArray<FString>& Args) { HOLControllerCommandsC9a::Run("hol.Units.Garrison", Args); }
void HOLUnitsUngarrisonAllCommand(const TArray<FString>& Args) { HOLControllerCommandsC9a::Run("hol.Units.UngarrisonAll", Args); }
void HOLProdRallySiteCommand(const TArray<FString>& Args) { HOLControllerCommandsC9a::Run("hol.Prod.RallySite", Args); }

const FConsoleCommand HOLMatchPauseConsole("hol.Match.Pause", n"HOLMatchPauseCommand");
const FConsoleCommand HOLMatchResumeConsole("hol.Match.Resume", n"HOLMatchResumeCommand");
const FConsoleCommand HOLVictorySurrenderConsole("hol.Victory.Surrender", n"HOLVictorySurrenderCommand");
const FConsoleCommand HOLUnitsAttackMoveConsole("hol.Units.AttackMove", n"HOLUnitsAttackMoveCommand");
const FConsoleCommand HOLUnitsOrderLineConsole("hol.Units.OrderLine", n"HOLUnitsOrderLineCommand");
const FConsoleCommand HOLUnitsAttackBuildingConsole("hol.Units.AttackBuilding", n"HOLUnitsAttackBuildingCommand");
const FConsoleCommand HOLUnitsGatherOrderConsole("hol.Units.GatherOrder", n"HOLUnitsGatherOrderCommand");
const FConsoleCommand HOLUnitsDropOffConsole("hol.Units.DropOff", n"HOLUnitsDropOffCommand");
const FConsoleCommand HOLUnitsBuildConsole("hol.Units.Build", n"HOLUnitsBuildCommand");
const FConsoleCommand HOLUnitsGarrisonConsole("hol.Units.Garrison", n"HOLUnitsGarrisonCommand");
const FConsoleCommand HOLUnitsUngarrisonAllConsole("hol.Units.UngarrisonAll", n"HOLUnitsUngarrisonAllCommand");
const FConsoleCommand HOLProdRallySiteConsole("hol.Prod.RallySite", n"HOLProdRallySiteCommand");
