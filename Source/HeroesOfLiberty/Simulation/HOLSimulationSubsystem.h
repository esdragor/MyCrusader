// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessingTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLSimulationSubsystem.generated.h"

/** Timing of one pipeline processor over the measured steps (stress tests, D218, D220). */
struct FHOLSimulationProcessorStats
{
	FString Name;
	double TotalMs = 0.0;
	/** Steps the processor actually ran: a throttled one runs less often than the simulation does. */
	int32 Runs = 0;
	/** One run out of this many steps at the last step (D220: 1 = every step), for the report only. */
	int32 StepInterval = 1;

	double GetAverageMs() const { return Runs > 0 ? TotalMs / Runs : 0.0; }
};

/** Timing of the fixed steps since the last reset (stress tests, D218). */
struct FHOLSimulationStepStats
{
	int32 NumSteps = 0;
	double TotalMs = 0.0;
	double MaxMs = 0.0;
	/** Frames that hit MaxStepsPerFrame and dropped time. */
	int32 NumDroppedFrames = 0;
	/** Per processor, in pipeline order. */
	TArray<FHOLSimulationProcessorStats> Processors;
	/**
	 * Births and destructions (D224): they are not processors, they run at the head and the tail of the step
	 * because Mass forbids the synchronous entity API inside a processor's execution scope.
	 */
	double TotalBirthMs = 0.0;
	int32 NumUnitsBorn = 0;
	double TotalDestructionMs = 0.0;
	int32 NumUnitsDestroyed = 0;

	double GetAverageMs() const { return NumSteps > 0 ? TotalMs / NumSteps : 0.0; }
	double GetAverageBirthMs() const { return NumSteps > 0 ? TotalBirthMs / NumSteps : 0.0; }
	double GetAverageDestructionMs() const { return NumSteps > 0 ? TotalDestructionMs / NumSteps : 0.0; }
};

/** Head of a fixed step, before births and processors: what is issued here is applied by this very step. */
DECLARE_MULTICAST_DELEGATE_OneParam(FHOLOnSimulationStepStarted, int64 /*StepIndex*/);

/**
 * Runs the authoritative simulation at a fixed step (D217, D219): at the start of Mass' PrePhysics phase,
 * the accumulator runs 0 to N steps of the simulation pipeline; per-frame processors run afterwards and
 * interpolate with GetInterpolationAlpha().
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLSimulationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Fraction of a step elapsed since the last fixed step, in [0, 1]. */
	float GetInterpolationAlpha() const { return StepSeconds > 0.f ? FMath::Clamp(Accumulator / StepSeconds, 0.f, 1.f) : 1.f; }

	float GetStepSeconds() const { return StepSeconds; }
	int64 GetStepIndex() const { return StepIndex; }

	/** Fires at the head of every fixed step: a stress test drives its orders and its phases by steps, not by frames. */
	FHOLOnSimulationStepStarted& GetOnStepStarted() { return OnStepStarted; }

	const FHOLSimulationStepStats& GetStepStats() const { return StepStats; }
	void ResetStepStats() { StepStats = FHOLSimulationStepStats(); }

	/**
	 * Whether the throttled processors (the avoidance solvers, D220) run during the step being built (D230). Set
	 * by RunStep from the same decision that runs them, so a reader cannot disagree with the pipeline.
	 */
	bool AreThrottledProcessorsRunningThisStep() const { return bThrottledProcessorsRunThisStep; }
	/** One run out of this many steps for the throttled processors, override of D220 included. */
	int32 GetThrottledStepInterval() const;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

private:
	void OnPrePhysicsPhaseStarted(const float DeltaSeconds);
	void RunStep();
	/** Fills StepStats.Processors with the pipeline's processors, after a reset of the statistics. */
	void RegisterProcessorStats(const TConstArrayView<TObjectPtr<UMassProcessor>> Processors);
	/** Whether this processor is in the throttled processor list (D220). */
	bool IsThrottled(const UMassProcessor& Processor) const;

	/** Simulation processors in dependency order, run at each fixed step. */
	UPROPERTY(Transient)
	FMassRuntimePipeline StepPipeline;

	/** Processors that only need one step out of ThrottledStepInterval (D220). */
	TArray<TSubclassOf<UMassProcessor>> ThrottledProcessors;
	int32 ThrottledStepInterval = 1;
	/** Per processor of StepPipeline, in its order: whether it is throttled. Part of the pipeline, not of the stats. */
	TArray<bool> ProcessorThrottled;
	/** Decided at the head of each step, read by UHOLAvoidanceHoldProcessor (D230). */
	bool bThrottledProcessorsRunThisStep = false;

	TSharedPtr<struct FMassEntityManager> EntityManager;
	FDelegateHandle PhaseStartedHandle;

	float StepSeconds = 0.f;
	int32 MaxStepsPerFrame = 1;
	float Accumulator = 0.f;
	int64 StepIndex = 0;
	FHOLSimulationStepStats StepStats;
	FHOLOnSimulationStepStarted OnStepStarted;
};
