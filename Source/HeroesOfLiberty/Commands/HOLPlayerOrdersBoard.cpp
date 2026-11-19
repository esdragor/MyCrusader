// Copyright Epic Games, Inc. All Rights Reserved.

#include "Commands/HOLPlayerOrdersBoard.h"

#include "Engine/Engine.h"
#include "Engine/World.h"

UHOLPlayerOrdersBoard* UHOLPlayerOrdersBoard::Find(const UWorld* World)
{
	if (!World)
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
