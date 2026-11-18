// MIG2-S3 L2 orders, first 8 (A1, D846): the pure refusals of the script RPCs ServerMoveUnits, ServerAttackUnit, ServerHealUnit, ServerAttackBuilding, ServerAttackMove, ServerStopUnits, ServerToggleDeploy and ServerMoveUnitsInLine of Script/Net/HOLRpcPlayer.as.
// The sender must hold a seat (0..7) and name at least one unit; a point must be finite; AttackBuilding needs a building. The rules (cap 200, spectator, formation) stay the C++ Do<X> bodies' (called by the bridge).
// RED FIRST for the main session: replace the body of each refusal in HOLRpc (UnitOrderRefusal, PointOrderRefusal, BuildingOrderRefusal, LineOrderRefusal, OrderIds) by a stub (`return 0;` / an empty array), or falsify one expected value
// (for example the "2" of "no unit named" in Test_RpcOrderUnitRefusal), run quick_check -Parts tests, see the count N-1/N, then restore.

void Test_RpcOrderUnitRefusal(FUnitTest& T)
{
	// AttackUnit, HealUnit, Stop and ToggleDeploy share UnitOrderRefusal.
	T.AssertEquals(0, HOLRpc::UnitOrderRefusal(0, 1), "slot 0 with one unit: passed to the C++ body");
	T.AssertEquals(0, HOLRpc::UnitOrderRefusal(7, 200), "slot 7 (the last) with many units is in range");
	T.AssertEquals(1, HOLRpc::UnitOrderRefusal(-1, 1), "a spectator (no slot) is refused");
	T.AssertEquals(1, HOLRpc::UnitOrderRefusal(8, 1), "a sender past the 8 slots is refused");
	T.AssertEquals(2, HOLRpc::UnitOrderRefusal(0, 0), "no unit named is refused");
	T.AssertEquals(1, HOLRpc::UnitOrderRefusal(-1, 0), "the seat is checked first");
}

void Test_RpcOrderPointRefusal(FUnitTest& T)
{
	// Move and AttackMove share PointOrderRefusal. The infinity is COMPUTED (1e300 * 1e300 overflows), the NaN too (infinity - infinity), not written as constants.
	const float Huge = 1e300;
	const float Infinite = Huge * Huge;
	const float NotANumber = Infinite - Infinite;
	T.AssertEquals(0, HOLRpc::PointOrderRefusal(0, 3, 1000.0, -250.5, 0.0), "a finite point is passed on");
	T.AssertEquals(1, HOLRpc::PointOrderRefusal(-1, 3, 1000.0, 0.0, 0.0), "a spectator is refused");
	T.AssertEquals(2, HOLRpc::PointOrderRefusal(0, 0, 1000.0, 0.0, 0.0), "no unit named is refused");
	T.AssertEquals(3, HOLRpc::PointOrderRefusal(0, 3, Infinite, 0.0, 0.0), "an infinite X is refused");
	T.AssertEquals(3, HOLRpc::PointOrderRefusal(0, 3, 0.0, -Infinite, 0.0), "a negative infinite Y is refused");
	T.AssertEquals(3, HOLRpc::PointOrderRefusal(0, 3, 0.0, 0.0, NotANumber), "a NaN Z is refused");
	T.AssertEquals(1, HOLRpc::PointOrderRefusal(9, 3, Infinite, 0.0, 0.0), "the seat is checked before the point");
}

void Test_RpcOrderBuildingRefusal(FUnitTest& T)
{
	T.AssertEquals(0, HOLRpc::BuildingOrderRefusal(0, 2, true), "units and a building: passed on");
	T.AssertEquals(1, HOLRpc::BuildingOrderRefusal(-1, 2, true), "a spectator is refused");
	T.AssertEquals(2, HOLRpc::BuildingOrderRefusal(0, 0, true), "no unit named is refused");
	T.AssertEquals(3, HOLRpc::BuildingOrderRefusal(0, 2, false), "no building named is refused");
	T.AssertEquals(2, HOLRpc::BuildingOrderRefusal(0, 0, false), "the units are checked before the building");
}

void Test_RpcOrderLineRefusal(FUnitTest& T)
{
	const float Huge = 1e300;
	const float Infinite = Huge * Huge;
	T.AssertEquals(0, HOLRpc::LineOrderRefusal(0, 5, 0.0, 0.0, 0.0, 500.0, 100.0, 0.0), "two finite points are passed on");
	T.AssertEquals(1, HOLRpc::LineOrderRefusal(-1, 5, 0.0, 0.0, 0.0, 500.0, 100.0, 0.0), "a spectator is refused");
	T.AssertEquals(2, HOLRpc::LineOrderRefusal(0, 0, 0.0, 0.0, 0.0, 500.0, 100.0, 0.0), "no unit named is refused");
	T.AssertEquals(3, HOLRpc::LineOrderRefusal(0, 5, Infinite, 0.0, 0.0, 500.0, 100.0, 0.0), "an infinite Start is refused");
	T.AssertEquals(3, HOLRpc::LineOrderRefusal(0, 5, 0.0, 0.0, 0.0, 500.0, Infinite, 0.0), "an infinite End is refused (the second point is read too)");
}

void Test_RpcOrderIds(FUnitTest& T)
{
	TArray<uint32> UnitIds;
	UnitIds.Add(7);
	UnitIds.Add(42);
	UnitIds.Add(100000);
	TArray<int> Ids = HOLRpc::OrderIds(UnitIds);
	T.AssertEquals(3, Ids.Num(), "every id is copied");
	T.AssertEquals(7, Ids[0], "the first id is kept");
	T.AssertEquals(42, Ids[1], "the order is kept");
	T.AssertEquals(100000, Ids[2], "the last id is kept");
	TArray<uint32> None;
	TArray<int> NoIds = HOLRpc::OrderIds(None);
	T.AssertEquals(0, NoIds.Num(), "an empty list stays empty");
}

// ---- L2 orders, last 8 (A2, D846): the pure refusals of ServerGatherUnits (UnitOrderRefusal), ServerPlaceBuilding, ServerBuildUnits and ServerGarrisonUnits (BuildingOrderRefusal), ServerDropOffUnits, ServerUngarrisonAll and ServerCancelBuilding, ServerSetRallyPoint.
// RED FIRST for the main session: make a body return 0 (`PlaceBuildingRefusal`, `DropOffRefusal`, `RallyPointRefusal`, `BuildingOnlyRefusal`) or falsify one expected value (the "4" of "no type name" in Test_RpcOrderPlaceBuildingRefusal), run quick_check -Parts tests, see N-1/N, then restore.

void Test_RpcOrderBuildingOnlyRefusal(FUnitTest& T)
{
	// UngarrisonAll and CancelBuilding: no unit list, so no "2".
	T.AssertEquals(0, HOLRpc::BuildingOnlyRefusal(0, true), "a seat and a building: passed on");
	T.AssertEquals(0, HOLRpc::BuildingOnlyRefusal(7, true), "slot 7 (the last) is in range");
	T.AssertEquals(1, HOLRpc::BuildingOnlyRefusal(-1, true), "a spectator is refused");
	T.AssertEquals(1, HOLRpc::BuildingOnlyRefusal(8, true), "a sender past the 8 slots is refused");
	T.AssertEquals(3, HOLRpc::BuildingOnlyRefusal(0, false), "no building named is refused");
	T.AssertEquals(1, HOLRpc::BuildingOnlyRefusal(-1, false), "the seat is checked first");
}

void Test_RpcOrderDropOffRefusal(FUnitTest& T)
{
	const float Huge = 1e300;
	const float Infinite = Huge * Huge;
	const float NotANumber = Infinite - Infinite;
	T.AssertEquals(0, HOLRpc::DropOffRefusal(0, 4, true, 100.0, 200.0, 0.0), "units, a building and a finite point: passed on");
	T.AssertEquals(1, HOLRpc::DropOffRefusal(-1, 4, true, 100.0, 200.0, 0.0), "a spectator is refused");
	T.AssertEquals(2, HOLRpc::DropOffRefusal(0, 0, true, 100.0, 200.0, 0.0), "no unit named is refused");
	T.AssertEquals(3, HOLRpc::DropOffRefusal(0, 4, false, 100.0, 200.0, 0.0), "no building named is refused");
	T.AssertEquals(3, HOLRpc::DropOffRefusal(0, 4, true, Infinite, 200.0, 0.0), "an infinite X is refused");
	T.AssertEquals(3, HOLRpc::DropOffRefusal(0, 4, true, 100.0, NotANumber, 0.0), "a NaN Y is refused");
	T.AssertEquals(3, HOLRpc::DropOffRefusal(0, 4, true, 100.0, 200.0, -Infinite), "a negative infinite Z is refused");
	T.AssertEquals(2, HOLRpc::DropOffRefusal(0, 0, false, Infinite, 0.0, 0.0), "the units are checked before the building and the point");
}

void Test_RpcOrderPlaceBuildingRefusal(FUnitTest& T)
{
	const float Huge = 1e300;
	const float Infinite = Huge * Huge;
	T.AssertEquals(0, HOLRpc::PlaceBuildingRefusal(0, 2, 500.0, -300.0, 10.0, true), "builders, a finite point and a type name: passed on");
	T.AssertEquals(1, HOLRpc::PlaceBuildingRefusal(-1, 2, 500.0, -300.0, 10.0, true), "a spectator is refused");
	T.AssertEquals(2, HOLRpc::PlaceBuildingRefusal(0, 0, 500.0, -300.0, 10.0, true), "no builder named is refused");
	T.AssertEquals(3, HOLRpc::PlaceBuildingRefusal(0, 2, Infinite, -300.0, 10.0, true), "an infinite X is refused");
	T.AssertEquals(4, HOLRpc::PlaceBuildingRefusal(0, 2, 500.0, -300.0, 10.0, false), "no type name is refused");
	T.AssertEquals(3, HOLRpc::PlaceBuildingRefusal(0, 2, Infinite, -300.0, 10.0, false), "the point is checked before the type name");
	// The name test of the RPC itself: the empty name is NAME_None, a real type is not.
	T.AssertTrue(n"Farm" != NAME_None, "a type name is not NAME_None");
	T.AssertFalse(NAME_None != NAME_None, "the empty name is NAME_None");
}

void Test_RpcOrderRallyPointRefusal(FUnitTest& T)
{
	const float Huge = 1e300;
	const float Infinite = Huge * Huge;
	const float NotANumber = Infinite - Infinite;
	T.AssertEquals(0, HOLRpc::RallyPointRefusal(0, true, 300.0, 400.0, 0.0), "a seat, a building and a finite point: passed on");
	T.AssertEquals(1, HOLRpc::RallyPointRefusal(9, true, 300.0, 400.0, 0.0), "a sender past the 8 slots is refused");
	T.AssertEquals(3, HOLRpc::RallyPointRefusal(0, false, 300.0, 400.0, 0.0), "no building named is refused");
	T.AssertEquals(3, HOLRpc::RallyPointRefusal(0, true, Infinite, 400.0, 0.0), "an infinite X is refused");
	T.AssertEquals(3, HOLRpc::RallyPointRefusal(0, true, 300.0, 400.0, NotANumber), "a NaN Z is refused");
	T.AssertEquals(1, HOLRpc::RallyPointRefusal(-1, false, Infinite, 0.0, 0.0), "the seat is checked first");
}
