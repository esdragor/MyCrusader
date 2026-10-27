// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAIBrain.h"

#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLPlayerState.h"
#include "Simulation/HOLSimulationSubsystem.h"

namespace UE::HeroesOfLiberty::AI
{
	/** IA0.4-Q4 (not settled, 5 kept): steps between two thoughts of one AI slot, 5 = 4 thoughts per second at 20 steps. */
	int32 ThinkSteps = 5;
	FAutoConsoleVariableRef CVarThinkSteps(TEXT("hol.AI.ThinkSteps"), ThinkSteps,
		TEXT("Steps between two thoughts of one AI slot (at least 1; spread over the interval by slot)"));
}

UHOLAIBrainSubsystem* UHOLAIBrainSubsystem::Find(const UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	const TArray<UHOLAIBrainSubsystem*> Brains = World->GetSubsystemArrayCopy<UHOLAIBrainSubsystem>();
	return Brains.Num() > 0 ? Brains[0] : nullptr;
}

int32 UHOLAIBrainSubsystem::GetThinkSteps()
{
	return FMath::Max(1, UE::HeroesOfLiberty::AI::ThinkSteps);
}

bool UHOLAIBrainSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// IA0.4 B1: never the abstract C++ class, nor a script class replaced by a reload (as UScriptWorldSubsystem does).
	if (IsUnreachable() || GetClass()->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists))
	{
		return false;
	}
	return Super::ShouldCreateSubsystem(Outer);
}

bool UHOLAIBrainSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHOLAIBrainSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// The server (or a solo match) decides alone (D282); a client's brain stays asleep.
	if (InWorld.GetNetMode() == NM_Client)
	{
		return;
	}
	// Tail of the step (IA0.4, relevé 2): the summary reads the finished step, the orders apply at the next one.
	if (UHOLSimulationSubsystem* Simulation = InWorld.GetSubsystem<UHOLSimulationSubsystem>())
	{
		StepEndedHandle = Simulation->GetOnStepEnded().AddUObject(this, &UHOLAIBrainSubsystem::OnStepEnded);
	}
}

void UHOLAIBrainSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		if (UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>())
		{
			Simulation->GetOnStepEnded().Remove(StepEndedHandle);
		}
	}
	StepEndedHandle.Reset();
	Super::Deinitialize();
}

TArray<int32> UHOLAIBrainSubsystem::GetAISlots() const
{
	TArray<int32> Slots;
	const UWorld* World = GetWorld();
	const AGameStateBase* State = World ? World->GetGameState() : nullptr;
	if (!State)
	{
		return Slots;
	}
	for (const APlayerState* Each : State->PlayerArray)
	{
		const AHOLPlayerState* HOLEach = Cast<AHOLPlayerState>(Each);
		if (HOLEach && HOLEach->IsABot())
		{
			Slots.AddUnique(HOLEach->GetPlayerIndex());
		}
	}
	Slots.Sort();
	return Slots;
}

bool UHOLAIBrainSubsystem::ThinkNow(int32 PlayerIndex)
{
	const UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		return false;
	}
	const bool bThought = Think(PlayerIndex);
	if (bThought)
	{
		++NumThoughts;
	}
	return bThought;
}

void UHOLAIBrainSubsystem::OnStepEnded(int64 StepIndex)
{
	const int32 Interval = GetThinkSteps();
	for (const int32 Slot : GetAISlots())
	{
		// Spread: slot S thinks at the steps where (step + S) is a multiple of the interval.
		if ((StepIndex + Slot) % Interval != 0)
		{
			continue;
		}
		UE_LOG(LogHOLMatch, Verbose, TEXT("AI brain: slot %d thinks (step %lld)"), Slot, StepIndex);
		ThinkNow(Slot);
	}
}
