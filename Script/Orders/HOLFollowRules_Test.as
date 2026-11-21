// Follow, pure rule (GDD § 7.3, D30, D340, D818 Q2 = B, OrdersPatrolInProgress.md step 4). The tests walk a follower and a target tick by tick and assert
// WHERE THE FOLLOWER ENDS and WHICH ORDER it holds: following an ally = bodyguard (stays near the ally, attacks what comes in range),
// following an enemy = a plain attack on it (focus), any manual order ends the follow.

namespace HOLFollowTest
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

	// One tick of the follower: it walks toward the target only if the rule orders a move.
	FVector Tick(FHOLFollowState& State, FVector Position, FVector TargetLocation, bool bEnemyInRange)
	{
		const EHOLFollowOrder Order = HOLFollowRules::Decide(State, Position, TargetLocation, bEnemyInRange);
		if (Order == EHOLFollowOrder::Move)
			return MoveToward(Position, HOLFollowRules::Destination(TargetLocation), 120.f);
		return Position;
	}
}

// EFFECT: an ally walking away is kept within the follow distance by the follower (it does not stay where it was ordered).
void Test_FollowAllyKeepsUpWithAWalkingAlly(FUnitTest& T)
{
	FHOLFollowState State = HOLFollowRules::Start(true);
	FVector Ally = FVector(500.f, 0.f, 0.f);
	FVector Position = FVector(0.f, 0.f, 0.f);
	for (int Index = 0; Index < 40; Index++)
	{
		Ally.X += 80.f;
		Position = HOLFollowTest::Tick(State, Position, Ally, false);
	}
	const float Gap = (Ally - Position).Size();
	T.AssertTrue(Gap <= HOLFollowRules::FollowDistance + 130.f, f"the follower kept up with the ally (gap {Gap} cm, ally at X {Ally.X}, follower at X {Position.X})");
	T.AssertTrue(Position.X > 1000.f, f"the follower actually travelled (X {Position.X})");
}

// EFFECT: next to its ally the follower stands still (no jitter around the ally).
void Test_FollowAllyStandsStillWhenClose(FUnitTest& T)
{
	FHOLFollowState State = HOLFollowRules::Start(true);
	const FVector Ally = FVector(200.f, 0.f, 0.f);
	FVector Position = FVector(0.f, 0.f, 0.f);
	for (int Index = 0; Index < 10; Index++)
		Position = HOLFollowTest::Tick(State, Position, Ally, false);
	T.AssertTrue((Position - FVector(0.f, 0.f, 0.f)).Size() < 1.f, f"the follower did not move next to its ally (X {Position.X})");
}

// EFFECT: bodyguard = an enemy in range makes the follower attack, even when the ally is far (the guard fights what reaches the ally).
void Test_FollowAllyAttacksAnEnemyInRange(FUnitTest& T)
{
	FHOLFollowState State = HOLFollowRules::Start(true);
	const EHOLFollowOrder Order = HOLFollowRules::Decide(State, FVector(0.f, 0.f, 0.f), FVector(2000.f, 0.f, 0.f), true);
	T.AssertTrue(Order == EHOLFollowOrder::Attack, "an enemy in range: the bodyguard attacks");
}

// EFFECT: following an ENEMY is an attack on it (focus): the order is Attack whatever the distance, never a plain move.
void Test_FollowEnemyIsAnAttack(FUnitTest& T)
{
	FHOLFollowState State = HOLFollowRules::Start(false);
	const EHOLFollowOrder Far = HOLFollowRules::Decide(State, FVector(0.f, 0.f, 0.f), FVector(5000.f, 0.f, 0.f), false);
	const EHOLFollowOrder Near = HOLFollowRules::Decide(State, FVector(0.f, 0.f, 0.f), FVector(100.f, 0.f, 0.f), false);
	T.AssertTrue(Far == EHOLFollowOrder::Attack, "far from an enemy target: attack");
	T.AssertTrue(Near == EHOLFollowOrder::Attack, "next to an enemy target: attack");
}

// EFFECT: a manual order ends the follow: the unit no longer walks after the ally.
void Test_FollowManualOrderStopsIt(FUnitTest& T)
{
	FHOLFollowState State = HOLFollowRules::Start(true);
	HOLFollowRules::OnManualOrder(State);
	const EHOLFollowOrder Order = HOLFollowRules::Decide(State, FVector(0.f, 0.f, 0.f), FVector(3000.f, 0.f, 0.f), true);
	T.AssertTrue(Order == EHOLFollowOrder::None, "no order once a manual order ended the follow");
	T.AssertTrue(!State.bActive, "the follow is no longer active");
}
