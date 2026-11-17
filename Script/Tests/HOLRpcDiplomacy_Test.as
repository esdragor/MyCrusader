// MIG2-S3 pilot (D842 Q1 = A, ELROND-1117o): the rule of the script RPC ServerDiplomacyStance (Script/Net/HOLRpcPlayer.as), pure: HOLRpcStanceRefusal(Sender, Target, Stance)
// says why an order is not queued (0 = queue it). The Sender is the owner's slot read by the server (D288), never a number the client says; the rest (the mode, the pair, the
// lock) stays HOLDiplomacy::SetStance's at the next step, as for the C++ RPC. RED FIRST: this file compiles only with HOLRpcPlayer.as (absent at the red commit: no function).
// The network proof (2 clients) is Tools\Scenarios\rpc_diplomacy_run.ps1; it needs the C++ lot (HOLScriptRpcPlayerActor) built.

void Test_RpcStanceQueued(FUnitTest& T)
{
	T.AssertEquals(0, HOLRpcStanceRefusal(0, 1, 0), "slot 0 says Enemy toward slot 1: queued");
	T.AssertEquals(0, HOLRpcStanceRefusal(3, 7, 1), "slot 3 says Neutral toward slot 7: queued");
	T.AssertEquals(0, HOLRpcStanceRefusal(7, 0, 2), "slot 7 says Ally toward slot 0: queued (the last slot is in range)");
}

void Test_RpcStanceRefused(FUnitTest& T)
{
	T.AssertEquals(1, HOLRpcStanceRefusal(-1, 1, 1), "a spectator (no slot) is refused");
	T.AssertEquals(1, HOLRpcStanceRefusal(8, 1, 1), "a sender past the 8 slots is refused");
	T.AssertEquals(2, HOLRpcStanceRefusal(0, -1, 1), "a target below 0 is refused");
	T.AssertEquals(2, HOLRpcStanceRefusal(0, 8, 1), "a target past the 8 slots is refused");
	T.AssertEquals(3, HOLRpcStanceRefusal(0, 1, -1), "a stance below Enemy is refused");
	T.AssertEquals(3, HOLRpcStanceRefusal(0, 1, 3), "a stance above Ally is refused");
}

void Test_RpcStanceOrderOfChecks(FUnitTest& T)
{
	T.AssertEquals(1, HOLRpcStanceRefusal(-1, 99, 99), "the sender is checked first: a spectator with garbage arguments is refused as a spectator");
	T.AssertEquals(2, HOLRpcStanceRefusal(0, 99, 99), "then the target, then the stance");
}
