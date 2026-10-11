// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/MCSimulationSubsystem.h"

#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformTime.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "MassProcessor.h"
#include "MassProcessorDependencySolver.h"
#include "MassSimulationSubsystem.h"
#include "MyCrusaderLog.h"
#include "Simulation/MCSimulationSettings.h"

namespace UE::MyCrusader::Simulation
{
	/** Stress test knob (D220): overrides the settings' interval when > 0. */
	int32 ThrottledStepIntervalOverride = 0;
	FAutoConsoleVariableRef CVarThrottledStepInterval(TEXT("mc.Simulation.ThrottledStepInterval"), ThrottledStepIntervalOverride,
		TEXT("Steps between two runs of the throttled processors, 0 = UMCSimulationSettings value"));
}

bool UMCSimulationSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UMCSimulationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// The simulation is authoritative: clients only interpolate replicated states (D29).
	if (InWorld.GetNetMode() == NM_Client)
	{
		return;
	}

	UMassEntitySubsystem* EntitySubsystem = InWorld.GetSubsystem<UMassEntitySubsystem>();
	UMassSimulationSubsystem* MassSimulation = InWorld.GetSubsystem<UMassSimulationSubsystem>();
	if (!EntitySubsystem || !MassSimulation)
	{
		UE_LOG(LogMCSimulation, Error, TEXT("Mass subsystems missing: fixed-step simulation disabled"));
		return;
	}

	const UMCSimulationSettings* Settings = GetDefault<UMCSimulationSettings>();
	StepSeconds = Settings->GetStepSeconds();
	MaxStepsPerFrame = Settings->MaxStepsPerFrame;
	ThrottledProcessors = Settings->ThrottledProcessors;
	ThrottledStepInterval = FMath::Max(1, Settings->ThrottledProcessorStepInterval);
	EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();

	TArray<UMassProcessor*> Processors;
	for (const TSubclassOf<UMassProcessor>& ProcessorClass : Settings->SimulationProcessors)
	{
		if (!ProcessorClass)
		{
			continue;
		}
		// A processor left in Mass' phases would run twice, once per frame with the frame delta.
		if (ProcessorClass->GetDefaultObject<UMassProcessor>()->ShouldAutoAddToGlobalList())
		{
			UE_LOG(LogMCSimulation, Error, TEXT("%s still auto-registers with Mass phases: set bAutoRegisterWithProcessingPhases=False in DefaultMass.ini"), *ProcessorClass->GetName());
		}
		UMassProcessor* Processor = NewObject<UMassProcessor>(this, ProcessorClass);
		Processor->CallInitialize(this, EntityManager.ToSharedRef());
		Processors.Add(Processor);
	}

	// Ordered from the processors' declared groups and requirements. Solved once, without runtime pruning:
	// at begin play no unit exists yet, so every processor would be pruned for matching no archetype.
	FMassProcessorDependencySolver Solver(Processors, /*bIsGameRuntime*/ false);
	TArray<FMassProcessorOrderInfo> SortedProcessors;
	Solver.ResolveDependencies(SortedProcessors, EntityManager);
	for (const FMassProcessorOrderInfo& Info : SortedProcessors)
	{
		if (Info.NodeType == FMassProcessorOrderInfo::EDependencyNodeType::Processor && Info.Processor)
		{
			StepPipeline.AppendProcessor(*Info.Processor);
		}
	}

	PhaseStartedHandle = MassSimulation->GetOnProcessingPhaseStarted(EMassProcessingPhase::PrePhysics)
		.AddUObject(this, &UMCSimulationSubsystem::OnPrePhysicsPhaseStarted);

	FString Pipeline;
	for (const UMassProcessor* Processor : StepPipeline.GetProcessors())
	{
		Pipeline += FString::Printf(TEXT(" %s"), *Processor->GetProcessorName());
	}
	UE_LOG(LogMCSimulation, Log, TEXT("Fixed-step simulation at %d Hz, %d of %d processors in order:%s"),
		Settings->SimulationRateHz, StepPipeline.Num(), Processors.Num(), *Pipeline);
	if (StepPipeline.Num() != Processors.Num())
	{
		UE_LOG(LogMCSimulation, Error, TEXT("Some simulation processors were dropped by the dependency solver (%d sorted nodes)"), SortedProcessors.Num());
	}
}

void UMCSimulationSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		if (UMassSimulationSubsystem* MassSimulation = World->GetSubsystem<UMassSimulationSubsystem>())
		{
			MassSimulation->GetOnProcessingPhaseStarted(EMassProcessingPhase::PrePhysics).Remove(PhaseStartedHandle);
		}
	}
	PhaseStartedHandle.Reset();
	StepPipeline.Reset();
	EntityManager.Reset();

	Super::Deinitialize();
}

void UMCSimulationSubsystem::OnPrePhysicsPhaseStarted(const float DeltaSeconds)
{
	Accumulator += DeltaSeconds;

	int32 NumSteps = 0;
	while (Accumulator >= StepSeconds && NumSteps < MaxStepsPerFrame)
	{
		RunStep();
		Accumulator -= StepSeconds;
		++NumSteps;
	}

	if (Accumulator >= StepSeconds)
	{
		// The host cannot keep up: drop the late time rather than spiral (the simulation slows down).
		Accumulator = FMath::Fmod(Accumulator, StepSeconds);
		++StepStats.NumDroppedFrames;
	}
}

void UMCSimulationSubsystem::RunStep()
{
	const double StartSeconds = FPlatformTime::Seconds();

	{
		FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
		const TConstArrayView<TObjectPtr<UMassProcessor>> Processors = StepPipeline.GetProcessors();
		RegisterProcessorStats(Processors);
		// Run one by one to time each processor (stress test breakdown). A throttled processor keeps its own
		// runs and total (D220), so its breakdown stays a per-run cost.
		for (int32 Index = 0; Index < Processors.Num(); ++Index)
		{
			FMCSimulationProcessorStats& ProcessorStats = StepStats.Processors[Index];
			const int32 StepInterval = FMath::Max(1, ProcessorStats.StepInterval);
			if (StepInterval > 1 && (StepIndex % StepInterval) != 0)
			{
				continue;
			}
			const double ProcessorStartSeconds = FPlatformTime::Seconds();
			UE::Mass::Executor::Run(*Processors[Index], ProcessingContext);
			ProcessorStats.TotalMs += (FPlatformTime::Seconds() - ProcessorStartSeconds) * 1000.0;
			++ProcessorStats.Runs;
		}
	}

	const double StepMs = (FPlatformTime::Seconds() - StartSeconds) * 1000.0;
	++StepIndex;
	++StepStats.NumSteps;
	StepStats.TotalMs += StepMs;
	StepStats.MaxMs = FMath::Max(StepStats.MaxMs, StepMs);
}

void UMCSimulationSubsystem::RegisterProcessorStats(const TConstArrayView<TObjectPtr<UMassProcessor>> Processors)
{
	if (StepStats.Processors.Num() == Processors.Num())
	{
		return;
	}
	StepStats.Processors.Reset();
	for (const UMassProcessor* Processor : Processors)
	{
		FMCSimulationProcessorStats& Stats = StepStats.Processors.AddDefaulted_GetRef();
		Stats.Name = Processor->GetClass()->GetName();
		Stats.StepInterval = GetStepInterval(*Processor);
	}
}

int32 UMCSimulationSubsystem::GetStepInterval(const UMassProcessor& Processor) const
{
	for (const TSubclassOf<UMassProcessor>& Throttled : ThrottledProcessors)
	{
		if (Throttled && Processor.GetClass()->IsChildOf(Throttled.Get()))
		{
			const int32 Interval = UE::MyCrusader::Simulation::ThrottledStepIntervalOverride > 0
				? UE::MyCrusader::Simulation::ThrottledStepIntervalOverride
				: ThrottledStepInterval;
			return FMath::Max(1, Interval);
		}
	}
	return 1;
}
