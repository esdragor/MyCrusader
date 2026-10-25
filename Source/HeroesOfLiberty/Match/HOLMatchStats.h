// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLResources.h"
#include "HOLMatchStats.generated.h"

/**
 * The match statistics (C2, M6-Q10, M6-Q11): what the end-of-match screen shows to everyone and the spectators see during
 * the match, as Age of Empires' (its tables by player, its curves over time). Recorded where the simulation runs (solo,
 * server) by UHOLMatchStatsSubsystem, which only reads the simulation: nothing here changes an outcome.
 */

/** One point of a player's curves, taken every SampleIntervalSeconds of match time. */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLMatchStatsSample
{
	GENERATED_BODY()

	/** Match time of the sample (seconds since the first step). */
	UPROPERTY()
	int32 TimeSeconds = 0;

	/** Living units that gather (a CarryCapacity: the Peasants), the dying left out. */
	UPROPERTY()
	int32 Workers = 0;

	/** Living units that do not gather: the army. */
	UPROPERTY()
	int32 Military = 0;

	/** Places used, as the player's top bar shows them (units and units in production). */
	UPROPERTY()
	int32 Population = 0;

	/** Buildings standing, sites included. */
	UPROPERTY()
	int32 Buildings = 0;

	/** Resources gathered since the start (unloaded at a deposit), cumulative. */
	UPROPERTY()
	FHOLResources Gathered;

	/** The stock at that time. */
	UPROPERTY()
	FHOLResources Stock;

	bool operator==(const FHOLMatchStatsSample& Other) const = default;
};

/** What happened to one player at one time: the marks on the curves. */
UENUM()
enum class EHOLMatchStatsEvent : uint8
{
	/** The player is defeated (D358). */
	Defeated,
	/** Its main forum fell (a life lost, V2). */
	ForumFell,
};

/** The curves the statistics screen's « Chronologie » can show (C5), one value a sample. */
enum class EHOLMatchStatsSeries : uint8
{
	Workers,
	Military,
	Population,
	Buildings,
	/** The four resources gathered together, cumulative. */
	Gathered,
	/** The four resources in stock together. */
	Stock,
	Num,
};

USTRUCT()
struct HEROESOFLIBERTY_API FHOLMatchStatsEventEntry
{
	GENERATED_BODY()

	UPROPERTY()
	int32 TimeSeconds = 0;

	UPROPERTY()
	int32 PlayerIndex = INDEX_NONE;

	UPROPERTY()
	EHOLMatchStatsEvent Kind = EHOLMatchStatsEvent::Defeated;
};

/** One player's table and curves. */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLPlayerMatchStats
{
	GENERATED_BODY()

	/** Slot of the match configuration (D244), as OwnerPlayer (D233). */
	UPROPERTY()
	int32 PlayerIndex = INDEX_NONE;

	/** Military: enemy units it killed (a hit or an arrow of its units). */
	UPROPERTY()
	int32 UnitsKilled = 0;

	/** Military: its units dead, whatever killed them (combat, collapse at a defeat). */
	UPROPERTY()
	int32 UnitsLost = 0;

	/** Of UnitsLost, the workers. */
	UPROPERTY()
	int32 WorkersLost = 0;

	/** Military: its buildings destroyed (razed, or collapsed at a defeat; a cancelled site is not one). */
	UPROPERTY()
	int32 BuildingsLost = 0;

	/** Economy: units its buildings produced. */
	UPROPERTY()
	int32 UnitsTrained = 0;

	/** Economy: resources gathered (unloaded), cumulative; the last sample's or fresher. */
	UPROPERTY()
	FHOLResources Gathered;

	/** Highest values seen in the samples. */
	UPROPERTY()
	int32 MaxWorkers = 0;

	UPROPERTY()
	int32 MaxMilitary = 0;

	UPROPERTY()
	int32 MaxPopulation = 0;

	/** The curves, oldest first. */
	UPROPERTY()
	TArray<FHOLMatchStatsSample> Samples;
};

/** The whole match's statistics: what C3 replicates. */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLMatchStats
{
	GENERATED_BODY()

	UPROPERTY()
	int32 SampleIntervalSeconds = 10;

	/** By PlayerIndex: entry N is slot N (empty slots have PlayerIndex INDEX_NONE until they record something). */
	UPROPERTY()
	TArray<FHOLPlayerMatchStats> Players;

	UPROPERTY()
	TArray<FHOLMatchStatsEventEntry> Events;
};

/**
 * A piece of the statistics sent to a client (C3): the tables whole, each player's samples from FromSample on (MaxSamples
 * at most), the events from FromEvent on. A client asks again from where it is until bComplete.
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLMatchStatsChunk
{
	GENERATED_BODY()

	UPROPERTY()
	FHOLMatchStats Stats;

	UPROPERTY()
	int32 FromSample = 0;

	UPROPERTY()
	int32 FromEvent = 0;

	/** Every sample and event the server had is in the chunk. */
	UPROPERTY()
	bool bComplete = true;
};

/** The pure rules of the recording (aggregation, sampling), tested apart from any world. */
namespace HOLMatchStats
{
	/** Steps between two samples: IntervalSeconds of simulation, one step at least. */
	HEROESOFLIBERTY_API int32 GetSampleSteps(float StepSeconds, int32 IntervalSeconds);

	/** A sample is taken at step 0 and every SampleSteps after. */
	HEROESOFLIBERTY_API bool IsSampleStep(int64 StepIndex, int32 SampleSteps);

	/** The player's entry, the array grown to hold it. */
	HEROESOFLIBERTY_API FHOLPlayerMatchStats& FindOrAddPlayer(FHOLMatchStats& Stats, int32 PlayerIndex);

	/**
	 * A unit died: a loss for its owner (and a worker loss for a worker), a kill for KillerPlayer unless it is none or the
	 * victim's own player.
	 */
	HEROESOFLIBERTY_API void RecordUnitDeath(FHOLMatchStats& Stats, int32 VictimPlayer, bool bWorker, int32 KillerPlayer);

	HEROESOFLIBERTY_API void RecordUnitTrained(FHOLMatchStats& Stats, int32 PlayerIndex);

	HEROESOFLIBERTY_API void RecordBuildingLost(FHOLMatchStats& Stats, int32 PlayerIndex);

	HEROESOFLIBERTY_API void RecordEvent(FHOLMatchStats& Stats, int32 PlayerIndex, EHOLMatchStatsEvent Kind, int32 TimeSeconds);

	/** Appends a sample to the player's curves and raises its maxima and its gathered totals. */
	HEROESOFLIBERTY_API void AddSample(FHOLMatchStats& Stats, int32 PlayerIndex, const FHOLMatchStatsSample& Sample);

	/**
	 * M6-Q11: everyone gets the statistics once the match is over; during the match, a spectator without a slot only (a
	 * player, even defeated, would learn about his opponents).
	 */
	HEROESOFLIBERTY_API bool CanReceiveStats(bool bOnlyASpectator, bool bMatchOver);

	/** The chunk of Stats from FromSample and FromEvent on, MaxSamples samples a player at most (C3). */
	HEROESOFLIBERTY_API FHOLMatchStatsChunk MakeChunk(const FHOLMatchStats& Stats, int32 FromSample, int32 FromEvent, int32 MaxSamples);

	/**
	 * Merges a chunk into a client's copy: the tables replaced, each player's samples cut at the chunk's FromSample then the
	 * chunk's appended (the server may have replaced its last sample), the events likewise.
	 */
	HEROESOFLIBERTY_API void MergeChunk(FHOLMatchStats& Copy, const FHOLMatchStatsChunk& Chunk);

	/** Where a client asks from next: its last sample again (it may have been replaced), its events count. */
	HEROESOFLIBERTY_API int32 GetNextFromSample(const FHOLMatchStats& Copy);

	/**
	 * The scores of the statistics screen (C4), simple and shown as such: military, 10 points an enemy unit killed;
	 * economy, a point every 10 resources gathered (the four together) and 5 points a unit trained.
	 */
	HEROESOFLIBERTY_API int32 GetMilitaryScore(const FHOLPlayerMatchStats& Player);
	HEROESOFLIBERTY_API int32 GetEconomyScore(const FHOLPlayerMatchStats& Player);

	/** The value a sample gives to a curve of the « Chronologie » (C5). */
	HEROESOFLIBERTY_API int32 GetSeriesValue(const FHOLMatchStatsSample& Sample, EHOLMatchStatsSeries Series);

	/** A player's curve: one point a sample, X its match time in seconds, Y its value. */
	HEROESOFLIBERTY_API TArray<FVector2D> MakeCurve(const FHOLPlayerMatchStats& Player, EHOLMatchStatsSeries Series);

	/**
	 * A "round" step between the graduations of an axis going from 0 to MaxValue, as a chart's: 1, 2 or 5 times a power of
	 * ten (TimeAxis: 10, 15, 30 s, 1, 2, 5, 10 min, ...), so that MaxTicks graduations at most cover MaxValue. 1 at least.
	 */
	HEROESOFLIBERTY_API int32 GetNiceStep(int32 MaxValue, int32 MaxTicks, bool bTimeAxis = false);

	/**
	 * The axis's end: MaxValue rounded up to a whole Step (one Step for nothing to show). Headroom (a value axis): the end
	 * leaves a tenth of MaxValue at least above it, so that a curve at its highest does not touch the graph's top edge.
	 */
	HEROESOFLIBERTY_API int32 GetAxisEnd(int32 MaxValue, int32 Step, bool bHeadroom = false);

	/** A match time as the timeline's axis shows it: "m:ss" ("0:00", "1:05", "12:30"). */
	HEROESOFLIBERTY_API FString FormatMatchTime(int32 TimeSeconds);

	/** "slot 0: killed 3, lost 2 (1 workers), buildings lost 0, trained 5, gathered F .. , max workers 7, ...". */
	HEROESOFLIBERTY_API FString DescribePlayer(const FHOLPlayerMatchStats& Player);
	/** "t=120 s workers 7 military 2 population 9 buildings 3 gathered F .. stock F ..". */
	HEROESOFLIBERTY_API FString DescribeSample(const FHOLMatchStatsSample& Sample);
}
