// The order of the gates of a production order and the cancel verdict (MIG-CPP step 5, lot R2a, GALADRIEL-1119z; D839): tests of
// HOLProductionBoard::OrderGateVerdict, CancelVerdict and StockAfterCancel (Script/Buildings/HOLProductionBoard.as). NOT COMPILED when written.
// Gates in the C++ order of UHOLProductionSubsystem::ApplyOrders: no player state, not its building, no type row, not finished (unless an early rally),
// forum down (unless a rally). Verdict as a whole number: 0 Pass 1 NoPlayerState 2 NotItsBuilding 3 NoType 4 NotFinished 5 ForumDown.
// Arguments: OrderGateVerdict(bHasState, bOwner, bHasType, bFinished, bEarlyRally, bForumDown, bRally).

FHOLResources PBRes(int Food, int Wood, int Stone, int Gold)
{
	FHOLResources Out;
	Out.Food = Food;
	Out.Wood = Wood;
	Out.Stone = Stone;
	Out.Gold = Gold;
	return Out;
}

bool PBResEq(FHOLResources A, FHOLResources B)
{
	return A.Food == B.Food && A.Wood == B.Wood && A.Stone == B.Stone && A.Gold == B.Gold;
}

// Each gate alone, the other facts being good: a queue order of a finished building of its owner passes.
void Test_ProductionBoard_OrderGates(FUnitTest& T)
{
	int Pass = int(EHOLOrderGateVerdict::Pass);
	T.AssertEquals(Pass, int(HOLProductionBoard::OrderGateVerdict(true, true, true, true, false, false, false)), "a good order passes");
	T.AssertEquals(int(EHOLOrderGateVerdict::NoPlayerState), int(HOLProductionBoard::OrderGateVerdict(false, true, true, true, false, false, false)), "no player state");
	T.AssertEquals(int(EHOLOrderGateVerdict::NotItsBuilding), int(HOLProductionBoard::OrderGateVerdict(true, false, true, true, false, false, false)), "not its building");
	T.AssertEquals(int(EHOLOrderGateVerdict::NoType), int(HOLProductionBoard::OrderGateVerdict(true, true, false, true, false, false, false)), "no type row");
	T.AssertEquals(int(EHOLOrderGateVerdict::NotFinished), int(HOLProductionBoard::OrderGateVerdict(true, true, true, false, false, false, false)), "a building under construction");
	T.AssertEquals(int(EHOLOrderGateVerdict::ForumDown), int(HOLProductionBoard::OrderGateVerdict(true, true, true, true, false, true, false)), "a forum in ruin");
}

// Two gates at once: the first of the C++ order wins.
void Test_ProductionBoard_OrderGateOrder(FUnitTest& T)
{
	T.AssertEquals(int(EHOLOrderGateVerdict::NoPlayerState), int(HOLProductionBoard::OrderGateVerdict(false, false, true, true, false, false, false)), "no state AND not its building: no state first");
	T.AssertEquals(int(EHOLOrderGateVerdict::NoPlayerState), int(HOLProductionBoard::OrderGateVerdict(false, false, false, false, false, true, false)), "every gate shut: no state first");
	T.AssertEquals(int(EHOLOrderGateVerdict::NotItsBuilding), int(HOLProductionBoard::OrderGateVerdict(true, false, false, false, false, true, false)), "not its building before no type, not finished, forum");
	T.AssertEquals(int(EHOLOrderGateVerdict::NoType), int(HOLProductionBoard::OrderGateVerdict(true, true, false, false, false, true, false)), "no type before not finished (silent)");
	T.AssertEquals(int(EHOLOrderGateVerdict::NotFinished), int(HOLProductionBoard::OrderGateVerdict(true, true, true, false, false, true, false)), "not finished AND forum down: not finished first");
}

// The rally point is the owner's to move while the building rises or the forum is down (D351, the order of 2026-10-19).
void Test_ProductionBoard_RallyGates(FUnitTest& T)
{
	int Pass = int(EHOLOrderGateVerdict::Pass);
	T.AssertEquals(Pass, int(HOLProductionBoard::OrderGateVerdict(true, true, true, false, true, false, true)), "an early rally passes on a building not finished");
	T.AssertEquals(int(EHOLOrderGateVerdict::NotFinished), int(HOLProductionBoard::OrderGateVerdict(true, true, true, false, false, false, true)), "a rally of a building that produces nothing waits for the end");
	T.AssertEquals(Pass, int(HOLProductionBoard::OrderGateVerdict(true, true, true, true, false, true, true)), "a rally passes with the forum down");
	T.AssertEquals(Pass, int(HOLProductionBoard::OrderGateVerdict(true, true, true, false, true, true, true)), "an early rally passes with the forum down too");
	T.AssertEquals(int(EHOLOrderGateVerdict::NotItsBuilding), int(HOLProductionBoard::OrderGateVerdict(true, false, true, false, true, true, true)), "a rally of another player's building is still refused");
	T.AssertEquals(int(EHOLOrderGateVerdict::NoPlayerState), int(HOLProductionBoard::OrderGateVerdict(false, true, true, true, true, false, true)), "a rally with no state is still refused");
}

// A cancel: an invalid index is ignored silently, a valid one gives the cost of the type back in full (J3 / D282).
void Test_ProductionBoard_Cancel(FUnitTest& T)
{
	T.AssertEquals(int(EHOLProductionCancelVerdict::CancelItem), int(HOLProductionBoard::CancelVerdict(true)), "a valid index is cancelled");
	T.AssertEquals(int(EHOLProductionCancelVerdict::IgnoreOrder), int(HOLProductionBoard::CancelVerdict(false)), "an invalid index is ignored");

	FHOLResources Cost = PBRes(50, 0, 0, 20);
	T.AssertTrue(PBResEq(HOLProductionBoard::StockAfterCancel(PBRes(20, 5, 0, 0), Cost), PBRes(70, 5, 0, 20)), "the cost goes back to the stock");
	T.AssertTrue(PBResEq(HOLProductionBoard::StockAfterCancel(PBRes(0, 0, 0, 0), Cost), Cost), "an empty stock gets exactly the cost");
	T.AssertTrue(PBResEq(HOLProductionBoard::StockAfterCancel(PBRes(20, 5, 0, 0), PBRes(0, 0, 0, 0)), PBRes(20, 5, 0, 0)), "a free item gives nothing back");
}
