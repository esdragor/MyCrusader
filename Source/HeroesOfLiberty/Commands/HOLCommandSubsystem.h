// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Commands/HOLConstructionCommand.h"
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
	void IssueMoveOrder(TConstArrayView<uint32> UnitIds, const FVector& Target, int32 PlayerIndex = INDEX_NONE, bool bFromTask = false);

	/** Takes the orders waiting for the simulation (the order processor calls this at each step). */
	void ConsumeMoveOrders(TArray<FHOLMoveOrder>& OutOrders);

	int32 GetNumPendingOrders() const { return PendingMoveOrders.Num(); }

	/**
	 * Queues "gather this source" for the given units (J5, D296); PlayerIndex as for a move order. With bQueue (Shift + right
	 * click, the user's trial of J6a.4), a build order naming the source instead: it goes at the end of each unit's queue,
	 * after its sites, applied by the builders in the order given.
	 */
	void IssueGatherOrder(TConstArrayView<uint32> UnitIds, int32 SourceIndex, int32 PlayerIndex = INDEX_NONE, bool bQueue = false);
	/** Takes the gather orders waiting for the next step (the gathering calls this at each step). */
	void ConsumeGatherOrders(TArray<FHOLGatherOrder>& OutOrders);

	/** Queues a production order (J3, D282), applied by the production at the next step. */
	void IssueProductionOrder(const FHOLProductionCommand& Command);
	/** Takes the production orders waiting for the next step (UHOLProductionSubsystem calls this at each step). */
	void ConsumeProductionOrders(TArray<FHOLProductionCommand>& OutCommands);

	/** Queues a construction order (J6a.3, D311), applied by the construction at the next step. */
	void IssueConstructionOrder(const FHOLConstructionCommand& Command);
	/** Takes the construction orders waiting for the next step (UHOLConstructionSubsystem calls this at each step). */
	void ConsumeConstructionOrders(TArray<FHOLConstructionCommand>& OutCommands);

	/** Queues a build order (J6a.4, D312), applied by the builders at the next step. */
	void IssueBuildOrder(const FHOLBuildOrder& Order);
	/** Takes the build orders waiting for the next step (UHOLBuildProcessor calls this at each step). */
	void ConsumeBuildOrders(TArray<FHOLBuildOrder>& OutOrders);

	/**
	 * The navmesh changes there (a source used up, a building placed; server and solo): the paths the simulation kept
	 * for the gathering's walks across that area are forgotten at the next step (D305).
	 */
	void NotifyNavigationChanged(const FBox& Area);
	/** Takes the areas changed since the last step (the order processor calls this at each step). */
	void ConsumeNavigationChanges(TArray<FBox>& OutAreas);

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	TArray<FHOLMoveOrder> PendingMoveOrders;
	TArray<FHOLGatherOrder> PendingGatherOrders;
	TArray<FHOLProductionCommand> PendingProductionOrders;
	TArray<FHOLConstructionCommand> PendingConstructionOrders;
	TArray<FHOLBuildOrder> PendingBuildOrders;
	TArray<FBox> PendingNavigationChanges;
};
