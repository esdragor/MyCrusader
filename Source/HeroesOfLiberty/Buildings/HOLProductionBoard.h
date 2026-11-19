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
 * lines) stay in UHOLProductionSubsystem, in the same order. The queue verdict (AskQueue, lot R2b) and the rally outcome (lot R3) are not here yet.
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
	virtual int32 OrderGateAnswer_Implementation(bool bHasState, bool bOwner, bool bHasType, bool bFinished, bool bEarlyRally, bool bForumDown, bool bRally);
	virtual int32 CancelAnswer_Implementation(bool bValidIndex);
	virtual FHOLResources StockAfterCancelAnswer_Implementation(FHOLResources Stock, FHOLResources Cost);
};
