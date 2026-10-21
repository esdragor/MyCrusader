// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Commands/HOLConstructionCommand.h"
#include "Mass/EntityHandle.h"
#include "Match/HOLResources.h"
#include "Simulation/HOLSimulationProcessors.h"
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
	S3_Melee,	// two armies of Spearmen and Men-at-arms, slot 0 against slot 1, charging each other (J7): target search, melee
	S4_Mixed,	// the same with each army in thirds, Spearmen, Men-at-arms and Archers behind them (J7): melee and arrows
};

/**
 * Load tests launched from the console on the StressTest map:
 *   hol.StressTest S1|S2|C|S7|S3|S4 [Count=2000] [Seconds=20] [quit]
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
	/** S7 (D314): notes when each camp placed as a foundation is finished. */
	void WatchCampSites();
	/** S7 (D314): issues the camps' build orders drawn up by OrderGathering (at the start of the measure). */
	void OrderCampBuilding();
	void TearDownGathering();
	/** S7: what was unloaded, trees used up, paths asked for, where the gatherers are. */
	void LogGatheringSummary() const;
	/** S7: where a player's Peasants are born, between its centre and the middle of the map. */
	FVector GetGatheringSpawnPoint(int32 Player) const;
	int32 GetNumPlayers() const;

	/** S3 and S4: two armies fighting each other. */
	bool IsBattle() const;
	/** S3, S4: the unit types of one army, melee first (each army shares its count between them). */
	TArray<FName> GetBattleTypes() const;
	/** S3, S4: where a group of an army is born (one disc per type and side), and where its attack-move leads. */
	FVector GetBattleGroupCentre(int32 Side, int32 TypeIndex) const;
	/** S4: the Archers of an army are born in BattleArcherDiscs small discs side by side, a band just behind its melee. */
	FVector GetArcherDiscCentre(int32 Side, int32 Disc) const;
	/** S4: which of the Archer discs a point belongs to (by X). */
	int32 GetArcherDisc(double X) const;
	/** S3, S4: once every unit is born, each group of each army is given an attack-move through the enemy army. */
	void OrderCharge();
	/** S3, S4: the armies are joined (enough melee hits since the charge) or the longest warm-up went by. */
	bool IsBattleJoined() const;
	/** S3, S4: the units of the run are every living unit (the dead leave: the identifiers are read again). */
	void RefreshBattleUnitIds();
	/** S3, S4: units standing (not dying) per side. */
	void CountBattleAlive(int32 OutAlive[2]) const;
	/** S3, S4: survivors, hits, kills and arrows of the measure. */
	void LogBattleSummary() const;
	/** S3, S4 (J7c.4, jitter probe): each fighting unit's 2D move over the step just run, read only (no fragment written). */
	void SampleJitter();
	/** S3, S4: the "Jitter:" line of the report (per step moves of the fighting units, reversals). */
	void LogJitter() const;

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
	/** Walkers given a new path by a new footprint, and kept paths it dropped (J6b.2, D314), when the measure began. */
	int64 CutRepathsAtMeasureStart = 0;
	int64 CutDroppedAtMeasureStart = 0;
	/** Walkers given a new path because they were held on theirs (J6b.3), when the measure began. */
	int64 StuckRepathsAtMeasureStart = 0;
	/**
	 * S7 built camps (D314, hol.StressTest.S7BuildCamps): the foundations it placed, the seconds from the start of the
	 * run when each was finished (negative: not yet), the steps since the start, and the J6b.2 counts at the start.
	 */
	TArray<TWeakObjectPtr<AHOLBuilding>> GatheringCampSites;
	TArray<float> CampFinishedSeconds;
	/** S7 (D314): the seconds from the start of the run when each camp took its first work (negative: not yet). */
	TArray<float> CampStartedSeconds;
	/**
	 * S7 (D314): the build orders of the camps, drawn up when the Peasants are sent (their builders gather meanwhile) and
	 * issued when the measure begins, so the new footprints and cut paths fall inside it.
	 */
	TArray<FHOLBuildOrder> PendingBuildOrders;
	/**
	 * The run stopped: its deposits and trees are removed once its units are gone (destroyed at the tail of a step,
	 * D224), so no gatherer is left without a deposit in between; the steps waited so far.
	 */
	bool bTearDownPending = false;
	int32 TearDownWaitSteps = 0;
	int32 RunSteps = 0;
	int64 CutRepathsAtRunStart = 0;
	int64 CutDroppedAtRunStart = 0;
	/** UHOLGatherProcessor::FCounters when the measure began (as plain numbers: the header stays free of the processor). */
	int64 GatherCountersAtMeasureStart[6] = {};

	/**
	 * S3, S4: whether the charge was ordered, the step count and the combat counters (UHOLCombatProcessor, as plain numbers)
	 * when it was, and when the measure began; the units standing per side then; the arrows in flight seen on each
	 * measured step (sum and most).
	 */
	bool bChargeOrdered = false;
	int32 ChargeStep = 0;
	int64 MeleeHitsAtCharge = 0;
	int64 HitsAtMeasureStart = 0;
	int64 KillsAtMeasureStart = 0;
	int64 ArrowsReleasedAtMeasureStart = 0;
	int64 ArrowsLandedAtMeasureStart = 0;
	int64 ArrowsLostAtMeasureStart = 0;
	int32 AliveAtMeasureStart[2] = {};
	float ContactSeconds = -1.f;
	double ArrowsInFlightSum = 0.0;
	int64 ArrowsInFlightMax = 0;
	int32 ArrowSamples = 0;
	/** S4: the arrows released by the end of each of the last steps of the warm-up (one second of them, oldest first). */
	TArray<int64> ArrowsReleasedHistory;
	/** S4: arrows released in the last second when the measure began. */
	int64 ArrowsPerSecondAtStart = 0;
	/**
	 * S3, S4 jitter probe (J7c.4): per unit, its location and its move at the last sampled step; per class (0 attacking,
	 * 1 approaching or with a target, 2 walking without one), the 2D moves of the measured steps (cm), the step pairs whose
	 * moves both exceed 5 cm, and those of them whose moves point opposite ways.
	 */
	struct FJitterTrack
	{
		FVector2D Location = FVector2D::ZeroVector;
		FVector2D Move = FVector2D::ZeroVector;
		int64 Step = -1;
	};
	TMap<uint32, FJitterTrack> JitterTracks;
	TArray<float> JitterMoves[3];
	int64 JitterPairs[3] = {};
	int64 JitterReversals[3] = {};
	int64 JitterStep = 0;
	/** Per sampled step, the separation's longest correction over all units (cm, read only). */
	TArray<float> SeparationMaxes;
	/** The separation's source probe summed over the measured steps (UHOLUnitSeparationProcessor, read only). */
	FHOLSeparationSourceProbe SourceSum;
	int64 SourceSteps = 0;

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
