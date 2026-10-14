// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLSimulationSubsystem.h"

#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformTime.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "MassProcessor.h"
#include "MassProcessorDependencySolver.h"
#include "MassSimulationSubsystem.h"
#include "HeroesOfLibertyLog.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitSubsystem.h"

namespace UE::HeroesOfLiberty::Simulation
{
	/** Stress test knob (D220): overrides the settings' interval when > 0. */
	int32 ThrottledStepIntervalOverride = 0;
	FAutoConsoleVariableRef CVarThrottledStepInterval(TEXT("hol.Simulation.ThrottledStepInterval"), ThrottledStepIntervalOverride,
		TEXT("Steps between two runs of the throttled processors, 0 = UHOLSimulationSettings value"));
}

bool UHOLSimulationSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHOLSimulationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
		UE_LOG(LogHOLSimulation, Error, TEXT("Mass subsystems missing: fixed-step simulation disabled"));
		return;
	}

	const UHOLSimulationSettings* Settings = GetDefault<UHOLSimulationSettings>();
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
			UE_LOG(LogHOLSimulation, Error, TEXT("%s still auto-registers with Mass phases: set bAutoRegisterWithProcessingPhases=False in DefaultMass.ini"), *ProcessorClass->GetName());
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
			ProcessorThrottled.Add(IsThrottled(*Info.Processor));
		}
	}

	PhaseStartedHandle = MassSimulation->GetOnProcessingPhaseStarted(EMassProcessingPhase::PrePhysics)
		.AddUObject(this, &UHOLSimulationSubsystem::OnPrePhysicsPhaseStarted);

	FString Pipeline;
	for (const UMassProcessor* Processor : StepPipeline.GetProcessors())
	{
		Pipeline += FString::Printf(TEXT(" %s"), *Processor->GetProcessorName());
	}
	UE_LOG(LogHOLSimulation, Log, TEXT("Fixed-step simulation at %d Hz, %d of %d processors in order:%s"),
		Settings->SimulationRateHz, StepPipeline.Num(), Processors.Num(), *Pipeline);
	if (StepPipeline.Num() != Processors.Num())
	{
		UE_LOG(LogHOLSimulation, Error, TEXT("Some simulation processors were dropped by the dependency solver (%d sorted nodes)"), SortedProcessors.Num());
	}
}

void UHOLSimulationSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		if (UMassSimulationSubsystem* MassSimulation = World->GetSubsystem<UMassSimulationSubsystem>())
		{
			MassSimulation->GetOnProcessingPhaseStarted(EMassProcessingPhase::PrePhysics).Remove(PhaseStartedHandle);
		}
	}
	PhaseStartedHandle.Reset();
	OnStepStarted.Clear();
	OnStepEnded.Clear();
	StepPipeline.Reset();
	ProcessorThrottled.Reset();
	EntityManager.Reset();

	Super::Deinitialize();
}

void UHOLSimulationSubsystem::OnPrePhysicsPhaseStarted(const float DeltaSeconds)
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

void UHOLSimulationSubsystem::RunStep()
{
	// Listeners (the stress test) run before the step's chrono: their orders belong to this step, their cost does not.
	OnStepStarted.Broadcast(StepIndex);

	const double StartSeconds = FPlatformTime::Seconds();

	// Births are served at the head of the step and destructions applied at its tail (D224): neither is a
	// processor, because Mass forbids the synchronous entity API inside a processor's execution scope. The
	// step is the unit of the simulation (D219), so they belong to it.
	UHOLUnitSubsystem* Units = GetWorld() ? GetWorld()->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	const double BirthStartSeconds = FPlatformTime::Seconds();
	const int32 NumUnitsBorn = Units ? Units->ServeBirthRequests() : 0;
	const double BirthMs = (FPlatformTime::Seconds() - BirthStartSeconds) * 1000.0;

	{
		FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
		const TConstArrayView<TObjectPtr<UMassProcessor>> Processors = StepPipeline.GetProcessors();
		RegisterProcessorStats(Processors);
		// One decision per step (D220), read live so the console override applies at once, and published for the
		// processors that depend on it (D230).
		const int32 ThrottledInterval = GetThrottledStepInterval();
		bThrottledProcessorsRunThisStep = (StepIndex % ThrottledInterval) == 0;
		// Run one by one to time each processor (stress test breakdown). A throttled processor keeps its own
		// runs and total (D220), so its breakdown stays a per-run cost.
		for (int32 Index = 0; Index < Processors.Num(); ++Index)
		{
			FHOLSimulationProcessorStats& ProcessorStats = StepStats.Processors[Index];
			const bool bThrottled = ProcessorThrottled.IsValidIndex(Index) && ProcessorThrottled[Index];
			ProcessorStats.StepInterval = bThrottled ? ThrottledInterval : 1;
			if (bThrottled && !bThrottledProcessorsRunThisStep)
			{
				continue;
			}
			const double ProcessorStartSeconds = FPlatformTime::Seconds();
			UE::Mass::Executor::Run(*Processors[Index], ProcessingContext);
			ProcessorStats.TotalMs += (FPlatformTime::Seconds() - ProcessorStartSeconds) * 1000.0;
			++ProcessorStats.Runs;
		}
	}

	const double DestructionStartSeconds = FPlatformTime::Seconds();
	const int32 NumUnitsDestroyed = Units ? Units->FlushDestructions() : 0;
	const double DestructionMs = (FPlatformTime::Seconds() - DestructionStartSeconds) * 1000.0;

	const double StepMs = (FPlatformTime::Seconds() - StartSeconds) * 1000.0;
	++StepIndex;
	++StepStats.NumSteps;
	StepStats.TotalMs += StepMs;
	StepStats.MaxMs = FMath::Max(StepStats.MaxMs, StepMs);
	StepStats.TotalBirthMs += BirthMs;
	StepStats.NumUnitsBorn += NumUnitsBorn;
	StepStats.TotalDestructionMs += DestructionMs;
	StepStats.NumUnitsDestroyed += NumUnitsDestroyed;

	OnStepEnded.Broadcast(StepIndex);
}

void UHOLSimulationSubsystem::RegisterProcessorStats(const TConstArrayView<TObjectPtr<UMassProcessor>> Processors)
{
	if (StepStats.Processors.Num() == Processors.Num())
	{
		return;
	}
	StepStats.Processors.Reset();
	for (const UMassProcessor* Processor : Processors)
	{
		FHOLSimulationProcessorStats& Stats = StepStats.Processors.AddDefaulted_GetRef();
		Stats.Name = Processor->GetClass()->GetName();
	}
}

bool UHOLSimulationSubsystem::IsThrottled(const UMassProcessor& Processor) const
{
	for (const TSubclassOf<UMassProcessor>& Throttled : ThrottledProcessors)
	{
		if (Throttled && Processor.GetClass()->IsChildOf(Throttled.Get()))
		{
			return true;
		}
	}
	return false;
}

int32 UHOLSimulationSubsystem::GetThrottledStepInterval() const
{
	using namespace UE::HeroesOfLiberty::Simulation;
	const int32 Interval = ThrottledStepIntervalOverride > 0 ? ThrottledStepIntervalOverride : ThrottledStepInterval;
	return FMath::Max(1, Interval);
}
