// BUG (ARWEN-1126m3, proof spec_voie_queue_refused.png): the REAL click of a Chapel cell reaches the server by ServerQueueProduction ->
// HOLRpcBridge::QueueProduction -> AHOLPlayerController::DoQueueProduction -> IssueProductionOrder, which never goes through
// HOLPlayerOrders::QueueProductionFor (the lane gate of D893 step 3, HOLPlayerOrders.as:214). A Templars seat with no lane chosen queued the
// Infirmary Brother. The RPC now asks HOLRpc::QueueProductionLaneRefusal (the seat's own read, the specialization board) before the bridge.
// 0 = the order goes on, 4 = a lane unit the seat has not unlocked (nothing queued).

UHOLSpecializationBoardScript QLBoard()
{
	UHOLSpecializationBoardScript Board = NewObject(GetTransientPackage(), UHOLSpecializationBoardScript);
	Board.SetSeatFaction(0, int(EHOLFaction::Templars));
	Board.SetSeatFaction(1, int(EHOLFaction::DawnOrder));
	return Board;
}

void Test_RpcQueueRefusesALaneUnitBeforeTheLaneIsChosen(FUnitTest& T)
{
	UHOLSpecializationBoardScript Board = QLBoard();
	const int NoLaneBrother = HOLRpc::QueueProductionLaneRefusal(Board, 0, n"InfirmaryBrother");
	const int NoLaneSergeant = HOLRpc::QueueProductionLaneRefusal(Board, 0, n"TempleSergeant");
	T.AssertEquals(4, NoLaneBrother, "a Templars seat with no lane: the Infirmary Brother is refused by the RPC (nothing queued)");
	T.AssertEquals(4, NoLaneSergeant, "a Templars seat with no lane: the Temple Sergeant is refused too");
}

void Test_RpcQueueFollowsTheChosenLane(FUnitTest& T)
{
	UHOLSpecializationBoardScript Board = QLBoard();
	// Seat 0 chooses Faith (option 1) at tier 1 and the delay runs out (the state the choice writes).
	FHOLSpecializationState& State = Board.StateOf(0);
	T.AssertEquals(HOLSpecialization::Ok, HOLSpecialization::Start(State, 1, 1, 1), "seat 0 starts Faith");
	HOLSpecialization::Advance(State, HOLSpecialization::DelayMs(1), true);

	const int Brother = HOLRpc::QueueProductionLaneRefusal(Board, 0, n"InfirmaryBrother");
	const int Sergeant = HOLRpc::QueueProductionLaneRefusal(Board, 0, n"TempleSergeant");
	const int Monk = HOLRpc::QueueProductionLaneRefusal(Board, 0, n"Monk");
	T.AssertEquals(0, Brother, "Faith chosen: the Infirmary Brother goes on to the production");
	T.AssertEquals(4, Sergeant, "Faith chosen: the Temple Sergeant (Iron) stays refused");
	T.AssertEquals(0, Monk, "a common unit goes on whatever the lane");
}

void Test_RpcQueueNoBoardLeavesTheOrderAlone(FUnitTest& T)
{
	const int NoBoard = HOLRpc::QueueProductionLaneRefusal(nullptr, 0, n"InfirmaryBrother");
	T.AssertEquals(0, NoBoard, "no board in the world (a test world): the gate stays out, as QueueProductionAnswer does");
}
