// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "Simulation/MCUnitTypes.h"
#include "MCUnitTrait.generated.h"

/**
 * Fragments common to every unit type (D225): the factory poses them through this trait, so no type poses them
 * itself. The type's values ride along as a const shared fragment (D222), one copy per type.
 */
UCLASS(meta = (DisplayName = "MyCrusader Unit"))
class MYCRUSADER_API UMCUnitTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	/** Fills the trait from one row of the type table; called by the factory (D225). */
	void Setup(FName InTypeName, const FMCUnitTypeConstants& InConstants);

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

private:
	FMCUnitTypeConstants Constants;
};
