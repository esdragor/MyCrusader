// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Mass/EntityHandle.h"
#include "Subsystems/WorldSubsystem.h"
#include "MCStressTestSubsystem.generated.h"

class UMCUnitSubsystem;

/** Stress test scenarios (D190, § 16.6). */
enum class EMCStressScenario : uint8
{
	S1_Idle,	// units standing still
	S2_Moving,	// units walking on the navmesh
};

/**
 * Load tests launched from the console on the StressTest map:
 *   mc.StressTest S1|S2 [Count=2000] [Seconds=20] [quit]
 *   mc.StressTest.Stop
 * Measures the fixed-step simulation time, the frame rate and the memory, and checks them against D218.
 * Since M1 the units are created by the unit subsystem (D224) and S2 walks by move orders (D229): this
 * subsystem orders units, it no longer knows how a unit is built.
 */
UCLASS()
class MYCRUSADER_API UMCStressTestSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	void StartTest(EMCStressScenario InScenario, int32 InCount, float InMeasureSeconds, bool bInQuitWhenDone);
	void StopTest();

	// UTickableWorldSubsystem
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Deinitialize() override;

private:
	enum class EPhase : uint8 { Idle, WarmingUp, Measuring };

	void SpawnUnits();
	void DestroyUnits();
	void PlaceViewer() const;
	void BeginMeasure();
	void FinishMeasure();
	/** S2 (D229): orders a new destination to the units that arrived, within a per-frame budget. */
	void KeepUnitsWalking();
	TArray<FMassEntityHandle> GetUnitEntities() const;
	TArray<FVector> GetUnitLocations() const;
	UMCUnitSubsystem* GetUnitSubsystem() const;
	void LogRepresentationSummary() const;
	/** Mass' movement action and our own unit state (D226), side by side in the report. */
	void LogUnitStateSummary() const;
	/** Counts the units and checks the identifier table against the fragments (D223). */
	void LogUnitRegistrySummary() const;

	/** Identifiers of the units of this run (D223): the stress test never holds Mass entities itself. */
	TArray<uint32> UnitIds;
	FName UnitTypeName;

	TArray<float> FrameTimes;
	/** Sums over the measured frames, to tell which of game thread, render thread or GPU bounds the frame rate. */
	double GameThreadMsSum = 0.0;
	double RenderThreadMsSum = 0.0;
	double GpuMsSum = 0.0;
	bool bScreenshotTaken = false;
	/** Unit locations when the measure starts, to check that S2 units really move. */
	TArray<FVector> LocationsAtMeasureStart;
	/** Move orders issued during the run: shows that S2 walks by orders and not by a test behaviour (D229). */
	int32 NumOrdersIssued = 0;
	/** Step the last order pass belongs to: orders are applied at the fixed step, not every frame (D216, D219). */
	int64 LastOrderStepIndex = -1;

	EMCStressScenario Scenario = EMCStressScenario::S1_Idle;
	EPhase Phase = EPhase::Idle;
	int32 Count = 0;
	float MeasureSeconds = 0.f;
	float PhaseElapsed = 0.f;
	bool bQuitWhenDone = false;
	/** The run asked to quit: its units are destroyed at the tail of the next step (D224) before leaving. */
	bool bQuitPending = false;
	float QuitDelayElapsed = 0.f;
	uint64 UsedPhysicalBeforeSpawn = 0;
};
