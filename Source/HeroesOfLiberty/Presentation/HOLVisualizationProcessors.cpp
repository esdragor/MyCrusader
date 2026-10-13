// Copyright Epic Games, Inc. All Rights Reserved.

#include "Presentation/HOLVisualizationProcessors.h"

#include "MassCommonTypes.h"
#include "Simulation/HOLSimulationFragments.h"

//----------------------------------------------------------------------//
// UHOLVisualizationLODProcessor
//----------------------------------------------------------------------//

UHOLVisualizationLODProcessor::UHOLVisualizationLODProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::LOD;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::LODCollector);
}

void UHOLVisualizationLODProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::ConfigureQueries(EntityManager);
	CloseEntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
	CloseEntityAdjustDistanceQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
	FarEntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
	DebugEntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
}

//----------------------------------------------------------------------//
// UHOLVisualizationProcessor
//----------------------------------------------------------------------//

UHOLVisualizationProcessor::UHOLVisualizationProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::LOD);
	bRequiresGameThreadExecution = true;
}

void UHOLVisualizationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::ConfigureQueries(EntityManager);
	EntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
}
