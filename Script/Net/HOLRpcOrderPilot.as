// L2 A3 (D846): the CLIENT PILOT of the latency and order measure of the 16 player orders (Tools\Scenarios\order_latency_run.ps1 and order_order_run.ps1; contract of the journal lines:
// ELROND-1118f.md, "L2 M1 fait"). A world subsystem (UScriptWorldSubsystem, ticked on every machine, like UHOLBiomeApplier): it needs NO line in HOLRpcPlayer.as. It does nothing unless a
// cvar is set (-ExecCmds="hol.Rpc.OrderTrace 1,hol.Rpc.OrderLatency <N>" or "...,hol.Rpc.OrderOrder <pairs> <mode>"), and only on a machine whose player 0 is a local controller (never on the
// dedicated server). The OrderSend / OrderRecv lines are written by the C++ (C1), NOT here. This file lives in Script/Net (never Dev, Developers, Examples, Editor: the plugin skips them).
//
// hol.Rpc.OrderLatency <N>: 4 s after the first tick of the local controller, "OrderLatency: start units <N> orders 400", then 4 blocks of 100 Move, ONE order per tick, mode A/B/A/B
// (hol.Rpc.OrderScript 0, 1, 0, 1 set at the start of each block, then "OrderLatency: mode <m> block <b>"), 2 s pause between blocks, "OrderLatency: done sent <k>". The Move is the console
// command hol.Units.Order X Y (all the local player's units known here), key = X = 1000 + n. <N> is only the number the scenario asked of the match config: the real number of units ordered is
// in the C++ line "Order: <n> units to (X, Y)" the same command writes (no script access to the unit list).
// hol.Rpc.OrderOrder <pairs> <mode>: same wait, hol.Rpc.OrderScript <mode>, "OrderOrder: start pairs <p> mode <m>", then <p> series of 6 orders, ONE per tick: Move, Move (key + 1), Stop, Move,
// PlaceBuilding, BuildUnits; "OrderOrder: done sent <6p>". NO client console command sends a QUEUED (Shift) move without the screen (hol.Units.Move is server or solo, hol.Select.Order projects
// on the viewport): the 6th step BuildUnits is hol.Units.Build (D848; it was hol.Prod.Queue Peasant, a production order, not an L2 order): the second Move of the first pair is therefore a plain hol.Units.Order with the next key, not a queued one.

const FConsoleVariable CVar_RpcOrderLatency("hol.Rpc.OrderLatency", 0);

// hol.Rpc.OrderOrder takes TWO numbers, which a cvar cannot: the console command below stores them in these two cvars.
const FConsoleVariable CVar_RpcOrderOrderPairs("hol.Rpc.OrderOrderPairs", 0);
const FConsoleVariable CVar_RpcOrderOrderMode("hol.Rpc.OrderOrderMode", 0);

void HOLRpcOrderOrderCommand(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		Log(n"LogHOLMatch", "OrderOrder: usage hol.Rpc.OrderOrder <pairs> <mode 0|1>");
		return;
	}
	FString ModeText = "0";
	if (Args.Num() >= 2)
		ModeText = Args[1];
	System::ExecuteConsoleCommand(f"hol.Rpc.OrderOrderMode {ModeText}", nullptr);
	System::ExecuteConsoleCommand(f"hol.Rpc.OrderOrderPairs {Args[0]}", nullptr);
}

const FConsoleCommand HOLRpcOrderOrderConsole("hol.Rpc.OrderOrder", n"HOLRpcOrderOrderCommand");

class UHOLRpcOrderPilot : UScriptWorldSubsystem
{
	// 0 waiting for the local controller and the 4 s, 1 latency run, 2 order run, 3 done (nothing more is ever sent).
	int Phase = 0;

	// Seconds since the first tick with a local controller.
	float ControllerClock = 0.0;

	// Orders sent so far (both runs).
	int Sent = 0;

	// Latency run: the block (0 to 3), the orders already sent in it, the pause left before the next block.
	int Block = 0;
	int InBlock = 0;
	float PauseLeft = 0.0;

	// Order run: the series (0 to pairs - 1) and the step of the series (0 to 5).
	int Series = 0;
	int Step = 0;

	UFUNCTION(BlueprintOverride)
	void Tick(float DeltaTime)
	{
		if (Phase >= 3)
			return;
		const int Latency = CVar_RpcOrderLatency.GetInt();
		const int Pairs = CVar_RpcOrderOrderPairs.GetInt();
		if (Latency <= 0 && Pairs <= 0)
			return;
		APlayerController Controller = Gameplay::GetPlayerController(0);
		if (Controller == nullptr || !Controller.IsLocalController())
			return;

		if (Phase == 0)
		{
			ControllerClock += DeltaTime;
			if (ControllerClock < 4.0)
				return;
			if (Latency > 0)
			{
				Phase = 1;
				Log(n"LogHOLMatch", f"OrderLatency: start units {Latency} orders 400");
			}
			else
			{
				Phase = 2;
				const int Mode = CVar_RpcOrderOrderMode.GetInt();
				System::ExecuteConsoleCommand(f"hol.Rpc.OrderScript {Mode}", Controller);
				Log(n"LogHOLMatch", f"OrderOrder: start pairs {Pairs} mode {Mode}");
				System::ExecuteConsoleCommand("hol.Select.Own", Controller);
			}
			return;
		}

		if (Phase == 1)
			TickLatency(Controller, DeltaTime);
		else if (Phase == 2)
			TickOrder(Controller, Pairs);
	}

	// ONE Move per tick; each block opens by setting hol.Rpc.OrderScript (blocks 0 and 2: 0, blocks 1 and 3: 1) and writing the marker.
	void TickLatency(APlayerController Controller, float DeltaTime)
	{
		if (PauseLeft > 0.0)
		{
			PauseLeft -= DeltaTime;
			return;
		}
		if (InBlock == 0)
		{
			const int Mode = Block % 2;
			System::ExecuteConsoleCommand(f"hol.Rpc.OrderScript {Mode}", Controller);
			Log(n"LogHOLMatch", f"OrderLatency: mode {Mode} block {Block}");
		}
		const int Key = 1000 + Sent;
		System::ExecuteConsoleCommand(f"hol.Units.Order {Key} 0", Controller);
		Sent += 1;
		InBlock += 1;
		if (InBlock >= 100)
		{
			InBlock = 0;
			Block += 1;
			if (Block >= 4)
			{
				Phase = 3;
				Log(n"LogHOLMatch", f"OrderLatency: done sent {Sent}");
			}
			else
				PauseLeft = 2.0;
		}
	}

	// ONE order per tick, the 6 steps of a series in this order: Move, Move, Stop, Move, PlaceBuilding, BuildUnits (keys: Move 2000 + 3 * series + 0..2; Place by its offset, Stop and BuildUnits key 0).
	void TickOrder(APlayerController Controller, int Pairs)
	{
		const int MoveKey = 2000 + 3 * Series;
		if (Step == 0)
			System::ExecuteConsoleCommand(f"hol.Units.Order {MoveKey} 0", Controller);
		else if (Step == 1)
		{
			const int SecondKey = MoveKey + 1;
			System::ExecuteConsoleCommand(f"hol.Units.Order {SecondKey} 0", Controller);
		}
		else if (Step == 2)
		{
			// The selection first (the previous BuildUnits selected the main centre).
			System::ExecuteConsoleCommand("hol.Select.Own", Controller);
			System::ExecuteConsoleCommand("hol.Select.Stop", Controller);
		}
		else if (Step == 3)
		{
			const int ThirdKey = MoveKey + 2;
			System::ExecuteConsoleCommand(f"hol.Units.Order {ThirdKey} 0", Controller);
		}
		else if (Step == 4)
		{
			// A foundation near the main centre; Dx and Dy both grow with the series so that the key (Point.X) of each stays distinct whatever the centre's rotation.
			const int Dx = 400 + 20 * Series;
			const int Dy = 7 * Series;
			System::ExecuteConsoleCommand("hol.Select.Own", Controller);
			System::ExecuteConsoleCommand(f"hol.Build.PlaceNear House {Dx} {Dy}", Controller);
		}
		else
		{
			// BuildUnits (D848): the units go and help build the site step 4 just placed (hol.Units.Build, ServerBuildUnits). The former "hol.Prod.Queue Peasant" was ServerQueueProduction, NOT an L2 order.
			System::ExecuteConsoleCommand("hol.Select.Own", Controller);
			System::ExecuteConsoleCommand("hol.Units.Build", Controller);
		}

		Sent += 1;
		Step += 1;
		if (Step >= 6)
		{
			Step = 0;
			Series += 1;
			if (Series >= Pairs)
			{
				Phase = 3;
				Log(n"LogHOLMatch", f"OrderOrder: done sent {Sent}");
			}
		}
	}
}
