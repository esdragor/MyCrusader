// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLLightUnitAvoidanceTrait.h"

#include "HAL/IConsoleManager.h"

namespace UE::HeroesOfLiberty::Avoidance
{
	/** Stress test knob (D220): overrides the trait's value when > 0. */
	float ObstacleDetectionDistanceOverride = 0.f;
	FAutoConsoleVariableRef CVarObstacleDetectionDistance(TEXT("hol.Avoidance.ObstacleDetectionDistance"), ObstacleDetectionDistanceOverride,
		TEXT("Distance (cm) within which light units look for obstacles to avoid, 0 = trait value"));
}

void UHOLLightUnitAvoidanceTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	using namespace UE::HeroesOfLiberty::Avoidance;
	const float Distance = ObstacleDetectionDistanceOverride > 0.f ? ObstacleDetectionDistanceOverride : ObstacleDetectionDistance;
	if (Distance > 0.f)
	{
		// The parameters live in the base trait, protected, and BuildTemplate is const: tune them on ourselves.
		UHOLLightUnitAvoidanceTrait* MutableThis = const_cast<UHOLLightUnitAvoidanceTrait*>(this);
		MutableThis->MovingParameters.ObstacleDetectionDistance = Distance;
	}
	Super::BuildTemplate(BuildContext, World);
}