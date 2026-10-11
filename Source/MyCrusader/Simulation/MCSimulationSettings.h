// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MCSimulationSettings.generated.h"

class UMassProcessor;

/**
 * Fixed-step simulation settings (D217, D219).
 * Every processor listed in SimulationProcessors must have bAutoRegisterWithProcessingPhases=False
 * (Config/DefaultMass.ini for engine processors, constructor for ours): they run only at the fixed step.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "MyCrusader Simulation"))
class MYCRUSADER_API UMCSimulationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Simulation steps per second (D217: 20 Hz). */
	UPROPERTY(config, EditAnywhere, Category = "Simulation", meta = (ClampMin = "1", ClampMax = "120"))
	int32 SimulationRateHz = 20;

	/** Catch-up cap: steps run in one frame at most; the late time beyond it is dropped. */
	UPROPERTY(config, EditAnywhere, Category = "Simulation", meta = (ClampMin = "1"))
	int32 MaxStepsPerFrame = 4;

	/** Processors run at each fixed step, ordered by their Mass execution order. */
	UPROPERTY(config, EditAnywhere, Category = "Simulation")
	TArray<TSubclassOf<UMassProcessor>> SimulationProcessors;

	/**
	 * Processors of SimulationProcessors that only need one step out of ThrottledProcessorStepInterval (D220).
	 * The Mass avoidance solver is nearly the whole simulation cost at 2 000 units (about 4 µs per unit, single
	 * threaded) and its result hardly changes within a 20 Hz step.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Simulation")
	TArray<TSubclassOf<UMassProcessor>> ThrottledProcessors;

	/** Steps between two runs of the throttled processors: 4 = 5 Hz at the 20 Hz step (D220). */
	UPROPERTY(config, EditAnywhere, Category = "Simulation", meta = (ClampMin = "1"))
	int32 ThrottledProcessorStepInterval = 4;

	float GetStepSeconds() const { return 1.f / static_cast<float>(SimulationRateHz); }
};
