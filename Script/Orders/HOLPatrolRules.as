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

	FVector LegTarget(const FHOLPatrolState& State)
	{
		return State.PointB;
	}

	EHOLPatrolOrder LegOrder(const FHOLPatrolState& State)
	{
		return EHOLPatrolOrder::AttackMove;
	}

	// RED STUB (test first): never turns round.
	bool Advance(FHOLPatrolState& State, FVector UnitLocation, float Radius)
	{
		return false;
	}

	// RED STUB (test first): a manual order does nothing yet.
	void OnManualOrder(FHOLPatrolState& State)
	{
	}
}
