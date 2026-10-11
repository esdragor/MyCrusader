// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MCLightUnitTrait.generated.h"

/** Base fragments of a light unit (D29) on top of the Mass navigation and movement traits. */
UCLASS(meta = (DisplayName = "MyCrusader Light Unit"))
class MYCRUSADER_API UMCLightUnitTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	/** Stress test behaviour: walk to random points (M1 trial). */
	UPROPERTY(EditAnywhere, Category = "Light Unit")
	bool bWander = false;

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
