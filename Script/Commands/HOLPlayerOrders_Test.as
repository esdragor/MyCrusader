// P1a: tests of the player's order rules (HOLPlayerOrders.as) on a detached queue (MakeDetachedForTest: no world, no navmesh projection, no
// defeated-slot refusal). They cover the same cases as the C++ HOLPlayerOrders.cpp / HOLAIOrdersTests.cpp:159-164 (two copies until P1b: agreement to
// prove then). NOT COMPILED at the time of writing. Red planned: in Test_PlayerOrdersIsAccepted change 201 to 200 in the case (1, 201): the order
// would be accepted, the test fails.
// Ids are built by POUnits (own name: a global function is not visible from another file, and this one is not in a namespace on purpose).

TArray<int> POUnits(int Count)
{
	TArray<int> Ids;
	for (int I = 0; I < Count; I++)
	{
		Ids.Add(100 + I);
	}
	return Ids;
}

void Test_PlayerOrdersIsAccepted(FUnitTest& T)
{
	int MaxSlots = HOLCommandScript::GetMaxSlots();
	T.AssertTrue(MaxSlots > 0, "there are player slots");
	T.AssertTrue(HOLPlayerOrders::IsAccepted(1, 200), "slot 1, 200 units: accepted");
	T.AssertTrue(HOLPlayerOrders::IsAccepted(0, 1), "slot 0, 1 unit: accepted");
	T.AssertTrue(HOLPlayerOrders::IsAccepted(MaxSlots - 1, 0), "last slot, 0 units: accepted");
	T.AssertFalse(HOLPlayerOrders::IsAccepted(1, 201), "201 units: refused");
	T.AssertFalse(HOLPlayerOrders::IsAccepted(-1, 1), "slot -1: refused");
	T.AssertFalse(HOLPlayerOrders::IsAccepted(MaxSlots, 1), "slot MaxSlots: refused");
	T.AssertFalse(HOLPlayerOrders::IsAccepted(1, -1), "-1 units: refused");
	int Limit = HOLPlayerOrders::GetMaxOrderedUnits();
	T.AssertEquals(200, Limit, "the most units one order may name");
}

void Test_PlayerOrdersMoveEntersQueue(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	bool bDone = HOLPlayerOrders::Move(Commands, 0, POUnits(2), FVector(1200.0, -340.0, 5.0), true, FVector2D(0.0, 0.0), false);
	T.AssertTrue(bDone, "the move is accepted");
	TArray<FHOLScriptOrderRead> Orders = HOLCommandScript::ConsumeMoves(Commands);
	T.AssertEquals(1, Orders.Num(), "one order queued");
	T.AssertTrue(Orders[0].Kind == n"Move", "its kind is Move");
	T.AssertEquals(0, Orders[0].PlayerIndex, "the player who gave it");
	T.AssertEquals(2, Orders[0].UnitIds.Num(), "two units");
	T.AssertEquals(100, Orders[0].UnitIds[0], "first unit");
	T.AssertEquals(1200, int(Orders[0].Target.X), "target X as given (no navmesh)");
	T.AssertEquals(-340, int(Orders[0].Target.Y), "target Y as given");
	T.AssertTrue(Orders[0].bQueue, "Shift kept");
	T.AssertFalse(Orders[0].bFromTask, "a player's order, not a task's walk");
	TArray<FHOLScriptOrderRead> Cmds2 = HOLCommandScript::ConsumeMoves(Commands);
	T.AssertEquals(0, Cmds2.Num(), "Consume empties the queue");
}

void Test_PlayerOrdersRefusedForBadSlotOrTooMany(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	int MaxSlots = HOLCommandScript::GetMaxSlots();
	FVector Point(10.0, 20.0, 0.0);
	T.AssertFalse(HOLPlayerOrders::Move(Commands, -1, POUnits(1), Point, false, FVector2D(0.0, 0.0), false), "slot -1: move refused");
	T.AssertFalse(HOLPlayerOrders::Move(Commands, MaxSlots, POUnits(1), Point, false, FVector2D(0.0, 0.0), false), "slot MaxSlots: move refused");
	T.AssertFalse(HOLPlayerOrders::Move(Commands, 0, POUnits(201), Point, false, FVector2D(0.0, 0.0), false), "201 units: move refused");
	T.AssertFalse(HOLPlayerOrders::Attack(Commands, 0, POUnits(201), 55, false), "201 units: attack refused");
	T.AssertFalse(HOLPlayerOrders::AttackMove(Commands, -1, POUnits(1), Point, false), "slot -1: attack-move refused");
	T.AssertFalse(HOLPlayerOrders::Stop(Commands, MaxSlots, POUnits(1), false), "slot MaxSlots: stop refused");
	T.AssertFalse(HOLPlayerOrders::ToggleDeploy(Commands, -1, POUnits(1)), "slot -1: deploy refused");
	T.AssertFalse(HOLPlayerOrders::Move(nullptr, 0, POUnits(1), Point, false, FVector2D(0.0, 0.0), false), "no subsystem: refused");
	int Pending = HOLCommandScript::GetNumPendingMoveOrders(Commands);
	T.AssertEquals(0, Pending, "nothing entered the queue");
	T.AssertTrue(HOLPlayerOrders::Move(Commands, 0, POUnits(200), Point, false, FVector2D(0.0, 0.0), false), "200 units: accepted");
	int PendingAfter = HOLCommandScript::GetNumPendingMoveOrders(Commands);
	T.AssertEquals(1, PendingAfter, "that one entered");
}

void Test_PlayerOrdersToggleDeployKind(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	T.AssertTrue(HOLPlayerOrders::ToggleDeploy(Commands, 1, POUnits(3)), "the deploy toggle is accepted");
	TArray<FHOLScriptOrderRead> Orders = HOLCommandScript::ConsumeMoves(Commands);
	T.AssertEquals(1, Orders.Num(), "one order");
	T.AssertTrue(Orders[0].Kind == n"ToggleDeploy", "its kind is ToggleDeploy");
	T.AssertEquals(1, Orders[0].PlayerIndex, "the player who gave it");
	T.AssertEquals(3, Orders[0].UnitIds.Num(), "three units");
}

void Test_PlayerOrdersStopVsHold(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	T.AssertTrue(HOLPlayerOrders::Stop(Commands, 0, POUnits(2), false), "stop accepted");
	T.AssertTrue(HOLPlayerOrders::Stop(Commands, 0, POUnits(2), true), "hold accepted");
	TArray<FHOLScriptOrderRead> Orders = HOLCommandScript::ConsumeMoves(Commands);
	T.AssertEquals(2, Orders.Num(), "two orders, in the order given");
	T.AssertTrue(Orders[0].Kind == n"Stop", "bHold false: Stop");
	T.AssertTrue(Orders[1].Kind == n"Hold", "bHold true: Hold");
}

void Test_PlayerOrdersAttackKindsAndTarget(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	T.AssertTrue(HOLPlayerOrders::Attack(Commands, 0, POUnits(2), 55, false), "attack accepted");
	T.AssertTrue(HOLPlayerOrders::AttackMove(Commands, 0, POUnits(2), FVector(500.0, 600.0, 0.0), true), "attack-move accepted");
	TArray<FHOLScriptOrderRead> Orders = HOLCommandScript::ConsumeMoves(Commands);
	T.AssertEquals(2, Orders.Num(), "two orders");
	T.AssertTrue(Orders[0].Kind == n"Attack", "first: Attack");
	T.AssertEquals(55, Orders[0].TargetUnitId, "the target unit");
	T.AssertTrue(Orders[1].Kind == n"AttackMove", "second: AttackMove");
	T.AssertTrue(Orders[1].bQueue, "Shift kept on the attack-move");
}

void Test_PlayerOrdersAttackBuildingNullRefused(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	AHOLBuilding NoBuilding = nullptr;
	T.AssertFalse(HOLPlayerOrders::AttackBuilding(Commands, 0, POUnits(2), NoBuilding, false), "no building: refused");
	int Pending = HOLCommandScript::GetNumPendingMoveOrders(Commands);
	T.AssertEquals(0, Pending, "nothing queued");
}

void Test_PlayerOrdersPlayerScopeDrawsChain(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	T.AssertTrue(HOLPlayerOrders::Move(Commands, 0, POUnits(2), FVector(10.0, 20.0, 0.0), false, FVector2D(0.0, 0.0), false), "player move accepted");
	TArray<FHOLScriptOrderRead> Player = HOLCommandScript::ConsumeMoves(Commands);
	T.AssertEquals(1, Player.Num(), "one player order");
	T.AssertTrue(Player[0].bChain, "a player's order draws its chain (D328)");
	HOLCommandScript::IssueMove(Commands, POUnits(2), FVector(10.0, 20.0, 0.0), 0);
	TArray<FHOLScriptOrderRead> Raw = HOLCommandScript::ConsumeMoves(Commands);
	T.AssertEquals(1, Raw.Num(), "one raw order");
	T.AssertFalse(Raw[0].bChain, "a raw IssueMove (no player scope) does not draw a chain");
	// IssueCombatOrder: bChain = player scope && Kind == Attack (HOLCommandSubsystem.cpp:266): Attack draws, Stop does not.
	T.AssertTrue(HOLPlayerOrders::Attack(Commands, 0, POUnits(1), 55, false), "player attack accepted");
	TArray<FHOLScriptOrderRead> AttackRead = HOLCommandScript::ConsumeMoves(Commands);
	T.AssertEquals(1, AttackRead.Num(), "one attack");
	T.AssertTrue(AttackRead[0].bChain, "a player's attack draws its chain");
	T.AssertTrue(HOLPlayerOrders::Stop(Commands, 0, POUnits(1), false), "player stop accepted");
	TArray<FHOLScriptOrderRead> StopRead = HOLCommandScript::ConsumeMoves(Commands);
	T.AssertEquals(1, StopRead.Num(), "one stop");
	T.AssertFalse(StopRead[0].bChain, "a stop draws no chain, as in C++ (the scope is closed and reopened per order)");
}

// ---------------- P2 tests (second half of the player's orders), on a detached queue. NOT COMPILED. ----------------
// Red planned (P2): in HOLPlayerOrders.as, Place: change `UnitIds.Num() <= MaxOrderedUnitsValue` to `true`: Test_PlayerOrdersPlaceDropsBuildersWhenTooMany fails
// (201 builders kept instead of 0). Second red: in Gather, `bQueue && bSource` to `bSource`: Test_PlayerOrdersGatherEntersGatherQueue fails (no order in the gather queue).
// A rule that names a building (Build, DropOff deposit, QueueProduction, MarketTrade, SetRally, Garrison, UngarrisonAll) cannot get a real AHOLBuilding without a
// world: here only its refusals are tested (null building, slot, count); the accepted path is proven by the C++ tests (HOLGarrisonOrderTests, HOLGatherTests...)
// that now go through the bridge, and by the network replay.

void Test_PlayerOrdersGatherEntersGatherQueue(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	bool bDone = HOLPlayerOrders::Gather(Commands, 0, POUnits(2), 7, false, true, FVector(10.0, 20.0, 0.0));
	T.AssertTrue(bDone, "the gather order is accepted");
	TArray<FHOLScriptOrderRead> Gathers = HOLCommandScript::ConsumeGathers(Commands);
	T.AssertEquals(1, Gathers.Num(), "one order in the gather queue");
	T.AssertEquals(7, Gathers[0].SourceIndex, "the source asked");
	T.AssertEquals(2, Gathers[0].UnitIds.Num(), "its two workers");
	T.AssertEquals(0, Gathers[0].PlayerIndex, "the player who gave it");
}

void Test_PlayerOrdersGatherRefusedOrEmpty(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	FVector Spot(0.0, 0.0, 0.0);
	T.AssertFalse(HOLPlayerOrders::Gather(Commands, -1, POUnits(1), 0, false, true, Spot), "slot -1: refused");
	T.AssertFalse(HOLPlayerOrders::Gather(Commands, 0, POUnits(201), 0, false, true, Spot), "201 workers: refused");
	T.AssertFalse(HOLPlayerOrders::Gather(nullptr, 0, POUnits(1), 0, false, true, Spot), "no subsystem: refused");
	T.AssertEquals(0, HOLCommandScript::ConsumeGathers(Commands).Num(), "nothing queued by the refusals");
	T.AssertTrue(HOLPlayerOrders::Gather(Commands, 0, POUnits(0), 3, false, true, Spot), "no worker left (all soldiers): accepted, as the C++ did");
	T.AssertEquals(0, HOLCommandScript::ConsumeGathers(Commands).Num(), "but nothing queued");
}

void Test_PlayerOrdersDropOffWalksWhenNotADeposit(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	AHOLBuilding NoBuilding = nullptr;
	bool bDone = HOLPlayerOrders::DropOff(Commands, 1, POUnits(3), NoBuilding, FVector(300.0, 400.0, 0.0), true, false);
	T.AssertTrue(bDone, "accepted");
	TArray<FHOLScriptOrderRead> Orders = HOLCommandScript::ConsumeMoves(Commands);
	T.AssertEquals(1, Orders.Num(), "all walk to the point: one move");
	T.AssertTrue(Orders[0].Kind == n"Move", "a plain move");
	T.AssertEquals(3, Orders[0].UnitIds.Num(), "the three units");
	T.AssertTrue(Orders[0].bChain, "the player's order draws its chain (D328)");
	T.AssertTrue(Orders[0].bQueue, "Shift kept");
	T.AssertFalse(HOLPlayerOrders::DropOff(Commands, 0, POUnits(201), NoBuilding, FVector(0.0, 0.0, 0.0), false, false), "201 units: refused");
}

void Test_PlayerOrdersPlaceEntersQueue(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	T.AssertTrue(HOLPlayerOrders::Place(Commands, 0, n"House", FVector(500.0, 600.0, 0.0), POUnits(2), true), "the place order is accepted");
	TArray<FHOLScriptOrderRead> Orders = HOLCommandScript::ConsumeConstructions(Commands);
	T.AssertEquals(1, Orders.Num(), "one construction order");
	T.AssertTrue(Orders[0].Kind == n"Place", "its kind is Place");
	T.AssertTrue(Orders[0].TypeName == n"House", "the type asked");
	T.AssertEquals(2, Orders[0].UnitIds.Num(), "its two builders");
	T.AssertTrue(Orders[0].bQueue, "Shift kept");
	int MaxSlots = HOLCommandScript::GetMaxSlots();
	T.AssertFalse(HOLPlayerOrders::Place(Commands, -1, n"House", FVector(0.0, 0.0, 0.0), POUnits(1), false), "slot -1: refused");
	T.AssertFalse(HOLPlayerOrders::Place(Commands, MaxSlots, n"House", FVector(0.0, 0.0, 0.0), POUnits(1), false), "slot MaxSlots: refused");
}

void Test_PlayerOrdersPlaceDropsBuildersWhenTooMany(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	T.AssertTrue(HOLPlayerOrders::Place(Commands, 0, n"House", FVector(1.0, 2.0, 0.0), POUnits(201), false), "201 builders: the foundation is placed all the same");
	TArray<FHOLScriptOrderRead> Orders = HOLCommandScript::ConsumeConstructions(Commands);
	T.AssertEquals(1, Orders.Num(), "one construction order");
	T.AssertEquals(0, Orders[0].UnitIds.Num(), "but without the builders");
	T.AssertTrue(HOLPlayerOrders::Place(Commands, 0, n"House", FVector(1.0, 2.0, 0.0), POUnits(200), false), "200 builders accepted");
	TArray<FHOLScriptOrderRead> Kept = HOLCommandScript::ConsumeConstructions(Commands);
	T.AssertEquals(200, Kept[0].UnitIds.Num(), "200 builders kept");
}

void Test_PlayerOrdersBuildingRulesRefuseNullBuilding(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	AHOLBuilding NoBuilding = nullptr;
	FVector Spot(0.0, 0.0, 0.0);
	T.AssertFalse(HOLPlayerOrders::Build(Commands, 0, POUnits(2), NoBuilding, false), "build: no site refused");
	T.AssertFalse(HOLPlayerOrders::QueueProduction(Commands, 0, NoBuilding, n"Spearman"), "production: no building refused");
	T.AssertFalse(HOLPlayerOrders::MarketTrade(Commands, 0, NoBuilding, 0, 1, true), "market: no building refused");
	T.AssertFalse(HOLPlayerOrders::SetRally(Commands, 0, NoBuilding, Spot, -1, false, NoBuilding, false), "rally: no building refused");
	T.AssertFalse(HOLPlayerOrders::Garrison(Commands, 0, POUnits(2), NoBuilding), "garrison: no building refused");
	T.AssertFalse(HOLPlayerOrders::UngarrisonAll(Commands, 0, NoBuilding), "ungarrison: no building refused");
	T.AssertFalse(HOLPlayerOrders::QueueProduction(Commands, -1, NoBuilding, n"Spearman"), "slot -1 refused too");
	T.AssertEquals(0, HOLCommandScript::GetNumPendingMoveOrders(Commands), "nothing in the move queue");
	T.AssertEquals(0, HOLCommandScript::ConsumeProductions(Commands).Num(), "nothing in the production queue");
	T.AssertEquals(0, HOLCommandScript::ConsumeGarrisons(Commands).Num(), "nothing in the garrison queue");
	T.AssertEquals(0, HOLCommandScript::ConsumeBuilds(Commands).Num(), "nothing in the build queue");
}