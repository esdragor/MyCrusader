// C10a (SAURON-1119ba, D851, UNCOMPILED): hol.Select.Click / ClickUnit / Box / BoxGround / OrderGround / OrderLineGround / ClickGround / OrderNear of AHOLPlayerController.cpp, now script.
// Same names, same arguments, same journal lines at the same level (category LogHOLMatch; Display for the click / box lines, Warning for the Usage and "not on the screen" lines),
// for every LOCAL controller. The projection of a ground point and its on-screen rule stay ONE C++ function (UHOLControllerCommandLibrary::ProjectGroundFraction, which the
// controller's own ProjectGroundPoint now calls too): the script does not know the viewport. The %.3f of the C++ lines is Fixed3 (integer rounding), the %.0f is Math::RoundToInt.
// Pattern of HOLControllerCommandsC9b.as; the numeric parser of C1 is reused (HOLControllerCommands::ParseReal).

struct FHOLConsolePlanC10a
{
	bool bValid = false;
	FString Usage;
	double X1 = 0.0;
	double Y1 = 0.0;
	double X2 = 0.0;
	double Y2 = 0.0;
	bool bAdd = false;
	bool bDouble = false;
	bool bOrder = false;
	bool bEnemy = false;
	bool bAlly = false;
	bool bShift = false;
	bool bCtrl = false;
	bool bHasPoint = false;
	int Slot = -1;
	int Wanted = 0;
}

struct FHOLFractionBoxC10a
{
	double MinX = 0.0;
	double MinY = 0.0;
	double MaxX = 0.0;
	double MaxY = 0.0;
}

namespace HOLControllerCommandsC10a
{
	TArray<FString> Names()
	{
		TArray<FString> Result;
		Result.Add("hol.Select.Click"); Result.Add("hol.Select.ClickUnit"); Result.Add("hol.Select.Box");
		Result.Add("hol.Select.BoxGround"); Result.Add("hol.Select.OrderGround"); Result.Add("hol.Select.OrderLineGround");
		Result.Add("hol.Select.ClickGround"); Result.Add("hol.Select.OrderNear");
		return Result;
	}

	// A word among the arguments after the four numbers of OrderLineGround ("ctrl", "shift"), case insensitive as the C++ HasLineFlag.
	bool HasLineFlag(const TArray<FString>& Args, const FString& Flag)
	{
		for (int Index = 4; Index < Args.Num(); Index++)
		{
			if (Args[Index].ToLower() == Flag)
				return true;
		}
		return false;
	}

	FHOLConsolePlanC10a PlanOf(const FString& Command, const TArray<FString>& Args)
	{
		FHOLConsolePlanC10a Plan;
		if (Command == "hol.Select.Click" || Command == "hol.Select.ClickGround")
		{
			if (Args.Num() < 2)
			{
				Plan.Usage = f"Usage: {Command} <X> <Y> [add]";
				return Plan;
			}
			Plan.X1 = HOLControllerCommands::ParseReal(Args[0]);
			Plan.Y1 = HOLControllerCommands::ParseReal(Args[1]);
			if (Command == "hol.Select.Click")
			{
				Plan.bAdd = Args.Contains("add");
				Plan.bDouble = Args.Contains("double");
			}
			else
				Plan.bAdd = Args.Num() >= 3 && Args[2] == "add";
			Plan.bValid = true;
		}
		else if (Command == "hol.Select.ClickUnit")
		{
			// No Usage line: no argument is unit 0.
			Plan.Wanted = Args.IsEmpty() ? 0 : int(HOLControllerCommands::ParseReal(Args[0]));
			Plan.bAdd = Args.Contains("add");
			Plan.bDouble = Args.Contains("double");
			Plan.bOrder = Args.Contains("order");
			Plan.bEnemy = Args.Contains("enemy");
			Plan.bAlly = Args.Contains("ally");
			for (int Index = 0; Index < Args.Num(); Index++)
			{
				if (Args[Index].StartsWith("slot="))
					Plan.Slot = int(HOLControllerCommands::ParseReal(Args[Index].Mid(5)));
			}
			Plan.bValid = true;
		}
		else if (Command == "hol.Select.Box" || Command == "hol.Select.BoxGround")
		{
			if (Args.Num() < 4)
			{
				Plan.Usage = f"Usage: {Command} <X1> <Y1> <X2> <Y2> [add]";
				return Plan;
			}
			Plan.X1 = HOLControllerCommands::ParseReal(Args[0]);
			Plan.Y1 = HOLControllerCommands::ParseReal(Args[1]);
			Plan.X2 = HOLControllerCommands::ParseReal(Args[2]);
			Plan.Y2 = HOLControllerCommands::ParseReal(Args[3]);
			Plan.bAdd = Args.Num() >= 5 && Args[4] == "add";
			Plan.bValid = true;
		}
		else if (Command == "hol.Select.OrderGround")
		{
			if (Args.Num() < 2)
			{
				Plan.Usage = "Usage: hol.Select.OrderGround <X> <Y> [shift]";
				return Plan;
			}
			Plan.X1 = HOLControllerCommands::ParseReal(Args[0]);
			Plan.Y1 = HOLControllerCommands::ParseReal(Args[1]);
			Plan.bShift = Args.Num() >= 3 && Args[2] == "shift";
			Plan.bValid = true;
		}
		else if (Command == "hol.Select.OrderLineGround")
		{
			if (Args.Num() < 4)
			{
				Plan.Usage = "Usage: hol.Select.OrderLineGround <X1> <Y1> <X2> <Y2>";
				return Plan;
			}
			Plan.X1 = HOLControllerCommands::ParseReal(Args[0]);
			Plan.Y1 = HOLControllerCommands::ParseReal(Args[1]);
			Plan.X2 = HOLControllerCommands::ParseReal(Args[2]);
			Plan.Y2 = HOLControllerCommands::ParseReal(Args[3]);
			Plan.bCtrl = HasLineFlag(Args, "ctrl");
			Plan.bShift = HasLineFlag(Args, "shift");
			Plan.bValid = true;
		}
		else if (Command == "hol.Select.OrderNear")
		{
			// Fewer than two numbers: no point near the centre (the C++ GetPointNear refused), so the "not on the screen" Warning is logged.
			Plan.bHasPoint = Args.Num() >= 2;
			if (Plan.bHasPoint)
			{
				Plan.X1 = HOLControllerCommands::ParseReal(Args[0]);
				Plan.Y1 = HOLControllerCommands::ParseReal(Args[1]);
			}
			Plan.bEnemy = Args.Num() >= 3 && Args[2] == "enemy";
			Plan.bValid = true;
		}
		return Plan;
	}

	// printf("%.3f") of a viewport fraction, by integer rounding (half up; the C++ rounded the binary value, the two differ only on an exact tie).
	FString Fixed3(double Value)
	{
		bool bNegative = Value < 0.0;
		int Milli = Math::RoundToInt(Math::Abs(Value) * 1000.0);
		int Whole = Milli / 1000;
		int Part = Milli % 1000;
		FString PartText = f"{Part}";
		while (PartText.Len() < 3)
			PartText = "0" + PartText;
		FString Sign = bNegative ? "-" : "";
		return f"{Sign}{Whole}.{PartText}";
	}

	FString GroundClickText(double Fx, double Fy)
	{
		return f"Click on the ground at ({Fixed3(Fx)}, {Fixed3(Fy)})";
	}

	FString RightClickGroundText(double Fx, double Fy, bool bShift)
	{
		FString Tail = bShift ? ", Shift" : "";
		return f"Right click on the ground at ({Fixed3(Fx)}, {Fixed3(Fy)}){Tail}";
	}

	FString BoxGroundText(const FHOLFractionBoxC10a& Box)
	{
		return f"Box on the ground from ({Fixed3(Box.MinX)}, {Fixed3(Box.MinY)}) to ({Fixed3(Box.MaxX)}, {Fixed3(Box.MaxY)})";
	}

	FString RightDragText(double Fx1, double Fy1, double Fx2, double Fy2)
	{
		return f"Right drag on the ground from ({Fixed3(Fx1)}, {Fixed3(Fy1)}) to ({Fixed3(Fx2)}, {Fixed3(Fy2)})";
	}

	FString UnitClickText(bool bOrder, int UnitId, double Fx, double Fy)
	{
		FString What = bOrder ? "Right click" : "Click";
		return f"{What} on unit #{UnitId} at ({Fixed3(Fx)}, {Fixed3(Fy)})";
	}

	FString UnitMissText(int Slot, bool bAlly, bool bEnemy, int N)
	{
		FString Kind = Slot != -1 ? "such" : (bAlly ? "ally" : (bEnemy ? "enemy" : "own"));
		return f"hol.Select.ClickUnit: no {Kind} unit {N} on the screen";
	}

	FString NearText(double X, double Y)
	{
		int Rx = Math::RoundToInt(X);
		int Ry = Math::RoundToInt(Y);
		return f"Right click near the centre at ({Rx}, {Ry})";
	}

	// The box around four projected corners (any order).
	FHOLFractionBoxC10a BoxOfCorners(const TArray<double>& Xs, const TArray<double>& Ys)
	{
		FHOLFractionBoxC10a Box;
		if (Xs.Num() == 0 || Ys.Num() == 0)
			return Box;
		Box.MinX = Xs[0]; Box.MaxX = Xs[0]; Box.MinY = Ys[0]; Box.MaxY = Ys[0];
		for (int Index = 1; Index < Xs.Num() && Index < Ys.Num(); Index++)
		{
			Box.MinX = Math::Min(Box.MinX, Xs[Index]); Box.MaxX = Math::Max(Box.MaxX, Xs[Index]);
			Box.MinY = Math::Min(Box.MinY, Ys[Index]); Box.MaxY = Math::Max(Box.MaxY, Ys[Index]);
		}
		return Box;
	}

	// A right click on a unit of hol.Select.ClickUnit (the one place the script knows the unit under the cursor): ally or own = Follow, enemy = Attack.
	// STUB (red test first): always Move, the Book untouched.
	EHOLRightClickOrder RightClickUnitOrder(FHOLFollowBook& Book, const TArray<int>& SelectedIds, int TargetId, bool bEnemy, int Slot)
	{
		return EHOLRightClickOrder::Move;
	}

	void DoForController(AHOLPlayerController Controller, const FString& Command, const FHOLConsolePlanC10a& Plan)
	{
		double Fx = 0.0;
		double Fy = 0.0;
		if (Command == "hol.Select.Click")
			HOLControllerCommand::SelectAtFraction(Controller, Plan.X1, Plan.Y1, Plan.bAdd, Plan.bDouble);
		else if (Command == "hol.Select.Box")
			HOLControllerCommand::SelectInBoxFraction(Controller, Plan.X1, Plan.Y1, Plan.X2, Plan.Y2, Plan.bAdd);
		else if (Command == "hol.Select.ClickUnit")
		{
			int UnitId = 0;
			int N = 0;
			if (!HOLControllerCommand::UnitScreenPoint(Controller, Plan.Wanted, Plan.bEnemy, Plan.bAlly, Plan.Slot, UnitId, Fx, Fy, N))
			{
				HOLScriptLog::LogWarning(n"LogHOLMatch", UnitMissText(Plan.Slot, Plan.bAlly, Plan.bEnemy, N));
				return;
			}
			HOLScriptLog::LogDisplay(n"LogHOLMatch", UnitClickText(Plan.bOrder, UnitId, Fx, Fy));
			if (Plan.bOrder)
				HOLControllerCommand::OrderAtFraction(Controller, Fx, Fy, false);
			else
				HOLControllerCommand::SelectAtFraction(Controller, Fx, Fy, Plan.bAdd, Plan.bDouble);
		}
		else if (Command == "hol.Select.BoxGround")
		{
			TArray<double> Xs;
			TArray<double> Ys;
			bool bOnScreen = true;
			TArray<double> CornerX;
			TArray<double> CornerY;
			CornerX.Add(Plan.X1); CornerY.Add(Plan.Y1);
			CornerX.Add(Plan.X1); CornerY.Add(Plan.Y2);
			CornerX.Add(Plan.X2); CornerY.Add(Plan.Y1);
			CornerX.Add(Plan.X2); CornerY.Add(Plan.Y2);
			for (int Index = 0; Index < 4; Index++)
			{
				bOnScreen = HOLControllerCommand::ProjectGroundFraction(Controller, CornerX[Index], CornerY[Index], Fx, Fy) && bOnScreen;
				Xs.Add(Fx);
				Ys.Add(Fy);
			}
			if (!bOnScreen)
			{
				HOLScriptLog::LogWarning(n"LogHOLMatch", "hol.Select.BoxGround: the rectangle is not wholly on the screen");
				return;
			}
			FHOLFractionBoxC10a Box = BoxOfCorners(Xs, Ys);
			HOLScriptLog::LogDisplay(n"LogHOLMatch", BoxGroundText(Box));
			HOLControllerCommand::SelectInBoxFraction(Controller, Box.MinX, Box.MinY, Box.MaxX, Box.MaxY, Plan.bAdd);
		}
		else if (Command == "hol.Select.OrderGround" || Command == "hol.Select.ClickGround")
		{
			if (!HOLControllerCommand::ProjectGroundFraction(Controller, Plan.X1, Plan.Y1, Fx, Fy))
			{
				HOLScriptLog::LogWarning(n"LogHOLMatch", f"{Command}: the point is not on the screen");
				return;
			}
			if (Command == "hol.Select.OrderGround")
			{
				HOLScriptLog::LogDisplay(n"LogHOLMatch", RightClickGroundText(Fx, Fy, Plan.bShift));
				HOLControllerCommand::OrderAtFraction(Controller, Fx, Fy, Plan.bShift);
			}
			else
			{
				HOLScriptLog::LogDisplay(n"LogHOLMatch", GroundClickText(Fx, Fy));
				HOLControllerCommand::SelectAtFraction(Controller, Fx, Fy, Plan.bAdd, false);
			}
		}
		else if (Command == "hol.Select.OrderLineGround")
		{
			double Gx = 0.0;
			double Gy = 0.0;
			// Both points are projected (no short circuit needed: a failure only logs).
			bool bFrom = HOLControllerCommand::ProjectGroundFraction(Controller, Plan.X1, Plan.Y1, Fx, Fy);
			bool bTo = HOLControllerCommand::ProjectGroundFraction(Controller, Plan.X2, Plan.Y2, Gx, Gy);
			if (!bFrom || !bTo)
			{
				HOLScriptLog::LogWarning(n"LogHOLMatch", "hol.Select.OrderLineGround: a point is not on the screen");
				return;
			}
			HOLScriptLog::LogDisplay(n"LogHOLMatch", RightDragText(Fx, Fy, Gx, Gy));
			HOLControllerCommand::OrderLineAtFractions(Controller, Fx, Fy, Gx, Gy, Plan.bCtrl, Plan.bShift);
		}
		else if (Command == "hol.Select.OrderNear")
		{
			double Px = 0.0;
			double Py = 0.0;
			if (!Plan.bHasPoint || !HOLControllerCommand::PointNearOf(Controller, Plan.X1, Plan.Y1, Plan.bEnemy, Px, Py)
				|| !HOLControllerCommand::ProjectGroundFraction(Controller, Px, Py, Fx, Fy))
			{
				HOLScriptLog::LogWarning(n"LogHOLMatch", "hol.Select.OrderNear: the point is not on the screen");
				return;
			}
			HOLScriptLog::LogDisplay(n"LogHOLMatch", NearText(Px, Py));
			HOLControllerCommand::OrderAtFraction(Controller, Fx, Fy, false);
		}
	}

	void Run(const FString& Command, const TArray<FString>& Args)
	{
		const FHOLConsolePlanC10a Plan = PlanOf(Command, Args);
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

void HOLSelectClickC10aCommand(const TArray<FString>& Args) { HOLControllerCommandsC10a::Run("hol.Select.Click", Args); }
void HOLSelectClickUnitC10aCommand(const TArray<FString>& Args) { HOLControllerCommandsC10a::Run("hol.Select.ClickUnit", Args); }
void HOLSelectBoxC10aCommand(const TArray<FString>& Args) { HOLControllerCommandsC10a::Run("hol.Select.Box", Args); }
void HOLSelectBoxGroundC10aCommand(const TArray<FString>& Args) { HOLControllerCommandsC10a::Run("hol.Select.BoxGround", Args); }
void HOLSelectOrderGroundC10aCommand(const TArray<FString>& Args) { HOLControllerCommandsC10a::Run("hol.Select.OrderGround", Args); }
void HOLSelectOrderLineGroundC10aCommand(const TArray<FString>& Args) { HOLControllerCommandsC10a::Run("hol.Select.OrderLineGround", Args); }
void HOLSelectClickGroundC10aCommand(const TArray<FString>& Args) { HOLControllerCommandsC10a::Run("hol.Select.ClickGround", Args); }
void HOLSelectOrderNearC10aCommand(const TArray<FString>& Args) { HOLControllerCommandsC10a::Run("hol.Select.OrderNear", Args); }

const FConsoleCommand HOLSelectClickC10aConsole("hol.Select.Click", n"HOLSelectClickC10aCommand");
const FConsoleCommand HOLSelectClickUnitC10aConsole("hol.Select.ClickUnit", n"HOLSelectClickUnitC10aCommand");
const FConsoleCommand HOLSelectBoxC10aConsole("hol.Select.Box", n"HOLSelectBoxC10aCommand");
const FConsoleCommand HOLSelectBoxGroundC10aConsole("hol.Select.BoxGround", n"HOLSelectBoxGroundC10aCommand");
const FConsoleCommand HOLSelectOrderGroundC10aConsole("hol.Select.OrderGround", n"HOLSelectOrderGroundC10aCommand");
const FConsoleCommand HOLSelectOrderLineGroundC10aConsole("hol.Select.OrderLineGround", n"HOLSelectOrderLineGroundC10aCommand");
const FConsoleCommand HOLSelectClickGroundC10aConsole("hol.Select.ClickGround", n"HOLSelectClickGroundC10aCommand");
const FConsoleCommand HOLSelectOrderNearC10aConsole("hol.Select.OrderNear", n"HOLSelectOrderNearC10aCommand");
