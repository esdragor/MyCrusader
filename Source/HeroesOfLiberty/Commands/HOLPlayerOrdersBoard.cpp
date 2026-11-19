// Copyright Epic Games, Inc. All Rights Reserved.

#include "Commands/HOLPlayerOrdersBoard.h"

#include "Engine/Engine.h"
#include "Engine/World.h"

UHOLPlayerOrdersBoard* UHOLPlayerOrdersBoard::Find(const UWorld* World)
{
	// Game thread only: the script runs there; an order asked from another thread answers -1 (HOLPlayerOrders.cpp refuses it).
	if (!World || !IsInGameThread())
	{
		return nullptr;
	}
	const TArray<UHOLPlayerOrdersBoard*> Boards = World->GetSubsystemArrayCopy<UHOLPlayerOrdersBoard>();
	return Boards.Num() > 0 ? Boards[0] : nullptr;
}

UHOLPlayerOrdersBoard* UHOLPlayerOrdersBoard::FindAny()
{
	if (!GEngine || !IsInGameThread())
	{
		return nullptr;
	}
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (UHOLPlayerOrdersBoard* Board = Find(Context.World()))
		{
			return Board;
		}
	}
	return nullptr;
}

int32 UHOLPlayerOrdersBoard::AskIsAccepted(const UWorld* World, const int32 PlayerIndex, const int32 NumUnits)
{
	UHOLPlayerOrdersBoard* Board = Find(World);
	return Board ? Board->IsAcceptedAnswer(PlayerIndex, NumUnits) : -1;
}

int32 UHOLPlayerOrdersBoard::AskIsAcceptedAnywhere(const int32 PlayerIndex, const int32 NumUnits)
{
	UHOLPlayerOrdersBoard* Board = FindAny();
	return Board ? Board->IsAcceptedAnswer(PlayerIndex, NumUnits) : -1;
}

int32 UHOLPlayerOrdersBoard::AskMove(const UWorld* World, const int32 PlayerIndex, const TArray<int32>& UnitIds, const FVector& Target, const bool bQueue,
	const FVector2D& FrontLine, const bool bFrontWidth)
{
	UHOLPlayerOrdersBoard* Board = Find(World);
	return Board ? Board->MoveAnswer(PlayerIndex, UnitIds, Target, bQueue, FrontLine, bFrontWidth) : -1;
}

int32 UHOLPlayerOrdersBoard::AskAttack(const UWorld* World, const int32 PlayerIndex, const TArray<int32>& UnitIds, const int32 TargetUnitId, const bool bQueue)
{
	UHOLPlayerOrdersBoard* Board = Find(World);
	return Board ? Board->AttackAnswer(PlayerIndex, UnitIds, TargetUnitId, bQueue) : -1;
}

int32 UHOLPlayerOrdersBoard::AskAttackBuilding(const UWorld* World, const int32 PlayerIndex, const TArray<int32>& UnitIds, AHOLBuilding* Building,
	const bool bQueue)
{
	UHOLPlayerOrdersBoard* Board = Find(World);
	return Board ? Board->AttackBuildingAnswer(PlayerIndex, UnitIds, Building, bQueue) : -1;
}

int32 UHOLPlayerOrdersBoard::AskAttackMove(const UWorld* World, const int32 PlayerIndex, const TArray<int32>& UnitIds, const FVector& Target, const bool bQueue)
{
	UHOLPlayerOrdersBoard* Board = Find(World);
	return Board ? Board->AttackMoveAnswer(PlayerIndex, UnitIds, Target, bQueue) : -1;
}

int32 UHOLPlayerOrdersBoard::AskStop(const UWorld* World, const int32 PlayerIndex, const TArray<int32>& UnitIds, const bool bHold)
{
	UHOLPlayerOrdersBoard* Board = Find(World);
	return Board ? Board->StopAnswer(PlayerIndex, UnitIds, bHold) : -1;
}

int32 UHOLPlayerOrdersBoard::AskToggleDeploy(const UWorld* World, const int32 PlayerIndex, const TArray<int32>& UnitIds)
{
	UHOLPlayerOrdersBoard* Board = Find(World);
	return Board ? Board->ToggleDeployAnswer(PlayerIndex, UnitIds) : -1;
}

int32 UHOLPlayerOrdersBoard::AskGather(const UWorld* World, int32 PlayerIndex, const TArray<int32>& UnitIds, int32 SourceIndex, bool bQueue, bool bSource, FVector SourceLocation)
{
	UHOLPlayerOrdersBoard* Board = Find(World);
	return Board ? Board->GatherAnswer(PlayerIndex, UnitIds, SourceIndex, bQueue, bSource, SourceLocation) : -1;
}

int32 UHOLPlayerOrdersBoard::AskBuild(const UWorld* World, int32 PlayerIndex, const TArray<int32>& UnitIds, AHOLBuilding* Building, bool bQueue)
{
	UHOLPlayerOrdersBoard* Board = Find(World);
	return Board ? Board->BuildAnswer(PlayerIndex, UnitIds, Building, bQueue) : -1;
}

int32 UHOLPlayerOrdersBoard::AskDropOff(const UWorld* World, int32 PlayerIndex, const TArray<int32>& UnitIds, AHOLBuilding* Building, FVector Point, bool bQueue, bool bDeposit)
{
	UHOLPlayerOrdersBoard* Board = Find(World);
	return Board ? Board->DropOffAnswer(PlayerIndex, UnitIds, Building, Point, bQueue, bDeposit) : -1;
}

int32 UHOLPlayerOrdersBoard::AskPlace(const UWorld* World, int32 PlayerIndex, FName TypeName, FVector Point, const TArray<int32>& UnitIds, bool bQueue)
{
	UHOLPlayerOrdersBoard* Board = Find(World);
	return Board ? Board->PlaceAnswer(PlayerIndex, TypeName, Point, UnitIds, bQueue) : -1;
}

int32 UHOLPlayerOrdersBoard::AskQueueProduction(const UWorld* World, int32 PlayerIndex, AHOLBuilding* Building, FName TypeName)
{
	UHOLPlayerOrdersBoard* Board = Find(World);
	return Board ? Board->QueueProductionAnswer(PlayerIndex, Building, TypeName) : -1;
}

int32 UHOLPlayerOrdersBoard::AskMarketTrade(const UWorld* World, int32 PlayerIndex, AHOLBuilding* Building, int32 Resource, int32 Lots, bool bBuy)
{
	UHOLPlayerOrdersBoard* Board = Find(World);
	return Board ? Board->MarketTradeAnswer(PlayerIndex, Building, Resource, Lots, bBuy) : -1;
}

int32 UHOLPlayerOrdersBoard::AskSetRally(const UWorld* World, int32 PlayerIndex, AHOLBuilding* Building, FVector Point, int32 SourceIndex, bool bSourceStanding, AHOLBuilding* Site, bool bQueue)
{
	UHOLPlayerOrdersBoard* Board = Find(World);
	return Board ? Board->SetRallyAnswer(PlayerIndex, Building, Point, SourceIndex, bSourceStanding, Site, bQueue) : -1;
}

int32 UHOLPlayerOrdersBoard::AskGarrison(const UWorld* World, int32 PlayerIndex, const TArray<int32>& UnitIds, AHOLBuilding* Building)
{
	UHOLPlayerOrdersBoard* Board = Find(World);
	return Board ? Board->GarrisonAnswer(PlayerIndex, UnitIds, Building) : -1;
}

int32 UHOLPlayerOrdersBoard::AskUngarrisonAll(const UWorld* World, int32 PlayerIndex, AHOLBuilding* Building)
{
	UHOLPlayerOrdersBoard* Board = Find(World);
	return Board ? Board->UngarrisonAllAnswer(PlayerIndex, Building) : -1;
}

bool UHOLPlayerOrdersBoard::ShouldCreateSubsystem(UObject* Outer) const
{
	// Never the abstract C++ class, nor a script class replaced by a reload (as UHOLMatchFlowSubsystem and UHOLConstructionBoard).
	if (IsUnreachable() || GetClass()->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists))
	{
		return false;
	}
	return Super::ShouldCreateSubsystem(Outer);
}

bool UHOLPlayerOrdersBoard::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

int32 UHOLPlayerOrdersBoard::IsAcceptedAnswer_Implementation(const int32, const int32) { return -1; }
int32 UHOLPlayerOrdersBoard::MoveAnswer_Implementation(const int32, const TArray<int32>&, const FVector, const bool, const FVector2D, const bool) { return -1; }
int32 UHOLPlayerOrdersBoard::AttackAnswer_Implementation(const int32, const TArray<int32>&, const int32, const bool) { return -1; }
int32 UHOLPlayerOrdersBoard::AttackBuildingAnswer_Implementation(const int32, const TArray<int32>&, AHOLBuilding*, const bool) { return -1; }
int32 UHOLPlayerOrdersBoard::AttackMoveAnswer_Implementation(const int32, const TArray<int32>&, const FVector, const bool) { return -1; }
int32 UHOLPlayerOrdersBoard::StopAnswer_Implementation(const int32, const TArray<int32>&, const bool) { return -1; }
int32 UHOLPlayerOrdersBoard::ToggleDeployAnswer_Implementation(const int32, const TArray<int32>&) { return -1; }
int32 UHOLPlayerOrdersBoard::GatherAnswer_Implementation(int32, const TArray<int32>&, int32, bool, bool, FVector) { return -1; }
int32 UHOLPlayerOrdersBoard::BuildAnswer_Implementation(int32, const TArray<int32>&, AHOLBuilding*, bool) { return -1; }
int32 UHOLPlayerOrdersBoard::DropOffAnswer_Implementation(int32, const TArray<int32>&, AHOLBuilding*, FVector, bool, bool) { return -1; }
int32 UHOLPlayerOrdersBoard::PlaceAnswer_Implementation(int32, FName, FVector, const TArray<int32>&, bool) { return -1; }
int32 UHOLPlayerOrdersBoard::QueueProductionAnswer_Implementation(int32, AHOLBuilding*, FName) { return -1; }
int32 UHOLPlayerOrdersBoard::MarketTradeAnswer_Implementation(int32, AHOLBuilding*, int32, int32, bool) { return -1; }
int32 UHOLPlayerOrdersBoard::SetRallyAnswer_Implementation(int32, AHOLBuilding*, FVector, int32, bool, AHOLBuilding*, bool) { return -1; }
int32 UHOLPlayerOrdersBoard::GarrisonAnswer_Implementation(int32, const TArray<int32>&, AHOLBuilding*) { return -1; }
int32 UHOLPlayerOrdersBoard::UngarrisonAllAnswer_Implementation(int32, AHOLBuilding*) { return -1; }
