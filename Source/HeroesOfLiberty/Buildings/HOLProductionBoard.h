// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLResources.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLProductionBoard.generated.h"

/**
 * The order of the gates of a production order and the cancel of a queued item, where the match runs (MIG-CPP step 5, lot R2a; D839: a binding to expose;
 * the replicated queue, the stock, the world queries and the journal lines stay in UHOLProductionSubsystem::ApplyOrders). The verdict is the script's
 * (Script/Buildings/HOLProductionBoard.as, namespace HOLProductionBoard, tested in Script/Tests/HOLProductionBoard_Test.as); this world subsystem asks
 * its script subclass (UHOLProductionBoardScript). Abstract: only a script subclass is born. Every Ask* answers a whole number, or -1 when the world
 * has no script board (ApplyOrders then keeps the C++ gates, still the twin of the script until the network proof, then removed in a later lot).
 * Order gates: 0 pass, 1 no player state, 2 not its building, 3 no type row, 4 not finished, 5 forum down (EHOLOrderGateVerdict of the script).
 * Cancel: 0 cancel the item, 1 ignore (EHOLProductionCancelVerdict). The effects (the item removed, SetResources, NotifyProductionChanged, the journal
 * lines) stay in UHOLProductionSubsystem, in the same order. * Queue (lot R2b, AskQueue): 0 enqueue, 1 not produced here, 2 queue full, 3 not enough resources, 4 research of another faction, 5 research blocked by
 * its check, 6 siege engine below its tier, 7 unit below its age, 8 unit of another faction (EHOLQueueVerdict); a research and a unit do not refuse in the
 * same order (see the script). The enqueue itself (HOLProduction::TryEnqueue) stays a C++ effect. The rally outcome (lot R3) is not here yet.
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLProductionBoard : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The world's production board (its script subclass), or null (no script compiled, or no world). */
	static UHOLProductionBoard* Find(const UWorld* World);

	/** The verdict of the gates of a production order (see above), or -1 (no board). The facts are computed by the caller. */
	static int32 AskOrderGate(const UWorld* World, bool bHasState, bool bOwner, bool bHasType, bool bFinished, bool bEarlyRally, bool bForumDown, bool bRally);
	/** The verdict of a cancel (see above), or -1 (no board). */
	static int32 AskCancel(const UWorld* World, bool bValidIndex);
	/** The stock once the cost of the cancelled item is given back; only to call after AskCancel answered 0. Without a board: Stock plus Cost. */
	static FHOLResources AskStockAfterCancel(const UWorld* World, const FHOLResources& Stock, const FHOLResources& Cost);

	/** The verdict of a queue order (see above), or -1 (no board). The facts are computed by the caller (TechBlocked / SiegeBlocked: the check said no). */
	static int32 AskQueue(const UWorld* World, bool bProduces, bool bTechnology, bool bTechFactionOpen, bool bTechBlocked, bool bSiegeBlocked, bool bTierTooLow,
		bool bUnitFactionOpen, int32 QueueLength, int32 MaxQueue, const FHOLResources& Stock, const FHOLResources& Cost);

	/** The outcome of a Rally order (lot R3): 0 no step (Shift on the body), 1 clear, 2 add a step, 3 set; or -1 (no board). The facts are computed by the caller. */
	static int32 AskRally(const UWorld* World, bool bOnBody, bool bShift, bool bHasRally);
	/** Whether a named rally site is kept: 1 kept, 0 dropped (the flag stays on the ground), -1 (no board). */
	static int32 AskRallySite(const UWorld* World, bool bOtherBuilding, bool bOwnedByPlayer, bool bNeedsBuilders, bool bIsCamp);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	/** The script subclass overrides these (HOLProductionBoard::...); the default is -1 (no rule here). */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Production")
	int32 OrderGateAnswer(bool bHasState, bool bOwner, bool bHasType, bool bFinished, bool bEarlyRally, bool bForumDown, bool bRally);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Production")
	int32 CancelAnswer(bool bValidIndex);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Production")
	FHOLResources StockAfterCancelAnswer(FHOLResources Stock, FHOLResources Cost);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Production")
	int32 QueueAnswer(bool bProduces, bool bTechnology, bool bTechFactionOpen, bool bTechBlocked, bool bSiegeBlocked, bool bTierTooLow, bool bUnitFactionOpen,
		int32 QueueLength, int32 MaxQueue, FHOLResources Stock, FHOLResources Cost);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Production")
	int32 RallyAnswer(bool bOnBody, bool bShift, bool bHasRally);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Production")
	int32 RallySiteAnswer(bool bOtherBuilding, bool bOwnedByPlayer, bool bNeedsBuilders, bool bIsCamp);
	virtual int32 OrderGateAnswer_Implementation(bool bHasState, bool bOwner, bool bHasType, bool bFinished, bool bEarlyRally, bool bForumDown, bool bRally);
	virtual int32 CancelAnswer_Implementation(bool bValidIndex);
	virtual FHOLResources StockAfterCancelAnswer_Implementation(FHOLResources Stock, FHOLResources Cost);
	virtual int32 QueueAnswer_Implementation(bool bProduces, bool bTechnology, bool bTechFactionOpen, bool bTechBlocked, bool bSiegeBlocked, bool bTierTooLow,
		bool bUnitFactionOpen, int32 QueueLength, int32 MaxQueue, FHOLResources Stock, FHOLResources Cost);
	virtual int32 RallyAnswer_Implementation(bool bOnBody, bool bShift, bool bHasRally);
	virtual int32 RallySiteAnswer_Implementation(bool bOtherBuilding, bool bOwnedByPlayer, bool bNeedsBuilders, bool bIsCamp);
};
