// Copyright Epic Games, Inc. All Rights Reserved.

#include "Commands/HOLPlayerOrders.h"

#include "Buildings/HOLBuilding.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Engine/World.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLMatchConfig.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLUnitView.h"

namespace HOLPlayerOrders
{
	namespace
	{
		/** The queue of World, when an order of PlayerIndex naming NumUnits units may enter it. */
		UHOLCommandSubsystem* GetQueue(UWorld& World, const int32 PlayerIndex, const int32 NumUnits)
		{
			return IsAccepted(PlayerIndex, NumUnits) ? World.GetSubsystem<UHOLCommandSubsystem>() : nullptr;
		}
	}

	bool IsAccepted(const int32 PlayerIndex, const int32 NumUnits)
	{
		return PlayerIndex >= 0 && PlayerIndex < FHOLMatchConfig::MaxSlots && NumUnits >= 0 && NumUnits <= MaxOrderedUnits;
	}

	void SendNonWorkersTo(UWorld& World, UHOLCommandSubsystem& Commands, const int32 PlayerIndex, TArray<uint32>& InOutUnitIds, const FVector& Point)
	{
		TArray<uint32> Walkers;
		for (int32 Index = InOutUnitIds.Num() - 1; Index >= 0; --Index)
		{
			FHOLUnitView View;
			if (HOLUnitView::Find(World, InOutUnitIds[Index], View) && View.OwnerPlayer == PlayerIndex && View.CarryCapacity <= 0)
			{
				Walkers.Add(InOutUnitIds[Index]);
				InOutUnitIds.RemoveAt(Index);
			}
		}
		if (!Walkers.IsEmpty())
		{
			UE_LOG(LogHOLMatch, Display, TEXT("Order of slot %d: %d units that do not work walk to (%.0f, %.0f)"), PlayerIndex, Walkers.Num(), Point.X, Point.Y);
			Commands.IssueMoveOrder(Walkers, Point, PlayerIndex);
		}
	}

	bool Move(UWorld& World, const int32 PlayerIndex, const TConstArrayView<uint32> UnitIds, const FVector& Target, const bool bQueue,
		const FVector2D& FrontLine, const bool bFrontWidth)
	{
		UHOLCommandSubsystem* Commands = GetQueue(World, PlayerIndex, UnitIds.Num());
		if (!Commands)
		{
			return false;
		}
		const UHOLCommandSubsystem::FPlayerOrderScope PlayerOrder(*Commands);
		Commands->IssueMoveOrder(UnitIds, Target, PlayerIndex, /*bFromTask*/ false, FrontLine, bFrontWidth, bQueue);
		return true;
	}

	bool Attack(UWorld& World, const int32 PlayerIndex, const TConstArrayView<uint32> UnitIds, const uint32 TargetUnitId, const bool bQueue)
	{
		UHOLCommandSubsystem* Commands = GetQueue(World, PlayerIndex, UnitIds.Num());
		if (!Commands)
		{
			return false;
		}
		const UHOLCommandSubsystem::FPlayerOrderScope PlayerOrder(*Commands);
		Commands->IssueCombatOrder(UnitIds, EHOLOrderKind::Attack, TargetUnitId, PlayerIndex, bQueue);
		return true;
	}

	bool AttackBuilding(UWorld& World, const int32 PlayerIndex, const TConstArrayView<uint32> UnitIds, AHOLBuilding* Building, const bool bQueue)
	{
		UHOLCommandSubsystem* Commands = Building ? GetQueue(World, PlayerIndex, UnitIds.Num()) : nullptr;
		if (!Commands)
		{
			return false;
		}
		const UHOLCommandSubsystem::FPlayerOrderScope PlayerOrder(*Commands);
		Commands->IssueAttackBuildingOrder(UnitIds, Building, PlayerIndex, bQueue);
		return true;
	}

	bool AttackMove(UWorld& World, const int32 PlayerIndex, const TConstArrayView<uint32> UnitIds, const FVector& Target, const bool bQueue)
	{
		UHOLCommandSubsystem* Commands = GetQueue(World, PlayerIndex, UnitIds.Num());
		if (!Commands)
		{
			return false;
		}
		const UHOLCommandSubsystem::FPlayerOrderScope PlayerOrder(*Commands);
		Commands->IssueAttackMoveOrder(UnitIds, Target, PlayerIndex, bQueue);
		return true;
	}

	bool Stop(UWorld& World, const int32 PlayerIndex, const TConstArrayView<uint32> UnitIds, const bool bHold)
	{
		UHOLCommandSubsystem* Commands = GetQueue(World, PlayerIndex, UnitIds.Num());
		if (!Commands)
		{
			return false;
		}
		const UHOLCommandSubsystem::FPlayerOrderScope PlayerOrder(*Commands);
		Commands->IssueCombatOrder(UnitIds, bHold ? EHOLOrderKind::Hold : EHOLOrderKind::Stop, 0, PlayerIndex);
		return true;
	}

	bool Gather(UWorld& World, const int32 PlayerIndex, const TConstArrayView<uint32> UnitIds, const int32 SourceIndex, const bool bQueue)
	{
		UHOLCommandSubsystem* Commands = GetQueue(World, PlayerIndex, UnitIds.Num());
		if (!Commands)
		{
			return false;
		}
		const UHOLCommandSubsystem::FPlayerOrderScope PlayerOrder(*Commands);
		TArray<uint32> Workers(UnitIds);
		const UHOLSourceSubsystem* Sources = World.GetSubsystem<UHOLSourceSubsystem>();
		if (Sources && Sources->GetSources().IsValidIndex(SourceIndex))
		{
			SendNonWorkersTo(World, *Commands, PlayerIndex, Workers, Sources->GetSources()[SourceIndex].Location);
		}
		if (!Workers.IsEmpty())
		{
			Commands->IssueGatherOrder(Workers, SourceIndex, PlayerIndex, bQueue);
		}
		return true;
	}

	bool Build(UWorld& World, const int32 PlayerIndex, const TConstArrayView<uint32> UnitIds, AHOLBuilding* Building, const bool bQueue)
	{
		UHOLCommandSubsystem* Commands = Building ? GetQueue(World, PlayerIndex, UnitIds.Num()) : nullptr;
		if (!Commands)
		{
			return false;
		}
		const UHOLCommandSubsystem::FPlayerOrderScope PlayerOrder(*Commands);
		TArray<uint32> Workers(UnitIds);
		SendNonWorkersTo(World, *Commands, PlayerIndex, Workers, Building->GetActorLocation());
		if (!Workers.IsEmpty())
		{
			FHOLBuildOrder Order;
			Order.UnitIds = MoveTemp(Workers);
			Order.Building = Building;
			Order.PlayerIndex = PlayerIndex;
			Order.bQueue = bQueue;
			Commands->IssueBuildOrder(Order);
		}
		return true;
	}

	bool Place(UWorld& World, const int32 PlayerIndex, const FName TypeName, const FVector& Point, const TConstArrayView<uint32> UnitIds, const bool bQueue)
	{
		// Too many builders: the foundation is placed all the same, without them.
		UHOLCommandSubsystem* Commands = GetQueue(World, PlayerIndex, 0);
		if (!Commands)
		{
			return false;
		}
		FHOLConstructionCommand Command;
		Command.Kind = FHOLConstructionCommand::EKind::Place;
		Command.PlayerIndex = PlayerIndex;
		Command.TypeName = TypeName;
		Command.Location = Point;
		if (UnitIds.Num() <= MaxOrderedUnits)
		{
			Command.UnitIds = TArray<uint32>(UnitIds);
		}
		Command.bQueue = bQueue;
		Commands->IssueConstructionOrder(Command);
		return true;
	}
}
