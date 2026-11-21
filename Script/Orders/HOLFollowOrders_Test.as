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
