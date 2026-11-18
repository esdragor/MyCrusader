// MIG2-S3 T5 lot L1 (HALDIR-1117p): the pure rules of the script RPCs of the Signals and chat lot (ServerPostSignal, ClientReceiveSignal, ServerPostChat, ClientReceiveChat) of Script/Net/HOLRpcPlayer.as.
// The sender is the owner's slot (D288); a spectator places no signal but talks (D376, slot -1 valid for the chat); the boards (HOLSignal::Post, the chat's length / rate / reach) decide the rest, as the retired C++ RPC did.
// RED FIRST for the main session: replace each body by `return 0;` (stub), run quick_check -Parts tests, then restore.

void Test_RpcSignalRefusal(FUnitTest& T)
{
	T.AssertEquals(0, HOLRpc::SignalRefusal(0), "slot 0 posts a signal: queued");
	T.AssertEquals(0, HOLRpc::SignalRefusal(7), "slot 7 (the last) is in range");
	T.AssertEquals(1, HOLRpc::SignalRefusal(-1), "a spectator (no slot) places no signal");
	T.AssertEquals(1, HOLRpc::SignalRefusal(8), "a sender past the 8 slots is refused");
}

void Test_RpcClientSignalRefusal(FUnitTest& T)
{
	T.AssertEquals(0, HOLRpc::ClientSignalRefusal(2, 0), "a signal of slot 2, kind 0, is shown");
	T.AssertEquals(0, HOLRpc::ClientSignalRefusal(7, 3), "slot 7, kind 3 is shown");
	T.AssertEquals(1, HOLRpc::ClientSignalRefusal(-1, 0), "a signal has a seat sender");
	T.AssertEquals(1, HOLRpc::ClientSignalRefusal(8, 0), "a sender past the 8 slots is refused");
	T.AssertEquals(2, HOLRpc::ClientSignalRefusal(0, -1), "a negative kind is refused");
	T.AssertEquals(1, HOLRpc::ClientSignalRefusal(9, -1), "the sender is checked first");
}

void Test_RpcChatRefusal(FUnitTest& T)
{
	const FString Hello = "hello";
	const FString Empty = "";
	T.AssertEquals(0, HOLRpc::ChatRefusal(0, Hello), "channel 0, some text: passed to the board");
	T.AssertEquals(0, HOLRpc::ChatRefusal(3, Hello), "channel 3, some text: passed to the board");
	T.AssertEquals(1, HOLRpc::ChatRefusal(-1, Hello), "a negative channel is refused");
	T.AssertEquals(2, HOLRpc::ChatRefusal(0, Empty), "an empty text is refused");
	T.AssertEquals(1, HOLRpc::ChatRefusal(-2, Empty), "the channel is checked first");
}

void Test_RpcClientChatRefusal(FUnitTest& T)
{
	T.AssertEquals(0, HOLRpc::ClientChatRefusal(0, 0), "a message of slot 0 on channel 0 is shown");
	T.AssertEquals(0, HOLRpc::ClientChatRefusal(-1, 2), "a spectator's talk (slot -1) is shown");
	T.AssertEquals(0, HOLRpc::ClientChatRefusal(7, 1), "slot 7 is in range");
	T.AssertEquals(1, HOLRpc::ClientChatRefusal(-2, 0), "a sender below -1 is refused");
	T.AssertEquals(1, HOLRpc::ClientChatRefusal(8, 0), "a sender past the 8 slots is refused");
	T.AssertEquals(2, HOLRpc::ClientChatRefusal(0, -1), "a negative channel is refused");
}
