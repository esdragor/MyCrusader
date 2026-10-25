// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLMatchStats.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLMatchStatsSubsystem.generated.h"

/**
 * Records the match statistics (C2) where the simulation runs (solo, server): counters fed by the simulation's own events
 * (a death, a birth from production, a building destroyed), and a sample of every player every SampleIntervalSeconds of
 * simulation, taken at the tail of the step (outside of its chrono). It only reads the simulation: the outcome of a match
 * is the same with it (the battle checksums of the stress tests are unchanged).
 *
 * On a client (C3), it holds the copy received from the server: RequestStats asks for what is new (the server refuses a
 * player during the match, M6-Q11), GetStats returns the copy and OnStatsUpdated says when it changed. In solo GetStats is
 * the recording itself, and RequestStats only broadcasts OnStatsUpdated: the interface (C4, C5) reads the same way in both.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLMatchStatsSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The recording (solo, server) or the last copy received (client). */
	const FHOLMatchStats& GetStats() const { return Stats; }

	DECLARE_MULTICAST_DELEGATE(FOnStatsUpdated);
	/** The copy is complete again (client), or a request was made (solo). */
	FOnStatsUpdated& GetOnStatsUpdated() { return OnStatsUpdated; }
	/** Client: the server refused the request (a player during the match, M6-Q11). */
	FOnStatsUpdated& GetOnStatsRefused() { return OnStatsRefused; }

	/** Client: asks the server for what is new since the copy, chunk after chunk until complete. Solo: OnStatsUpdated now. */
	void RequestStats();
	/** Client: a chunk from the server (AHOLPlayerController::ClientReceiveMatchStats). */
	void ReceiveChunk(const FHOLMatchStatsChunk& Chunk);
	/** Client: the server refused (a player during the match). */
	void ReceiveRefusal();
	/** Client: the dump once the copy is complete (hol.Stats.Dump on a client). */
	void RequestDump();

	void RecordUnitDeath(int32 VictimPlayer, bool bWorker, int32 KillerPlayer) { HOLMatchStats::RecordUnitDeath(Stats, VictimPlayer, bWorker, KillerPlayer); }
	void RecordUnitTrained(int32 PlayerIndex) { HOLMatchStats::RecordUnitTrained(Stats, PlayerIndex); }
	void RecordBuildingLost(int32 PlayerIndex) { HOLMatchStats::RecordBuildingLost(Stats, PlayerIndex); }
	void RecordEvent(int32 PlayerIndex, EHOLMatchStatsEvent Kind);

	/** Takes a sample of every player now (the dump, the end of the match). */
	void TakeSample();

	/** The table and each player's last sample, in the log (hol.Stats.Dump). */
	void Dump() const;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

private:
	void OnStepEnded(int64 StepIndex);
	int32 GetTimeSeconds() const;

	FHOLMatchStats Stats;
	/** 0 until the first step. */
	int32 SampleSteps = 0;
	int64 LastStepIndex = 0;
	FDelegateHandle StepEndedHandle;
	FOnStatsUpdated OnStatsUpdated;
	FOnStatsUpdated OnStatsRefused;
	/** Client: a dump is due when the copy is complete. */
	bool bDumpWhenComplete = false;
};
