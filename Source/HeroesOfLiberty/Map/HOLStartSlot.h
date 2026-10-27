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

	/** The point on the ground at the foot of the start (its capsule stands on the ground). */
	FVector GetGroundLocation() const;

	/**
	 * Where the slot's main centre stands (D311): the ground point moved onto the tile grid (its footprint's edges on
	 * the grid lines, 1.5 m at most each way), so the houses and camps placed tile by tile line up with it. The
	 * camera starts above it; the scripted checks aim at it.
	 */
	FVector GetMainCentreLocation() const;

	/** The main centre's yaw: the start's, turned to the nearest quarter turn to keep the footprint on the grid. */
	float GetMainCentreYaw() const;

	/** The start of the given place of the map (its SlotIndex), or null if the map has none. */
	static AHOLStartSlot* Find(const UWorld* World, int32 Index);

	/**
	 * TC3: the start of a slot of the match configuration, at its place (AHOLGameState::GetStartPlace: fixed, slot i at
	 * place i, or drawn at the launch); null for a slot without a place or a map without that place.
	 */
	static AHOLStartSlot* FindForSlot(const UWorld* World, int32 Slot);

	/** Whether the map has start slots, i.e. is a match map. */
	static bool WorldHasSlots(const UWorld* World);

	/** Tests: the place this start stands for. */
	void SetSlotIndex(const int32 InSlotIndex) { SlotIndex = InSlotIndex; }

protected:
	/**
	 * The place on the map, 0 to 7 (FHOLMatchConfig::MaxSlots), numbered in order around the map's circle: slot i of the
	 * configuration starts here with fixed places (tests, scenarios), the slot drawn on it otherwise (TC3, FindForSlot).
	 */
	UPROPERTY(EditAnywhere, Category = "Match", meta = (ClampMin = 0, ClampMax = 7))
	int32 SlotIndex = 0;
};
