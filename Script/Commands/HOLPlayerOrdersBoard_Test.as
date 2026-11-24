// P1b: agreement test. The way the bridge answers (HOLPlayerOrdersBoard::Answer*, what UHOLPlayerOrdersBoardScript returns to HOLPlayerOrders.cpp) and the
// direct rule (HOLPlayerOrders::*) must give the same verdict AND leave the same orders in the queue, for the cases of HOLPlayerOrders_Test.as.
// The C++ dispatch itself (HOLPlayerOrders.cpp -> UHOLPlayerOrdersBoard::Ask* -> the script class) needs a world: it is proven by the network replay, not here.
// NOT COMPILED at the time of writing. Red planned: in HOLPlayerOrdersBoard.as, Answer(): return bDone ? 1 : 0 -> return bDone ? 0 : 1 (every case fails),
// or finer: in AnswerStop, pass !bHold: Test_PlayerOrdersBoardAgreesStopHold fails on the Kind of the queued order (Stop vs Hold). Put once, put back, see green.

TArray<int> POBUnits(int Count)
{
	TArray<int> Ids;
	for (int I = 0; I < Count; I++)
	{
		Ids.Add(100 + I);
	}
	return Ids;
}

// 1 when the verdict is accepted, 0 when refused: the same as the direct bool.
int POBFromBool(bool bDone)
{
	return bDone ? 1 : 0;
}

void Test_PlayerOrdersBoardAgreesIsAccepted(FUnitTest& T)
{
	int MaxSlots = HOLCommandScript::GetMaxSlots();
	TArray<int> Slots;
	TArray<int> Counts;
	Slots.Add(1); Counts.Add(200);
	Slots.Add(0); Counts.Add(1);
	Slots.Add(MaxSlots - 1); Counts.Add(0);
	Slots.Add(1); Counts.Add(201);
	Slots.Add(-1); Counts.Add(1);
	Slots.Add(MaxSlots); Counts.Add(1);
	Slots.Add(1); Counts.Add(-1);
	for (int I = 0; I < Slots.Num(); I++)
	{
		int Bridge = HOLPlayerOrdersBoard::AnswerIsAccepted(Slots[I], Counts[I]);
		int Direct = POBFromBool(HOLPlayerOrders::IsAccepted(Slots[I], Counts[I]));
		T.AssertEquals(Direct, Bridge, "IsAccepted: the bridge and the rule agree (case " + I + ")");
	}
	int Refused = HOLPlayerOrdersBoard::AnswerIsAccepted(1, 201);
	T.AssertEquals(0, Refused, "201 units: the bridge answers 0, not -1 (-1 is the C++ 'no board')");
}

void Test_PlayerOrdersBoardAgreesMove(FUnitTest& T)
{
	UHOLCommandSubsystem ViaBridge = HOLCommandScript::MakeDetachedForTest();
	UHOLCommandSubsystem Direct = HOLCommandScript::MakeDetachedForTest();
	FVector Point(1200.0, -340.0, 5.0);
	int Answer = HOLPlayerOrdersBoard::AnswerMove(ViaBridge, 0, POBUnits(2), Point, true, FVector2D(0.0, 0.0), false);
	bool bDone = HOLPlayerOrders::Move(Direct, 0, POBUnits(2), Point, true, FVector2D(0.0, 0.0), false);
	T.AssertEquals(POBFromBool(bDone), Answer, "move: same verdict");
	TArray<FHOLScriptOrderRead> A = HOLCommandScript::ConsumeMoves(ViaBridge);
	TArray<FHOLScriptOrderRead> B = HOLCommandScript::ConsumeMoves(Direct);
	T.AssertEquals(B.Num(), A.Num(), "move: same number of queued orders");
	T.AssertEquals(1, A.Num(), "move: one order");
	T.AssertTrue(A[0].Kind == B[0].Kind, "move: same kind");
	T.AssertEquals(B[0].UnitIds.Num(), A[0].UnitIds.Num(), "move: same units");
	T.AssertEquals(int(B[0].Target.X), int(A[0].Target.X), "move: same target X");
	T.AssertTrue(A[0].bQueue == B[0].bQueue, "move: same Shift flag");
	T.AssertTrue(A[0].bChain, "move: the bridge keeps the chain of the player's order (D328)");
	T.AssertTrue(A[0].bChain == B[0].bChain, "move: same chain flag as the direct rule");
}

void Test_PlayerOrdersBoardAgreesRefusals(FUnitTest& T)
{
	UHOLCommandSubsystem ViaBridge = HOLCommandScript::MakeDetachedForTest();
	int MaxSlots = HOLCommandScript::GetMaxSlots();
	FVector Point(10.0, 20.0, 0.0);
	T.AssertEquals(0, HOLPlayerOrdersBoard::AnswerMove(ViaBridge, -1, POBUnits(1), Point, false, FVector2D(0.0, 0.0), false), "slot -1: move refused (0)");
	T.AssertEquals(0, HOLPlayerOrdersBoard::AnswerMove(ViaBridge, MaxSlots, POBUnits(1), Point, false, FVector2D(0.0, 0.0), false), "slot MaxSlots: move refused");
	T.AssertEquals(0, HOLPlayerOrdersBoard::AnswerMove(ViaBridge, 0, POBUnits(201), Point, false, FVector2D(0.0, 0.0), false), "201 units: move refused");
	T.AssertEquals(0, HOLPlayerOrdersBoard::AnswerAttack(ViaBridge, 0, POBUnits(201), 55, false), "201 units: attack refused");
	T.AssertEquals(0, HOLPlayerOrdersBoard::AnswerAttackMove(ViaBridge, -1, POBUnits(1), Point, false), "slot -1: attack-move refused");
	T.AssertEquals(0, HOLPlayerOrdersBoard::AnswerStop(ViaBridge, MaxSlots, POBUnits(1), false), "slot MaxSlots: stop refused");
	T.AssertEquals(0, HOLPlayerOrdersBoard::AnswerToggleDeploy(ViaBridge, -1, POBUnits(1)), "slot -1: deploy refused");
	T.AssertEquals(0, HOLPlayerOrdersBoard::AnswerMove(nullptr, 0, POBUnits(1), Point, false, FVector2D(0.0, 0.0), false), "no subsystem: refused");
	AHOLBuilding NoBuilding = nullptr;
	T.AssertEquals(0, HOLPlayerOrdersBoard::AnswerAttackBuilding(ViaBridge, 0, POBUnits(2), NoBuilding, false), "no building: refused");
	int Pending = HOLCommandScript::GetNumPendingMoveOrders(ViaBridge);
	T.AssertEquals(0, Pending, "nothing entered the queue");
	T.AssertEquals(1, HOLPlayerOrdersBoard::AnswerMove(ViaBridge, 0, POBUnits(200), Point, false, FVector2D(0.0, 0.0), false), "200 units: accepted (1)");
}

void Test_PlayerOrdersBoardAgreesStopHold(FUnitTest& T)
{
	UHOLCommandSubsystem ViaBridge = HOLCommandScript::MakeDetachedForTest();
	UHOLCommandSubsystem Direct = HOLCommandScript::MakeDetachedForTest();
	T.AssertEquals(1, HOLPlayerOrdersBoard::AnswerStop(ViaBridge, 0, POBUnits(2), false), "stop accepted");
	T.AssertEquals(1, HOLPlayerOrdersBoard::AnswerStop(ViaBridge, 0, POBUnits(2), true), "hold accepted");
	T.AssertEquals(1, HOLPlayerOrdersBoard::AnswerToggleDeploy(ViaBridge, 1, POBUnits(3)), "deploy toggle accepted");
	HOLPlayerOrders::Stop(Direct, 0, POBUnits(2), false);
	HOLPlayerOrders::Stop(Direct, 0, POBUnits(2), true);
	HOLPlayerOrders::ToggleDeploy(Direct, 1, POBUnits(3));
	TArray<FHOLScriptOrderRead> A = HOLCommandScript::ConsumeMoves(ViaBridge);
	TArray<FHOLScriptOrderRead> B = HOLCommandScript::ConsumeMoves(Direct);
	T.AssertEquals(3, A.Num(), "three orders through the bridge");
	T.AssertEquals(B.Num(), A.Num(), "same number as the direct rule");
	T.AssertTrue(A[0].Kind == n"Stop" && A[0].Kind == B[0].Kind, "bHold false: Stop, as the direct rule");
	T.AssertTrue(A[1].Kind == n"Hold" && A[1].Kind == B[1].Kind, "bHold true: Hold, as the direct rule");
	T.AssertTrue(A[2].Kind == n"ToggleDeploy" && A[2].Kind == B[2].Kind, "ToggleDeploy, as the direct rule");
	T.AssertEquals(1, A[2].PlayerIndex, "the player who gave the deploy");
}

void Test_PlayerOrdersBoardAgreesAttackKinds(FUnitTest& T)
{
	UHOLCommandSubsystem ViaBridge = HOLCommandScript::MakeDetachedForTest();
	UHOLCommandSubsystem Direct = HOLCommandScript::MakeDetachedForTest();
	T.AssertEquals(1, HOLPlayerOrdersBoard::AnswerAttack(ViaBridge, 0, POBUnits(2), 55, false), "attack accepted");
	T.AssertEquals(1, HOLPlayerOrdersBoard::AnswerAttackMove(ViaBridge, 0, POBUnits(2), FVector(500.0, 600.0, 0.0), true), "attack-move accepted");
	HOLPlayerOrders::Attack(Direct, 0, POBUnits(2), 55, false);
	HOLPlayerOrders::AttackMove(Direct, 0, POBUnits(2), FVector(500.0, 600.0, 0.0), true);
	TArray<FHOLScriptOrderRead> A = HOLCommandScript::ConsumeMoves(ViaBridge);
	TArray<FHOLScriptOrderRead> B = HOLCommandScript::ConsumeMoves(Direct);
	T.AssertEquals(2, A.Num(), "two orders through the bridge");
	T.AssertEquals(B.Num(), A.Num(), "same number as the direct rule");
	T.AssertTrue(A[0].Kind == n"Attack" && A[0].Kind == B[0].Kind, "first: Attack");
	T.AssertEquals(55, A[0].TargetUnitId, "the target unit is kept");
	T.AssertTrue(A[0].bChain && A[0].bChain == B[0].bChain, "the attack keeps the chain of the player's order");
	T.AssertTrue(A[1].Kind == n"AttackMove" && A[1].Kind == B[1].Kind, "second: AttackMove");
	T.AssertTrue(A[1].bQueue, "Shift kept on the attack-move");
}

// ---------------- P2 agreement tests: the bridge (Answer*) and the direct rule leave the same orders. NOT COMPILED. ----------------
// Red planned (P2): in HOLPlayerOrdersBoard.as, AnswerPlace: pass an empty TArray instead of UnitIds: Test_PlayerOrdersBoardAgreesPlace fails on the number of builders.

void Test_PlayerOrdersBoardAgreesPlace(FUnitTest& T)
{
	UHOLCommandSubsystem ViaBridge = HOLCommandScript::MakeDetachedForTest();
	UHOLCommandSubsystem Direct = HOLCommandScript::MakeDetachedForTest();
	FVector Point(700.0, 800.0, 0.0);
	int Answer = HOLPlayerOrdersBoard::AnswerPlace(ViaBridge, 0, n"House", Point, POBUnits(3), true);
	bool bDone = HOLPlayerOrders::Place(Direct, 0, n"House", Point, POBUnits(3), true);
	T.AssertEquals(POBFromBool(bDone), Answer, "place: same verdict");
	TArray<FHOLScriptOrderRead> A = HOLCommandScript::ConsumeConstructions(ViaBridge);
	TArray<FHOLScriptOrderRead> B = HOLCommandScript::ConsumeConstructions(Direct);
	T.AssertEquals(1, A.Num(), "place: one order through the bridge");
	T.AssertEquals(B.Num(), A.Num(), "place: same number of orders");
	T.AssertTrue(A[0].TypeName == B[0].TypeName, "place: same type");
	T.AssertEquals(B[0].UnitIds.Num(), A[0].UnitIds.Num(), "place: same builders");
	T.AssertTrue(A[0].bQueue == B[0].bQueue, "place: same Shift flag");
	T.AssertEquals(0, HOLPlayerOrdersBoard::AnswerPlace(ViaBridge, -1, n"House", Point, POBUnits(1), false), "place: slot -1 answers 0, not -1");
}

void Test_PlayerOrdersBoardAgreesGatherAndDropOff(FUnitTest& T)
{
	UHOLCommandSubsystem ViaBridge = HOLCommandScript::MakeDetachedForTest();
	UHOLCommandSubsystem Direct = HOLCommandScript::MakeDetachedForTest();
	FVector Spot(30.0, 40.0, 0.0);
	T.AssertEquals(1, HOLPlayerOrdersBoard::AnswerGather(ViaBridge, 1, POBUnits(2), 4, false, true, Spot), "gather accepted through the bridge");
	HOLPlayerOrders::Gather(Direct, 1, POBUnits(2), 4, false, true, Spot);
	AHOLBuilding NoBuilding = nullptr;
	T.AssertEquals(1, HOLPlayerOrdersBoard::AnswerDropOff(ViaBridge, 1, POBUnits(2), NoBuilding, Spot, false, false), "drop-off (not a deposit) accepted through the bridge");
	HOLPlayerOrders::DropOff(Direct, 1, POBUnits(2), NoBuilding, Spot, false, false);
	TArray<FHOLScriptOrderRead> GA = HOLCommandScript::ConsumeGathers(ViaBridge);
	TArray<FHOLScriptOrderRead> GB = HOLCommandScript::ConsumeGathers(Direct);
	T.AssertEquals(1, GA.Num(), "one gather through the bridge");
	T.AssertEquals(GB.Num(), GA.Num(), "same number of gathers as the direct rule");
	T.AssertEquals(GB[0].SourceIndex, GA[0].SourceIndex, "same source");
	TArray<FHOLScriptOrderRead> MA = HOLCommandScript::ConsumeMoves(ViaBridge);
	TArray<FHOLScriptOrderRead> MB = HOLCommandScript::ConsumeMoves(Direct);
	T.AssertEquals(1, MA.Num(), "one move through the bridge");
	T.AssertEquals(MB.Num(), MA.Num(), "same number of moves as the direct rule");
	T.AssertTrue(MA[0].bChain && MA[0].bChain == MB[0].bChain, "the drop-off walk keeps the chain of the player's order");
}

void Test_PlayerOrdersBoardAgreesBuildingRefusals(FUnitTest& T)
{
	UHOLCommandSubsystem ViaBridge = HOLCommandScript::MakeDetachedForTest();
	AHOLBuilding NoBuilding = nullptr;
	FVector Spot(0.0, 0.0, 0.0);
	T.AssertEquals(0, HOLPlayerOrdersBoard::AnswerBuild(ViaBridge, 0, POBUnits(1), NoBuilding, false), "build: refused is 0, not -1");
	T.AssertEquals(0, HOLPlayerOrdersBoard::AnswerQueueProduction(ViaBridge, 0, NoBuilding, n"Spearman"), "production: refused");
	T.AssertEquals(0, HOLPlayerOrdersBoard::AnswerMarketTrade(ViaBridge, 0, NoBuilding, 0, 1, false), "market: refused");
	T.AssertEquals(0, HOLPlayerOrdersBoard::AnswerSetRally(ViaBridge, 0, NoBuilding, Spot, -1, false, NoBuilding, false), "rally: refused");
	T.AssertEquals(0, HOLPlayerOrdersBoard::AnswerGarrison(ViaBridge, 0, POBUnits(1), NoBuilding), "garrison: refused");
	T.AssertEquals(0, HOLPlayerOrdersBoard::AnswerUngarrisonAll(ViaBridge, 0, NoBuilding), "ungarrison: refused");
	T.AssertEquals(0, HOLPlayerOrdersBoard::AnswerGather(ViaBridge, 0, POBUnits(201), 0, false, false, Spot), "gather: 201 refused");
	T.AssertEquals(0, HOLCommandScript::ConsumeProductions(ViaBridge).Num(), "nothing queued");
}

// D893 step 1 (script half): the bridge's production answer takes the seat's specialization: a Templars seat that chose the Faith lane (tier 1 option 1) is refused the Iron
// lane unit (TempleSergeant: 0, nothing queued), gets its own lane unit (InfirmaryBrother) and a unit of no lane (Spearman); the answers come from the whole chain (rule -> queue).
void Test_PlayerOrdersBoardRefusesLockedLaneUnit(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "a test world is created");
	if (TestWorld == nullptr)
		return;
	AHOLBuilding Commandery = HOLTestWorld::SpawnBuilding(TestWorld, n"MainCentre", 0, FVector(0.0, 0.0, 0.0), EHOLConstructionState::Finished);
	T.AssertTrue(Commandery != nullptr, "a finished building to queue at");
	if (Commandery == nullptr)
		return;
	UHOLCommandSubsystem ViaBridge = HOLCommandScript::MakeDetachedForTest();
	FHOLSpecializationView Faith;
	Faith.Chosen.Add(-1);
	Faith.Chosen.Add(1);
	int Locked = HOLPlayerOrdersBoard::AnswerQueueProductionFor(ViaBridge, 0, Commandery, n"TempleSergeant", Faith, EHOLFaction::Templars);
	T.AssertEquals(0, Locked, "the Iron lane unit is refused to a seat of the Faith lane");
	int NumAfterLocked = HOLCommandScript::ConsumeProductions(ViaBridge).Num();
	T.AssertEquals(0, NumAfterLocked, "nothing queued for the locked lane unit");
	int Own = HOLPlayerOrdersBoard::AnswerQueueProductionFor(ViaBridge, 0, Commandery, n"InfirmaryBrother", Faith, EHOLFaction::Templars);
	T.AssertEquals(1, Own, "the Faith lane unit is accepted to a seat of the Faith lane");
	int Plain = HOLPlayerOrdersBoard::AnswerQueueProductionFor(ViaBridge, 0, Commandery, n"Spearman", Faith, EHOLFaction::Templars);
	T.AssertEquals(1, Plain, "a unit of no lane is accepted");
	int NumQueued = HOLCommandScript::ConsumeProductions(ViaBridge).Num();
	T.AssertEquals(2, NumQueued, "the two accepted productions are queued");
}