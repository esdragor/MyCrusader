// Copyright Epic Games, Inc. All Rights Reserved.

#include "Presentation/HOLCorpseTrait.h"

#include "MassActorSubsystem.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassRepresentationFragments.h"
#include "Presentation/HOLPresentationFragments.h"

void UHOLCorpseTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddTag<FHOLCorpseTag>();
	BuildContext.AddFragment<FTransformFragment>();
	BuildContext.AddFragment<FMassActorFragment>();
	BuildContext.AddFragment<FHOLCorpseFragment>();
	// What its skinned instance is given (UHOLInterpolatedInstancesProcessor): Mass' visualization trait does not add it (the
	// unit's trait does, UHOLUnitTrait).
	BuildContext.AddFragment<FMassRepresentationAnimationFragment>();
}
