// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLDeployProcessor.h"

#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "Siege/HOLDeployRules.h"
#include "Simulation/HOLCombatRules.h"

UHOLDeployProcessor::UHOLDeployProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	// Before the combat of the step: a unit that has just deployed fires in this step's combat.
	ExecutionOrder.ExecuteBefore.Add(TEXT("HOLCombatProcessor"));
}

void UHOLDeployProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FHOLDeployFragment>(EMassFragmentAccess::ReadWrite);
}

void UHOLDeployProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// Red-test stub: no behaviour yet.
}
