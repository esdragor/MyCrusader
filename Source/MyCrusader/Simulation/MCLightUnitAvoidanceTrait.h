// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Avoidance/MassAvoidanceTrait.h"

#include "MCLightUnitAvoidanceTrait.generated.h"

/**
 * Obstacle avoidance of the light units (D29): the Mass trait with our own detection distance (D220).
 * Mass looks 400 cm around each unit, which is nearly the whole simulation cost at 2 000 units; a 40 cm
 * wide unit does not need to see that far, so the distance is kept as tight as the units allow.
 * The value is provisional: it moves to the units' data when the prototype units are described.
 */
UCLASS()
class MYCRUSADER_API UMCLightUnitAvoidanceTrait : public UMassObstacleAvoidanceTrait
{
	GENERATED_BODY()

public:
	/** Distance (cm) within which the unit looks for other units to avoid (0 = Mass' 400 cm default). */
	UPROPERTY(EditAnywhere, Category = "Avoidance", meta = (ClampMin = "0", ForceUnits = "cm"))
	float ObstacleDetectionDistance = 250.f;

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
