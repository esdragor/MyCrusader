// Copyright Epic Games, Inc. All Rights Reserved.

#include "Dev/HOLHeroStepProbe.h"

#include "Engine/World.h"
#include "Match/HOLGameState.h"
#include "Simulation/HOLHeroSubsystem.h"

void UHOLHeroStepProbe::Listen(UHOLHeroSubsystem& Heroes, const int32 InSlot)
{
	Slot = InSlot;
	Heroes.OnHeroStep.AddDynamic(this, &UHOLHeroStepProbe::HandleHeroStep);
}

void UHOLHeroStepProbe::HandleHeroStep(const int32 Step)
{
	Steps.Add(Step);
	const UHOLHeroSubsystem* Heroes = Cast<UHOLHeroSubsystem>(GetOuter());
	const AHOLGameState* GameState = Heroes && Heroes->GetWorld() ? Heroes->GetWorld()->GetGameState<AHOLGameState>() : nullptr;
	States.Add(GameState ? GameState->GetHeroSheet(Slot).State : EHOLHeroState::None);
}
