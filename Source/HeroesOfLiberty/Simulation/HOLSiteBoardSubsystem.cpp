// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLSiteBoardSubsystem.h"

#include "Engine/World.h"
#include "Simulation/HOLSiteRules.h"

UHOLSiteBoardSubsystem* UHOLSiteBoardSubsystem::Find(const UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	const TArray<UHOLSiteBoardSubsystem*> Boards = World->GetSubsystemArrayCopy<UHOLSiteBoardSubsystem>();
	return Boards.Num() > 0 ? Boards[0] : nullptr;
}

FHOLSiteState UHOLSiteBoardSubsystem::StepSite(UHOLSiteBoardSubsystem* Board, const FHOLSiteState& State, const TArray<int32>& Takers,
	const TArray<int32>& Disturbers, const TArray<int32>& TeamBySlot, const int32 HoldSteps, const int32 DecaySteps)
{
	if (Board)
	{
		return Board->StepSiteNow(State, Takers, Disturbers, TeamBySlot, HoldSteps, DecaySteps);
	}
	return HOLSite::Step(State, Takers, Disturbers, TeamBySlot, HoldSteps, DecaySteps);
}

FHOLSiteVictoryState UHOLSiteBoardSubsystem::VictoryStep(UHOLSiteBoardSubsystem* Board, const FHOLSiteVictoryState& State, const TArray<int32>& OwnerBySite,
	const TArray<int32>& ContestedBySite, const TArray<int32>& TeamBySlot, const int32 VictorySteps)
{
	if (Board)
	{
		return Board->VictoryStepNow(State, OwnerBySite, ContestedBySite, TeamBySlot, VictorySteps);
	}
	return HOLSite::VictoryStep(State, OwnerBySite, ContestedBySite, TeamBySlot, VictorySteps);
}

FHOLSiteState UHOLSiteBoardSubsystem::StepSiteNow(const FHOLSiteState& State, const TArray<int32>& Takers, const TArray<int32>& Disturbers,
	const TArray<int32>& TeamBySlot, const int32 HoldSteps, const int32 DecaySteps)
{
	return StepSiteRule(State, Takers, Disturbers, TeamBySlot, HoldSteps, DecaySteps);
}

FHOLSiteVictoryState UHOLSiteBoardSubsystem::VictoryStepNow(const FHOLSiteVictoryState& State, const TArray<int32>& OwnerBySite,
	const TArray<int32>& ContestedBySite, const TArray<int32>& TeamBySlot, const int32 VictorySteps)
{
	return VictoryStepRule(State, OwnerBySite, ContestedBySite, TeamBySlot, VictorySteps);
}

bool UHOLSiteBoardSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Never the abstract C++ class, nor a script class replaced by a reload (as UHOLUnitTierSubsystem).
	if (IsUnreachable() || GetClass()->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists))
	{
		return false;
	}
	return Super::ShouldCreateSubsystem(Outer);
}

bool UHOLSiteBoardSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

FHOLSiteState UHOLSiteBoardSubsystem::StepSiteRule_Implementation(const FHOLSiteState& State, const TArray<int32>& Takers, const TArray<int32>& Disturbers,
	const TArray<int32>& TeamBySlot, const int32 HoldSteps, const int32 DecaySteps)
{
	return HOLSite::Step(State, Takers, Disturbers, TeamBySlot, HoldSteps, DecaySteps);
}

FHOLSiteVictoryState UHOLSiteBoardSubsystem::VictoryStepRule_Implementation(const FHOLSiteVictoryState& State, const TArray<int32>& OwnerBySite,
	const TArray<int32>& ContestedBySite, const TArray<int32>& TeamBySlot, const int32 VictorySteps)
{
	return HOLSite::VictoryStep(State, OwnerBySite, ContestedBySite, TeamBySlot, VictorySteps);
}
