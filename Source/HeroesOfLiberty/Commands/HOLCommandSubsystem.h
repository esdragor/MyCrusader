// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Commands/HOLProductionCommand.h"
#include "Commands/HOLUnitCommand.h"
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLCommandSubsystem.generated.h"

/**
 * The order queue (D216, D226): orders are queued here and applied by the order processor at the next fixed
 * step, never in the frame that issued them. The destination is brought back to the navmesh when the order is
 * issued (D228), so a point outside the navigable area cannot send a unit off the navmesh.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLCommandSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Queues "go to this location" for the given units, one order for the group (D286, D287). PlayerIndex: the player
	 * who gave it, whose own units alone obey (D288); INDEX_NONE for the server's own orders (rally point, tests).
	 */
	void IssueMoveOrder(TConstArrayView<uint32> UnitIds, const FVector& Target, int32 PlayerIndex = INDEX_NONE);

	/** Takes the orders waiting for the simulation (the order processor calls this at each step). */
	void ConsumeMoveOrders(TArray<FHOLMoveOrder>& OutOrders);

	int32 GetNumPendingOrders() const { return PendingMoveOrders.Num(); }

	/** Queues a production order (J3, D282), applied by the production at the next step. */
	void IssueProductionOrder(const FHOLProductionCommand& Command);
	/** Takes the production orders waiting for the next step (UHOLProductionSubsystem calls this at each step). */
	void ConsumeProductionOrders(TArray<FHOLProductionCommand>& OutCommands);

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	TArray<FHOLMoveOrder> PendingMoveOrders;
	TArray<FHOLProductionCommand> PendingProductionOrders;
};
