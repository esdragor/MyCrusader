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

// ---- R2b (GALADRIEL-1119aa): the verdict of the queue, HOLProductionBoard::QueueVerdict. NOT COMPILED when written. ----
// QueueVerdict(bProduces, bTechnology, bTechFactionOpen, bTechBlocked, bSiegeBlocked, bTierTooLow, bUnitFactionOpen, QueueLength, MaxQueue, Stock, Cost):
// 0 Enqueue, 1 NotProduced, 2 QueueFull, 3 NotEnoughResources, 4 TechFaction, 5 TechBlocked, 6 SiegeBlocked, 7 TierTooLow, 8 UnitFaction.
// THE HARD POINT: a research and a unit do not refuse in the same order. A research says "cannot pay" through its technology check (5) BEFORE the
// queue is asked (2, 3); a unit meets the queue (2 full, then 3 no resources) after its siege, age and faction gates.

int PBQ(bool bProduces, bool bTechnology, bool bTechFactionOpen, bool bTechBlocked, bool bSiegeBlocked, bool bTierTooLow, bool bUnitFactionOpen, int Length, int Max, FHOLResources Stock, FHOLResources Cost)
{
	return int(HOLProductionBoard::QueueVerdict(bProduces, bTechnology, bTechFactionOpen, bTechBlocked, bSiegeBlocked, bTierTooLow, bUnitFactionOpen, Length, Max, Stock, Cost));
}

// A unit of the building, every gate open.
int PBUnit(int Length, int Max, FHOLResources Stock, FHOLResources Cost)
{
	return PBQ(true, false, true, false, false, false, true, Length, Max, Stock, Cost);
}

// A research of the building's faction, its check passed (or not).
int PBTech(bool bTechBlocked, int Length, int Max, FHOLResources Stock, FHOLResources Cost)
{
	return PBQ(false, true, true, bTechBlocked, false, false, true, Length, Max, Stock, Cost);
}

// One case for each code 0 to 8.
void Test_ProductionBoard_QueueCodes(FUnitTest& T)
{
	FHOLResources Rich = PBRes(100, 100, 100, 100);
	FHOLResources Cost = PBRes(50, 0, 0, 20);
	T.AssertEquals(int(EHOLQueueVerdict::Enqueue), PBUnit(0, 5, Rich, Cost), "0: a unit with room and money is queued");
	T.AssertEquals(int(EHOLQueueVerdict::Enqueue), PBTech(false, 0, 5, Rich, Cost), "0: a research with room and money is queued");
	T.AssertEquals(int(EHOLQueueVerdict::NotProduced), PBQ(false, false, true, false, false, false, true, 0, 5, Rich, Cost), "1: a type neither produced here nor a technology");
	T.AssertEquals(int(EHOLQueueVerdict::QueueFull), PBUnit(5, 5, Rich, Cost), "2: a unit in a full queue");
	T.AssertEquals(int(EHOLQueueVerdict::NotEnoughResources), PBUnit(0, 5, PBRes(10, 0, 0, 0), Cost), "3: a unit the stock cannot pay");
	T.AssertEquals(int(EHOLQueueVerdict::TechFaction), PBQ(false, true, false, false, false, false, true, 0, 5, Rich, Cost), "4: a technology of another faction");
	T.AssertEquals(int(EHOLQueueVerdict::TechBlocked), PBTech(true, 0, 5, Rich, Cost), "5: a research blocked by its technology check");
	T.AssertEquals(int(EHOLQueueVerdict::SiegeBlocked), PBQ(true, false, true, false, true, false, true, 0, 5, Rich, Cost), "6: a siege engine below its tier");
	T.AssertEquals(int(EHOLQueueVerdict::TierTooLow), PBQ(true, false, true, false, false, true, true, 0, 5, Rich, Cost), "7: a unit below its age");
	T.AssertEquals(int(EHOLQueueVerdict::UnitFaction), PBQ(true, false, true, false, false, false, false, 0, 5, Rich, Cost), "8: a unit of another faction");
}

// The two crossed cases: the same two faults (queue full AND no resources), two different refusals.
void Test_ProductionBoard_QueueCrossed(FUnitTest& T)
{
	FHOLResources Poor = PBRes(0, 0, 0, 0);
	FHOLResources Cost = PBRes(50, 0, 0, 20);
	T.AssertEquals(int(EHOLQueueVerdict::QueueFull), PBUnit(5, 5, Poor, Cost), "a unit with a full queue AND no resources: queue full (2), not 3");
	T.AssertEquals(int(EHOLQueueVerdict::TechBlocked), PBTech(true, 5, 5, Poor, Cost), "a research with no resources (its check blocks) AND a full queue: the technology check (5), not 2");
	// A research whose check passed meets the queue like a unit.
	T.AssertEquals(int(EHOLQueueVerdict::QueueFull), PBTech(false, 5, 5, PBRes(100, 100, 100, 100), Cost), "a research that may be paid, queue full: 2");
	// The edge of the queue: one place left queues, a queue at its maximum refuses.
	T.AssertEquals(int(EHOLQueueVerdict::Enqueue), PBUnit(4, 5, PBRes(100, 100, 100, 100), Cost), "one place left: queued");
	T.AssertEquals(int(EHOLQueueVerdict::QueueFull), PBUnit(5, 5, PBRes(100, 100, 100, 100), Cost), "queue at its maximum: refused");
}

// The order of the other gates, and the cost to the unit.
void Test_ProductionBoard_QueueOrder(FUnitTest& T)
{
	FHOLResources Poor = PBRes(0, 0, 0, 0);
	FHOLResources Cost = PBRes(50, 0, 0, 20);
	T.AssertEquals(int(EHOLQueueVerdict::NotProduced), PBQ(false, false, false, true, true, true, false, 5, 5, Poor, Cost), "not produced here: 1 before siege, age, faction, queue, cost");
	T.AssertEquals(int(EHOLQueueVerdict::TechFaction), PBQ(false, true, false, true, false, false, true, 5, 5, Poor, Cost), "a research: faction (4) before its check (5)");
	T.AssertEquals(int(EHOLQueueVerdict::SiegeBlocked), PBQ(true, false, true, false, true, true, false, 5, 5, Poor, Cost), "a unit: siege (6) before age, faction, queue, cost");
	T.AssertEquals(int(EHOLQueueVerdict::TierTooLow), PBQ(true, false, true, false, false, true, false, 5, 5, Poor, Cost), "a unit: age (7) before faction (8), queue, cost");
	T.AssertEquals(int(EHOLQueueVerdict::UnitFaction), PBQ(true, false, true, false, false, false, false, 5, 5, Poor, Cost), "a unit: faction (8) before queue, cost");
	// A cost exactly equal to the stock is paid (Covers), one more is not.
	T.AssertEquals(int(EHOLQueueVerdict::Enqueue), PBUnit(0, 5, Cost, Cost), "a stock equal to the cost pays");
	T.AssertEquals(int(EHOLQueueVerdict::NotEnoughResources), PBUnit(0, 5, PBRes(50, 0, 0, 19), Cost), "one gold short: 3");
	T.AssertEquals(int(EHOLQueueVerdict::Enqueue), PBUnit(0, 5, Poor, PBRes(0, 0, 0, 0)), "a free unit is queued on an empty stock");
}
