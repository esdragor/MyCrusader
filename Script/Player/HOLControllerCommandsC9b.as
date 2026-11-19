// C9b (SAURON-1119ab, D851 step 5, UNCOMPILED): hol.Select.Stop / Hold / NextIdle / Own / Type and hol.Build.CancelSelected of AHOLPlayerController.cpp, now script. Same names, same arguments, same effect and journal lines
// ("Select own: N units", "Select type T: N units" at Display, "Usage: hol.Select.Type <Type>" at Warning, category LogHOLMatch), for every LOCAL controller; the controller's own calls through HOLControllerCommand:: (C9 section of UHOLControllerCommandLibrary).
// hol.Select.Order and hol.Select.OrderLine were REMOVED (0 use in Tools\ and Script\, no other command called them): not ported.
// Pattern of HOLControllerCommandsC2b.as.

struct FHOLConsolePlanC9b
{
	bool bValid = false;
	FString Usage;
	FString Type;
	bool bLast = false;
}

namespace HOLControllerCommandsC9b
{
	TArray<FString> Names()
	{
		TArray<FString> Result;
		Result.Add("hol.Select.Stop"); Result.Add("hol.Select.Hold"); Result.Add("hol.Select.NextIdle");
		Result.Add("hol.Select.Own"); Result.Add("hol.Select.Type"); Result.Add("hol.Build.CancelSelected");
		return Result;
	}

	// hol.Select.Type <Type> [last]: the plan; no argument is the C++ Usage line (Warning).
	FHOLConsolePlanC9b PlanOf(const FString& Command, const TArray<FString>& Args)
	{
		FHOLConsolePlanC9b Plan;
		if (Command == "hol.Select.Type")
		{
			if (Args.Num() == 0)
			{
				Plan.Usage = "Usage: hol.Select.Type <Type>";
				return Plan;
			}
			Plan.bValid = true;
			Plan.Type = Args[0];
			// `last` is compared as typed (the C++ used ==, case sensitive).
			Plan.bLast = Args.Num() > 1 && Args[1] == "last";
		}
		else if (Command == "hol.Select.Stop" || Command == "hol.Select.Hold" || Command == "hol.Select.NextIdle" || Command == "hol.Select.Own" || Command == "hol.Build.CancelSelected")
		{
			Plan.bValid = true;
		}
		return Plan;
	}

	// The decision of `last`: the newest (highest) id alone when there are two or more, else the list as it is. Ids come sorted (OwnUnitIdsOfType).
	TArray<int> KeepLast(const TArray<int>& Ids, bool bLast)
	{
		TArray<int> Result = Ids;
		if (bLast && Ids.Num() > 1)
		{
			Result.Reset();
			Result.Add(Ids[Ids.Num() - 1]);
		}
		return Result;
	}

	FString OwnText(int Count)
	{
		return f"Select own: {Count} units";
	}

	FString TypeText(const FString& Type, int Count)
	{
		return f"Select type {Type}: {Count} units";
	}

	void DoForController(AHOLPlayerController Controller, const FString& Command, const FHOLConsolePlanC9b& Plan)
	{
		if (Command == "hol.Select.Stop")
			HOLControllerCommand::PressStopOrHold(Controller, false);
		else if (Command == "hol.Select.Hold")
			HOLControllerCommand::PressStopOrHold(Controller, true);
		else if (Command == "hol.Select.NextIdle")
			HOLControllerCommand::SelectNextIdleWorkerOf(Controller);
		else if (Command == "hol.Build.CancelSelected")
			HOLControllerCommand::RequestCancelSelectedBuilding(Controller);
		else if (Command == "hol.Select.Own")
		{
			TArray<int> Ids;
			HOLControllerCommand::OwnUnitIds(Controller, Ids);
			// Logged even when they already were selected (the selection then logs nothing).
			HOLScriptLog::LogDisplay(n"LogHOLMatch", OwnText(Ids.Num()));
			HOLControllerCommand::SelectUnitIds(Controller, Ids);
		}
		else if (Command == "hol.Select.Type")
		{
			TArray<int> All;
			HOLControllerCommand::OwnUnitIdsOfType(Controller, FName(Plan.Type), All);
			TArray<int> Ids = KeepLast(All, Plan.bLast);
			HOLScriptLog::LogDisplay(n"LogHOLMatch", TypeText(Plan.Type, Ids.Num()));
			HOLControllerCommand::SelectUnitIds(Controller, Ids);
		}
	}

	void Run(const FString& Command, const TArray<FString>& Args)
	{
		const FHOLConsolePlanC9b Plan = PlanOf(Command, Args);
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

void HOLSelectStopCommand(const TArray<FString>& Args) { HOLControllerCommandsC9b::Run("hol.Select.Stop", Args); }
void HOLSelectHoldCommand(const TArray<FString>& Args) { HOLControllerCommandsC9b::Run("hol.Select.Hold", Args); }
void HOLSelectNextIdleCommand(const TArray<FString>& Args) { HOLControllerCommandsC9b::Run("hol.Select.NextIdle", Args); }
void HOLSelectOwnCommand(const TArray<FString>& Args) { HOLControllerCommandsC9b::Run("hol.Select.Own", Args); }
void HOLSelectTypeCommand(const TArray<FString>& Args) { HOLControllerCommandsC9b::Run("hol.Select.Type", Args); }
void HOLBuildCancelSelectedCommand(const TArray<FString>& Args) { HOLControllerCommandsC9b::Run("hol.Build.CancelSelected", Args); }

const FConsoleCommand HOLSelectStopConsole("hol.Select.Stop", n"HOLSelectStopCommand");
const FConsoleCommand HOLSelectHoldConsole("hol.Select.Hold", n"HOLSelectHoldCommand");
const FConsoleCommand HOLSelectNextIdleConsole("hol.Select.NextIdle", n"HOLSelectNextIdleCommand");
const FConsoleCommand HOLSelectOwnConsole("hol.Select.Own", n"HOLSelectOwnCommand");
const FConsoleCommand HOLSelectTypeConsole("hol.Select.Type", n"HOLSelectTypeCommand");
const FConsoleCommand HOLBuildCancelSelectedConsole("hol.Build.CancelSelected", n"HOLBuildCancelSelectedCommand");
