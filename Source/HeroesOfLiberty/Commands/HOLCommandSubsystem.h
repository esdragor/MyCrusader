// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Commands/HOLConstructionCommand.h"
#include "Commands/HOLProductionCommand.h"
#include "Commands/HOLUnitCommand.h"
#include "CoreMinimal.h"

class AHOLBuilding;
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
	 * While one lives, the orders issued are a player's (D328): their units' chains are drawn (bChain), with Shift or not.
	 * The controller's order calls open one; the server's own orders (tasks, rally points, stress tests) do not.
	 */
	struct FPlayerOrderScope
	{
		explicit FPlayerOrderScope(UHOLCommandSubsystem& InCommands) : Guard(InCommands.bPlayerOrders, true) {}
	private:
		TGuardValue<bool> Guard;
	};

	/**
	 * While one lives, the orders issued send units just born to their building's rally point (D328, the user's trial): their
	 * chain is drawn too, one white step (HOLOrderChain::EStepKind::Rally) until they get there, whatever the order.
	 */
	struct FRallyOrderScope
	{
		explicit FRallyOrderScope(UHOLCommandSubsystem& InCommands) : Player(InCommands.bPlayerOrders, true), Rally(InCommands.bRallyOrders, true) {}
	private:
		TGuardValue<bool> Player;
		TGuardValue<bool> Rally;
	};

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
	/**
	 * D345.2b: units strike an enemy building (a right click on it); the order processor checks it. bQueue (Shift, D345): at
	 * the end of each unit's queue, as an attack on a unit; it starts once the action before it is over.
	 */
	void IssueAttackBuildingOrder(TConstArrayView<uint32> UnitIds, AHOLBuilding* Building, int32 PlayerIndex = INDEX_NONE, bool bQueue = false);

	/** Takes the orders waiting for the simulation (the order processor calls this at each step). */
	void ConsumeMoveOrders(TArray<FHOLMoveOrder>& OutOrders);

	int32 GetNumPendingOrders() const { return PendingMoveOrders.Num(); }

	/**
	 * Queues "gather this source" for the given units (J5, D296); PlayerIndex as for a move order. With bQueue (Shift + right
	 * click, the user's trial of J6a.4), a build order naming the source instead: it goes at the end of each unit's queue,
	 * after its sites, applied by the builders in the order given.
	 */
	void IssueGatherOrder(TConstArrayView<uint32> UnitIds, int32 SourceIndex, int32 PlayerIndex = INDEX_NONE, bool bQueue = false);
	/**
	 * Units to unload what they carry at one of their player's buildings (D485, EtapeDefini L), then stand idle: only those
	 * carrying a resource it takes obey (checked again by the gatherers' step). Applied at once; with bQueue (Shift, L.3),
	 * it goes at the end of each worker's queue, among its sites and sources: at its turn, a worker carrying a resource it
	 * takes unloads there, the others go on to the next item.
	 */
	void IssueDropOffOrder(TConstArrayView<uint32> UnitIds, AHOLBuilding* Building, int32 PlayerIndex = INDEX_NONE, bool bQueue = false);
	/**
	 * A player's drop-off through the queue of moves (D485, EtapeDefini L.5b): EHOLOrderKind::DropOff, Point (the point
	 * clicked) for the units that carry nothing it takes. With bQueue (Shift), it waits for the units' walks before it
	 * (UHOLUnitOrderProcessor); its builders with a queue of work get it in that queue instead (IssueDropOffOrder, bQueue).
	 */
	void IssueDropOffMoveOrder(TConstArrayView<uint32> UnitIds, AHOLBuilding* Building, const FVector& Point, int32 PlayerIndex = INDEX_NONE,
		bool bQueue = false);
	/**
	 * A player's source (SourceIndex) or site (Building) with Shift through the queue of moves (EtapeDefini L.6):
	 * EHOLOrderKind::Gather or Build, Location for the logs. It waits for the units' walks before it (UHOLUnitOrderProcessor),
	 * then gives them the Shift order of the gathering or the builders (IssueGatherOrder, IssueBuildOrder: with no queue of
	 * work, at once); a worker with a queue of work gets it at the end of that queue at once, as before.
	 */
	void IssueQueuedWorkOrder(TConstArrayView<uint32> UnitIds, int32 SourceIndex, AHOLBuilding* Building, const FVector& Location,
		int32 PlayerIndex = INDEX_NONE);
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

	/**
	 * J8a.4: PlayerIndex's player buys its dead hero's return (ServerReturnHero, hol.Hero.Return), applied at the end of the
	 * next step by the hero subsystem: the price of that step, the gold taken and the birth decided there (script).
	 * Refused here for no player's slot (INDEX_NONE, out of range), a defeated slot (D358) and while the match waits.
	 */
	void IssueHeroReturnOrder(int32 PlayerIndex);
	/** Takes the hero return orders waiting (UHOLHeroSubsystem calls this at the end of each step), in the order given. */
	void ConsumeHeroReturnOrders(TArray<int32>& OutPlayerIndices);

	/** D358: drops every order of PlayerIndex still waiting (a slot just defeated). */
	void DropOrdersOf(int32 PlayerIndex);

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	/**
	 * D358: PlayerIndex is a defeated slot (AHOLGameState): its orders, productions and constructions are refused here,
	 * where every order of a player enters (the first refusal of each slot logged). INDEX_NONE (the server's own) never is.
	 */
	bool IsRefused(int32 PlayerIndex);
	/** Bit Slot: its first refusal was logged. */
	uint32 RefusalsLogged = 0;
	/** A FPlayerOrderScope lives (D328). */
	bool bPlayerOrders = false;
	/** A FRallyOrderScope lives (D328). */
	bool bRallyOrders = false;
	TArray<FHOLMoveOrder> PendingMoveOrders;
	TArray<FHOLGatherOrder> PendingGatherOrders;
	TArray<FHOLProductionCommand> PendingProductionOrders;
	TArray<FHOLConstructionCommand> PendingConstructionOrders;
	TArray<FHOLBuildOrder> PendingBuildOrders;
	/** J8a.4: the slots that asked their hero's return, in the order given. */
	TArray<int32> PendingHeroReturnOrders;
	TArray<FBox> PendingNavigationChanges;
	TArray<FHOLBlockedFootprint> PendingBlockedFootprints;
};
