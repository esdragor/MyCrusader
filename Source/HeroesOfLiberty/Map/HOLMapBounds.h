// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HOLMapBounds.generated.h"

class UBoxComponent;

/**
 * Playable area of a match map (D256), seen from above: the RTS camera keeps its focus inside (D254), and later the
 * minimap and building placement. One per map; its box is shown in the editor only.
 */
UCLASS()
class HEROESOFLIBERTY_API AHOLMapBounds : public AActor
{
	GENERATED_BODY()

public:
	AHOLMapBounds();

	/** The area on the ground plane (X, Y), world space. */
	FBox2D GetArea() const;

	/** The bounds of the map, or null if it has none. */
	static AHOLMapBounds* Find(const UWorld* World);

	/**
	 * The largest map (D344, the user's choice of 2026-10-22): 220 x 220 tiles of UHOLBuildingSettings::TileSize (4 m:
	 * 880 m a side), centred on the world's origin. The unit replication's positions reach just past it (13 bits at
	 * 11 cm: +-450 m).
	 */
	static constexpr int32 MaxSideTiles = 220;

	/** Whether an area fits in the largest map: within MaxSideTiles / 2 tiles of the origin on both axes. */
	static bool FitsMaxSize(const FBox2D& InArea);

protected:
	/** Warns when the map is larger than the largest map (D344). */
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Map")
	TObjectPtr<UBoxComponent> Area;
};
