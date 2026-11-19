// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLResources.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLConstructionBoard.generated.h"

/**
 * The order of the gates of a building's placement and cancel where the match runs (MIG-CPP step 5, lot K1a; D839: a binding to expose; the replicated
 * actor, the stock, the world queries and the journal lines stay in UHOLConstructionSubsystem::Place and ::Cancel). The verdict is the script's
 * (Script/Buildings/HOLConstructionBoard.as, namespace HOLConstructionBoard, tested in Script/Tests/HOLConstruction_Test.as); this world subsystem asks
 * its script subclass (UHOLConstructionBoardScript). Abstract: only a script subclass is born. Every Ask* answers a whole number, or -1 when the world
 * has no script board (Place and Cancel then keep the C++ gates, still the twin of the script until the network proof, then removed in a later lot).
 * Place: 0 place, 1 not buildable, 2 not of the faction, 3 age not reached, 4 placement refused, 5 not enough resources (EHOLPlaceVerdict of the script).
 * Cancel: 0 cancel, 1 ignore (EHOLCancelVerdict of the script).
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
	virtual int32 PlaceAnswer_Implementation(bool bBuildable, bool bFactionOpen, int32 BuildingAge, int32 Tier, int32 Refusal, FHOLResources Stock, FHOLResources Cost);
	virtual FHOLResources StockAfterPlaceAnswer_Implementation(FHOLResources Stock, FHOLResources Cost);
	virtual int32 CancelAnswer_Implementation(bool bHasBuilding, int32 OwnerPlayer, int32 AskerPlayer, int32 State);
	virtual FHOLResources CancelRefundAnswer_Implementation(bool bHasType, FHOLResources Cost, int32 State, int64 Work, int64 TotalWork);
};
