// GLORFINDEL-1124e (D900, F3): the pending cast of an approached Sacred Light. The hero walked to the stop point (HOLFactionPowerServer::ApproachPointOf), the cast waits;
// every step PendingStepOf says: nothing pending (0), still walking (1), arrived within the range of the aimed point (2: cast now). Values are COMPUTED by the rule.

FHOLPendingCast MakePending(int Slot, const FVector& Point)
{
	FHOLPendingCast Order;
	Order.Slot = Slot;
	Order.PowerIndex = 0;
	Order.Point = Point;
	return Order;
}

// The hero is still 2000 cm from the point, the range is 1200 cm: the cast waits (step 1), then at the stop point of ApproachPointOf (1150 cm from the point) it is due (step 2).
void Test_FactionApproachPendingWaitsThenCastsOnArrival(FUnitTest& T)
{
	TArray<FHOLPendingCast> Pending;
	const FVector Hero = FVector(0.0, 0.0, 0.0);
	const FVector Point = FVector(2000.0, 0.0, 0.0);
	HOLFactionPowerServer::SetPending(Pending, MakePending(0, Point));
	const int Walking = HOLFactionPowerServer::PendingStepOf(Pending, 0, Hero, 1200.0);
	T.AssertEquals(Walking, 1, "2000 cm from the point with a range of 1200 cm: the hero is still walking");
	const FHOLFactionApproach Walk = HOLFactionPowerServer::ApproachPointOf(Hero, Point, 1200.0);
	const int Arrived = HOLFactionPowerServer::PendingStepOf(Pending, 0, Walk.StopPoint, 1200.0);
	T.AssertEquals(Arrived, 2, "at the stop point the aimed point is in range: the cast is due");
}

// Another slot has nothing pending; a new order of the hero cancels the pending cast (default: any new order cancels).
void Test_FactionApproachPendingIsPerSlotAndCancelledByAnOrder(FUnitTest& T)
{
	TArray<FHOLPendingCast> Pending;
	HOLFactionPowerServer::SetPending(Pending, MakePending(0, FVector(2000.0, 0.0, 0.0)));
	const int Other = HOLFactionPowerServer::PendingStepOf(Pending, 1, FVector(0.0, 0.0, 0.0), 1200.0);
	T.AssertEquals(Other, 0, "slot 1 has no pending cast");
	HOLFactionPowerServer::CancelPending(Pending, 0);
	const int After = HOLFactionPowerServer::PendingStepOf(Pending, 0, FVector(1000.0, 0.0, 0.0), 1200.0);
	T.AssertEquals(After, 0, "a cancelled cast never fires, even once the hero is in range");
}

// A second aimed cast replaces the first (one pending cast per slot): the new point decides the arrival.
void Test_FactionApproachSecondCastReplacesTheFirst(FUnitTest& T)
{
	TArray<FHOLPendingCast> Pending;
	HOLFactionPowerServer::SetPending(Pending, MakePending(0, FVector(2000.0, 0.0, 0.0)));
	HOLFactionPowerServer::SetPending(Pending, MakePending(0, FVector(-5000.0, 0.0, 0.0)));
	T.AssertEquals(Pending.Num(), 1, "one pending cast per slot");
	const int Step = HOLFactionPowerServer::PendingStepOf(Pending, 0, FVector(1000.0, 0.0, 0.0), 1200.0);
	T.AssertEquals(Step, 1, "the first point (2000) would be in range from 1000, the replacing one (-5000) is not");
}
