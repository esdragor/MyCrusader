// Copyright Epic Games, Inc. All Rights Reserved.

#include "Commands/HOLPlayerOrders.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
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

	void SendNonWorkersTo(UWorld& World, UHOLCommandSubsystem& Commands, const int32 PlayerIndex, TArray<uint32>& InOutUnitIds, const FVector& Point,
		const bool bQueue)
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
			UE_LOG(LogHOLMatch, Display, TEXT("Order of slot %d: %d units that do not work walk to (%.0f, %.0f)%s"), PlayerIndex, Walkers.Num(), Point.X, Point.Y,
				bQueue ? TEXT(", queued") : TEXT(""));
			Commands.IssueMoveOrder(Walkers, Point, PlayerIndex, /*bFromTask*/ false, FVector2D::ZeroVector, false, bQueue);
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
		const bool bSource = Sources && Sources->GetSources().IsValidIndex(SourceIndex);
		if (bSource)
		{
			// With Shift, the soldiers' walk at the end of their queue (L.6, as a deposit's, L.3).
			SendNonWorkersTo(World, *Commands, PlayerIndex, Workers, Sources->GetSources()[SourceIndex].Location, bQueue);
		}
		if (!Workers.IsEmpty())
		{
			if (bQueue && bSource)
			{
				// EtapeDefini L.6 (the user's 2nd trial: Shift + Pa then Shift + a tree went to the tree at once, Pa dropped):
				// through the queue of moves, so that it waits for their walks; sorted there by each worker's queue of work.
				Commands->IssueQueuedWorkOrder(Workers, SourceIndex, nullptr, Sources->GetSources()[SourceIndex].Location, PlayerIndex);
			}
			else
			{
				Commands->IssueGatherOrder(Workers, SourceIndex, PlayerIndex, bQueue);
			}
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
		SendNonWorkersTo(World, *Commands, PlayerIndex, Workers, Building->GetActorLocation(), bQueue);
		if (!Workers.IsEmpty() && bQueue)
		{
			// EtapeDefini L.6, as a source with Shift: through the queue of moves (Shift + Pb then Shift + a site built it at once).
			Commands->IssueQueuedWorkOrder(Workers, INDEX_NONE, Building, Building->GetActorLocation(), PlayerIndex);
		}
		else if (!Workers.IsEmpty())
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

	bool DropOff(UWorld& World, const int32 PlayerIndex, const TConstArrayView<uint32> UnitIds, AHOLBuilding* Building, const FVector& Point,
		const bool bQueue)
	{
		UHOLCommandSubsystem* Commands = GetQueue(World, PlayerIndex, UnitIds.Num());
		if (!Commands)
		{
			return false;
		}
		const UHOLCommandSubsystem::FPlayerOrderScope PlayerOrder(*Commands);
		const FHOLBuildingTypeRow* Type = Building ? Building->GetType() : nullptr;
		const bool bDeposit = Type && !Type->Accepts.IsEmpty() && Building->TakesDeliveries() && Building->GetOwnerPlayer() == PlayerIndex;
		if (!bDeposit)
		{
			// Not one of its deposits: all walk to the point.
			Commands->IssueMoveOrder(UnitIds, Point, PlayerIndex, /*bFromTask*/ false, FVector2D::ZeroVector, false, bQueue);
			return true;
		}
		// D485 (EtapeDefini L.5b): through the queue of moves, so that a new order forgets the units' queue as any other, and
		// Shift waits for their walks; sorted there by each unit's load (UHOLUnitOrderProcessor): the carriers unload, the
		// others walk to the point. With Shift, the soldiers walk to the point after their own queue (L.3).
		TArray<uint32> Workers(UnitIds);
		if (bQueue)
		{
			SendNonWorkersTo(World, *Commands, PlayerIndex, Workers, Point, /*bQueue*/ true);
		}
		if (!Workers.IsEmpty())
		{
			UE_LOG(LogHOLMatch, Display, TEXT("Drop-off order of slot %d: %d units to %s (%.0f, %.0f)%s"), PlayerIndex, Workers.Num(),
				*Building->GetTypeName().ToString(), Building->GetActorLocation().X, Building->GetActorLocation().Y, bQueue ? TEXT(", queued") : TEXT(""));
			Commands->IssueDropOffMoveOrder(Workers, Building, Point, PlayerIndex, bQueue);
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

	bool QueueProduction(UWorld& World, const int32 PlayerIndex, AHOLBuilding* Building, const FName TypeName)
	{
		UHOLCommandSubsystem* Commands = GetQueue(World, PlayerIndex, 0);
		if (!Commands || !Building)
		{
			return false;
		}
		// As AHOLPlayerController::ServerQueueProduction: the production checks the owner, the type, the queue and the stock.
		FHOLProductionCommand Command;
		Command.Kind = FHOLProductionCommand::EKind::Queue;
		Command.PlayerIndex = PlayerIndex;
		Command.Building = Building;
		Command.TypeName = TypeName;
		Commands->IssueProductionOrder(Command);
		return true;
	}

	bool SetRally(UWorld& World, const int32 PlayerIndex, AHOLBuilding* Building, const FVector& Point, const int32 SourceIndex, AHOLBuilding* Site,
		const bool bQueue)
	{
		UHOLCommandSubsystem* Commands = GetQueue(World, PlayerIndex, 0);
		if (!Commands || !Building)
		{
			return false;
		}
		FHOLProductionCommand Command;
		Command.Kind = FHOLProductionCommand::EKind::Rally;
		Command.PlayerIndex = PlayerIndex;
		Command.Building = Building;
		Command.Location = Point;
		// A source that is not standing (any more) is the ground (as the controller's RPC).
		const UHOLSourceSubsystem* Sources = World.GetSubsystem<UHOLSourceSubsystem>();
		Command.SourceIndex = Sources && Sources->GetSources().IsValidIndex(SourceIndex) && Sources->GetSources()[SourceIndex].IsStanding()
			? SourceIndex : INDEX_NONE;
		Command.RallySite = Site;
		Command.bQueue = bQueue;
		Commands->IssueProductionOrder(Command);
		return true;
	}
}
