// Copyright Epic Games, Inc. All Rights Reserved.

#include "Commands/HOLCommandSubsystem.h"

#include "Engine/World.h"
#include "HeroesOfLibertyLog.h"
#include "NavigationSystem.h"
#include "Simulation/HOLSimulationSettings.h"

bool UHOLCommandSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHOLCommandSubsystem::IssueMoveOrder(const TConstArrayView<uint32> UnitIds, const FVector& Target, const int32 PlayerIndex, const bool bFromTask)
{
	if (UnitIds.Num() == 0)
	{
		return;
	}

	// Destination brought back to the navigable area (D228): one point query per order, not per unit, the way
	// a real time strategy game keeps a click next to the map walkable instead of walking into the void.
	// A task's walk (J5: a gatherer's spot, already on the navmesh) is taken as it is.
	FVector Destination = Target;
	UWorld* World = GetWorld();
	if (UNavigationSystemV1* NavSystem = World && !bFromTask ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr)
	{
		const UHOLSimulationSettings* Settings = GetDefault<UHOLSimulationSettings>();
		const FVector Extent(Settings ? Settings->OrderTargetProjectionExtent : 1000.f);
		FNavLocation Projected;
		if (NavSystem->ProjectPointToNavigation(Target, Projected, Extent))
		{
			Destination = Projected.Location;
		}
		else
		{
			UE_LOG(LogHOLSimulation, Warning, TEXT("Move order to %s is not on the navmesh: kept as given (D228)"), *Target.ToString());
		}
	}

	FHOLMoveOrder& Order = PendingMoveOrders.AddDefaulted_GetRef();
	Order.UnitIds = UnitIds;
	Order.Target = Destination;
	Order.PlayerIndex = PlayerIndex;
	Order.bFromTask = bFromTask;
}

void UHOLCommandSubsystem::IssueGatherOrder(const TConstArrayView<uint32> UnitIds, const int32 SourceIndex, const int32 PlayerIndex, const bool bQueue)
{
	if (UnitIds.Num() == 0 || SourceIndex == INDEX_NONE)
	{
		return;
	}
	if (bQueue)
	{
		// In the workers' queue, among the build orders: kept in the order given (Shift + place, Shift + right click...).
		FHOLBuildOrder& Order = PendingBuildOrders.AddDefaulted_GetRef();
		Order.UnitIds = UnitIds;
		Order.SourceIndex = SourceIndex;
		Order.PlayerIndex = PlayerIndex;
		Order.bQueue = true;
		return;
	}
	FHOLGatherOrder& Order = PendingGatherOrders.AddDefaulted_GetRef();
	Order.UnitIds = UnitIds;
	Order.SourceIndex = SourceIndex;
	Order.PlayerIndex = PlayerIndex;
}

void UHOLCommandSubsystem::ConsumeGatherOrders(TArray<FHOLGatherOrder>& OutOrders)
{
	OutOrders.Reset();
	Swap(OutOrders, PendingGatherOrders);
}

void UHOLCommandSubsystem::ConsumeMoveOrders(TArray<FHOLMoveOrder>& OutOrders)
{
	OutOrders.Reset();
	Swap(OutOrders, PendingMoveOrders);
}

void UHOLCommandSubsystem::IssueProductionOrder(const FHOLProductionCommand& Command)
{
	PendingProductionOrders.Add(Command);
}

void UHOLCommandSubsystem::ConsumeProductionOrders(TArray<FHOLProductionCommand>& OutCommands)
{
	OutCommands.Reset();
	Swap(OutCommands, PendingProductionOrders);
}

void UHOLCommandSubsystem::IssueConstructionOrder(const FHOLConstructionCommand& Command)
{
	PendingConstructionOrders.Add(Command);
}

void UHOLCommandSubsystem::ConsumeConstructionOrders(TArray<FHOLConstructionCommand>& OutCommands)
{
	OutCommands.Reset();
	Swap(OutCommands, PendingConstructionOrders);
}

void UHOLCommandSubsystem::IssueBuildOrder(const FHOLBuildOrder& Order)
{
	if (Order.UnitIds.Num() > 0 && (Order.Building.IsValid() || Order.SourceIndex != INDEX_NONE))
	{
		PendingBuildOrders.Add(Order);
	}
}

void UHOLCommandSubsystem::ConsumeBuildOrders(TArray<FHOLBuildOrder>& OutOrders)
{
	OutOrders.Reset();
	Swap(OutOrders, PendingBuildOrders);
}

void UHOLCommandSubsystem::NotifyNavigationChanged(const FBox& Area)
{
	// Only the simulation consumes them: a client would only pile them up.
	const UWorld* World = GetWorld();
	if (World && World->GetNetMode() != NM_Client && Area.IsValid)
	{
		PendingNavigationChanges.Add(Area);
	}
}

void UHOLCommandSubsystem::ConsumeNavigationChanges(TArray<FBox>& OutAreas)
{
	OutAreas.Reset();
	Swap(OutAreas, PendingNavigationChanges);
}
