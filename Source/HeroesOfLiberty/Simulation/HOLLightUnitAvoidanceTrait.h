// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Avoidance/MassAvoidanceTrait.h"

#include "HOLLightUnitAvoidanceTrait.generated.h"

/**
 * Obstacle avoidance of the light units (D29): the Mass trait with our own detection distance, read from the
 * unit type table since M1 (D221, D222). Mass looks 400 cm around each unit, which is nearly the whole
 * simulation cost at 2 000 units (D220); the table gives each type the distance it needs.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLLightUnitAvoidanceTrait : public UMassObstacleAvoidanceTrait
{
	GENERATED_BODY()

public:
	/** Distance (cm) within which the unit looks for other units to avoid; the factory sets it from the row. */
	void SetObstacleDetectionDistance(const float Distance) { ObstacleDetectionDistance = Distance; }

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

	/** Distance (cm) within which the unit looks for other units to avoid (0 = Mass' 400 cm default). */
	UPROPERTY(EditAnywhere, Category = "Avoidance", meta = (ClampMin = "0", ForceUnits = "cm"))
	float ObstacleDetectionDistance = 250.f;
};
