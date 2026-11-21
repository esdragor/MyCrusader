// The bridge the C++ controller calls on a right click on a unit (UHOLFollowBridge, GLORFINDEL-1121sk): called here as AHOLPlayerController::OrderAt calls it
// (UHOLFollowBridge::Get() then RightClickOnUnit with the selected ids and the unit under the cursor). The tests assert the book the Step reads.
// Red until Script/Orders/HOLFollowBridge.as exists (the C++ class is abstract: no script class, no instance).

namespace HOLFollowBridgeTest
{
	const int AllyId = 9;
}

// EFFECT: a right click on an ally outside the selection starts Follow: the bridge says so (no ordinary order) and every selected unit is a bodyguard of the ally.
void Test_FollowBridgeRightClickOnAllyStartsFollow(FUnitTest& T)
{
	UHOLFollowBridge Bridge = UHOLFollowBridge::Get();
	T.AssertTrue(Bridge != nullptr, "the script class of UHOLFollowBridge exists (Script/Orders/HOLFollowBridge.as)");
	UHOLFollowBookHolder Holder = UHOLFollowBookHolder::Get();
	T.AssertTrue(Holder != nullptr, "the follow book holder exists in the test world");
	if (Bridge == nullptr || Holder == nullptr)
		return;
	Holder.Book = FHOLFollowBook();
	TArray<int> Selected;
	Selected.Add(1);
	Selected.Add(2);
	const bool bStarted = Bridge.RightClickOnUnit(nullptr, Selected, HOLFollowBridgeTest::AllyId, true);
	T.AssertTrue(bStarted, "the right click on an ally starts Follow");
	T.AssertTrue(Holder.Book.UnitIds.Num() == 2, "both selected units are in the book");
	const int Index = HOLFollowOrders::IndexOf(Holder.Book, 1);
	T.AssertTrue(Index >= 0 && Holder.Book.TargetIds[Index] == HOLFollowBridgeTest::AllyId, "unit 1 follows the ally");
}

// EFFECT: an enemy under the cursor, or a unit of the selection itself, is NOT a Follow: the book stays empty and the controller keeps its Attack / Move.
void Test_FollowBridgeEnemyOrOwnSelectionIsNotFollow(FUnitTest& T)
{
	UHOLFollowBridge Bridge = UHOLFollowBridge::Get();
	UHOLFollowBookHolder Holder = UHOLFollowBookHolder::Get();
	T.AssertTrue(Bridge != nullptr && Holder != nullptr, "bridge and holder exist");
	if (Bridge == nullptr || Holder == nullptr)
		return;
	Holder.Book = FHOLFollowBook();
	TArray<int> Selected;
	Selected.Add(1);
	Selected.Add(2);
	T.AssertTrue(!Bridge.RightClickOnUnit(nullptr, Selected, HOLFollowBridgeTest::AllyId, false), "an enemy is not a Follow");
	T.AssertTrue(!Bridge.RightClickOnUnit(nullptr, Selected, 2, true), "a unit of the selection is not a Follow");
	T.AssertTrue(Holder.Book.UnitIds.Num() == 0, "the book is untouched");
}
