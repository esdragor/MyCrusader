// Follow orders on the REAL chain (HOLFollowOrders::Step -> HOLFollowRules::Decide -> HOLPlayerOrders:: -> the command queue, read back with ConsumeMoves
// on a detached queue). The tests assert what the queue holds and where a follower that obeys it ends: an ally walking away is followed by Move orders,
// an enemy target or an enemy in range is an Attack on it (queued once, not every step), a manual order ends the follow.
// Red against the stub of HOLFollowOrders.as (nothing is emitted): Test_FollowOrders* all fail.

namespace HOLFollowOrdersTest
{
	const int UnitId = 7;
	const int AllyId = 9;

	FVector WalkToward(FVector From, FVector To, float Step)
	{
		const FVector Away = To - From;
		const float Length = Away.Size();
		if (Length <= Step)
			return To;
		return From + Away * (Step / Length);
	}
}

// EFFECT: the follower that obeys the queued Move orders keeps up with an ally walking away.
void Test_FollowOrdersAllyWalkingAwayIsFollowedByMoveOrders(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	FHOLFollowBook Book;
	HOLFollowOrders::Begin(Book, HOLFollowOrdersTest::UnitId, HOLFollowOrdersTest::AllyId, true);
	FVector Position = FVector(0.f, 0.f, 0.f);
	FVector Ally = FVector(500.f, 0.f, 0.f);
	int NumMoves = 0;
	for (int Tick = 0; Tick < 40; Tick++)
	{
		Ally.X += 80.f;
		HOLFollowOrders::Step(Commands, 0, Book, HOLFollowOrdersTest::UnitId, Position, Ally, false, 0);
		TArray<FHOLScriptOrderRead> Orders = HOLCommandScript::ConsumeMoves(Commands);
		for (int Index = 0; Index < Orders.Num(); Index++)
		{
			if (Orders[Index].Kind == n"Move" && Orders[Index].UnitIds.Contains(HOLFollowOrdersTest::UnitId))
			{
				NumMoves++;
				Position = HOLFollowOrdersTest::WalkToward(Position, Orders[Index].Target, 120.f);
			}
		}
	}
	const float Gap = (Ally - Position).Size();
	T.AssertTrue(NumMoves > 0, "the follow queued Move orders for the follower");
	T.AssertTrue(Gap <= HOLFollowRules::FollowDistance + 130.f, f"the follower kept up with the ally (gap {Gap} cm, ally at X {Ally.X}, follower at X {Position.X})");
}

// EFFECT: following an enemy queues ONE Attack on it, and not another one on the next step.
void Test_FollowOrdersEnemyTargetIsOneAttack(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	FHOLFollowBook Book;
	HOLFollowOrders::Begin(Book, HOLFollowOrdersTest::UnitId, 55, false);
	HOLFollowOrders::Step(Commands, 0, Book, HOLFollowOrdersTest::UnitId, FVector(0.f, 0.f, 0.f), FVector(3000.f, 0.f, 0.f), false, 0);
	TArray<FHOLScriptOrderRead> First = HOLCommandScript::ConsumeMoves(Commands);
	HOLFollowOrders::Step(Commands, 0, Book, HOLFollowOrdersTest::UnitId, FVector(10.f, 0.f, 0.f), FVector(3000.f, 0.f, 0.f), false, 0);
	TArray<FHOLScriptOrderRead> Second = HOLCommandScript::ConsumeMoves(Commands);
	T.AssertEquals(1, First.Num(), "the first step queued one order");
	if (First.Num() == 1)
	{
		T.AssertTrue(First[0].Kind == n"Attack", "it is an Attack");
		T.AssertEquals(55, First[0].TargetUnitId, "on the followed enemy");
	}
	T.AssertEquals(0, Second.Num(), "the same Attack is not queued again on the next step");
}

// EFFECT: a bodyguard attacks the enemy that comes in range of the unit.
void Test_FollowOrdersAllyBodyguardAttacksEnemyInRange(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	FHOLFollowBook Book;
	HOLFollowOrders::Begin(Book, HOLFollowOrdersTest::UnitId, HOLFollowOrdersTest::AllyId, true);
	HOLFollowOrders::Step(Commands, 0, Book, HOLFollowOrdersTest::UnitId, FVector(0.f, 0.f, 0.f), FVector(100.f, 0.f, 0.f), true, 66);
	TArray<FHOLScriptOrderRead> Orders = HOLCommandScript::ConsumeMoves(Commands);
	T.AssertEquals(1, Orders.Num(), "one order queued");
	if (Orders.Num() == 1)
	{
		T.AssertTrue(Orders[0].Kind == n"Attack", "it is an Attack");
		T.AssertEquals(66, Orders[0].TargetUnitId, "on the enemy in range, not on the ally");
	}
}

// EFFECT: the position read by unit id says "gone" for an id the machine does not know (the followed unit died, was garrisoned, or no world): no Step is
// made from a made-up position. Red against the stub of UHOLUnitScriptLibrary (alive at the origin for any id). The living unit's position needs a Mass
// world (a unit is born by Mass): scenario, not this test.
void Test_FollowOrdersReadUnitUnknownIdIsGone(FUnitTest& T)
{
	FVector Location = FVector(123.f, 456.f, 789.f);
	const bool bFound = HOLFollowOrders::ReadUnit(nullptr, 987654, Location);
	T.AssertFalse(bFound, "an unknown unit id is not read as alive");
	T.AssertTrue(Location.Size() < 0.01f, f"and its position is cleared, not left at the caller's value (X {Location.X} Y {Location.Y} Z {Location.Z})");
	T.AssertFalse(HOLUnitScript::IsUnitAlive(987654), "IsUnitAlive is false for it");
}

// EFFECT: after a manual order the follow queues nothing.
void Test_FollowOrdersManualOrderEndsIt(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	FHOLFollowBook Book;
	HOLFollowOrders::Begin(Book, HOLFollowOrdersTest::UnitId, HOLFollowOrdersTest::AllyId, true);
	HOLFollowOrders::OnManualOrder(Book, HOLFollowOrdersTest::UnitId);
	const bool bIssued = HOLFollowOrders::Step(Commands, 0, Book, HOLFollowOrdersTest::UnitId, FVector(0.f, 0.f, 0.f), FVector(3000.f, 0.f, 0.f), true, 66);
	TArray<FHOLScriptOrderRead> Orders = HOLCommandScript::ConsumeMoves(Commands);
	T.AssertFalse(bIssued, "Step reports no order");
	T.AssertEquals(0, Orders.Num(), "nothing queued after a manual order");
}

// EFFECT: of the enemies around the unit, the NEAREST one within the attack range is picked; one out of range is never picked. Positions come from the
// calling code (the unit at the origin), the nearest is NOT the first of the list.
void Test_FollowOrdersPickEnemyInRangeTakesTheNearest(FUnitTest& T)
{
	TArray<int> Ids;
	TArray<FVector> Locations;
	Ids.Add(31); Locations.Add(FVector(900.f, 0.f, 0.f));
	Ids.Add(32); Locations.Add(FVector(0.f, 400.f, 0.f));
	Ids.Add(33); Locations.Add(FVector(0.f, 0.f, 250.f));
	Ids.Add(34); Locations.Add(FVector(200.f, 0.f, 0.f));
	const int Picked = HOLFollowOrders::PickEnemyInRange(FVector(0.f, 0.f, 0.f), 500.f, Ids, Locations);
	T.AssertEquals(34, Picked, "the nearest enemy within 500 cm is picked (200 cm), not the first in range of the list (400 cm) nor the one out of range (900 cm)");
}

// EFFECT: no enemy within range = no pick (-1), also for an empty list.
void Test_FollowOrdersPickEnemyInRangeNoneInRange(FUnitTest& T)
{
	TArray<int> Ids;
	TArray<FVector> Locations;
	const int PickedEmpty = HOLFollowOrders::PickEnemyInRange(FVector(0.f, 0.f, 0.f), 500.f, Ids, Locations);
	T.AssertEquals(-1, PickedEmpty, "an empty list picks nothing");
	Ids.Add(31); Locations.Add(FVector(900.f, 0.f, 0.f));
	Ids.Add(32); Locations.Add(FVector(0.f, 0.f, 501.f));
	const int PickedFar = HOLFollowOrders::PickEnemyInRange(FVector(0.f, 0.f, 0.f), 500.f, Ids, Locations);
	T.AssertEquals(-1, PickedFar, "enemies beyond the range pick nothing");
}

// EFFECT (real chain): the enemy PickEnemyInRange finds is the one the bodyguard's Step queues an Attack on, and with none in range the bodyguard queues nothing.
void Test_FollowOrdersBodyguardAttacksThePickedEnemy(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	FHOLFollowBook Book;
	HOLFollowOrders::Begin(Book, HOLFollowOrdersTest::UnitId, HOLFollowOrdersTest::AllyId, true);
	TArray<int> Ids;
	TArray<FVector> Locations;
	Ids.Add(41); Locations.Add(FVector(700.f, 0.f, 0.f));
	Ids.Add(42); Locations.Add(FVector(300.f, 0.f, 0.f));
	const FVector UnitLocation = FVector(0.f, 0.f, 0.f);
	const FVector AllyLocation = FVector(100.f, 0.f, 0.f);
	int Enemy = HOLFollowOrders::PickEnemyInRange(UnitLocation, 500.f, Ids, Locations);
	HOLFollowOrders::Step(Commands, 0, Book, HOLFollowOrdersTest::UnitId, UnitLocation, AllyLocation, Enemy >= 0, Enemy);
	TArray<FHOLScriptOrderRead> Orders = HOLCommandScript::ConsumeMoves(Commands);
	T.AssertEquals(1, Orders.Num(), "one order queued for the enemy in range");
	if (Orders.Num() == 1)
	{
		T.AssertTrue(Orders[0].Kind == n"Attack", "it is an Attack");
		T.AssertEquals(42, Orders[0].TargetUnitId, "on the picked enemy (300 cm), not the one out of range (700 cm)");
	}
	Enemy = HOLFollowOrders::PickEnemyInRange(UnitLocation, 200.f, Ids, Locations);
	HOLFollowOrders::Step(Commands, 0, Book, HOLFollowOrdersTest::UnitId, UnitLocation, AllyLocation, Enemy >= 0, Enemy);
	TArray<FHOLScriptOrderRead> After = HOLCommandScript::ConsumeMoves(Commands);
	T.AssertEquals(0, After.Num(), "no enemy within 200 cm: the bodyguard (already near its ally) queues nothing");
}

// The two bindings the bodyguard pick reads (HOLUnitScript::GetEnemiesNear / GetAttackRange). No Mass world in a script test: an unknown unit id reads as "nobody near, no range";
// the positive case (an enemy in range of a born unit) is a simulation test (D838). Red until the bindings exist: the script does not compile without them.
void Test_FollowOrdersUnitScriptBindingsAnswerEmptyForUnknownUnit(FUnitTest& T)
{
	TArray<int> Ids;
	Ids.Add(5);
	TArray<FVector> Locations;
	Locations.Add(FVector(1.f, 2.f, 3.f));
	const bool bFound = HOLUnitScript::GetEnemiesNear(987654, 1500.f, Ids, Locations);
	T.AssertFalse(bFound, "an unknown unit has no enemies near it");
	T.AssertEquals(0, Ids.Num(), "and the id list is cleared, not left as the caller filled it");
	T.AssertEquals(0, Locations.Num(), "and the location list is cleared");
	const float Range = HOLUnitScript::GetAttackRange(987654);
	T.AssertTrue(Range < 0.01f, f"an unknown unit has no attack range (got {Range})");
}

// PickEnemyForUnit reads the game through the bindings: an unknown unit id picks nobody (-1). The positive case needs a Mass world (simulation test).
void Test_FollowOrdersPickEnemyForUnknownUnitPicksNobody(FUnitTest& T)
{
	const int Picked = HOLFollowOrders::PickEnemyForUnit(nullptr, 987654, FVector(0.f, 0.f, 0.f));
	T.AssertEquals(-1, Picked, "a unit the game does not know picks no enemy");
}

// D818 Q2 = B, decision rule on the real chain (StepBodyguard -> Step -> Decide -> HOLPlayerOrders:: -> queue). EFFECT: a bodyguard WITHOUT an order of the player attacks the
// enemy in range, then walks to its ally again once the enemy is gone.
void Test_FollowOrdersBodyguardWithoutPlayerOrderAttacksThenFollowsAgain(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	FHOLFollowBook Book;
	HOLFollowOrders::Begin(Book, HOLFollowOrdersTest::UnitId, HOLFollowOrdersTest::AllyId, true);
	const FVector UnitLocation = FVector(0.f, 0.f, 0.f);
	const FVector AllyLocation = FVector(1500.f, 0.f, 0.f);
	const bool bAttackIssued = HOLFollowOrders::StepBodyguard(Commands, 0, Book, HOLFollowOrdersTest::UnitId, UnitLocation, AllyLocation, false, 42);
	TArray<FHOLScriptOrderRead> Attack = HOLCommandScript::ConsumeMoves(Commands);
	T.AssertTrue(bAttackIssued, "the step reports an order");
	T.AssertEquals(1, Attack.Num(), "one order queued while an enemy is in range");
	if (Attack.Num() == 1)
	{
		T.AssertTrue(Attack[0].Kind == n"Attack", "it is an Attack, not a Move toward the far ally");
		T.AssertEquals(42, Attack[0].TargetUnitId, "on the enemy picked for the bodyguard");
	}
	HOLFollowOrders::StepBodyguard(Commands, 0, Book, HOLFollowOrdersTest::UnitId, UnitLocation, AllyLocation, false, -1);
	TArray<FHOLScriptOrderRead> Resume = HOLCommandScript::ConsumeMoves(Commands);
	T.AssertEquals(1, Resume.Num(), "the enemy gone: one order queued");
	if (Resume.Num() == 1)
		T.AssertTrue(Resume[0].Kind == n"Move", "the bodyguard follows its ally again (Move)");
}

// EFFECT: a bodyguard that holds an order of the player is not interrupted: nothing is queued, whatever enemy is in range.
void Test_FollowOrdersBodyguardWithPlayerOrderIsNotInterrupted(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	FHOLFollowBook Book;
	HOLFollowOrders::Begin(Book, HOLFollowOrdersTest::UnitId, HOLFollowOrdersTest::AllyId, true);
	const bool bIssued = HOLFollowOrders::StepBodyguard(Commands, 0, Book, HOLFollowOrdersTest::UnitId, FVector(0.f, 0.f, 0.f), FVector(1500.f, 0.f, 0.f), true, 42);
	TArray<FHOLScriptOrderRead> Orders = HOLCommandScript::ConsumeMoves(Commands);
	T.AssertFalse(bIssued, "the step reports no order");
	T.AssertEquals(0, Orders.Num(), "nothing queued: neither the Attack on the enemy nor the Move toward the ally");
}

// StepUnit reads the game: a unit (or target) the game does not know gets no order (false), nothing queued.
void Test_FollowOrdersStepUnitUnknownUnitQueuesNothing(FUnitTest& T)
{
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	FHOLFollowBook Book;
	HOLFollowOrders::Begin(Book, 987654, 987655, true);
	const bool bIssued = HOLFollowOrders::StepUnit(nullptr, Commands, 0, Book, 987654, false);
	TArray<FHOLScriptOrderRead> Orders = HOLCommandScript::ConsumeMoves(Commands);
	T.AssertFalse(bIssued, "an unknown unit gets no order");
	T.AssertEquals(0, Orders.Num(), "nothing queued");
}

// Right click classification (D818 Q1 C: right click on an ALLY = Follow, on an ENEMY = Attack, on the ground = Move). The target in the selection is no follow target.
void Test_RightClickOnAllyUnitIsFollow(FUnitTest& T)
{
	TArray<int> Selected;
	Selected.Add(3);
	Selected.Add(4);
	const EHOLRightClickOrder Order = HOLFollowOrders::ClassifyRightClick(true, 9, true, Selected);
	T.AssertTrue(Order == EHOLRightClickOrder::Follow, "right click on an ally outside the selection is Follow");
}

void Test_RightClickOnEnemyUnitIsAttack(FUnitTest& T)
{
	TArray<int> Selected;
	Selected.Add(3);
	const EHOLRightClickOrder Order = HOLFollowOrders::ClassifyRightClick(true, 50, false, Selected);
	T.AssertTrue(Order == EHOLRightClickOrder::Attack, "right click on an enemy stays an attack (focus)");
}

void Test_RightClickOnGroundIsMove(FUnitTest& T)
{
	TArray<int> Selected;
	Selected.Add(3);
	const EHOLRightClickOrder Order = HOLFollowOrders::ClassifyRightClick(false, -1, true, Selected);
	T.AssertTrue(Order == EHOLRightClickOrder::Move, "right click on the ground is Move");
}

void Test_RightClickOnSelectedAllyIsNotFollow(FUnitTest& T)
{
	TArray<int> Selected;
	Selected.Add(3);
	Selected.Add(9);
	const EHOLRightClickOrder Order = HOLFollowOrders::ClassifyRightClick(true, 9, true, Selected);
	T.AssertTrue(Order == EHOLRightClickOrder::Move, "a unit of the selection does not follow itself: ordinary Move");
}

// EFFECT on the real book: every selected unit but the target becomes a bodyguard of the ally; an enemy click starts no follow.
void Test_ApplyRightClickOnAllyStartsFollowForTheOthers(FUnitTest& T)
{
	FHOLFollowBook Book;
	TArray<int> Selected;
	Selected.Add(3);
	Selected.Add(9);
	Selected.Add(4);
	const EHOLRightClickOrder Order = HOLFollowOrders::ApplyRightClick(Book, Selected, true, 9, true);
	T.AssertTrue(Order == EHOLRightClickOrder::Follow, "an ally click is Follow");
	const int Index3 = HOLFollowOrders::IndexOf(Book, 3);
	const int Index9 = HOLFollowOrders::IndexOf(Book, 9);
	T.AssertEquals(2, Book.UnitIds.Num(), "two followers: the target itself is left out");
	T.AssertTrue(Index3 >= 0, "unit 3 follows");
	T.AssertEquals(-1, Index9, "the followed unit does not follow itself");
	if (Index3 >= 0)
	{
		T.AssertEquals(9, Book.TargetIds[Index3], "unit 3 follows the ally 9");
		T.AssertTrue(Book.States[Index3].bActive && Book.States[Index3].bTargetIsAlly, "bodyguard state");
	}
}

void Test_ApplyRightClickOnEnemyStartsNoFollow(FUnitTest& T)
{
	FHOLFollowBook Book;
	TArray<int> Selected;
	Selected.Add(3);
	const EHOLRightClickOrder Order = HOLFollowOrders::ApplyRightClick(Book, Selected, true, 50, false);
	T.AssertTrue(Order == EHOLRightClickOrder::Attack, "an enemy click is an attack");
	T.AssertEquals(0, Book.UnitIds.Num(), "no follower: the attack goes through the normal order path");
}
