// Follow order, the script side that EMITS the orders (GDD § 7.3, D818 Q2 = B, OrdersPatrolInProgress.md step 4). No world access: the caller hands over the
// positions it reads (unit, followed target, an enemy in range) at each order step, as HOLPlayerOrdersBoard does for its arguments. Tested in HOLFollowOrders_Test.as.
// HOLFollowRules.as decides (pure); Step sends the Move / Attack through HOLPlayerOrders:: on the command queue (the same gate and chain as a click).
// Not wired to the game yet: the C++ must call Begin on a right click on a unit and Step at each order step (TYPES_A_EXPOSER in the report). NOT COMPILED.

struct FHOLFollowBook
{
	TArray<int> UnitIds;
	TArray<int> TargetIds;
	TArray<FHOLFollowState> States;
	// The enemy last attacked for each follower (-1 none): an Attack is queued once, not at every step.
	TArray<int> LastAttackIds;
}

namespace HOLFollowOrders
{
	int IndexOf(const FHOLFollowBook& Book, int UnitId)
	{
		for (int Index = 0; Index < Book.UnitIds.Num(); Index++)
		{
			if (Book.UnitIds[Index] == UnitId)
				return Index;
		}
		return -1;
	}

	// The right click of the player on a unit: an ally (bodyguard) or an enemy (plain attack). A unit follows one target at a time.
	void Begin(FHOLFollowBook& Book, int UnitId, int TargetId, bool bTargetIsAlly)
	{
		const FHOLFollowState State = HOLFollowRules::Start(bTargetIsAlly);
		const int Index = IndexOf(Book, UnitId);
		if (Index < 0)
		{
			Book.UnitIds.Add(UnitId);
			Book.TargetIds.Add(TargetId);
			Book.States.Add(State);
			Book.LastAttackIds.Add(-1);
		}
		else
		{
			Book.TargetIds[Index] = TargetId;
			Book.States[Index] = State;
			Book.LastAttackIds[Index] = -1;
		}
	}

	// Any manual order of the unit ends its follow.
	void OnManualOrder(FHOLFollowBook& Book, int UnitId)
	{
		const int Index = IndexOf(Book, UnitId);
		if (Index < 0)
			return;
		FHOLFollowState State = Book.States[Index];
		HOLFollowRules::OnManualOrder(State);
		Book.States[Index] = State;
		Book.LastAttackIds[Index] = -1;
	}

	// The read the caller does before a Step (Auto_Journey_File.md l.118): the position of a unit by id, false when it is gone (dead, garrisoned, unknown).
	// A follower whose target is gone gets no Step (the caller ends the follow). WorldContext: any object of the game world.
	bool ReadUnit(UObject WorldContext, int UnitId, FVector& OutLocation)
	{
		return HOLUnitScript::GetUnitLocation(WorldContext, UnitId, OutLocation);
	}

	// One order step of a follower. TargetLocation is the followed unit's position, bEnemyInRange / EnemyId an enemy in the follower's attack range (read by the
	// caller). Returns true when an order entered the queue.
	bool Step(UHOLCommandSubsystem Commands, int PlayerIndex, FHOLFollowBook& Book, int UnitId, const FVector& UnitLocation, const FVector& TargetLocation, bool bEnemyInRange, int EnemyId)
	{
		const int Index = IndexOf(Book, UnitId);
		if (Index < 0)
			return false;
		const FHOLFollowState State = Book.States[Index];
		const EHOLFollowOrder Order = HOLFollowRules::Decide(State, UnitLocation, TargetLocation, bEnemyInRange);
		TArray<int> Units;
		Units.Add(UnitId);
		if (Order == EHOLFollowOrder::Move)
		{
			Book.LastAttackIds[Index] = -1;
			return HOLPlayerOrders::Move(Commands, PlayerIndex, Units, HOLFollowRules::Destination(TargetLocation), false, FVector2D(0.0, 0.0), false);
		}
		if (Order == EHOLFollowOrder::Attack)
		{
			const int AttackId = State.bTargetIsAlly ? EnemyId : Book.TargetIds[Index];
			if (Book.LastAttackIds[Index] == AttackId)
				return false;
			Book.LastAttackIds[Index] = AttackId;
			return HOLPlayerOrders::Attack(Commands, PlayerIndex, Units, AttackId, false);
		}
		Book.LastAttackIds[Index] = -1;
		return false;
	}
}
