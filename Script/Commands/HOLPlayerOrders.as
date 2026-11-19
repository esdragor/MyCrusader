// P1a (lot P1 of the C++ -> Angelscript migration, step 5): the player's orders, first half. The rules of HOLPlayerOrders.cpp:16-124 (IsAccepted, Move,
// Attack, AttackBuilding, AttackMove, Stop, ToggleDeploy) in script. NOT CALLED BY THE GAME YET: HOLPlayerOrders.cpp still decides until the C++ bridge
// (P1b) replaces its bodies; until then the rule exists twice, and HOLPlayerOrders_Test.as covers the same cases as the C++ (agreement to prove in P1b).
// SendNonWorkersTo stays C++ (it reads HOLUnitView). NOT COMPILED at the time of writing.
// Every emitter takes the command subsystem (GetCommands(World) in the game, MakeDetachedForTest() in a test), gates the order, then calls the
// Issue*Player binding, which opens the player-order scope in C++ (the chain of the order is drawn, D328). Returns false when the order is refused.

namespace HOLPlayerOrders
{
	// The most units one order may name (HOLPlayerOrders.h MaxOrderedUnits, 200). A single source: the C++ constant goes in P1b.
	const int MaxOrderedUnitsValue = 200;

	int GetMaxOrderedUnits()
	{
		return MaxOrderedUnitsValue;
	}

	// An order of PlayerIndex naming NumUnits units may enter the queue: 0 <= PlayerIndex < MaxSlots and 0 <= NumUnits <= MaxOrderedUnits.
	bool IsAccepted(int PlayerIndex, int NumUnits)
	{
		return PlayerIndex >= 0 && PlayerIndex < HOLCommandScript::GetMaxSlots() && NumUnits >= 0 && NumUnits <= MaxOrderedUnitsValue;
	}

	bool Move(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& UnitIds, const FVector& Target, bool bQueue, const FVector2D& FrontLine, bool bFrontWidth)
	{
		if (Commands == nullptr || !IsAccepted(PlayerIndex, UnitIds.Num()))
		{
			return false;
		}
		HOLCommandScript::IssueMovePlayer(Commands, UnitIds, Target, PlayerIndex, bQueue, FrontLine, bFrontWidth);
		return true;
	}

	bool Attack(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& UnitIds, int TargetUnitId, bool bQueue)
	{
		if (Commands == nullptr || !IsAccepted(PlayerIndex, UnitIds.Num()))
		{
			return false;
		}
		HOLCommandScript::IssueAttackPlayer(Commands, UnitIds, TargetUnitId, PlayerIndex, bQueue);
		return true;
	}

	bool AttackBuilding(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& UnitIds, AHOLBuilding Building, bool bQueue)
	{
		if (Commands == nullptr || Building == nullptr || !IsAccepted(PlayerIndex, UnitIds.Num()))
		{
			return false;
		}
		HOLCommandScript::IssueAttackBuildingPlayer(Commands, UnitIds, Building, PlayerIndex, bQueue);
		return true;
	}

	bool AttackMove(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& UnitIds, const FVector& Target, bool bQueue)
	{
		if (Commands == nullptr || !IsAccepted(PlayerIndex, UnitIds.Num()))
		{
			return false;
		}
		HOLCommandScript::IssueAttackMovePlayer(Commands, UnitIds, Target, PlayerIndex, bQueue);
		return true;
	}

	// bHold: Hold, else Stop. No queue flag (a stop empties the queue).
	bool Stop(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& UnitIds, bool bHold)
	{
		if (Commands == nullptr || !IsAccepted(PlayerIndex, UnitIds.Num()))
		{
			return false;
		}
		HOLCommandScript::IssueStopPlayer(Commands, UnitIds, PlayerIndex, bHold);
		return true;
	}

	bool ToggleDeploy(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& UnitIds)
	{
		if (Commands == nullptr || !IsAccepted(PlayerIndex, UnitIds.Num()))
		{
			return false;
		}
		HOLCommandScript::IssueToggleDeployPlayer(Commands, UnitIds, PlayerIndex);
		return true;
	}
}