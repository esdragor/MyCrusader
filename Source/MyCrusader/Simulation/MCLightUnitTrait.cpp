// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/MCLightUnitTrait.h"

#include "MassActorSubsystem.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "Simulation/MCSimulationFragments.h"

void UMCLightUnitTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddTag<FMCLightUnitTag>();
	BuildContext.AddFragment<FTransformFragment>();
	BuildContext.AddFragment<FMCPreviousTransformFragment>();
	// Required by the visualization trait; light units never get an actor.
	BuildContext.AddFragment<FMassActorFragment>();

	if (bWander)
	{
		BuildContext.AddTag<FMCWanderTag>();
	}
}
