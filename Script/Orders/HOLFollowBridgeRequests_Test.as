// The other manual order paths end the Follow too (GLORFINDEL-1121sm): every order the controller sends goes through its Request* functions (move by keyboard
// shortcut or mini-map click = RequestMoveUnits, attack-move = RequestAttackMove, drag = RequestMoveUnitsInLine), which call UHOLFollowBridge::NativeManualOrder.
// The orders are sent here as the controller's own Request* functions (HOLControllerCommand::Request*), on a real AHOLPlayerController spawned in the test world.
// Red until the Request* functions call NativeManualOrder (C++, HOLPlayerController.cpp).

namespace HOLFollowBridgeRequestsTest
{
	const int AllyId = 9;

	// Units 1 and 2 follow the ally; returns the controller (nullptr when it cannot be spawned).
	AHOLPlayerController StartFollow(FUnitTest& T, UHOLFollowBookHolder& OutHolder)
	{
		UHOLFollowBridge Bridge = UHOLFollowBridge::Get();
		OutHolder = UHOLFollowBookHolder::Get();
		T.AssertTrue(Bridge != nullptr && OutHolder != nullptr, "bridge and holder exist");
		AHOLPlayerController Controller = Cast<AHOLPlayerController>(SpawnActor(AHOLPlayerController::StaticClass(), FVector::ZeroVector));
		T.AssertTrue(Controller != nullptr, "a player controller is spawned in the test world");
		if (Bridge == nullptr || OutHolder == nullptr || Controller == nullptr)
			return nullptr;
		OutHolder.Book = FHOLFollowBook();
		TArray<int> Selected;
		Selected.Add(1);
		Selected.Add(2);
		T.AssertTrue(Bridge.RightClickOnUnit(nullptr, Selected, AllyId, true), "Follow started for units 1 and 2");
		return Controller;
	}

	bool Follows(UHOLFollowBookHolder Holder, int UnitId)
	{
		const int Index = HOLFollowOrders::IndexOf(Holder.Book, UnitId);
		return Index >= 0 && Holder.Book.States[Index].bActive;
	}
}

// EFFECT: a move order (keyboard shortcut, mini-map click) of unit 1 ends its follow; unit 2, not ordered, keeps following.
void Test_FollowEndsOnRequestMove(FUnitTest& T)
{
	UHOLFollowBookHolder Holder;
	AHOLPlayerController Controller = HOLFollowBridgeRequestsTest::StartFollow(T, Holder);
	if (Controller == nullptr)
		return;
	T.AssertTrue(HOLFollowBridgeRequestsTest::Follows(Holder, 1), "unit 1 follows before the move order");
	TArray<int> Ordered;
	Ordered.Add(1);
	HOLControllerCommand::RequestMoveUnitIdsTo(Controller, Ordered, 500.0, 500.0);
	T.AssertTrue(!HOLFollowBridgeRequestsTest::Follows(Holder, 1), "unit 1 no longer follows after a move order");
	T.AssertTrue(HOLFollowBridgeRequestsTest::Follows(Holder, 2), "unit 2, not ordered, keeps following");
}

// EFFECT: an attack-move (A + click) of unit 1 ends its follow.
void Test_FollowEndsOnRequestAttackMove(FUnitTest& T)
{
	UHOLFollowBookHolder Holder;
	AHOLPlayerController Controller = HOLFollowBridgeRequestsTest::StartFollow(T, Holder);
	if (Controller == nullptr)
		return;
	TArray<int> Ordered;
	Ordered.Add(1);
	HOLControllerCommand::RequestAttackMoveUnits(Controller, Ordered, 500.0, 500.0, false);
	T.AssertTrue(!HOLFollowBridgeRequestsTest::Follows(Holder, 1), "unit 1 no longer follows after an attack-move");
	T.AssertTrue(HOLFollowBridgeRequestsTest::Follows(Holder, 2), "unit 2, not ordered, keeps following");
}

// EFFECT: a line move (right-button drag) of unit 1 ends its follow.
void Test_FollowEndsOnRequestMoveInLine(FUnitTest& T)
{
	UHOLFollowBookHolder Holder;
	AHOLPlayerController Controller = HOLFollowBridgeRequestsTest::StartFollow(T, Holder);
	if (Controller == nullptr)
		return;
	TArray<int> Ordered;
	Ordered.Add(1);
	HOLControllerCommand::RequestMoveUnitsInLineOf(Controller, Ordered, 0.0, 0.0, 800.0, 0.0, false, false);
	T.AssertTrue(!HOLFollowBridgeRequestsTest::Follows(Holder, 1), "unit 1 no longer follows after a line move");
	T.AssertTrue(HOLFollowBridgeRequestsTest::Follows(Holder, 2), "unit 2, not ordered, keeps following");
}
