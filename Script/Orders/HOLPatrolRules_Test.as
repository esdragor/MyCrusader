// Patrol, pure rule (GDD § 7.3 "Ordres : ... patrouille", OrdersPatrolInProgress.md). The tests walk a unit tick by tick toward the target
// the rule gives (HOLPatrolRules::LegTarget), at a fixed speed, and assert WHERE THE UNIT ENDS: it goes to B, then comes back to A by itself,
// each leg as an attack-move, and a manual order ends the patrol (the unit no longer walks).

namespace HOLPatrolTest
{
	// One tick of walking: Step centimetres toward To (or onto it).
	FVector MoveToward(FVector From, FVector To, float Step)
	{
		const FVector Away = To - From;
		const float Length = Away.Size();
		if (Length <= Step)
			return To;
		return From + Away * (Step / Length);
	}

	// One tick of the patrol: the unit walks along the current leg if the rule gives it an order, then the rule sees where it stands.
	FVector Tick(FHOLPatrolState& State, FVector Position)
	{
		FVector Next = Position;
		if (HOLPatrolRules::LegOrder(State) == EHOLPatrolOrder::AttackMove)
			Next = MoveToward(Position, HOLPatrolRules::LegTarget(State), 100.f);
		HOLPatrolRules::Advance(State, Next, HOLPatrolRules::ArriveRadius);
		return Next;
	}
}

// EFFECT: a unit ordered to patrol from A to B reaches B, then walks back to A by itself (and leaves A again for B).
void Test_PatrolUnitGoesBackToAOnceAtB(FUnitTest& T)
{
	const FVector A = FVector(0.f, 0.f, 0.f);
	const FVector B = FVector(1000.f, 0.f, 0.f);
	FHOLPatrolState State = HOLPatrolRules::Start(A, B);
	FVector Position = A;
	bool bReachedB = false;
	bool bBackAtA = false;
	float FarthestAfterA = 0.f;
	for (int Index = 0; Index < 30; Index++)
	{
		Position = HOLPatrolTest::Tick(State, Position);
		if (!bReachedB)
			bReachedB = (Position - B).Size() <= HOLPatrolRules::ArriveRadius;
		else if (!bBackAtA)
			bBackAtA = (Position - A).Size() <= HOLPatrolRules::ArriveRadius;
		else
			FarthestAfterA = Math::Max(FarthestAfterA, Position.X);
	}
	T.AssertTrue(bReachedB, "the unit reached B");
	T.AssertTrue(bBackAtA, f"the unit went back to A once at B (ends at X {Position.X})");
	T.AssertTrue(FarthestAfterA >= 400.f, f"at A it leaves for B again (farthest X after A {FarthestAfterA})");
}

// EFFECT: every leg, out and back, is an attack-move (the unit takes on the enemies met on the way), never a plain move.
void Test_PatrolEveryLegIsAnAttackMove(FUnitTest& T)
{
	FHOLPatrolState State = HOLPatrolRules::Start(FVector(0.f, 0.f, 0.f), FVector(600.f, 0.f, 0.f));
	FVector Position = FVector(0.f, 0.f, 0.f);
	int AttackMoveTicks = 0;
	for (int Index = 0; Index < 30; Index++)
	{
		if (HOLPatrolRules::LegOrder(State) == EHOLPatrolOrder::AttackMove)
			AttackMoveTicks++;
		Position = HOLPatrolTest::Tick(State, Position);
	}
	T.AssertEquals(30, AttackMoveTicks, "all 30 ticks of a patrol (two legs out, two back) are under an attack-move");
}

// EFFECT: a manual order (move, attack, stop, hold) ends the patrol: the unit stops walking the legs where it stands.
void Test_PatrolManualOrderStopsIt(FUnitTest& T)
{
	FHOLPatrolState State = HOLPatrolRules::Start(FVector(0.f, 0.f, 0.f), FVector(1000.f, 0.f, 0.f));
	FVector Position = FVector(0.f, 0.f, 0.f);
	for (int Index = 0; Index < 4; Index++)
		Position = HOLPatrolTest::Tick(State, Position);
	const FVector WhenOrdered = Position;
	HOLPatrolRules::OnManualOrder(State);
	for (int Index = 0; Index < 20; Index++)
		Position = HOLPatrolTest::Tick(State, Position);
	T.AssertTrue((Position - WhenOrdered).Size() < 1.f, f"after the manual order the patrol no longer moves the unit (moved {(Position - WhenOrdered).Size()})");
	T.AssertTrue(HOLPatrolRules::LegOrder(State) == EHOLPatrolOrder::None, "no patrol order any more");
}
