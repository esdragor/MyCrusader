// SAURON-1119b, lot M1 of PLAN-ENVELOPPE-1118.md (D851): UHOLCommandScriptLibrary (Source\HeroesOfLiberty\Commands\HOLCommandScriptLibrary.h/.cpp) exposes
// the order queue (UHOLCommandSubsystem, which stays C++: Mass reads it) to the script. Every test sends an order through the library and reads it back
// through the same queue the simulation takes (Consume*), on a detached subsystem (no world: no navmesh projection, no map clamp, no defeated-slot refusal).
// NOT COMPILED at the time of writing: if a Test_CommandLibrary* fails to compile, the cause is in the .cpp (a name of UHOLCommandSubsystem or of the
// order structs) or in a script name used here (FHOLScriptOrderRead fields, HOLCommandScript:: statics).
// Not testable without a world: orders that name a building (garrison, drop-off, attack building, rally site, site help) cannot get an AHOLBuilding here; they
// are tested only for the refusals that come before it (a null building queues nothing). Their full read-back waits for a world-bound test (lot P1/P2).

TArray<int> CLUnits(int A, int B)
{
	TArray<int> Ids;
	Ids.Add(A);
	Ids.Add(B);
	return Ids;
}

TArray<int> CLUnits(int A)
{
	TArray<int> Ids;
	Ids.Add(A);
	return Ids;
}

void Test_CommandLibraryMoveRoundTrip(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	T.AssertTrue(Commands != nullptr, "a detached subsystem is made");
	HOLCommandScript::IssueMove(Commands, CLUnits(7, 9), FVector(1200.0, -340.0, 5.0), 2);
	int Pending = HOLCommandScript::GetNumPendingMoveOrders(Commands);
	T.AssertEquals(1, Pending, "one move queued");
	TArray<FHOLScriptOrderRead> Orders = HOLCommandScript::ConsumeMoves(Commands);
	T.AssertEquals(1, Orders.Num(), "one move read back");
	T.AssertTrue(Orders[0].Kind == n"Move", "its kind is Move");
	T.AssertEquals(2, Orders[0].PlayerIndex, "the player who gave it");
	T.AssertEquals(2, Orders[0].UnitIds.Num(), "two units");
	T.AssertEquals(7, Orders[0].UnitIds[0], "first unit");
	T.AssertEquals(9, Orders[0].UnitIds[1], "second unit");
	T.AssertEquals(1200, int(Orders[0].Target.X), "target X as given (no navmesh)");
	T.AssertEquals(-340, int(Orders[0].Target.Y), "target Y as given");
	T.AssertFalse(Orders[0].bQueue, "not queued with Shift");
	int After = HOLCommandScript::GetNumPendingMoveOrders(Commands);
	T.AssertEquals(0, After, "Consume empties the queue");
}

void Test_CommandLibraryMoveShiftAndTask(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	HOLCommandScript::IssueMove(Commands, CLUnits(1, 2), FVector(10.0, 20.0, 0.0), 0, true, true);
	TArray<FHOLScriptOrderRead> Orders = HOLCommandScript::ConsumeMoves(Commands);
	T.AssertEquals(1, Orders.Num(), "one move");
	T.AssertTrue(Orders[0].bQueue, "Shift kept");
	T.AssertTrue(Orders[0].bFromTask, "a task's walk kept");
	HOLCommandScript::IssueMove(Commands, TArray<int>(), FVector(10.0, 20.0, 0.0), 0);
	TArray<FHOLScriptOrderRead> None = HOLCommandScript::ConsumeMoves(Commands);
	T.AssertEquals(0, None.Num(), "no unit, no order");
}

void Test_CommandLibraryCombatKinds(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	HOLCommandScript::IssueAttack(Commands, CLUnits(3, 4), 55, -1);
	HOLCommandScript::IssueStop(Commands, CLUnits(3, 4), -1);
	HOLCommandScript::IssueHold(Commands, CLUnits(5, 6), -1);
	HOLCommandScript::IssueAttackMove(Commands, CLUnits(5, 6), FVector(500.0, 600.0, 0.0), -1);
	TArray<FHOLScriptOrderRead> Orders = HOLCommandScript::ConsumeMoves(Commands);
	T.AssertEquals(4, Orders.Num(), "four orders in the move queue, in the order given");
	T.AssertTrue(Orders[0].Kind == n"Attack", "first: Attack");
	T.AssertEquals(55, Orders[0].TargetUnitId, "the target unit");
	T.AssertTrue(Orders[1].Kind == n"Stop", "second: Stop");
	T.AssertTrue(Orders[2].Kind == n"Hold", "third: Hold");
	T.AssertEquals(5, Orders[2].UnitIds[0], "Hold's first unit");
	T.AssertTrue(Orders[3].Kind == n"AttackMove", "fourth: AttackMove");
	T.AssertEquals(600, int(Orders[3].Target.Y), "its point");
}

void Test_CommandLibraryGather(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	HOLCommandScript::IssueGather(Commands, CLUnits(11, 12), 4, 1);
	TArray<FHOLScriptOrderRead> Gathers = HOLCommandScript::ConsumeGathers(Commands);
	T.AssertEquals(1, Gathers.Num(), "one gather order");
	T.AssertEquals(4, Gathers[0].SourceIndex, "the source");
	T.AssertEquals(1, Gathers[0].PlayerIndex, "the player");
	T.AssertEquals(2, Gathers[0].UnitIds.Num(), "two gatherers");
	// With Shift: a build order naming the source, at the end of each unit's queue.
	HOLCommandScript::IssueGather(Commands, CLUnits(11), 6, 1, true);
	TArray<FHOLScriptOrderRead> Builds = HOLCommandScript::ConsumeBuilds(Commands);
	T.AssertEquals(1, Builds.Num(), "Shift: through the build queue");
	T.AssertEquals(6, Builds[0].SourceIndex, "the source named there");
	HOLCommandScript::IssueGather(Commands, CLUnits(11), -1, 1);
	TArray<FHOLScriptOrderRead> NoSource = HOLCommandScript::ConsumeGathers(Commands);
	T.AssertEquals(0, NoSource.Num(), "no source (-1), no order");
}

void Test_CommandLibraryBuildNeedsASiteOrASource(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	HOLCommandScript::IssueBuild(Commands, CLUnits(1, 2), nullptr, 3, 0, true);
	HOLCommandScript::IssueBuild(Commands, CLUnits(1, 2), nullptr, -1, 0);
	TArray<FHOLScriptOrderRead> Builds = HOLCommandScript::ConsumeBuilds(Commands);
	T.AssertEquals(1, Builds.Num(), "only the order with a source is kept (no site, no source: refused)");
	T.AssertEquals(3, Builds[0].SourceIndex, "its source");
	T.AssertTrue(Builds[0].bQueue, "its Shift");
	T.AssertFalse(Builds[0].bHasBuilding, "no site named");
}

void Test_CommandLibraryProductionAndRally(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	HOLCommandScript::IssueProductionQueue(Commands, 1, nullptr, n"Spearman", true);
	HOLCommandScript::IssueProductionCancel(Commands, 1, nullptr, 2);
	HOLCommandScript::IssueRally(Commands, 1, nullptr, FVector(800.0, 900.0, 0.0), 5, nullptr);
	TArray<FHOLScriptOrderRead> Orders = HOLCommandScript::ConsumeProductions(Commands);
	T.AssertEquals(3, Orders.Num(), "three production orders, in the order given");
	T.AssertTrue(Orders[0].Kind == n"Queue", "Queue");
	T.AssertTrue(Orders[0].TypeName == n"Spearman", "its type");
	T.AssertTrue(Orders[0].bQueue, "its bQueue");
	T.AssertTrue(Orders[1].Kind == n"Cancel", "Cancel");
	T.AssertEquals(2, Orders[1].Index, "the index cancelled");
	T.AssertTrue(Orders[2].Kind == n"Rally", "Rally");
	T.AssertEquals(900, int(Orders[2].Target.Y), "the rally point");
	T.AssertEquals(5, Orders[2].SourceIndex, "the rally source");
}

void Test_CommandLibraryConstruction(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	HOLCommandScript::IssuePlace(Commands, 3, n"House", FVector(2000.0, 3000.0, 0.0), CLUnits(20, 21), true);
	HOLCommandScript::IssueConstructionCancel(Commands, 3, nullptr);
	TArray<FHOLScriptOrderRead> Orders = HOLCommandScript::ConsumeConstructions(Commands);
	T.AssertEquals(2, Orders.Num(), "two construction orders");
	T.AssertTrue(Orders[0].Kind == n"Place", "Place");
	T.AssertTrue(Orders[0].TypeName == n"House", "its type");
	T.AssertEquals(3000, int(Orders[0].Target.Y), "its point");
	T.AssertEquals(2, Orders[0].UnitIds.Num(), "its builders");
	T.AssertEquals(3, Orders[0].PlayerIndex, "its player");
	T.AssertTrue(Orders[1].Kind == n"Cancel", "Cancel");
}

void Test_CommandLibraryRageAndGarrisonRefusals(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	HOLCommandScript::IssueRage(Commands, 2, CLUnits(30, 31));
	HOLCommandScript::IssueRage(Commands, 99, CLUnits(30, 31));
	TArray<FHOLScriptOrderRead> Rages = HOLCommandScript::ConsumeRages(Commands);
	T.AssertEquals(1, Rages.Num(), "a slot out of range is refused");
	T.AssertEquals(2, Rages[0].PlayerIndex, "the slot kept");
	T.AssertEquals(31, Rages[0].UnitIds[1], "its units");
	HOLCommandScript::IssueGarrison(Commands, 2, CLUnits(30), nullptr);
	HOLCommandScript::IssueUngarrisonAll(Commands, 2, nullptr);
	TArray<FHOLScriptOrderRead> Garrisons = HOLCommandScript::ConsumeGarrisons(Commands);
	T.AssertEquals(0, Garrisons.Num(), "no building, no garrison order");
}

void Test_CommandLibraryNoSubsystem(FUnitTest& T)
{
	UHOLCommandSubsystem None = nullptr;
	HOLCommandScript::IssueMove(None, CLUnits(1, 2), FVector(0.0, 0.0, 0.0), 0);
	int Pending = HOLCommandScript::GetNumPendingMoveOrders(None);
	T.AssertEquals(0, Pending, "a null subsystem is a no-op");
	TArray<FHOLScriptOrderRead> Orders = HOLCommandScript::ConsumeMoves(None);
	T.AssertEquals(0, Orders.Num(), "and reads nothing");
}
