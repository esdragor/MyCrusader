// P1b (MIG-CPP step 5): the script side of the bridge UHOLPlayerOrdersBoard (Source/HeroesOfLiberty/Commands/HOLPlayerOrdersBoard.h; D839: a binding to
// expose). HOLPlayerOrders.cpp (Move, Attack, AttackBuilding, AttackMove, Stop, ToggleDeploy, IsAccepted) asks this board first; the answer is 1 (accepted and
// queued), 0 (refused). Without this class compiled, the C++ answers -1 and keeps its own body (the twin, removed in a later lot after the network proof).
// The rule is HOLPlayerOrders:: of HOLPlayerOrders.as, called with the world's command subsystem; this file only turns its bool into the whole number.
// The namespace functions take the subsystem as an argument so that a pure test can check the answer on a detached queue (HOLPlayerOrdersBoard_Test.as).
// NOT in a Dev / Developers / Examples / Editor folder: the dedicated server runs it. NOT COMPILED at the time of writing.

namespace HOLPlayerOrdersBoard
{
	int Answer(bool bDone)
	{
		return bDone ? 1 : 0;
	}

	int AnswerIsAccepted(int PlayerIndex, int NumUnits)
	{
		return Answer(HOLPlayerOrders::IsAccepted(PlayerIndex, NumUnits));
	}

	int AnswerMove(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& UnitIds, const FVector& Target, bool bQueue, const FVector2D& FrontLine, bool bFrontWidth)
	{
		return Answer(HOLPlayerOrders::Move(Commands, PlayerIndex, UnitIds, Target, bQueue, FrontLine, bFrontWidth));
	}

	int AnswerAttack(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& UnitIds, int TargetUnitId, bool bQueue)
	{
		return Answer(HOLPlayerOrders::Attack(Commands, PlayerIndex, UnitIds, TargetUnitId, bQueue));
	}

	int AnswerAttackBuilding(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& UnitIds, AHOLBuilding Building, bool bQueue)
	{
		return Answer(HOLPlayerOrders::AttackBuilding(Commands, PlayerIndex, UnitIds, Building, bQueue));
	}

	int AnswerAttackMove(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& UnitIds, const FVector& Target, bool bQueue)
	{
		return Answer(HOLPlayerOrders::AttackMove(Commands, PlayerIndex, UnitIds, Target, bQueue));
	}

	int AnswerStop(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& UnitIds, bool bHold)
	{
		return Answer(HOLPlayerOrders::Stop(Commands, PlayerIndex, UnitIds, bHold));
	}

	int AnswerToggleDeploy(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& UnitIds)
	{
		return Answer(HOLPlayerOrders::ToggleDeploy(Commands, PlayerIndex, UnitIds));
	}

	int AnswerGather(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& Workers, int SourceIndex, bool bQueue, bool bSource, const FVector& SourceLocation)
	{
		return Answer(HOLPlayerOrders::Gather(Commands, PlayerIndex, Workers, SourceIndex, bQueue, bSource, SourceLocation));
	}

	int AnswerBuild(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& Workers, AHOLBuilding Building, bool bQueue)
	{
		return Answer(HOLPlayerOrders::Build(Commands, PlayerIndex, Workers, Building, bQueue));
	}

	int AnswerDropOff(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& UnitIds, AHOLBuilding Building, const FVector& Point, bool bQueue, bool bDeposit)
	{
		return Answer(HOLPlayerOrders::DropOff(Commands, PlayerIndex, UnitIds, Building, Point, bQueue, bDeposit));
	}

	int AnswerPlace(UHOLCommandSubsystem Commands, int PlayerIndex, FName TypeName, const FVector& Point, const TArray<int>& UnitIds, bool bQueue)
	{
		return Answer(HOLPlayerOrders::Place(Commands, PlayerIndex, TypeName, Point, UnitIds, bQueue));
	}

	int AnswerQueueProduction(UHOLCommandSubsystem Commands, int PlayerIndex, AHOLBuilding Building, FName TypeName)
	{
		return Answer(HOLPlayerOrders::QueueProduction(Commands, PlayerIndex, Building, TypeName));
	}

	int AnswerMarketTrade(UHOLCommandSubsystem Commands, int PlayerIndex, AHOLBuilding Building, int Resource, int Lots, bool bBuy)
	{
		return Answer(HOLPlayerOrders::MarketTrade(Commands, PlayerIndex, Building, Resource, Lots, bBuy));
	}

	int AnswerSetRally(UHOLCommandSubsystem Commands, int PlayerIndex, AHOLBuilding Building, const FVector& Point, int SourceIndex, bool bSourceStanding, AHOLBuilding Site, bool bQueue)
	{
		return Answer(HOLPlayerOrders::SetRally(Commands, PlayerIndex, Building, Point, SourceIndex, bSourceStanding, Site, bQueue));
	}

	int AnswerGarrison(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& UnitIds, AHOLBuilding Building)
	{
		return Answer(HOLPlayerOrders::Garrison(Commands, PlayerIndex, UnitIds, Building));
	}

	int AnswerUngarrisonAll(UHOLCommandSubsystem Commands, int PlayerIndex, AHOLBuilding Building)
	{
		return Answer(HOLPlayerOrders::UngarrisonAll(Commands, PlayerIndex, Building));
	}}

/** The script side of UHOLPlayerOrdersBoard: gets the world's command subsystem (as the C++ GetQueue did: World.GetSubsystem) and asks the namespace. */
class UHOLPlayerOrdersBoardScript : UHOLPlayerOrdersBoard
{
	UHOLCommandSubsystem QueueOfWorld()
	{
		return HOLCommandScript::GetCommands();
	}

	UFUNCTION(BlueprintOverride)
	int IsAcceptedAnswer(int PlayerIndex, int NumUnits)
	{
		return HOLPlayerOrdersBoard::AnswerIsAccepted(PlayerIndex, NumUnits);
	}

	UFUNCTION(BlueprintOverride)
	int MoveAnswer(int PlayerIndex, const TArray<int>& UnitIds, FVector Target, bool bQueue, FVector2D FrontLine, bool bFrontWidth)
	{
		return HOLPlayerOrdersBoard::AnswerMove(QueueOfWorld(), PlayerIndex, UnitIds, Target, bQueue, FrontLine, bFrontWidth);
	}

	UFUNCTION(BlueprintOverride)
	int AttackAnswer(int PlayerIndex, const TArray<int>& UnitIds, int TargetUnitId, bool bQueue)
	{
		return HOLPlayerOrdersBoard::AnswerAttack(QueueOfWorld(), PlayerIndex, UnitIds, TargetUnitId, bQueue);
	}

	UFUNCTION(BlueprintOverride)
	int AttackBuildingAnswer(int PlayerIndex, const TArray<int>& UnitIds, AHOLBuilding Building, bool bQueue)
	{
		return HOLPlayerOrdersBoard::AnswerAttackBuilding(QueueOfWorld(), PlayerIndex, UnitIds, Building, bQueue);
	}

	UFUNCTION(BlueprintOverride)
	int AttackMoveAnswer(int PlayerIndex, const TArray<int>& UnitIds, FVector Target, bool bQueue)
	{
		return HOLPlayerOrdersBoard::AnswerAttackMove(QueueOfWorld(), PlayerIndex, UnitIds, Target, bQueue);
	}

	UFUNCTION(BlueprintOverride)
	int StopAnswer(int PlayerIndex, const TArray<int>& UnitIds, bool bHold)
	{
		return HOLPlayerOrdersBoard::AnswerStop(QueueOfWorld(), PlayerIndex, UnitIds, bHold);
	}

	UFUNCTION(BlueprintOverride)
	int ToggleDeployAnswer(int PlayerIndex, const TArray<int>& UnitIds)
	{
		return HOLPlayerOrdersBoard::AnswerToggleDeploy(QueueOfWorld(), PlayerIndex, UnitIds);
	}

	UFUNCTION(BlueprintOverride)
	int GatherAnswer(int PlayerIndex, const TArray<int>& UnitIds, int SourceIndex, bool bQueue, bool bSource, FVector SourceLocation)
	{
		return HOLPlayerOrdersBoard::AnswerGather(QueueOfWorld(), PlayerIndex, UnitIds, SourceIndex, bQueue, bSource, SourceLocation);
	}

	UFUNCTION(BlueprintOverride)
	int BuildAnswer(int PlayerIndex, const TArray<int>& UnitIds, AHOLBuilding Building, bool bQueue)
	{
		return HOLPlayerOrdersBoard::AnswerBuild(QueueOfWorld(), PlayerIndex, UnitIds, Building, bQueue);
	}

	UFUNCTION(BlueprintOverride)
	int DropOffAnswer(int PlayerIndex, const TArray<int>& UnitIds, AHOLBuilding Building, FVector Point, bool bQueue, bool bDeposit)
	{
		return HOLPlayerOrdersBoard::AnswerDropOff(QueueOfWorld(), PlayerIndex, UnitIds, Building, Point, bQueue, bDeposit);
	}

	UFUNCTION(BlueprintOverride)
	int PlaceAnswer(int PlayerIndex, FName TypeName, FVector Point, const TArray<int>& UnitIds, bool bQueue)
	{
		return HOLPlayerOrdersBoard::AnswerPlace(QueueOfWorld(), PlayerIndex, TypeName, Point, UnitIds, bQueue);
	}

	UFUNCTION(BlueprintOverride)
	int QueueProductionAnswer(int PlayerIndex, AHOLBuilding Building, FName TypeName)
	{
		return HOLPlayerOrdersBoard::AnswerQueueProduction(QueueOfWorld(), PlayerIndex, Building, TypeName);
	}

	UFUNCTION(BlueprintOverride)
	int MarketTradeAnswer(int PlayerIndex, AHOLBuilding Building, int Resource, int Lots, bool bBuy)
	{
		return HOLPlayerOrdersBoard::AnswerMarketTrade(QueueOfWorld(), PlayerIndex, Building, Resource, Lots, bBuy);
	}

	UFUNCTION(BlueprintOverride)
	int SetRallyAnswer(int PlayerIndex, AHOLBuilding Building, FVector Point, int SourceIndex, bool bSourceStanding, AHOLBuilding Site, bool bQueue)
	{
		return HOLPlayerOrdersBoard::AnswerSetRally(QueueOfWorld(), PlayerIndex, Building, Point, SourceIndex, bSourceStanding, Site, bQueue);
	}

	UFUNCTION(BlueprintOverride)
	int GarrisonAnswer(int PlayerIndex, const TArray<int>& UnitIds, AHOLBuilding Building)
	{
		return HOLPlayerOrdersBoard::AnswerGarrison(QueueOfWorld(), PlayerIndex, UnitIds, Building);
	}

	UFUNCTION(BlueprintOverride)
	int UngarrisonAllAnswer(int PlayerIndex, AHOLBuilding Building)
	{
		return HOLPlayerOrdersBoard::AnswerUngarrisonAll(QueueOfWorld(), PlayerIndex, Building);
	}}
