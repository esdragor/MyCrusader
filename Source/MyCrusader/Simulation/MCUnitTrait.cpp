// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/MCUnitTrait.h"

#include "MassActorSubsystem.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassEntityUtils.h"
#include "Simulation/MCSimulationFragments.h"

void UMCUnitTrait::Setup(const FName InTypeName, const FMCUnitTypeConstants& InConstants)
{
	Constants = InConstants;
	Constants.TypeName = InTypeName;
}

void UMCUnitTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddTag<FMCLightUnitTag>();
	BuildContext.AddFragment<FTransformFragment>();
	BuildContext.AddFragment<FMCPreviousTransformFragment>();
	// Required by the visualization trait; light units never get an actor.
	BuildContext.AddFragment<FMassActorFragment>();
	BuildContext.AddFragment<FMCUnitStateFragment>();

	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	BuildContext.AddConstSharedFragment(EntityManager.GetOrCreateConstSharedFragment(Constants));
}
