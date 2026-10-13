// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLUnitTrait.h"

#include "MassActorSubsystem.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassEntityUtils.h"
#include "MassRepresentationFragments.h"
#include "Presentation/HOLPresentationFragments.h"
#include "Simulation/HOLAvoidanceHoldFragments.h"
#include "Simulation/HOLSimulationFragments.h"

void UHOLUnitTrait::Setup(const FName InTypeName, const FHOLUnitTypeConstants& InConstants)
{
	Constants = InConstants;
	Constants.TypeName = InTypeName;
}

void UHOLUnitTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddTag<FHOLLightUnitTag>();
	BuildContext.AddFragment<FTransformFragment>();
	BuildContext.AddFragment<FHOLPreviousTransformFragment>();
	// Required by the visualization trait; light units never get an actor.
	BuildContext.AddFragment<FMassActorFragment>();
	BuildContext.AddFragment<FHOLUnitStateFragment>();
	// Memory of the last avoidance impulse (D230): without it a throttled avoidance has no effect at all.
	BuildContext.AddFragment<FHOLAvoidanceHoldFragment>();
	// Animation played by the skinned instance (D234); the visualization trait does not add it.
	BuildContext.AddFragment<FMassRepresentationAnimationFragment>();
	BuildContext.AddFragment<FHOLUnitAnimationStateFragment>();

	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	BuildContext.AddConstSharedFragment(EntityManager.GetOrCreateConstSharedFragment(Constants));
}
