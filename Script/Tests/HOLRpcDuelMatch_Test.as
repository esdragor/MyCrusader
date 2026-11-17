// MIG2-S3 T3 (SAURON-1117m): the pure rules of the script RPCs of the Duel lot (ServerDuelChallenge, ServerDuelAnswer) and the Match lot (ServerRequestPause, ServerRequestResume,
// ServerSurrender) of Script/Net/HOLRpcPlayer.as. The sender is the owner's slot read by the server (D288), never a number the client says; a spectator (no slot) is refused;
// the bounds are the 8 slots. The rest stays the duel rules' (HOLDuel::CanChallenge) and AHOLGameState's. RED FIRST: with the stub (every function returns 0) the refusal asserts fail.
// Red then green for the main session: see SAURON-1117m.md (stub = replace each function body by `return 0;`, run quick_check -Parts tests, then restore).

void Test_RpcDuelChallengeQueued(FUnitTest& T)
{
	T.AssertEquals(0, HOLRpc::DuelChallengeRefusal(0, 1), "slot 0 challenges slot 1: queued");
	T.AssertEquals(0, HOLRpc::DuelChallengeRefusal(7, 0), "slot 7 challenges slot 0: queued (the last slot is in range)");
	T.AssertEquals(0, HOLRpc::DuelChallengeRefusal(3, 3), "a challenge at oneself is not refused here (HOLDuel::CanChallenge decides at the step's end, as in C++)");
}

void Test_RpcDuelChallengeRefused(FUnitTest& T)
{
	T.AssertEquals(1, HOLRpc::DuelChallengeRefusal(-1, 1), "a spectator (no slot) cannot challenge");
	T.AssertEquals(1, HOLRpc::DuelChallengeRefusal(8, 1), "a sender past the 8 slots is refused");
	T.AssertEquals(2, HOLRpc::DuelChallengeRefusal(0, -1), "a target below 0 is refused");
	T.AssertEquals(2, HOLRpc::DuelChallengeRefusal(0, 8), "a target past the 8 slots is refused");
	T.AssertEquals(1, HOLRpc::DuelChallengeRefusal(-1, 99), "the sender is checked first");
}

void Test_RpcDuelAnswer(FUnitTest& T)
{
	T.AssertEquals(0, HOLRpc::DuelAnswerRefusal(0), "slot 0 answers: queued");
	T.AssertEquals(0, HOLRpc::DuelAnswerRefusal(7), "slot 7 answers: queued");
	T.AssertEquals(1, HOLRpc::DuelAnswerRefusal(-1), "a spectator cannot answer a duel");
	T.AssertEquals(1, HOLRpc::DuelAnswerRefusal(8), "a sender past the 8 slots is refused");
}

void Test_RpcMatchRefusal(FUnitTest& T)
{
	T.AssertEquals(0, HOLRpc::MatchRefusal(0), "slot 0 may ask pause, resume, surrender (the game state decides the rest)");
	T.AssertEquals(0, HOLRpc::MatchRefusal(7), "slot 7 too");
	T.AssertEquals(1, HOLRpc::MatchRefusal(-1), "a spectator cannot pause, resume or surrender");
	T.AssertEquals(1, HOLRpc::MatchRefusal(8), "a sender past the 8 slots is refused");
}
