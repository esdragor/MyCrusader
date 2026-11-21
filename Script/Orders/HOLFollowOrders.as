// Follow order, the script side that EMITS the orders (GDD § 7.3, D818 Q2 = B, OrdersPatrolInProgress.md step 4). No world access: the caller hands over the
// positions it reads (unit, followed target, an enemy in range) at each order step, as HOLPlayerOrdersBoard does for its arguments. Tested in HOLFollowOrders_Test.as.
// HOLFollowRules.as decides (pure); Step sends the Move / Attack through HOLPlayerOrders:: on the command queue (the same gate and chain as a click).
// Not wired to the game yet: the C++ must call Begin on a right click on a unit and Step at each order step (TYPES_A_EXPOSER in the report). NOT COMPILED.

// What a right click with units selected means (GDD § 7.3, D818 Q1 C: no grid case for Follow or focus): ground = Move, ally unit = Follow, enemy unit = Attack.
enum EHOLRightClickOrder
{
	Move,
	Attack,
	Follow
}

struct FHOLFollowBook
{
	TArray<int> UnitIds;
	TArray<int> TargetIds;
	TArray<FHOLFollowState> States;
	// The enemy last attacked for each follower (-1 none): an Attack is queued once, not at every step.
	TArray<int> LastAttackIds;
}

// The follows of the local player, filled by a right click on an ally (HOLControllerCommandsC10a::RightClickUnitOrder); the step caller (C++, TYPES_A_EXPOSER) reads it.
// A script global cannot be mutable: the book lives in a world subsystem (UHOLFollowBookHolder::Get().Book).
class UHOLFollowBookHolder : UScriptWorldSubsystem
{
	UPROPERTY()
	FHOLFollowBook Book;
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
		return HOLUnitScript::GetUnitLocation(UnitId, OutLocation);
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

	// The enemy an order step hands to Step as EnemyId (GDD D818 Q2 = B: a bodyguard attacks the enemies in the range of the unit). Pure: the caller reads the candidates
	// (ids and positions of the living enemies near the unit) and the attack range of the unit's type; the nearest enemy within Range wins, -1 when none.
	// Range is the attack range in cm measured between the two positions (the C++ binding gives the type's range, TYPES_A_EXPOSER).
	int PickEnemyInRange(const FVector& UnitLocation, float Range, const TArray<int>& EnemyIds, const TArray<FVector>& EnemyLocations)
	{
		int Picked = -1;
		float BestDistance = Range;
		const int Count = Math::Min(EnemyIds.Num(), EnemyLocations.Num());
		for (int Index = 0; Index < Count; Index++)
		{
			const float Distance = (EnemyLocations[Index] - UnitLocation).Size();
			if (Distance <= BestDistance)
			{
				BestDistance = Distance;
				Picked = EnemyIds[Index];
			}
		}
		return Picked;
	}

	// The enemy Step gets as EnemyId, read from the game (HOLUnitScript, TYPES_A_EXPOSER of 1121se): the living enemies within the unit's reach, then PickEnemyInRange.
	// The type's attack range is edge to edge, PickEnemyInRange measures centres: BodiesReach (two bodies of about 60 cm) is added to it (approximation, the C++ combat
	// uses the real radii). -1 when the unit is gone or nobody is in reach.
	int PickEnemyForUnit(UObject WorldContext, int UnitId, const FVector& UnitLocation)
	{
		const float BodiesReach = 120.0;
		const float Reach = HOLUnitScript::GetAttackRange(UnitId) + BodiesReach;
		TArray<int> EnemyIds;
		TArray<FVector> EnemyLocations;
		if (!HOLUnitScript::GetEnemiesNear(UnitId, Reach, EnemyIds, EnemyLocations))
			return -1;
		return PickEnemyInRange(UnitLocation, Reach, EnemyIds, EnemyLocations);
	}

	// One order step of a bodyguard (GDD D818 Q2 = B): a bodyguard WITHOUT an order of the player attacks the enemy picked for it (EnemyId, -1 none), then follows again once the
	// enemy is gone (Step: Decide gives Move again and the Attack memory is reset); one that holds a player order is never interrupted (nothing queued).
	// bPlayerOrderActive: the unit executes an order the player gave it (the caller reads it).
	bool StepBodyguard(UHOLCommandSubsystem Commands, int PlayerIndex, FHOLFollowBook& Book, int UnitId, const FVector& UnitLocation, const FVector& TargetLocation, bool bPlayerOrderActive, int EnemyId)
	{
		if (bPlayerOrderActive)
			return false;
		return Step(Commands, PlayerIndex, Book, UnitId, UnitLocation, TargetLocation, EnemyId >= 0, EnemyId);
	}

	// The step the game calls for a follower: reads the unit, the followed unit and the enemy picked for the unit (ReadUnit, PickEnemyForUnit), then StepBodyguard.
	// False when the unit or its target is gone (the caller ends the follow with OnManualOrder). An enemy is only looked for when the unit holds no player order.
	bool StepUnit(UObject WorldContext, UHOLCommandSubsystem Commands, int PlayerIndex, FHOLFollowBook& Book, int UnitId, bool bPlayerOrderActive)
	{
		const int Index = IndexOf(Book, UnitId);
		if (Index < 0)
			return false;
		FVector UnitLocation;
		FVector TargetLocation;
		if (!ReadUnit(WorldContext, UnitId, UnitLocation) || !ReadUnit(WorldContext, Book.TargetIds[Index], TargetLocation))
			return false;
		const int EnemyId = bPlayerOrderActive ? -1 : PickEnemyForUnit(WorldContext, UnitId, UnitLocation);
		return StepBodyguard(Commands, PlayerIndex, Book, UnitId, UnitLocation, TargetLocation, bPlayerOrderActive, EnemyId);
	}

	// The meaning of a right click with units selected (D818 Q1 C). Ground, or a unit of the selection itself = Move; an enemy unit = Attack (focus);
	// an ally unit outside the selection = Follow. bTargetIsAlly: same camp or allied by Diplomacy, decided by the caller.
	EHOLRightClickOrder ClassifyRightClick(bool bOnUnit, int TargetId, bool bTargetIsAlly, const TArray<int>& SelectedIds)
	{
		if (!bOnUnit)
			return EHOLRightClickOrder::Move;
		if (!bTargetIsAlly)
			return EHOLRightClickOrder::Attack;
		if (SelectedIds.Contains(TargetId))
			return EHOLRightClickOrder::Move;
		return EHOLRightClickOrder::Follow;
	}

	// The right click of the player: classifies it and, for Follow, makes every selected unit (but the target) a bodyguard of the ally (Begin).
	// Move / Attack: the book is untouched, the caller sends the ordinary order. A new order of a follower ends its follow through OnManualOrder (caller).
	EHOLRightClickOrder ApplyRightClick(FHOLFollowBook& Book, const TArray<int>& SelectedIds, bool bOnUnit, int TargetId, bool bTargetIsAlly)
	{
		const EHOLRightClickOrder Order = ClassifyRightClick(bOnUnit, TargetId, bTargetIsAlly, SelectedIds);
		if (Order == EHOLRightClickOrder::Follow)
		{
			for (int UnitId : SelectedIds)
			{
				if (UnitId != TargetId)
					Begin(Book, UnitId, TargetId, true);
			}
		}
		return Order;
	}
}
