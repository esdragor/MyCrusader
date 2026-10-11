// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/MCLightUnitAvoidanceTrait.h"

#include "HAL/IConsoleManager.h"

namespace UE::MyCrusader::Avoidance
{
	/** Stress test knob (D220): overrides the trait's value when > 0. */
	float ObstacleDetectionDistanceOverride = 0.f;
	FAutoConsoleVariableRef CVarObstacleDetectionDistance(TEXT("mc.Avoidance.ObstacleDetectionDistance"), ObstacleDetectionDistanceOverride,
		TEXT("Distance (cm) within which light units look for obstacles to avoid, 0 = trait value"));
}

void UMCLightUnitAvoidanceTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	using namespace UE::MyCrusader::Avoidance;
	const float Distance = ObstacleDetectionDistanceOverride > 0.f ? ObstacleDetectionDistanceOverride : ObstacleDetectionDistance;
	if (Distance > 0.f)
	{
		// The parameters live in the base trait, protected, and BuildTemplate is const: tune them on ourselves.
		UMCLightUnitAvoidanceTrait* MutableThis = const_cast<UMCLightUnitAvoidanceTrait*>(this);
		MutableThis->MovingParameters.ObstacleDetectionDistance = Distance;
	}
	Super::BuildTemplate(BuildContext, World);
}