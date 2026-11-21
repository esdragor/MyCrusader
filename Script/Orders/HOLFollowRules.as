// Follow order, pure rule (GDD § 7.3, D30, D340, D818 Q2 = B, OrdersPatrolInProgress.md step 4). STUB: compiles, behaves wrongly (red first).

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
	const float FollowDistance = 300.f;

	FHOLFollowState Start(bool bTargetIsAlly)
	{
		FHOLFollowState State;
		State.bTargetIsAlly = bTargetIsAlly;
		return State;
	}

	EHOLFollowOrder Decide(const FHOLFollowState& State, FVector UnitLocation, FVector TargetLocation, bool bEnemyInRange)
	{
		return EHOLFollowOrder::None;
	}

	FVector Destination(FVector TargetLocation)
	{
		return FVector(0.f, 0.f, 0.f);
	}

	void OnManualOrder(FHOLFollowState& State)
	{
	}
}
