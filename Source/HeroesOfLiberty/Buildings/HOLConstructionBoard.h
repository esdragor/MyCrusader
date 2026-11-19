// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLResources.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLConstructionBoard.generated.h"

/**
 * The order of the gates of a building's placement and cancel, and the decisions of its destruction and of the refund of its queue, where the match runs (MIG-CPP step 5, lots K1a and K1b; D839: a binding to expose; the replicated
 * actor, the stock, the world queries and the journal lines stay in UHOLConstructionSubsystem::Place and ::Cancel). The verdict is the script's
 * (Script/Buildings/HOLConstructionBoard.as, namespace HOLConstructionBoard, tested in Script/Tests/HOLConstruction_Test.as); this world subsystem asks
 * its script subclass (UHOLConstructionBoardScript). Abstract: only a script subclass is born. Every Ask* answers a whole number, or -1 when the world
 * has no script board (Place and Cancel then keep the C++ gates, still the twin of the script until the network proof, then removed in a later lot).
 * Place: 0 place, 1 not buildable, 2 not of the faction, 3 age not reached, 4 placement refused, 5 not enough resources (EHOLPlaceVerdict of the script).
 * Cancel: 0 cancel, 1 ignore (EHOLCancelVerdict of the script).
 * K1b: queue refund 0 refund, 1 nothing (EHOLQueueRefundVerdict); destroy 0 destroy, 1 leave (EHOLDestroyVerdict); rubble 0 none, 1 leave rubble (EHOLRubbleVerdict).
 * The effects (the queue emptied, AddSlotStock, rubble spawn, Destroy, journal lines) stay in UHOLConstructionSubsystem, in the same order.
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLConstructionBoard : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The world's construction board (its script subclass), or null (no script compiled, or no world). */
	static UHOLConstructionBoard* Find(const UWorld* World);

	/** The verdict of a placement (see above), or -1 (no board). Refusal = EHOLPlacementRefusal as a whole number. */
	static int32 AskPlace(const UWorld* World, bool bBuildable, bool bFactionOpen, int32 BuildingAge, int32 Tier, int32 Refusal,
		const FHOLResources& Stock, const FHOLResources& Cost);
	/** The stock to write once the foundation is born; only to call after AskPlace answered 0. */
	static FHOLResources AskStockAfterPlace(const UWorld* World, const FHOLResources& Stock, const FHOLResources& Cost);
	/** The verdict of a cancel (see above), or -1 (no board). State = EHOLConstructionState as a whole number. */
	static int32 AskCancel(const UWorld* World, bool bHasBuilding, int32 OwnerPlayer, int32 AskerPlayer, int32 State);
	/** The refund of an allowed cancel; only to call after AskCancel answered 0. */
	static FHOLResources AskCancelRefund(const UWorld* World, bool bHasType, const FHOLResources& Cost, int32 State, int64 Work, int64 TotalWork);
	/** K1b: does the queue refund happen (0), or is there nothing to refund (1)? -1: no board. */
	static int32 AskQueueRefund(const UWorld* World, bool bHasWorld, bool bAuthority, int32 QueueLength);
	/** K1b: the sum of the costs of the queued items (queue order). False (OutRefund untouched): no board, the caller sums them. */
	static bool AskQueueTotal(const UWorld* World, const TArray<FHOLResources>& ItemCosts, FHOLResources& OutRefund);
	/** K1b: is the building destroyed (0) or left (1)? -1: no board. */
	static int32 AskDestroy(const UWorld* World, bool bHasWorld, bool bAuthority, bool bValid);
	/** K1b: no rubble (0) or rubble (1) where it stood? -1: no board. */
	static int32 AskRubble(const UWorld* World, bool bFoundation, bool bHasType);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	/** The script subclass overrides these (HOLConstructionBoard::...); the default is -1 (no rule here). */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Construction")
	int32 PlaceAnswer(bool bBuildable, bool bFactionOpen, int32 BuildingAge, int32 Tier, int32 Refusal, FHOLResources Stock, FHOLResources Cost);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Construction")
	FHOLResources StockAfterPlaceAnswer(FHOLResources Stock, FHOLResources Cost);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Construction")
	int32 CancelAnswer(bool bHasBuilding, int32 OwnerPlayer, int32 AskerPlayer, int32 State);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Construction")
	FHOLResources CancelRefundAnswer(bool bHasType, FHOLResources Cost, int32 State, int64 Work, int64 TotalWork);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Construction")
	int32 QueueRefundAnswer(bool bHasWorld, bool bAuthority, int32 QueueLength);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Construction")
	FHOLResources QueueTotalAnswer(const TArray<FHOLResources>& ItemCosts);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Construction")
	int32 DestroyAnswer(bool bHasWorld, bool bAuthority, bool bValid);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Construction")
	int32 RubbleAnswer(bool bFoundation, bool bHasType);
	virtual int32 PlaceAnswer_Implementation(bool bBuildable, bool bFactionOpen, int32 BuildingAge, int32 Tier, int32 Refusal, FHOLResources Stock, FHOLResources Cost);
	virtual FHOLResources StockAfterPlaceAnswer_Implementation(FHOLResources Stock, FHOLResources Cost);
	virtual int32 CancelAnswer_Implementation(bool bHasBuilding, int32 OwnerPlayer, int32 AskerPlayer, int32 State);
	virtual FHOLResources CancelRefundAnswer_Implementation(bool bHasType, FHOLResources Cost, int32 State, int64 Work, int64 TotalWork);
	virtual int32 QueueRefundAnswer_Implementation(bool bHasWorld, bool bAuthority, int32 QueueLength);
	virtual FHOLResources QueueTotalAnswer_Implementation(const TArray<FHOLResources>& ItemCosts);
	virtual int32 DestroyAnswer_Implementation(bool bHasWorld, bool bAuthority, bool bValid);
	virtual int32 RubbleAnswer_Implementation(bool bFoundation, bool bHasType);
};
