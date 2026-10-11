// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessingTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "MCSimulationSubsystem.generated.h"

/** Timing of the fixed steps since the last reset (stress tests, D218). */
struct FMCSimulationStepStats
{
	int32 NumSteps = 0;
	double TotalMs = 0.0;
	double MaxMs = 0.0;
	/** Frames that hit MaxStepsPerFrame and dropped time. */
	int32 NumDroppedFrames = 0;
	/** Total time per processor, in pipeline order. */
	TArray<TPair<FString, double>> ProcessorTotalMs;

	double GetAverageMs() const { return NumSteps > 0 ? TotalMs / NumSteps : 0.0; }
};

/**
 * Runs the authoritative simulation at a fixed step (D217, D219): at the start of Mass' PrePhysics phase,
 * the accumulator runs 0 to N steps of the simulation pipeline; per-frame processors run afterwards and
 * interpolate with GetInterpolationAlpha().
 */
UCLASS()
class MYCRUSADER_API UMCSimulationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Fraction of a step elapsed since the last fixed step, in [0, 1]. */
	float GetInterpolationAlpha() const { return StepSeconds > 0.f ? FMath::Clamp(Accumulator / StepSeconds, 0.f, 1.f) : 1.f; }

	float GetStepSeconds() const { return StepSeconds; }
	int64 GetStepIndex() const { return StepIndex; }

	const FMCSimulationStepStats& GetStepStats() const { return StepStats; }
	void ResetStepStats() { StepStats = FMCSimulationStepStats(); }

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

private:
	void OnPrePhysicsPhaseStarted(const float DeltaSeconds);
	void RunStep();

	/** Simulation processors in dependency order, run at each fixed step. */
	UPROPERTY(Transient)
	FMassRuntimePipeline StepPipeline;

	TSharedPtr<struct FMassEntityManager> EntityManager;
	FDelegateHandle PhaseStartedHandle;

	float StepSeconds = 0.f;
	int32 MaxStepsPerFrame = 1;
	float Accumulator = 0.f;
	int64 StepIndex = 0;
	FMCSimulationStepStats StepStats;
};
