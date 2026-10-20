// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "HOLCorpseTrait.generated.h"

/**
 * Fragments of a body (J7c, bodies kept 30 s): its tag, its transform and actor fragment (required by Mass' visualization
 * trait), FHOLCorpseFragment and Mass' animation fragment; nothing of the simulation (no FHOLLightUnitTag), so the factory's third template per
 * type (EHOLUnitTemplateKind::Corpse) is seen by the drawing only.
 */
UCLASS(meta = (DisplayName = "HeroesOfLiberty Corpse"))
class HEROESOFLIBERTY_API UHOLCorpseTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
