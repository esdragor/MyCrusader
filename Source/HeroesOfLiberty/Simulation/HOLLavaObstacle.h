// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Simulation/HOLVolcanoRules.h"
#include "HOLLavaObstacle.generated.h"

class UBoxComponent;

/**
 * V6 (GDD 12.2, D379): a flow that has stopped and cools is impassable. Server side only: forbidden area of the navmesh made of the
 * boxes of HOLVolcano::GetObstacleBoxes (a dynamic obstacle as a building's footprint, D286), there while the flow cools and no more
 * (UHOLVolcanoSubsystem makes it and removes it). Not replicated: clients do not walk the navmesh.
 */
UCLASS()
class HEROESOFLIBERTY_API AHOLLavaObstacle : public AActor
{
	GENERATED_BODY()

public:
	AHOLLavaObstacle();

	/** Lays the boxes (height Height, cm) and gives them to the navigation; the paths whose way they cut are asked again. */
	void Build(const TArray<HOLVolcano::FObstacleBox>& Boxes, float Height);

	int32 GetNumBoxes() const { return BoxComponents.Num(); }
	/** The cycle of the site this obstacle belongs to. */
	int32 GetCycleIndex() const { return CycleIndex; }
	void SetCycleIndex(const int32 InCycleIndex) { CycleIndex = InCycleIndex; }
	/** Every box counted by the navigation. */
	bool IsBlockingNavigation() const;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UBoxComponent>> BoxComponents;
	int32 CycleIndex = 0;
};