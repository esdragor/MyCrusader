// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "HOLStartSlot.generated.h"

/**
 * Start of a slot of the match configuration (D244, D256): the player holding slot SlotIndex starts here. Placed in
 * the map, one per slot (maps are symmetric, § 14.4). A map with start slots is a match map: its players get the RTS
 * camera and no pawn (D255); a map without (StressTest, BattleMap) keeps the default pawn.
 */
UCLASS()
class HEROESOFLIBERTY_API AHOLStartSlot : public APlayerStart
{
	GENERATED_BODY()

public:
	AHOLStartSlot(const FObjectInitializer& ObjectInitializer);

	int32 GetSlotIndex() const { return SlotIndex; }

	/** The start of the given slot, or null if the map has none. */
	static AHOLStartSlot* Find(const UWorld* World, int32 Index);

	/** Whether the map has start slots, i.e. is a match map. */
	static bool WorldHasSlots(const UWorld* World);

protected:
	/** Slot of the match configuration, 0 to 7 (FHOLMatchConfig::MaxSlots). */
	UPROPERTY(EditAnywhere, Category = "Match", meta = (ClampMin = 0, ClampMax = 7))
	int32 SlotIndex = 0;
};
