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

protected:
	UPROPERTY(VisibleAnywhere, Category = "Map")
	TObjectPtr<UBoxComponent> Area;
};
