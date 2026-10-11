// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Commands/MCUnitCommand.h"
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MCCommandSubsystem.generated.h"

/**
 * The order queue (D216, D226): orders are queued here and applied by the order processor at the next fixed
 * step, never in the frame that issued them. The destination is brought back to the navmesh when the order is
 * issued (D228), so a point outside the navigable area cannot send a unit off the navmesh.
 */
UCLASS()
class MYCRUSADER_API UMCCommandSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Queues "go to this location" for the given units. */
	void IssueMoveOrder(TConstArrayView<uint32> UnitIds, const FVector& Target);

	/** Takes the orders waiting for the simulation (the order processor calls this at each step). */
	void ConsumeMoveOrders(TArray<FMCUnitMoveCommand>& OutCommands);

	int32 GetNumPendingOrders() const { return PendingMoveOrders.Num(); }

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	TArray<FMCUnitMoveCommand> PendingMoveOrders;
};
