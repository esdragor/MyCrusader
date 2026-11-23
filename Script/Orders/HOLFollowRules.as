// Follow order, pure rule (GDD § 7.3, D30, D340, D818 Q2 = B, OrdersPatrolInProgress.md step 4). No world access, tested in HOLFollowRules_Test.as.
// Following an ALLY = bodyguard: the unit stays within FollowDistance of the ally and attacks any enemy that comes in its range.
// Following an ENEMY = a plain attack on it (focus). Any manual order ends the follow.
// The right-click that starts it (ally / enemy under the cursor) and the grid are wired by the caller, not chosen here.

enum EHOLFollowOrder
{
	None,
	Move,
	Attack
}

struct FHOLFollowState
{
	bool bTargetIsAlly = true;
	bool bActive = false;
}

namespace HOLFollowRules
{
	// The bodyguard walks again once farther than this (cm) from its ally. One line to change.
	const float FollowDistance = 300.f;

	FHOLFollowState Start(bool bTargetIsAlly)
	{
		FHOLFollowState State;
		State.bTargetIsAlly = bTargetIsAlly;
		State.bActive = true;
		return State;
	}

	// The order the unit holds this tick. bEnemyInRange: an enemy is within the unit's attack range (only read for an ally target).
	EHOLFollowOrder Decide(const FHOLFollowState& State, FVector UnitLocation, FVector TargetLocation, bool bEnemyInRange)
	{
		if (!State.bActive)
			return EHOLFollowOrder::None;
		if (!State.bTargetIsAlly)
			return EHOLFollowOrder::Attack;
		if (bEnemyInRange)
			return EHOLFollowOrder::Attack;
		FVector Away = TargetLocation - UnitLocation;
		Away.Z = 0.f;
		return Away.Size() > FollowDistance ? EHOLFollowOrder::Move : EHOLFollowOrder::None;
	}

	// Where a Move order walks to: the ally itself (the unit stops once within FollowDistance).
	FVector Destination(FVector TargetLocation)
	{
		return TargetLocation;
	}

	// STUB (D895, red commit): does the order 'OrderedUnitId follows X' end the follow that X holds (TargetState, following TargetFollowsId) on the ordered unit?
	bool CancelsFollowOf(const FHOLFollowState& TargetState, int TargetFollowsId, int OrderedUnitId)
	{
		return false;
	}

	// Any manual order (move, attack, attack-move, stop, hold, patrol, another follow) ends this follow.
	void OnManualOrder(FHOLFollowState& State)
	{
		State.bActive = false;
	}
}
