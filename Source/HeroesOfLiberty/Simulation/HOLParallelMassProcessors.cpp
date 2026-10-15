// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLParallelMassProcessors.h"

#include "HAL/IConsoleManager.h"
#include "MassArchetypeTypes.h"
#include "MassEntityQuery.h"
#include "MassExecutionContext.h"

namespace UE::HeroesOfLiberty::Simulation
{
	int32 ParallelMass = 1;
	FAutoConsoleVariableRef CVarParallelMass(TEXT("hol.Simulation.ParallelMass"), ParallelMass,
		TEXT("1: Mass' moving avoidance, steering and orientation run their chunks in parallel (D302); 0: one after the other, as Mass does, for A / B measures"));

	/** Runs Execute (a Mass processor's own), each of its queries' chunks in parallel; see HOLParallelMassProcessors.h. */
	void ExecuteChunksInParallel(FMassExecutionContext& Context, const TFunctionRef<void(FMassExecutionContext&)> Execute)
	{
		if (ParallelMass == 0)
		{
			Execute(Context);
			return;
		}

		// Each query's per-chunk function, in the order Execute runs the queries (the same for every chunk of a run).
		struct FQueryRun
		{
			FMassEntityQuery* Query = nullptr;
			FMassExecuteFunction Function;
		};
		TArray<FQueryRun, TInlineAllocator<2>> Runs;
		{
			FMassExecutionContext CollectContext(Context);
			CollectContext.SetChunkQueueFunc([&Runs](FMassChunkProcessingQueueParams&& Params)
			{
				if (Params.SourceQuery && Params.ExecuteFunction && (Runs.IsEmpty() || Runs.Last().Query != Params.SourceQuery))
				{
					Runs.Add({ Params.SourceQuery, *Params.ExecuteFunction });
				}
			});
			Execute(CollectContext);
		}
		for (FQueryRun& Run : Runs)
		{
			Run.Query->ParallelForEachEntityChunk(Context, Run.Function);
		}
	}
}

UHOLParallelMovingAvoidanceProcessor::UHOLParallelMovingAvoidanceProcessor()
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone); // authoritative simulation (D231)
}

void UHOLParallelMovingAvoidanceProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UE::HeroesOfLiberty::Simulation::ExecuteChunksInParallel(Context, [this, &EntityManager](FMassExecutionContext& RunContext)
	{
		Super::Execute(EntityManager, RunContext);
	});
}

UHOLParallelSteerToMoveTargetProcessor::UHOLParallelSteerToMoveTargetProcessor()
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone); // authoritative simulation (D231)
}

void UHOLParallelSteerToMoveTargetProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UE::HeroesOfLiberty::Simulation::ExecuteChunksInParallel(Context, [this, &EntityManager](FMassExecutionContext& RunContext)
	{
		Super::Execute(EntityManager, RunContext);
	});
}
