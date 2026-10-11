// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Mass/EntityHandle.h"
#include "Subsystems/WorldSubsystem.h"
#include "MCStressTestSubsystem.generated.h"

class UMassEntityTraitBase;
struct FMassEntityConfig;

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
	void LogRepresentationSummary() const;
	/** Counts by movement action: shows whether the units are walking or standing (M1). */
	void LogMovementActionSummary() const;
	TArray<FVector> GetUnitLocations() const;
	const FMassEntityConfig& GetOrCreateConfig(bool bWander);

	TSharedPtr<FMassEntityConfig> IdleConfig;
	TSharedPtr<FMassEntityConfig> WanderConfig;
	/** Keeps the configs' traits alive (FMassEntityConfig is held outside of a UPROPERTY). */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMassEntityTraitBase>> Traits;

	TArray<FMassEntityHandle> Units;
	TArray<float> FrameTimes;
	/** Sums over the measured frames, to tell which of game thread, render thread or GPU bounds the frame rate. */
	double GameThreadMsSum = 0.0;
	double RenderThreadMsSum = 0.0;
	double GpuMsSum = 0.0;
	bool bScreenshotTaken = false;
	/** Unit locations when the measure starts, to check that S2 units really move. */
	TArray<FVector> LocationsAtMeasureStart;

	EMCStressScenario Scenario = EMCStressScenario::S1_Idle;
	EPhase Phase = EPhase::Idle;
	int32 Count = 0;
	float MeasureSeconds = 0.f;
	float PhaseElapsed = 0.f;
	bool bQuitWhenDone = false;
	uint64 UsedPhysicalBeforeSpawn = 0;
};
