// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Mass/EntityHandle.h"
#include "Match/HOLResources.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLStressTestSubsystem.generated.h"

class AHOLBuilding;
class UHOLUnitSubsystem;

/** Stress test scenarios (D190, § 16.6). */
enum class EHOLStressScenario : uint8
{
	S1_Idle,	// units standing still
	S2_Moving,	// units walking on the navmesh
	Crossing,	// two units ordered through the same point at hol.StressTest.CrossingAngle: does the avoidance make them step aside?
	S7_Gathering,	// Peasants gathering wood for their players' centres (J5, D299): round trips, paths, used-up trees
};

/**
 * Load tests launched from the console on the StressTest map:
 *   hol.StressTest S1|S2|C|S7 [Count=2000] [Seconds=20] [quit]
 *   hol.StressTest.Stop
 * Measures the fixed-step simulation time, the frame rate and the memory, and checks them against D218.
 * Since M1 the units are created by the unit subsystem (D224) and S2 walks by move orders (D229): this
 * subsystem orders units, it no longer knows how a unit is built.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLStressTestSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	void StartTest(EHOLStressScenario InScenario, int32 InCount, float InMeasureSeconds, bool bInQuitWhenDone);
	void StopTest();

	// UTickableWorldSubsystem
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Deinitialize() override;

private:
	enum class EPhase : uint8 { Idle, WarmingUp, Measuring };

	/**
	 * Head of every fixed step: phases, orders and the crossing's watch advance by steps, not by frames, so two
	 * runs of a scenario see the same orders at the same steps whatever the frame rate does (D230).
	 */
	void OnSimulationStep(int64 StepIndex);
	void SpawnUnits();
	void DestroyUnits();
	void PlaceViewer() const;
	void BeginMeasure();
	void FinishMeasure();
	/** S2 (D229): orders a new destination to the units that arrived, within a per-step budget. */
	void KeepUnitsWalking();
	/** Crossing: the two units walk back to their start once they stand, and the closest they came is recorded. */
	void MaintainCrossing();
	/** Closest distance the two crossing units came to each other, for the avoidance check (D228). */
	void LogCrossingSummary() const;
	TArray<FMassEntityHandle> GetUnitEntities() const;
	TArray<FVector> GetUnitLocations() const;
	/** Locations (unit feet) inside the player's view (D237: the game view leaves part of the crowd out of frame). */
	int32 CountLocationsInFrame(const TArray<FVector>& Locations) const;
	UHOLUnitSubsystem* GetUnitSubsystem() const;
	void LogRepresentationSummary() const;
	/** Mass' movement action and our own unit state (D226), side by side in the report. */
	void LogUnitStateSummary() const;
	/** Counts the units and checks the identifier table against the fragments (D223). */
	void LogUnitRegistrySummary() const;
	/** Constants the units really carry (D222): catches a shared fragment both types would share. */
	void LogUnitConstantsSummary() const;
	/** Interpenetration count and smallest gap between two bodies, over every living unit. */
	void CountInterpenetration(int32& OutNumPairs, double& OutSmallestGap) const;
	/** Interpenetration before and after the walk: a plateau means the avoidance is not doing its job (D228). */
	void LogAvoidanceSummary() const;

	/**
	 * S7 (D299): each player's main centre, lumber camps and a field of lone trees behind it, placed by code: the map
	 * StressTest stays as S1 and S2 measure it.
	 */
	void SetUpGathering();
	/** S7: once born, each player's Peasants are sent to its trees, four per tree (as a player spreads its groups). */
	void OrderGathering();
	void TearDownGathering();
	/** S7: what was unloaded, trees used up, paths asked for, where the gatherers are. */
	void LogGatheringSummary() const;
	/** S7: where a player's Peasants are born, between its centre and the middle of the map. */
	FVector GetGatheringSpawnPoint(int32 Player) const;
	int32 GetNumPlayers() const;

	/** Identifiers of the units of this run (D223): the stress test never holds Mass entities itself. */
	TArray<uint32> UnitIds;
	FName UnitTypeName;

	TArray<float> FrameTimes;
	/** Sums over the measured frames, to tell which of game thread, render thread or GPU bounds the frame rate. */
	double GameThreadMsSum = 0.0;
	double RenderThreadMsSum = 0.0;
	double GpuMsSum = 0.0;
	/** Sum of the RHI draw calls over the measured frames (D236). */
	double DrawCallsSum = 0.0;
	bool bScreenshotTaken = false;
	/** Unit locations when the measure starts, to check that S2 units really move. */
	TArray<FVector> LocationsAtMeasureStart;
	/** Interpenetrating pairs and smallest gap between two bodies when the measure starts (report calibration). */
	int32 InterpenetrationAtMeasureStart = 0;
	double SmallestGapAtMeasureStart = 0.0;
	/** Move orders issued during the run: shows that S2 walks by orders and not by a test behaviour (D229). */
	int32 NumOrdersIssued = 0;
	/** Crossing: how close the two units came to each other (cm), and how many swaps were ordered. */
	double CrossingClosestDistance = TNumericLimits<double>::Max();
	int32 CrossingSwaps = 0;
	/** Crossing: the two units' birth positions (one end of each path, the other being its mirror). */
	FVector CrossingStart[2] = { FVector::ZeroVector, FVector::ZeroVector };
	bool bCrossingStartRead = false;
	bool bCrossingFarEnd = true;
	/** Destination draw of S2: a fixed seed keeps two runs comparable (D230). */
	FRandomStream OrderRandom{ 1066 };

	/** S7: the deposits it made (centres and camps), each player's trees, whether the Peasants were sent, and the counts
	 *  when the measure began. */
	TArray<TWeakObjectPtr<AHOLBuilding>> GatheringCentres;
	TArray<TArray<int32>> GatheringTrees;
	bool bGatheringOrdered = false;
	FHOLResources UnloadedAtMeasureStart;
	int32 StandingAtMeasureStart = 0;
	int64 PathsAtMeasureStart = 0;
	int64 PathsReusedAtMeasureStart = 0;
	/** Bodies put back out of a footprint, and walkers given a new path for it (D310), when the measure began. */
	int64 PushedOutAtMeasureStart = 0;
	int64 RepathsAtMeasureStart = 0;
	/** UHOLGatherProcessor::FCounters when the measure began (as plain numbers: the header stays free of the processor). */
	int64 GatherCountersAtMeasureStart[6] = {};

	EHOLStressScenario Scenario = EHOLStressScenario::S1_Idle;
	EPhase Phase = EPhase::Idle;
	int32 Count = 0;
	float MeasureSeconds = 0.f;
	/** Fixed steps of the current phase, and the length of each phase in steps (from seconds at the step rate). */
	int32 PhaseSteps = 0;
	int32 WarmUpSteps = 0;
	int32 MeasureSteps = 0;
	FDelegateHandle StepStartedHandle;
	bool bQuitWhenDone = false;
	/** The run asked to quit: its units are destroyed at the tail of the next step (D224) before leaving. */
	bool bQuitPending = false;
	float QuitDelayElapsed = 0.f;
	uint64 UsedPhysicalBeforeSpawn = 0;
};
