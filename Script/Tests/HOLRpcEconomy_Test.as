// MIG2-S3 T8 lot L3 (HALDIR-1117q): the pure rules of the script RPCs of the Economy lot (ServerQueueProduction, ServerCancelProduction, ServerTribute, ServerMarketTrade) of Script/Net/HOLRpcPlayer.as.
// The sender is the owner's slot (D288); the production (UHOLCommandSubsystem::IssueProductionOrder) decides owner, type, queue, stock, Market, lots at its step, as for the C++ RPC.
// RED FIRST for the main session: replace each body by `return 0;` (stub), run quick_check -Parts tests, then restore.

void Test_RpcQueueProductionRefusal(FUnitTest& T)
{
	T.AssertEquals(0, HOLRpc::QueueProductionRefusal(0, true), "slot 0, a building: queued");
	T.AssertEquals(0, HOLRpc::QueueProductionRefusal(7, true), "slot 7 (the last) is in range");
	T.AssertEquals(1, HOLRpc::QueueProductionRefusal(-1, true), "a spectator (no slot) queues nothing");
	T.AssertEquals(1, HOLRpc::QueueProductionRefusal(8, true), "a sender past the 8 slots is refused");
	T.AssertEquals(2, HOLRpc::QueueProductionRefusal(3, false), "no building: refused");
	T.AssertEquals(1, HOLRpc::QueueProductionRefusal(-1, false), "the seat is checked first");
}

void Test_RpcCancelProductionRefusal(FUnitTest& T)
{
	T.AssertEquals(0, HOLRpc::CancelProductionRefusal(2, true, 0), "index 0 (the head of the queue) is cancelled");
	T.AssertEquals(0, HOLRpc::CancelProductionRefusal(2, true, 4), "a deeper index is passed to the production");
	T.AssertEquals(1, HOLRpc::CancelProductionRefusal(-1, true, 0), "a spectator cancels nothing");
	T.AssertEquals(2, HOLRpc::CancelProductionRefusal(2, false, 0), "no building: refused");
	T.AssertEquals(3, HOLRpc::CancelProductionRefusal(2, true, -1), "a negative index is refused");
	T.AssertEquals(2, HOLRpc::CancelProductionRefusal(2, false, -1), "the building is checked before the index");
}

void Test_RpcTributeRefusal(FUnitTest& T)
{
	T.AssertEquals(0, HOLRpc::TributeRefusal(0, true, 1, 0, 100), "slot 0 sends 100 of resource 0 to slot 1: queued");
	T.AssertEquals(0, HOLRpc::TributeRefusal(0, true, 7, 2, 1), "slot 7 is a valid receiver");
	T.AssertEquals(1, HOLRpc::TributeRefusal(-1, true, 1, 0, 100), "a spectator sends no tribute");
	T.AssertEquals(2, HOLRpc::TributeRefusal(0, false, 1, 0, 100), "no building: refused");
	T.AssertEquals(3, HOLRpc::TributeRefusal(0, true, 8, 0, 100), "a receiver past the 8 slots is refused");
	T.AssertEquals(3, HOLRpc::TributeRefusal(0, true, -1, 0, 100), "a negative receiver is refused");
	T.AssertEquals(4, HOLRpc::TributeRefusal(0, true, 1, -1, 100), "a negative resource is refused");
	T.AssertEquals(5, HOLRpc::TributeRefusal(0, true, 1, 0, 0), "a zero amount is refused");
	T.AssertEquals(5, HOLRpc::TributeRefusal(0, true, 1, 0, -50), "a negative amount is refused");
}

void Test_RpcMarketTradeRefusal(FUnitTest& T)
{
	T.AssertEquals(0, HOLRpc::MarketTradeRefusal(0, true, 0, 1), "one lot of resource 0: queued");
	T.AssertEquals(0, HOLRpc::MarketTradeRefusal(5, true, 3, 10), "ten lots of resource 3: passed to the production");
	T.AssertEquals(1, HOLRpc::MarketTradeRefusal(-1, true, 0, 1), "a spectator trades nothing");
	T.AssertEquals(2, HOLRpc::MarketTradeRefusal(0, false, 0, 1), "no building: refused");
	T.AssertEquals(3, HOLRpc::MarketTradeRefusal(0, true, -1, 1), "a negative resource is refused");
	T.AssertEquals(4, HOLRpc::MarketTradeRefusal(0, true, 0, 0), "zero lots are refused");
	T.AssertEquals(4, HOLRpc::MarketTradeRefusal(0, true, 0, -2), "negative lots are refused");
}
