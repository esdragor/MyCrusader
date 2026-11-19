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
