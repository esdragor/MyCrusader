// Copyright Epic Games, Inc. All Rights Reserved.

#include "Commands/MCCommandSubsystem.h"

#include "Engine/World.h"
#include "MyCrusaderLog.h"
#include "NavigationSystem.h"
#include "Simulation/MCSimulationSettings.h"

bool UMCCommandSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UMCCommandSubsystem::IssueMoveOrder(const TConstArrayView<uint32> UnitIds, const FVector& Target)
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
		const UMCSimulationSettings* Settings = GetDefault<UMCSimulationSettings>();
		const FVector Extent(Settings ? Settings->OrderTargetProjectionExtent : 1000.f);
		FNavLocation Projected;
		if (NavSystem->ProjectPointToNavigation(Target, Projected, Extent))
		{
			Destination = Projected.Location;
		}
		else
		{
			UE_LOG(LogMCSimulation, Warning, TEXT("Move order to %s is not on the navmesh: kept as given (D228)"), *Target.ToString());
		}
	}

	PendingMoveOrders.Reserve(PendingMoveOrders.Num() + UnitIds.Num());
	for (const uint32 UnitId : UnitIds)
	{
		PendingMoveOrders.Add(FMCUnitMoveCommand{ UnitId, Destination });
	}
}

void UMCCommandSubsystem::ConsumeMoveOrders(TArray<FMCUnitMoveCommand>& OutCommands)
{
	OutCommands.Reset();
	Swap(OutCommands, PendingMoveOrders);
}
