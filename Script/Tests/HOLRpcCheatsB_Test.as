// MIG2-S3 T9 lot L4 Cheats, group B (CELEBORN-1117q): the pure rules of the script RPCs ServerCheatWound, ServerCheatDamageBuilding, ServerCheatSpawnBuilding, ServerCheatDefeat,
// ServerCheatFallForum, ServerCheatSkipForum, ServerCheatHoldPresence of Script/Net/HOLRpcPlayer.as. The first argument is "cheats active" (R3.5: the bridge asks the C++ AHOLPlayerController::IsCheatRefused);
// the bodies stay the C++ _Implementation (called by the bridge), the script only validates. RED FIRST for the main session: replace each body by `return 0;` (stub), run quick_check -Parts tests, then restore.

void Test_RpcCheatWoundRefusal(FUnitTest& T)
{
	T.AssertEquals(0, HOLRpc::CheatWoundRefusal(true, 0), "cheats on, slot 0: wounds its own units");
	T.AssertEquals(0, HOLRpc::CheatWoundRefusal(true, 7), "slot 7 (the last) is in range");
	T.AssertEquals(1, HOLRpc::CheatWoundRefusal(false, 0), "cheats off: refused (R3.5)");
	T.AssertEquals(2, HOLRpc::CheatWoundRefusal(true, -1), "a spectator (no slot) has no units to wound");
	T.AssertEquals(1, HOLRpc::CheatWoundRefusal(false, -1), "cheats are checked first");
}

void Test_RpcCheatDamageBuildingRefusal(FUnitTest& T)
{
	T.AssertEquals(0, HOLRpc::CheatDamageBuildingRefusal(true, 100), "cheats on: the building nearest the point is hit");
	T.AssertEquals(0, HOLRpc::CheatDamageBuildingRefusal(true, 0), "an amount of 0 is the board's to ignore, not the RPC's");
	T.AssertEquals(1, HOLRpc::CheatDamageBuildingRefusal(false, 100), "cheats off: refused (R3.5)");
	T.AssertEquals(2, HOLRpc::CheatDamageBuildingRefusal(true, -5), "a negative amount (it would heal) is refused");
}

void Test_RpcCheatSpawnBuildingRefusal(FUnitTest& T)
{
	const FString Forum = "Forum";
	const FString Empty = "";
	T.AssertEquals(0, HOLRpc::CheatSpawnBuildingRefusal(true, Forum, 0), "cheats on, a type, slot 0: spawned");
	T.AssertEquals(0, HOLRpc::CheatSpawnBuildingRefusal(true, Forum, 255), "slot 255 is the C++ limit");
	T.AssertEquals(1, HOLRpc::CheatSpawnBuildingRefusal(false, Forum, 0), "cheats off: refused (R3.5)");
	T.AssertEquals(2, HOLRpc::CheatSpawnBuildingRefusal(true, Forum, 256), "a slot past 255 is refused");
	T.AssertEquals(2, HOLRpc::CheatSpawnBuildingRefusal(true, Forum, -1), "a negative slot is refused");
	T.AssertEquals(3, HOLRpc::CheatSpawnBuildingRefusal(true, Empty, 0), "no type name is refused");
}

void Test_RpcCheatDefeatRefusal(FUnitTest& T)
{
	T.AssertEquals(0, HOLRpc::CheatSlotRefusal(true, 3), "cheats on, slot 3: the board's DefeatSlot decides");
	T.AssertEquals(1, HOLRpc::CheatSlotRefusal(false, 3), "cheats off: refused (R3.5)");
	T.AssertEquals(2, HOLRpc::CheatSlotRefusal(true, 8), "a slot past the 8 seats is refused");
	T.AssertEquals(2, HOLRpc::CheatSlotRefusal(true, -1), "a negative slot is refused");
}

void Test_RpcCheatFallForumRefusal(FUnitTest& T)
{
	T.AssertEquals(0, HOLRpc::CheatSlotRefusal(true, 0), "cheats on, slot 0: its Forum falls");
	T.AssertEquals(0, HOLRpc::CheatSlotRefusal(true, 7), "slot 7 is in range");
	T.AssertEquals(1, HOLRpc::CheatSlotRefusal(false, 7), "cheats off: refused (R3.5)");
}

void Test_RpcCheatSkipForumRefusal(FUnitTest& T)
{
	T.AssertEquals(0, HOLRpc::CheatSlotRefusal(true, 1), "cheats on, slot 1: its Forum phase is skipped");
	T.AssertEquals(1, HOLRpc::CheatSlotRefusal(false, 1), "cheats off: refused (R3.5)");
	T.AssertEquals(2, HOLRpc::CheatSlotRefusal(true, 100), "a slot far past the seats is refused");
}

void Test_RpcCheatHoldPresenceRefusal(FUnitTest& T)
{
	T.AssertEquals(0, HOLRpc::CheatHoldPresenceRefusal(true, 2, false, true, false, 5), "attackers of slot 5 held at the Forum of slot 2");
	T.AssertEquals(0, HOLRpc::CheatHoldPresenceRefusal(true, 2, true, false, false, -1), "bOff: the attacker slot is not read");
	T.AssertEquals(0, HOLRpc::CheatHoldPresenceRefusal(true, 2, false, false, true, -1), "defenders only: no attacker slot needed");
	T.AssertEquals(1, HOLRpc::CheatHoldPresenceRefusal(false, 2, false, true, false, 5), "cheats off: refused (R3.5)");
	T.AssertEquals(2, HOLRpc::CheatHoldPresenceRefusal(true, 9, false, true, false, 5), "a Forum slot past the 8 seats is refused");
	T.AssertEquals(3, HOLRpc::CheatHoldPresenceRefusal(true, 2, false, true, false, 8), "attackers named but their slot is out of range");
}
