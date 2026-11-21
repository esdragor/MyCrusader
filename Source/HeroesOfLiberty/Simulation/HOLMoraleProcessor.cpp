// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLMoraleProcessor.h"

#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLMoraleRules.h"
#include "Simulation/HOLSimulationFragments.h"

UHOLMoraleProcessor::UHOLMoraleProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	// Before the combat of the step: the combat reads the levers as the time of this step has left them.
	ExecutionOrder.ExecuteBefore.Add(TEXT("HOLCombatProcessor"));
}

void UHOLMoraleProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FHOLMoraleFragment>(EMassFragmentAccess::ReadWrite);
}

void UHOLMoraleProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// Stub (red test first): the rule is not applied yet.
}
