// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Commands/HOLConstructionCommand.h"
#include "Commands/HOLProductionCommand.h"
#include "Commands/HOLUnitCommand.h"
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLCommandSubsystem.generated.h"

/**
 * A building's footprint that starts blocking the navmesh (J6b.2, D314): a site's first work (D312), or a building placed
 * finished by the code (the main centres, S7's camps). Its tiles of the navmesh are rebuilt a little later.
 */
struct FHOLBlockedFootprint
{
	FName TypeName;
	/** Its middle on the ground, its size (cm: its body, inset in the table's Footprint, D324) and its yaw (degrees). */
	FVector2D Centre = FVector2D::ZeroVector;
	FVector2D Size = FVector2D::ZeroVector;
	float Yaw = 0.f;
	/** The forbidden area given to the navmesh (the footprint and the bodies' clearance, D310). */
	FBox Area = FBox(ForceInit);
};

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
	void IssueMoveOrder(TConstArrayView<uint32> UnitIds, const FVector& Target, int32 PlayerIndex = INDEX_NONE, bool bFromTask = false,
		const FVector2D& FrontLine = FVector2D::ZeroVector, bool bFrontWidth = false, bool bQueue = false);

	/**
	 * The combat orders (J7b.3, J7-Q4), through the move queue: Attack (TargetUnitId), Stop, Hold; PlayerIndex as for a move
	 * order (D288). The server checks again that the target is an enemy unit alive when the order is applied.
	 */
	void IssueCombatOrder(TConstArrayView<uint32> UnitIds, EHOLOrderKind Kind, uint32 TargetUnitId, int32 PlayerIndex = INDEX_NONE, bool bQueue = false);
	/** Attack-move (J7b.3): a move order (the group in formation), its units taking on the enemies met on the way. */
	void IssueAttackMoveOrder(TConstArrayView<uint32> UnitIds, const FVector& Target, int32 PlayerIndex = INDEX_NONE, bool bQueue = false);
	/** A combat walk of one unit (its approach, its attack-move resumed), stamped (FHOLMoveOrder::CombatStamp). */
	void IssueCombatWalk(uint32 UnitId, const FVector& Target, uint32 CombatStamp, bool bAttackMove);

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

	/**
	 * A footprint starts blocking the navmesh (J6b.2, D314; server and solo): the kept paths across it are forgotten at the
	 * next step, and once its tiles are rebuilt the walkers whose way crosses it get a new path.
	 */
	void NotifyFootprintBlocked(const FHOLBlockedFootprint& Footprint);
	/** Takes the footprints blocked since the last step (the order processor calls this at each step). */
	void ConsumeBlockedFootprints(TArray<FHOLBlockedFootprint>& OutFootprints);

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	TArray<FHOLMoveOrder> PendingMoveOrders;
	TArray<FHOLGatherOrder> PendingGatherOrders;
	TArray<FHOLProductionCommand> PendingProductionOrders;
	TArray<FHOLConstructionCommand> PendingConstructionOrders;
	TArray<FHOLBuildOrder> PendingBuildOrders;
	TArray<FBox> PendingNavigationChanges;
	TArray<FHOLBlockedFootprint> PendingBlockedFootprints;
};
