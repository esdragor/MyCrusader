// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Movement/MassMovementTrait.h"
#include "MCUnitMovementTrait.generated.h"

/**
 * Mass' movement trait with the type's own speed: the factory reads the row of the table and sets the
 * parameters here, so speed is data (D221) and not a number buried in a trait (D222).
 */
UCLASS()
class MYCRUSADER_API UMCUnitMovementTrait : public UMassMovementTrait
{
	GENERATED_BODY()

public:
	void SetMovement(const FMassMovementParameters& InMovement) { Movement = InMovement; }
};
