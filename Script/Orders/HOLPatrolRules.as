// Patrol order, pure rule (GDD § 7.3 "Ordres : ... patrouille", OrdersPatrolInProgress.md). No world access, tested in HOLPatrolRules_Test.as.
// A patrol goes back and forth between two points: A = where the unit stood when ordered, B = the clicked point. Every leg is an
// attack-move (the unit takes on the enemies met on the way, then walks on); a manual order ends the patrol.
// The cell of the grid and the keyboard key are NOT chosen here (Retours/Q-patrol-follow.md).

enum EHOLPatrolOrder
{
	None,
	AttackMove
}

struct FHOLPatrolState
{
	FVector PointA;
	FVector PointB;
	bool bTowardB = true;
	bool bActive = false;
}

namespace HOLPatrolRules
{
	// A leg is reached when the unit stands within this distance (cm) of its end. One line to change.
	const float ArriveRadius = 150.f;

	FHOLPatrolState Start(FVector UnitLocation, FVector Target)
	{
		FHOLPatrolState State;
		State.PointA = UnitLocation;
		State.PointB = Target;
		State.bTowardB = true;
		State.bActive = true;
		return State;
	}

	// Where the current leg ends: B on the way out, A on the way back.
	FVector LegTarget(const FHOLPatrolState& State)
	{
		return State.bTowardB ? State.PointB : State.PointA;
	}

	// The order the unit walks the leg under: an attack-move while the patrol lasts, nothing once it is over.
	EHOLPatrolOrder LegOrder(const FHOLPatrolState& State)
	{
		return State.bActive ? EHOLPatrolOrder::AttackMove : EHOLPatrolOrder::None;
	}

	// Sees where the unit stands: at the end of the leg (within Radius, on the ground plane) it turns round toward the other point.
	// Returns true when a new leg starts (the caller gives the unit a new attack-move to LegTarget).
	bool Advance(FHOLPatrolState& State, FVector UnitLocation, float Radius)
	{
		if (!State.bActive)
			return false;
		FVector Away = LegTarget(State) - UnitLocation;
		Away.Z = 0.f;
		if (Away.Size() > Radius)
			return false;
		State.bTowardB = !State.bTowardB;
		return true;
	}

	// Any manual order (move, attack, attack-move, stop, hold, another patrol) ends this patrol.
	void OnManualOrder(FHOLPatrolState& State)
	{
		State.bActive = false;
	}
}
