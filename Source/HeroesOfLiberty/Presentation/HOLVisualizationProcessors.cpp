// Copyright Epic Games, Inc. All Rights Reserved.

#include "Presentation/HOLVisualizationProcessors.h"

#include "MassCommonTypes.h"
#include "Presentation/HOLPresentationFragments.h"
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
	// Units and bodies (J7c, bodies kept 30 s): Any matches an archetype holding at least one of the two tags.
	for (FMassEntityQuery* Query : { &CloseEntityQuery, &CloseEntityAdjustDistanceQuery, &FarEntityQuery, &DebugEntityQuery })
	{
		Query->AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::Any);
		Query->AddTagRequirement<FHOLCorpseTag>(EMassFragmentPresence::Any);
	}
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
	// Units and bodies (J7c, bodies kept 30 s).
	EntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::Any);
	EntityQuery.AddTagRequirement<FHOLCorpseTag>(EMassFragmentPresence::Any);
}
