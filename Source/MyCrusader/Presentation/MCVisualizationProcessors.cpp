// Copyright Epic Games, Inc. All Rights Reserved.

#include "Presentation/MCVisualizationProcessors.h"

#include "MassCommonTypes.h"
#include "Simulation/MCSimulationFragments.h"

//----------------------------------------------------------------------//
// UMCVisualizationLODProcessor
//----------------------------------------------------------------------//

UMCVisualizationLODProcessor::UMCVisualizationLODProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::LOD;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::LODCollector);
}

void UMCVisualizationLODProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::ConfigureQueries(EntityManager);
	CloseEntityQuery.AddTagRequirement<FMCLightUnitTag>(EMassFragmentPresence::All);
	CloseEntityAdjustDistanceQuery.AddTagRequirement<FMCLightUnitTag>(EMassFragmentPresence::All);
	FarEntityQuery.AddTagRequirement<FMCLightUnitTag>(EMassFragmentPresence::All);
	DebugEntityQuery.AddTagRequirement<FMCLightUnitTag>(EMassFragmentPresence::All);
}

//----------------------------------------------------------------------//
// UMCVisualizationProcessor
//----------------------------------------------------------------------//

UMCVisualizationProcessor::UMCVisualizationProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::LOD);
	bRequiresGameThreadExecution = true;
}

void UMCVisualizationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::ConfigureQueries(EntityManager);
	EntityQuery.AddTagRequirement<FMCLightUnitTag>(EMassFragmentPresence::All);
}
