// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MCSimulationSettings.generated.h"

class UMassProcessor;
class UDataTable;

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

	/**
	 * Unit type table (D221): one row per type, imported from Tools/Data/UnitTypes.csv by
	 * Tools/Editor/make_unit_types.py (D227). Nothing creates a unit without it.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Units")
	TSoftObjectPtr<UDataTable> UnitTypesTable;

	/**
	 * Units created per fixed step at most (D224): the births of one request are spread over the following
	 * steps. 0 = no budget, the whole queue in one step (a stress test measures a representative peak).
	 */
	UPROPERTY(config, EditAnywhere, Category = "Units", meta = (ClampMin = "0"))
	int32 MaxUnitBirthsPerStep = 500;

	/** Distance (cm) under which a unit has reached the goal of its move order and stands down (D226). */
	UPROPERTY(config, EditAnywhere, Category = "Units", meta = (ClampMin = "1", ForceUnits = "cm"))
	float OrderArrivalTolerance = 50.f;

	/** A move order's destination is looked for on the navmesh within this extent (cm) around it (D228). */
	UPROPERTY(config, EditAnywhere, Category = "Units", meta = (ClampMin = "1", ForceUnits = "cm"))
	float OrderTargetProjectionExtent = 1000.f;

	/**
	 * Steps an avoidance impulse is re-applied on after the solver computed it (D230), the total capped at the
	 * type's MaxAcceleration. The solver runs one step out of ThrottledProcessorStepInterval and Mass resets the
	 * force after applying it, so keeping the impulse is what separates the units of a crowd: S2 overlapping pairs
	 * 102 -> 68 with it against 173 -> 120 without (2 000 units, fixed seed, step-driven runs; the 137 -> 58 first
	 * given here compared unrepeatable runs of a faulty hold). Two units crossing head-on or at right angles still
	 * walk through each other (scenario C, D230). Console override: mc.Units.AvoidanceHold (negative = this value).
	 */
	UPROPERTY(config, EditAnywhere, Category = "Units", meta = (ClampMin = "0"))
	int32 AvoidanceHoldSteps = 5;

	/**
	 * Factor applied to the kept impulse after each re-application (D230): 1 keeps it as it was until the next
	 * solver run, 0 re-applies it once. Console override: mc.Units.AvoidanceHoldDecay (negative = this value).
	 */
	UPROPERTY(config, EditAnywhere, Category = "Units", meta = (ClampMin = "0", ClampMax = "1"))
	float AvoidanceHoldDecay = 1.f;

	float GetStepSeconds() const { return 1.f / static_cast<float>(SimulationRateHz); }
};
