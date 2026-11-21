// Follow order, the script side that EMITS the orders (GDD § 7.3, D818 Q2 = B, OrdersPatrolInProgress.md step 4). STUB for the red test: nothing is emitted yet.
// HOLFollowRules.as decides (pure); this file will keep the followers and send the Move / Attack through HOLPlayerOrders:: on the command queue.

struct FHOLFollowBook
{
	TArray<int> UnitIds;
	TArray<int> TargetIds;
	TArray<FHOLFollowState> States;
	TArray<int> LastAttackIds;
}

namespace HOLFollowOrders
{
	void Begin(FHOLFollowBook& Book, int UnitId, int TargetId, bool bTargetIsAlly)
	{
	}

	void OnManualOrder(FHOLFollowBook& Book, int UnitId)
	{
	}

	bool Step(UHOLCommandSubsystem Commands, int PlayerIndex, FHOLFollowBook& Book, int UnitId, const FVector& UnitLocation, const FVector& TargetLocation, bool bEnemyInRange, int EnemyId)
	{
		return false;
	}
}
