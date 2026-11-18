// MIG2-S3 L4 group A (ELROND-1117q): the pure refusals of the script cheat RPCs (ServerCheatResources, FastBuild, Population, Technology, Tier, UnitSpeed, GatherSpeed) of Script/Net/HOLRpcPlayer.as.
// R3.5: refused unless the cheats are active on the server (the bridge reads FHOLCheatRules::AreCheatsAllowed); the sender is the owner's slot (D288, a seat is needed). The effect is the C++ Do<X> body's (called by the bridge).
// RED FIRST for the main session: replace each body in HOLRpc (CheatRefusal, CheatTierRefusal, CheatTechnologyRefusal, CheatScaleRefusal) by `return 0;` (stub), run quick_check -Parts tests, then restore.

void Test_RpcCheatSeatRefusal(FUnitTest& T)
{
	// Resources, FastBuild and Population share CheatRefusal.
	T.AssertEquals(0, HOLRpc::CheatRefusal(true, 0), "cheats on, slot 0: passed to the C++ body");
	T.AssertEquals(0, HOLRpc::CheatRefusal(true, 7), "cheats on, slot 7 (the last) is in range");
	T.AssertEquals(1, HOLRpc::CheatRefusal(false, 0), "cheats off: refused (R3.5)");
	T.AssertEquals(2, HOLRpc::CheatRefusal(true, -1), "a spectator (no slot) is refused");
	T.AssertEquals(2, HOLRpc::CheatRefusal(true, 8), "a sender past the 8 slots is refused");
	T.AssertEquals(1, HOLRpc::CheatRefusal(false, -1), "the cheat gate is checked first");
}

void Test_RpcCheatTierRefusal(FUnitTest& T)
{
	T.AssertEquals(0, HOLRpc::CheatTierRefusal(true, 1, -2), "target -2 (everyone) is passed on");
	T.AssertEquals(0, HOLRpc::CheatTierRefusal(true, 1, -1), "target -1 (own slot) is passed on");
	T.AssertEquals(0, HOLRpc::CheatTierRefusal(true, 1, 7), "target slot 7 is passed on");
	T.AssertEquals(1, HOLRpc::CheatTierRefusal(false, 1, 3), "cheats off: refused");
	T.AssertEquals(2, HOLRpc::CheatTierRefusal(true, -1, 3), "a spectator is refused");
	T.AssertEquals(3, HOLRpc::CheatTierRefusal(true, 1, -3), "a target below -2 is refused");
	T.AssertEquals(3, HOLRpc::CheatTierRefusal(true, 1, 8), "a target slot past the 8 slots is refused");
}

void Test_RpcCheatTechnologyRefusal(FUnitTest& T)
{
	T.AssertEquals(0, HOLRpc::CheatTechnologyRefusal(true, 0, n"Longbow"), "a named technology is passed on (the C++ body checks that it exists)");
	T.AssertEquals(1, HOLRpc::CheatTechnologyRefusal(false, 0, n"Longbow"), "cheats off: refused");
	T.AssertEquals(2, HOLRpc::CheatTechnologyRefusal(true, -1, n"Longbow"), "a spectator is refused");
	T.AssertEquals(3, HOLRpc::CheatTechnologyRefusal(true, 0, n"None"), "no technology named: refused");
}

void Test_RpcCheatScaleRefusal(FUnitTest& T)
{
	// The value tested is COMPUTED (1e300 * 1e300 overflows to infinity), not written as a constant.
	const float Huge = 1e300;
	const float Infinite = Huge * Huge;
	T.AssertEquals(0, HOLRpc::CheatScaleRefusal(true, 0, 2.5), "a finite scale is passed on (the C++ body clamps it to 0.1..10)");
	T.AssertEquals(0, HOLRpc::CheatScaleRefusal(true, 0, 0.0), "zero is finite: passed on (clamped by the C++ body)");
	T.AssertEquals(1, HOLRpc::CheatScaleRefusal(false, 0, 2.5), "cheats off: refused");
	T.AssertEquals(2, HOLRpc::CheatScaleRefusal(true, 9, 2.5), "a sender past the 8 slots is refused");
	T.AssertEquals(3, HOLRpc::CheatScaleRefusal(true, 0, Infinite), "an infinite scale is refused");
	T.AssertEquals(3, HOLRpc::CheatScaleRefusal(true, 0, -Infinite), "a negative infinite scale is refused");
}
