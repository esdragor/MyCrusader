// Copyright Epic Games, Inc. All Rights Reserved.

#include "Commands/HOLCommandScriptLibrary.h"

#include "Buildings/HOLBuilding.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLMatchConfig.h"

namespace
{
	TArray<uint32> ToUnitIds(const TArray<int32>& Ids)
	{
		TArray<uint32> Out;
		Out.Reserve(Ids.Num());
		for (const int32 Id : Ids)
		{
			Out.Add(static_cast<uint32>(Id));
		}
		return Out;
	}

	TArray<int32> FromUnitIds(const TArray<uint32>& Ids)
	{
		TArray<int32> Out;
		Out.Reserve(Ids.Num());
		for (const uint32 Id : Ids)
		{
			Out.Add(static_cast<int32>(Id));
		}
		return Out;
	}

	FName KindName(const EHOLOrderKind Kind)
	{
		switch (Kind)
		{
		case EHOLOrderKind::Move: return TEXT("Move");
		case EHOLOrderKind::Attack: return TEXT("Attack");
		case EHOLOrderKind::AttackMove: return TEXT("AttackMove");
		case EHOLOrderKind::Stop: return TEXT("Stop");
		case EHOLOrderKind::Hold: return TEXT("Hold");
		case EHOLOrderKind::AttackBuilding: return TEXT("AttackBuilding");
		case EHOLOrderKind::DropOff: return TEXT("DropOff");
		case EHOLOrderKind::Gather: return TEXT("Gather");
		case EHOLOrderKind::Build: return TEXT("Build");
		case EHOLOrderKind::ToggleDeploy: return TEXT("ToggleDeploy");
		default: return TEXT("Other");
		}
	}

	FHOLScriptOrderRead MakeRead(const FName Kind, const int32 PlayerIndex, const TArray<uint32>& UnitIds)
	{
		FHOLScriptOrderRead Read;
		Read.Kind = Kind;
		Read.PlayerIndex = PlayerIndex;
		Read.UnitIds = FromUnitIds(UnitIds);
		return Read;
	}
}

UHOLCommandSubsystem* UHOLCommandScriptLibrary::GetCommands(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	return World ? World->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
}

UHOLCommandSubsystem* UHOLCommandScriptLibrary::MakeDetachedForTest()
{
	return NewObject<UHOLCommandSubsystem>(GetTransientPackage());
}

void UHOLCommandScriptLibrary::IssueMove(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, const FVector& Target, const int32 PlayerIndex, const bool bQueue, const bool bFromTask)
{
	if (Commands)
	{
		Commands->IssueMoveOrder(ToUnitIds(UnitIds), Target, PlayerIndex, bFromTask, FVector2D::ZeroVector, false, bQueue);
	}
}

void UHOLCommandScriptLibrary::IssueAttack(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, const int32 TargetUnitId, const int32 PlayerIndex, const bool bQueue)
{
	if (Commands)
	{
		Commands->IssueCombatOrder(ToUnitIds(UnitIds), EHOLOrderKind::Attack, static_cast<uint32>(TargetUnitId), PlayerIndex, bQueue);
	}
}

void UHOLCommandScriptLibrary::IssueAttackMove(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, const FVector& Target, const int32 PlayerIndex, const bool bQueue)
{
	if (Commands)
	{
		Commands->IssueAttackMoveOrder(ToUnitIds(UnitIds), Target, PlayerIndex, bQueue);
	}
}

void UHOLCommandScriptLibrary::IssueStop(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, const int32 PlayerIndex)
{
	if (Commands)
	{
		Commands->IssueCombatOrder(ToUnitIds(UnitIds), EHOLOrderKind::Stop, 0, PlayerIndex);
	}
}

void UHOLCommandScriptLibrary::IssueHold(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, const int32 PlayerIndex)
{
	if (Commands)
	{
		Commands->IssueCombatOrder(ToUnitIds(UnitIds), EHOLOrderKind::Hold, 0, PlayerIndex);
	}
}

void UHOLCommandScriptLibrary::IssueAttackBuilding(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, AHOLBuilding* Building, const int32 PlayerIndex, const bool bQueue)
{
	if (Commands)
	{
		Commands->IssueAttackBuildingOrder(ToUnitIds(UnitIds), Building, PlayerIndex, bQueue);
	}
}

void UHOLCommandScriptLibrary::IssueDropOffMove(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, AHOLBuilding* Building, const FVector& Point, const int32 PlayerIndex, const bool bQueue)
{
	if (Commands)
	{
		Commands->IssueDropOffMoveOrder(ToUnitIds(UnitIds), Building, Point, PlayerIndex, bQueue);
	}
}

void UHOLCommandScriptLibrary::IssueQueuedWork(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, const int32 SourceIndex, AHOLBuilding* Building, const FVector& Location, const int32 PlayerIndex)
{
	if (Commands)
	{
		Commands->IssueQueuedWorkOrder(ToUnitIds(UnitIds), SourceIndex, Building, Location, PlayerIndex);
	}
}

void UHOLCommandScriptLibrary::IssueMovePlayer(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, const FVector& Target, const int32 PlayerIndex, const bool bQueue, const FVector2D& FrontLine, const bool bFrontWidth)
{
	if (Commands)
	{
		const UHOLCommandSubsystem::FPlayerOrderScope PlayerOrder(*Commands);
		Commands->IssueMoveOrder(ToUnitIds(UnitIds), Target, PlayerIndex, /*bFromTask*/ false, FrontLine, bFrontWidth, bQueue);
	}
}

void UHOLCommandScriptLibrary::IssueAttackPlayer(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, const int32 TargetUnitId, const int32 PlayerIndex, const bool bQueue)
{
	if (Commands)
	{
		const UHOLCommandSubsystem::FPlayerOrderScope PlayerOrder(*Commands);
		Commands->IssueCombatOrder(ToUnitIds(UnitIds), EHOLOrderKind::Attack, static_cast<uint32>(TargetUnitId), PlayerIndex, bQueue);
	}
}

void UHOLCommandScriptLibrary::IssueAttackMovePlayer(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, const FVector& Target, const int32 PlayerIndex, const bool bQueue)
{
	if (Commands)
	{
		const UHOLCommandSubsystem::FPlayerOrderScope PlayerOrder(*Commands);
		Commands->IssueAttackMoveOrder(ToUnitIds(UnitIds), Target, PlayerIndex, bQueue);
	}
}

void UHOLCommandScriptLibrary::IssueAttackBuildingPlayer(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, AHOLBuilding* Building, const int32 PlayerIndex, const bool bQueue)
{
	if (Commands)
	{
		const UHOLCommandSubsystem::FPlayerOrderScope PlayerOrder(*Commands);
		Commands->IssueAttackBuildingOrder(ToUnitIds(UnitIds), Building, PlayerIndex, bQueue);
	}
}

void UHOLCommandScriptLibrary::IssueStopPlayer(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, const int32 PlayerIndex, const bool bHold)
{
	if (Commands)
	{
		const UHOLCommandSubsystem::FPlayerOrderScope PlayerOrder(*Commands);
		Commands->IssueCombatOrder(ToUnitIds(UnitIds), bHold ? EHOLOrderKind::Hold : EHOLOrderKind::Stop, 0, PlayerIndex);
	}
}

void UHOLCommandScriptLibrary::IssueToggleDeployPlayer(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, const int32 PlayerIndex)
{
	if (Commands)
	{
		const UHOLCommandSubsystem::FPlayerOrderScope PlayerOrder(*Commands);
		Commands->IssueCombatOrder(ToUnitIds(UnitIds), EHOLOrderKind::ToggleDeploy, 0, PlayerIndex);
	}
}

int32 UHOLCommandScriptLibrary::GetMaxSlots()
{
	return FHOLMatchConfig::MaxSlots;
}

void UHOLCommandScriptLibrary::IssueGather(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, const int32 SourceIndex, const int32 PlayerIndex, const bool bQueue)
{
	if (Commands)
	{
		Commands->IssueGatherOrder(ToUnitIds(UnitIds), SourceIndex, PlayerIndex, bQueue);
	}
}

void UHOLCommandScriptLibrary::IssueDropOff(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, AHOLBuilding* Building, const int32 PlayerIndex, const bool bQueue)
{
	if (Commands)
	{
		Commands->IssueDropOffOrder(ToUnitIds(UnitIds), Building, PlayerIndex, bQueue);
	}
}

void UHOLCommandScriptLibrary::IssueBuild(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, AHOLBuilding* Building, const int32 SourceIndex, const int32 PlayerIndex, const bool bQueue)
{
	if (Commands)
	{
		FHOLBuildOrder Order;
		Order.UnitIds = ToUnitIds(UnitIds);
		Order.Building = Building;
		Order.SourceIndex = SourceIndex;
		Order.PlayerIndex = PlayerIndex;
		Order.bQueue = bQueue;
		Commands->IssueBuildOrder(Order);
	}
}

void UHOLCommandScriptLibrary::IssueProductionQueue(UHOLCommandSubsystem* Commands, const int32 PlayerIndex, AHOLBuilding* Building, const FName TypeName, const bool bQueue)
{
	if (Commands)
	{
		FHOLProductionCommand Command;
		Command.Kind = FHOLProductionCommand::EKind::Queue;
		Command.PlayerIndex = PlayerIndex;
		Command.Building = Building;
		Command.TypeName = TypeName;
		Command.bQueue = bQueue;
		Commands->IssueProductionOrder(Command);
	}
}

void UHOLCommandScriptLibrary::IssueProductionCancel(UHOLCommandSubsystem* Commands, const int32 PlayerIndex, AHOLBuilding* Building, const int32 Index)
{
	if (Commands)
	{
		FHOLProductionCommand Command;
		Command.Kind = FHOLProductionCommand::EKind::Cancel;
		Command.PlayerIndex = PlayerIndex;
		Command.Building = Building;
		Command.Index = Index;
		Commands->IssueProductionOrder(Command);
	}
}

void UHOLCommandScriptLibrary::IssueRally(UHOLCommandSubsystem* Commands, const int32 PlayerIndex, AHOLBuilding* Building, const FVector& Location, const int32 SourceIndex, AHOLBuilding* RallySite)
{
	if (Commands)
	{
		FHOLProductionCommand Command;
		Command.Kind = FHOLProductionCommand::EKind::Rally;
		Command.PlayerIndex = PlayerIndex;
		Command.Building = Building;
		Command.Location = Location;
		Command.SourceIndex = SourceIndex;
		Command.RallySite = RallySite;
		Commands->IssueProductionOrder(Command);
	}
}

void UHOLCommandScriptLibrary::IssuePlace(UHOLCommandSubsystem* Commands, const int32 PlayerIndex, const FName TypeName, const FVector& Location, const TArray<int32>& UnitIds, const bool bQueue)
{
	if (Commands)
	{
		FHOLConstructionCommand Command;
		Command.Kind = FHOLConstructionCommand::EKind::Place;
		Command.PlayerIndex = PlayerIndex;
		Command.TypeName = TypeName;
		Command.Location = Location;
		Command.UnitIds = ToUnitIds(UnitIds);
		Command.bQueue = bQueue;
		Commands->IssueConstructionOrder(Command);
	}
}

void UHOLCommandScriptLibrary::IssueConstructionCancel(UHOLCommandSubsystem* Commands, const int32 PlayerIndex, AHOLBuilding* Building)
{
	if (Commands)
	{
		FHOLConstructionCommand Command;
		Command.Kind = FHOLConstructionCommand::EKind::Cancel;
		Command.PlayerIndex = PlayerIndex;
		Command.Building = Building;
		Commands->IssueConstructionOrder(Command);
	}
}

void UHOLCommandScriptLibrary::IssueGarrison(UHOLCommandSubsystem* Commands, const int32 PlayerIndex, const TArray<int32>& UnitIds, AHOLBuilding* Building)
{
	if (Commands)
	{
		Commands->IssueGarrisonOrder(PlayerIndex, ToUnitIds(UnitIds), Building);
	}
}

void UHOLCommandScriptLibrary::IssueUngarrisonAll(UHOLCommandSubsystem* Commands, const int32 PlayerIndex, AHOLBuilding* Building)
{
	if (Commands)
	{
		Commands->IssueUngarrisonAllOrder(PlayerIndex, Building);
	}
}

void UHOLCommandScriptLibrary::IssueRage(UHOLCommandSubsystem* Commands, const int32 PlayerIndex, const TArray<int32>& UnitIds)
{
	if (Commands)
	{
		Commands->IssueRageOrder(PlayerIndex, ToUnitIds(UnitIds));
	}
}

int32 UHOLCommandScriptLibrary::GetNumPendingMoveOrders(const UHOLCommandSubsystem* Commands)
{
	return Commands ? Commands->GetNumPendingOrders() : 0;
}

TArray<FHOLScriptOrderRead> UHOLCommandScriptLibrary::ConsumeMoves(UHOLCommandSubsystem* Commands)
{
	TArray<FHOLScriptOrderRead> Out;
	TArray<FHOLMoveOrder> Orders;
	if (Commands)
	{
		Commands->ConsumeMoveOrders(Orders);
	}
	for (const FHOLMoveOrder& Order : Orders)
	{
		FHOLScriptOrderRead& Read = Out.Add_GetRef(MakeRead(KindName(Order.Kind), Order.PlayerIndex, Order.UnitIds));
		Read.Target = Order.Target;
		Read.TargetUnitId = static_cast<int32>(Order.TargetUnitId);
		Read.SourceIndex = Order.SourceIndex;
		Read.bQueue = Order.bQueue;
		Read.bFromTask = Order.bFromTask;
		Read.bChain = Order.bChain;
		Read.bHasBuilding = Order.TargetBuilding.IsValid();
	}
	return Out;
}

TArray<FHOLScriptOrderRead> UHOLCommandScriptLibrary::ConsumeGathers(UHOLCommandSubsystem* Commands)
{
	TArray<FHOLScriptOrderRead> Out;
	TArray<FHOLGatherOrder> Orders;
	if (Commands)
	{
		Commands->ConsumeGatherOrders(Orders);
	}
	for (const FHOLGatherOrder& Order : Orders)
	{
		FHOLScriptOrderRead& Read = Out.Add_GetRef(MakeRead(TEXT("Gather"), Order.PlayerIndex, Order.UnitIds));
		Read.SourceIndex = Order.SourceIndex;
		Read.bHasBuilding = Order.Deposit.IsValid();
	}
	return Out;
}

TArray<FHOLScriptOrderRead> UHOLCommandScriptLibrary::ConsumeBuilds(UHOLCommandSubsystem* Commands)
{
	TArray<FHOLScriptOrderRead> Out;
	TArray<FHOLBuildOrder> Orders;
	if (Commands)
	{
		Commands->ConsumeBuildOrders(Orders);
	}
	for (const FHOLBuildOrder& Order : Orders)
	{
		FHOLScriptOrderRead& Read = Out.Add_GetRef(MakeRead(TEXT("Build"), Order.PlayerIndex, Order.UnitIds));
		Read.SourceIndex = Order.SourceIndex;
		Read.bQueue = Order.bQueue;
		Read.bHasBuilding = Order.Building.IsValid();
	}
	return Out;
}

TArray<FHOLScriptOrderRead> UHOLCommandScriptLibrary::ConsumeProductions(UHOLCommandSubsystem* Commands)
{
	TArray<FHOLScriptOrderRead> Out;
	TArray<FHOLProductionCommand> Orders;
	if (Commands)
	{
		Commands->ConsumeProductionOrders(Orders);
	}
	for (const FHOLProductionCommand& Order : Orders)
	{
		const TCHAR* Name = TEXT("Queue");
		switch (Order.Kind)
		{
		case FHOLProductionCommand::EKind::Cancel: Name = TEXT("Cancel"); break;
		case FHOLProductionCommand::EKind::Rally: Name = TEXT("Rally"); break;
		case FHOLProductionCommand::EKind::Trade: Name = TEXT("Trade"); break;
		case FHOLProductionCommand::EKind::Tribute: Name = TEXT("Tribute"); break;
		default: break;
		}
		FHOLScriptOrderRead& Read = Out.Add_GetRef(MakeRead(Name, Order.PlayerIndex, TArray<uint32>()));
		Read.Target = Order.Location;
		Read.SourceIndex = Order.SourceIndex;
		Read.bQueue = Order.bQueue;
		Read.bHasBuilding = Order.Building.IsValid() || Order.RallySite.IsValid();
		Read.TypeName = Order.TypeName;
		Read.Index = Order.Index;
	}
	return Out;
}

TArray<FHOLScriptOrderRead> UHOLCommandScriptLibrary::ConsumeConstructions(UHOLCommandSubsystem* Commands)
{
	TArray<FHOLScriptOrderRead> Out;
	TArray<FHOLConstructionCommand> Orders;
	if (Commands)
	{
		Commands->ConsumeConstructionOrders(Orders);
	}
	for (const FHOLConstructionCommand& Order : Orders)
	{
		FHOLScriptOrderRead& Read = Out.Add_GetRef(MakeRead(Order.Kind == FHOLConstructionCommand::EKind::Place ? TEXT("Place") : TEXT("Cancel"), Order.PlayerIndex, Order.UnitIds));
		Read.Target = Order.Location;
		Read.bQueue = Order.bQueue;
		Read.bHasBuilding = Order.Building.IsValid();
		Read.TypeName = Order.TypeName;
	}
	return Out;
}

TArray<FHOLScriptOrderRead> UHOLCommandScriptLibrary::ConsumeGarrisons(UHOLCommandSubsystem* Commands)
{
	TArray<FHOLScriptOrderRead> Out;
	TArray<FHOLGarrisonOrder> Orders;
	if (Commands)
	{
		Commands->ConsumeGarrisonOrders(Orders);
	}
	for (const FHOLGarrisonOrder& Order : Orders)
	{
		FHOLScriptOrderRead& Read = Out.Add_GetRef(MakeRead(Order.bExitAll ? TEXT("UngarrisonAll") : TEXT("Garrison"), Order.PlayerIndex, Order.UnitIds));
		Read.bHasBuilding = Order.Building.IsValid();
	}
	return Out;
}

TArray<FHOLScriptOrderRead> UHOLCommandScriptLibrary::ConsumeRages(UHOLCommandSubsystem* Commands)
{
	TArray<FHOLScriptOrderRead> Out;
	TArray<FHOLRageOrder> Orders;
	if (Commands)
	{
		Commands->ConsumeRageOrders(Orders);
	}
	for (const FHOLRageOrder& Order : Orders)
	{
		Out.Add(MakeRead(TEXT("Rage"), Order.PlayerIndex, Order.UnitIds));
	}
	return Out;
}
