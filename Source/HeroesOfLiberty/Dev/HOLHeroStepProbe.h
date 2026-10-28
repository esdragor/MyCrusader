// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Simulation/HOLHeroSheet.h"
#include "UObject/Object.h"
#include "HOLHeroStepProbe.generated.h"

class UHOLHeroSubsystem;

/**
 * Tests only (HeroesOfLiberty.Hero.StepEvent): listens to UHOLHeroSubsystem::OnHeroStep as a script would (a dynamic
 * delegate needs a UFUNCTION), and notes each step heard with the slot's public sheet state at that moment.
 */
UCLASS(Transient)
class HEROESOFLIBERTY_API UHOLHeroStepProbe : public UObject
{
	GENERATED_BODY()

public:
	/** Listens to Heroes' steps, noting slot Slot's public sheet state (the game state's copy). */
	void Listen(UHOLHeroSubsystem& Heroes, int32 InSlot);

	TArray<int32> Steps;
	TArray<EHOLHeroState> States;

private:
	UFUNCTION()
	void HandleHeroStep(int32 Step);

	int32 Slot = 0;
};
