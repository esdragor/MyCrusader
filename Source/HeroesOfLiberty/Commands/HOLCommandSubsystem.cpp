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

void UHOLCommandSubsystem::IssueMoveOrder(const TConstArrayView<uint32> UnitIds, const FVector& Target)
{
	if (UnitIds.Num() == 0)
	{
		return;
	}

	// Destination brought back to the navigable area (D228): one point query per order, not per unit, the way
	// a real time strategy game keeps a click next to the map walkable instead of walking into the void.
	FVector Destination = Target;
	UWorld* World = GetWorld();
	if (UNavigationSystemV1* NavSystem = World ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr)
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
