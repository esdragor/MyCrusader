// C2b (SAURON-1119v, D851 step 5): the console commands that stayed in AHOLPlayerController.cpp after C2 (hero rage, market trade, tribute, production rally / queue / cancel, the reports, the forced diplomacy,
// the tribute notice), now script. Same names `hol.*`, same arguments, same effect, same journal lines (text, level and category LogHOLMatch):
// the controller's own Request<X> are called through HOLControllerCommand:: (UHOLControllerCommandLibrary, SAURON-1119u: their guards, spectator check and selection reads stay the C++ ones), the lines are written with
// HOLScriptLog:: (Display for the reports, Warning for the "Usage" / "unknown resource" lines, as the C++ UE_LOG).
// Same pattern as HOLControllerCommandsC2.as; the numeric parsers of C1 are reused (HOLControllerCommands::ParseInt / ParseReal), the stance word too (HOLControllerCommandsC2::StanceOf).

struct FHOLConsolePlanC2b
{
	bool bValid = false;
	FString Route;
	TArray<int> Ints;
	FString Name;
	FString Building;
	double X = 0.0;
	double Y = 0.0;
	FString Usage;
}

namespace HOLControllerCommandsC2b
{
	TArray<FString> Names()
	{
		TArray<FString> Result;
		Result.Add("hol.Hero.Rage");
		Result.Add("hol.Market.Trade"); Result.Add("hol.Tribute"); Result.Add("hol.Tribute.Notice");
		Result.Add("hol.Prod.Rally"); Result.Add("hol.Prod.Queue"); Result.Add("hol.Prod.Cancel");
		Result.Add("hol.Signal.Report"); Result.Add("hol.Chat.Report"); Result.Add("hol.Spec.Report");
		Result.Add("hol.Prod.Report"); Result.Add("hol.Market.Report");
		Result.Add("hol.Diplomacy.Force"); Result.Add("hol.Diplomacy.ForceVision"); Result.Add("hol.Diplomacy.Report");
		return Result;
	}

	FHOLConsolePlanC2b Refused(const FString& Usage)
	{
		FHOLConsolePlanC2b Plan;
		Plan.Usage = Usage;
		return Plan;
	}

	FHOLConsolePlanC2b Accepted(const FString& Route)
	{
		FHOLConsolePlanC2b Plan;
		Plan.bValid = true;
		Plan.Route = Route;
		return Plan;
	}

	// food 0, wood 1, stone 2, gold 3 (EHOLResource order), the name compared in lowercase; -1 (INDEX_NONE) for any other word.
	int ResourceOf(const FString& Name)
	{
		const FString Lower = Name.ToLower();
		if (Lower == "food")
			return 0;
		if (Lower == "wood")
			return 1;
		if (Lower == "stone")
			return 2;
		if (Lower == "gold")
			return 3;
		return -1;
	}

	// The plan of one command by its name; bValid false with Usage empty is an unknown command (or arguments the C++ dropped in silence), with a Usage the line the C++ command logged at Warning level.
	FHOLConsolePlanC2b PlanOf(const FString& Command, const TArray<FString>& Args)
	{
		if (Command == "hol.Hero.Rage")
			return Accepted("Rage");
		if (Command == "hol.Market.Trade")
		{
			if (Args.Num() < 3)
				return Refused("Usage: hol.Market.Trade <food|wood|stone> <Lots> <buy|sell>");
			const int Resource = ResourceOf(Args[0]);
			// Gold is not tradable: the C++ knew food, wood and stone only.
			if (Resource == -1 || Resource == 3)
				return Refused(f"hol.Market.Trade: unknown resource {Args[0]}");
			FHOLConsolePlanC2b Plan = Accepted("MarketTrade");
			Plan.Ints.Add(Resource);
			Plan.Ints.Add(HOLControllerCommands::ParseInt(Args[1]));
			Plan.Ints.Add(Args[2].ToLower() == "buy" ? 1 : 0);
			return Plan;
		}
		if (Command == "hol.Tribute")
		{
			if (Args.Num() < 3)
				return Refused("Usage: hol.Tribute <Slot> <food|wood|stone|gold> <Amount>");
			const int Resource = ResourceOf(Args[1]);
			if (Resource == -1)
				return Refused(f"hol.Tribute: unknown resource {Args[1]}");
			FHOLConsolePlanC2b Plan = Accepted("Tribute");
			Plan.Ints.Add(HOLControllerCommands::ParseInt(Args[0]));
			Plan.Ints.Add(Resource);
			Plan.Ints.Add(HOLControllerCommands::ParseInt(Args[2]));
			return Plan;
		}
		if (Command == "hol.Tribute.Notice")
		{
			if (Args.Num() < 4)
				return Refused("Usage: hol.Tribute.Notice <GiverSlot> <food|wood|stone|gold> <Arrives> <Tax> (in a match)");
			FHOLConsolePlanC2b Plan = Accepted("TributeNotice");
			Plan.Ints.Add(HOLControllerCommands::ParseInt(Args[0]));
			// An unknown word is gold here (the C++ chain ended on 3).
			const int Resource = ResourceOf(Args[1]);
			Plan.Ints.Add(Resource == -1 ? 3 : Resource);
			Plan.Ints.Add(HOLControllerCommands::ParseInt(Args[2]));
			Plan.Ints.Add(HOLControllerCommands::ParseInt(Args[3]));
			return Plan;
		}
		if (Command == "hol.Prod.Rally")
		{
			// Fewer than two numbers: the C++ returned without a line.
			if (Args.Num() < 2)
				return FHOLConsolePlanC2b();
			FHOLConsolePlanC2b Plan = Accepted("Rally");
			Plan.X = HOLControllerCommands::ParseReal(Args[0]);
			Plan.Y = HOLControllerCommands::ParseReal(Args[1]);
			return Plan;
		}
		if (Command == "hol.Prod.Queue")
		{
			FHOLConsolePlanC2b Plan = Accepted("Queue");
			Plan.Name = Args.Num() == 0 ? "Peasant" : Args[0];
			Plan.Building = Args.Num() >= 2 ? Args[1] : "";
			return Plan;
		}
		if (Command == "hol.Prod.Cancel")
		{
			FHOLConsolePlanC2b Plan = Accepted("Cancel");
			Plan.Ints.Add(Args.Num() == 0 ? 0 : HOLControllerCommands::ParseInt(Args[0]));
			return Plan;
		}
		if (Command == "hol.Signal.Report")
			return Accepted("SignalReport");
		if (Command == "hol.Chat.Report")
			return Accepted("ChatReport");
		if (Command == "hol.Spec.Report")
			return Accepted("SpecReport");
		if (Command == "hol.Prod.Report")
			return Accepted("ProdReport");
		if (Command == "hol.Market.Report")
			return Accepted("MarketReport");
		if (Command == "hol.Diplomacy.Report")
			return Accepted("DiplomacyReport");
		if (Command == "hol.Diplomacy.Force")
		{
			if (Args.Num() < 3)
				return Refused("Usage: hol.Diplomacy.Force <FromSlot> <ToSlot> <ally|neutral|enemy> (in a match, on the server)");
			FHOLConsolePlanC2b Plan = Accepted("ForceStance");
			Plan.Ints.Add(HOLControllerCommands::ParseInt(Args[0]));
			Plan.Ints.Add(HOLControllerCommands::ParseInt(Args[1]));
			Plan.Ints.Add(HOLControllerCommandsC2::StanceOf(Args[2]));
			return Plan;
		}
		if (Command == "hol.Diplomacy.ForceVision")
		{
			if (Args.Num() < 3)
				return Refused("Usage: hol.Diplomacy.ForceVision <FromSlot> <ToSlot> <0|1> (in a match, on the server)");
			FHOLConsolePlanC2b Plan = Accepted("ForceVision");
			Plan.Ints.Add(HOLControllerCommands::ParseInt(Args[0]));
			Plan.Ints.Add(HOLControllerCommands::ParseInt(Args[1]));
			Plan.Ints.Add(HOLControllerCommands::ParseInt(Args[2]) != 0 ? 1 : 0);
			return Plan;
		}
		return FHOLConsolePlanC2b();
	}

	// One controller does the plan (the body of the C++ lambda, per local controller).
	void DoForController(AHOLPlayerController Controller, const FHOLConsolePlanC2b& Plan)
	{
		if (Plan.Route == "Rage")
			HOLControllerCommand::RequestCastRageOfSelection(Controller);
		else if (Plan.Route == "MarketTrade")
		{
			HOLControllerCommand::SelectOwnBuilding(Controller, n"Market");
			HOLControllerCommand::RequestMarketTrade(Controller, Plan.Ints[0], Plan.Ints[1], Plan.Ints[2] != 0);
		}
		else if (Plan.Route == "Tribute")
		{
			HOLControllerCommand::SelectOwnBuilding(Controller, n"Market");
			HOLControllerCommand::RequestTribute(Controller, Plan.Ints[0], Plan.Ints[1], Plan.Ints[2]);
		}
		else if (Plan.Route == "Rally")
		{
			HOLControllerCommand::EnsureOwnSelection(Controller, HOLCentreKind::OwnedCentreTypeOf(Controller));
			HOLControllerCommand::RequestRallyPointAt(Controller, Plan.X, Plan.Y);
		}
		else if (Plan.Route == "Queue")
		{
			// J7a.6: [Building], its own building of that type selected first (the net part's barracks).
			if (!Plan.Building.IsEmpty())
				HOLControllerCommand::SelectOwnBuilding(Controller, FName(Plan.Building));
			HOLControllerCommand::EnsureOwnSelection(Controller, HOLCentreKind::OwnedCentreTypeOf(Controller));
			HOLControllerCommand::RequestProduction(Controller, FName(Plan.Name));
		}
		else if (Plan.Route == "Cancel")
			HOLControllerCommand::RequestCancelProduction(Controller, Plan.Ints[0]);
		else if (Plan.Route == "SignalReport" || Plan.Route == "ChatReport")
		{
			int Signals = 0;
			int Chats = 0;
			int Slot = -1;
			if (!HOLControllerCommand::ReceivedCounts(Controller, Signals, Chats, Slot))
				return;
			if (Plan.Route == "SignalReport")
				HOLScriptLog::LogDisplay(n"LogHOLMatch", f"Signal report: slot {Slot} received {Signals}");
			else
				HOLScriptLog::LogDisplay(n"LogHOLMatch", f"Chat report: slot {Slot} received {Chats}");
		}
		else if (Plan.Route == "SpecReport")
		{
			TArray<FString> Lines = HOLControllerCommand::DescribeSpecializationStates(Controller);
			for (const FString& Line : Lines)
				HOLScriptLog::LogDisplay(n"LogHOLMatch", Line);
		}
		else if (Plan.Route == "ProdReport")
		{
			const FString Line = HOLControllerCommand::DescribeProductionReport(Controller);
			if (!Line.IsEmpty())
				HOLScriptLog::LogDisplay(n"LogHOLMatch", Line);
		}
	}

	// A command of the world (no controller loop): the C++ FAutoConsoleCommandWithWorld(AndArgs) ones.
	void DoInWorld(const FHOLConsolePlanC2b& Plan)
	{
		if (Plan.Route == "MarketReport")
		{
			TArray<FString> Lines = HOLControllerCommand::DescribeMarketReport();
			for (const FString& Line : Lines)
				HOLScriptLog::LogDisplay(n"LogHOLMatch", Line);
		}
		else if (Plan.Route == "DiplomacyReport")
			HOLControllerCommand::ReportDiplomacy();
		else if (Plan.Route == "ForceStance")
		{
			if (!HOLControllerCommand::ForceStance(Plan.Ints[0], Plan.Ints[1], Plan.Ints[2]))
				HOLScriptLog::LogWarning(n"LogHOLMatch", "Usage: hol.Diplomacy.Force <FromSlot> <ToSlot> <ally|neutral|enemy> (in a match, on the server)");
		}
		else if (Plan.Route == "ForceVision")
		{
			if (!HOLControllerCommand::ForceVision(Plan.Ints[0], Plan.Ints[1], Plan.Ints[2] != 0))
				HOLScriptLog::LogWarning(n"LogHOLMatch", "Usage: hol.Diplomacy.ForceVision <FromSlot> <ToSlot> <0|1> (in a match, on the server)");
		}
		else if (Plan.Route == "TributeNotice")
		{
			if (!HOLControllerCommand::NoteTributeNotice(Plan.Ints[0], Plan.Ints[1], Plan.Ints[2], Plan.Ints[3]))
				HOLScriptLog::LogWarning(n"LogHOLMatch", "Usage: hol.Tribute.Notice <GiverSlot> <food|wood|stone|gold> <Arrives> <Tax> (in a match)");
		}
	}

	bool IsWorldRoute(const FString& Route)
	{
		return Route == "MarketReport" || Route == "DiplomacyReport" || Route == "ForceStance" || Route == "ForceVision" || Route == "TributeNotice";
	}

	// The command as typed: refused arguments log their usage line (Warning), else the world commands run once and the others for every local controller (GetLocalControllers of the C++).
	void Run(const FString& Command, const TArray<FString>& Args)
	{
		const FHOLConsolePlanC2b Plan = PlanOf(Command, Args);
		if (!Plan.bValid)
		{
			if (!Plan.Usage.IsEmpty())
				HOLScriptLog::LogWarning(n"LogHOLMatch", Plan.Usage);
			return;
		}
		if (IsWorldRoute(Plan.Route))
		{
			DoInWorld(Plan);
			return;
		}
		TArray<AHOLPlayerController> Controllers;
		GetAllActorsOfClass(Controllers);
		for (AHOLPlayerController Controller : Controllers)
		{
			if (Controller.IsLocalController())
				DoForController(Controller, Plan);
		}
	}
}

void HOLHeroRageCommand(const TArray<FString>& Args) { HOLControllerCommandsC2b::Run("hol.Hero.Rage", Args); }
void HOLMarketTradeCommand(const TArray<FString>& Args) { HOLControllerCommandsC2b::Run("hol.Market.Trade", Args); }
void HOLTributeCommand(const TArray<FString>& Args) { HOLControllerCommandsC2b::Run("hol.Tribute", Args); }
void HOLTributeNoticeCommand(const TArray<FString>& Args) { HOLControllerCommandsC2b::Run("hol.Tribute.Notice", Args); }
void HOLProdRallyCommand(const TArray<FString>& Args) { HOLControllerCommandsC2b::Run("hol.Prod.Rally", Args); }
void HOLProdQueueCommand(const TArray<FString>& Args) { HOLControllerCommandsC2b::Run("hol.Prod.Queue", Args); }
void HOLProdCancelCommand(const TArray<FString>& Args) { HOLControllerCommandsC2b::Run("hol.Prod.Cancel", Args); }
void HOLSignalReportCommand(const TArray<FString>& Args) { HOLControllerCommandsC2b::Run("hol.Signal.Report", Args); }
void HOLChatReportCommand(const TArray<FString>& Args) { HOLControllerCommandsC2b::Run("hol.Chat.Report", Args); }
void HOLSpecReportCommand(const TArray<FString>& Args) { HOLControllerCommandsC2b::Run("hol.Spec.Report", Args); }
void HOLProdReportCommand(const TArray<FString>& Args) { HOLControllerCommandsC2b::Run("hol.Prod.Report", Args); }
void HOLMarketReportCommand(const TArray<FString>& Args) { HOLControllerCommandsC2b::Run("hol.Market.Report", Args); }
void HOLDiplomacyForceCommand(const TArray<FString>& Args) { HOLControllerCommandsC2b::Run("hol.Diplomacy.Force", Args); }
void HOLDiplomacyForceVisionCommand(const TArray<FString>& Args) { HOLControllerCommandsC2b::Run("hol.Diplomacy.ForceVision", Args); }
void HOLDiplomacyReportCommand(const TArray<FString>& Args) { HOLControllerCommandsC2b::Run("hol.Diplomacy.Report", Args); }

const FConsoleCommand HOLHeroRageConsole("hol.Hero.Rage", n"HOLHeroRageCommand");
const FConsoleCommand HOLMarketTradeConsole("hol.Market.Trade", n"HOLMarketTradeCommand");
const FConsoleCommand HOLTributeConsole("hol.Tribute", n"HOLTributeCommand");
const FConsoleCommand HOLTributeNoticeConsole("hol.Tribute.Notice", n"HOLTributeNoticeCommand");
const FConsoleCommand HOLProdRallyConsole("hol.Prod.Rally", n"HOLProdRallyCommand");
const FConsoleCommand HOLProdQueueConsole("hol.Prod.Queue", n"HOLProdQueueCommand");
const FConsoleCommand HOLProdCancelConsole("hol.Prod.Cancel", n"HOLProdCancelCommand");
const FConsoleCommand HOLSignalReportConsole("hol.Signal.Report", n"HOLSignalReportCommand");
const FConsoleCommand HOLChatReportConsole("hol.Chat.Report", n"HOLChatReportCommand");
const FConsoleCommand HOLSpecReportConsole("hol.Spec.Report", n"HOLSpecReportCommand");
const FConsoleCommand HOLProdReportConsole("hol.Prod.Report", n"HOLProdReportCommand");
const FConsoleCommand HOLMarketReportConsole("hol.Market.Report", n"HOLMarketReportCommand");
const FConsoleCommand HOLDiplomacyForceConsole("hol.Diplomacy.Force", n"HOLDiplomacyForceCommand");
const FConsoleCommand HOLDiplomacyForceVisionConsole("hol.Diplomacy.ForceVision", n"HOLDiplomacyForceVisionCommand");
const FConsoleCommand HOLDiplomacyReportConsole("hol.Diplomacy.Report", n"HOLDiplomacyReportCommand");
