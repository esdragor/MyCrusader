// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/MCUnitTrait.h"

#include "MassActorSubsystem.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassEntityUtils.h"
#include "MassRepresentationFragments.h"
#include "Presentation/MCPresentationFragments.h"
#include "Simulation/MCAvoidanceHoldFragments.h"
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
	// Memory of the last avoidance impulse (D230): without it a throttled avoidance has no effect at all.
	BuildContext.AddFragment<FMCAvoidanceHoldFragment>();
	// Animation played by the skinned instance (D234); the visualization trait does not add it.
	BuildContext.AddFragment<FMassRepresentationAnimationFragment>();
	BuildContext.AddFragment<FMCUnitAnimationStateFragment>();

	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	BuildContext.AddConstSharedFragment(EntityManager.GetOrCreateConstSharedFragment(Constants));
}
