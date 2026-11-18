// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Match/HOLResources.h"
#include "HOLMatchStatsReadLibrary.generated.h"

/**
 * The statistics structs as the script sees them (S1g suite, SAURON-1117g): flat BlueprintType copies of the plain USTRUCT()s of
 * HOLMatchStats.h (same fields, same names), converted by value in the wrappers below; the event kind and the series are ints (the order of
 * EHOLMatchStatsEvent: 0 Defeated, 1 ForumFell).
 */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLMatchStatsSampleState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	int32 TimeSeconds = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	int32 Workers = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	int32 Military = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	int32 Population = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	int32 Buildings = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	FHOLResources Gathered;

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	FHOLResources Stock;
};

USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLPlayerMatchStatsState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	int32 PlayerIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	int32 UnitsKilled = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	int32 UnitsLost = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	int32 WorkersLost = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	int32 BuildingsLost = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	int32 UnitsTrained = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	FHOLResources Gathered;

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	int32 MaxWorkers = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	int32 MaxMilitary = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	int32 MaxPopulation = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	TArray<FHOLMatchStatsSampleState> Samples;
};

USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLMatchStatsEventState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	int32 TimeSeconds = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	int32 PlayerIndex = INDEX_NONE;

	/** The order of EHOLMatchStatsEvent: 0 Defeated, 1 ForumFell. */
	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	int32 Kind = 0;
};

/** FHOLMatchStats as the script sees it. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLMatchStatsState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	int32 SampleIntervalSeconds = 10;

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	TArray<FHOLPlayerMatchStatsState> Players;

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	TArray<FHOLMatchStatsEventState> Events;
};

/** FHOLMatchStatsChunk as the script sees it. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLMatchStatsChunkState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	FHOLMatchStatsState Stats;

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	int32 FromSample = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	int32 FromEvent = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	bool bComplete = true;
};

/** A row of the statistics tables (a slot held by a human or an AI, closed ones left out), in the slots' order (ST-1). */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLStatsRowState
{
	GENERATED_BODY()

	/** The slot, also the index of its FHOLPlayerMatchStatsState in FHOLMatchStatsState::Players. */
	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	int32 SlotIndex = INDEX_NONE;

	/** The lobby's name, an AI's "IA Difficile 3", else "Joueur N" (HOLForumMessages::GetPlayerName). */
	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	FText Name;

	/** The player's colour (HOLPlayerColors). */
	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	FLinearColor Color = FLinearColor::White;

	/** 0: no team (shown "—"), else the team number. */
	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	int32 Team = 0;

	/** 0 Playing, 1 Winner, 2 Defeated. */
	UPROPERTY(BlueprintReadWrite, Category = "MatchStats")
	int32 Result = 0;
};

/**
 * The pure rules of the match statistics (HOLMatchStats, Match/HOLMatchStats.h, C++, recorded by the server) for Angelscript (namespace
 * HOLMatchStatsRead there): D836 keeps the rules in C++ and the script reads them here, so that their test (HOLMatchStatsTests.cpp) can be
 * ported to script (socle step 1, S1g, D830, D839). Wrappers only: no rule here. The first functions take flat arguments; the Record*, AddSample, chunk and curve ones take the
 * FHOLMatchStatsState copies above (the plain USTRUCT()s of HOLMatchStats.h are not exposed).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLMatchStatsReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLMatchStats::GetSampleSteps. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|MatchStats")
	static int32 GetSampleSteps(float StepSeconds, int32 IntervalSeconds);

	/** HOLMatchStats::IsSampleStep. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|MatchStats")
	static bool IsSampleStep(int64 StepIndex, int32 SampleSteps);

	/** HOLMatchStats::CanReceiveStats. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|MatchStats")
	static bool CanReceiveStats(bool bOnlyASpectator, bool bMatchOver);

	/** HOLMatchStats::GetMilitaryScore of a player table with these UnitsKilled (the other fields do not count). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|MatchStats")
	static int32 GetMilitaryScore(int32 UnitsKilled);

	/** HOLMatchStats::GetEconomyScore of a player table with this Gathered and these UnitsTrained. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|MatchStats")
	static int32 GetEconomyScore(const FHOLResources& Gathered, int32 UnitsTrained);

	/**
	 * HOLMatchStats::GetSeriesValue of a sample with these values. SeriesIndex: the order of EHOLMatchStatsSeries (0 Workers, 1 Military,
	 * 2 Population, 3 Buildings, 4 Gathered, 5 Stock); out of range: 0.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|MatchStats")
	static int32 GetSeriesValue(int32 Workers, int32 Military, int32 Population, int32 Buildings, const FHOLResources& Gathered,
		const FHOLResources& Stock, int32 SeriesIndex);

	/** HOLMatchStats::GetNiceStep. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|MatchStats")
	static int32 GetNiceStep(int32 MaxValue, int32 MaxTicks, bool bTimeAxis);

	/** HOLMatchStats::GetAxisEnd. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|MatchStats")
	static int32 GetAxisEnd(int32 MaxValue, int32 Step, bool bHeadroom);

	/** HOLMatchStats::FormatMatchTime ("m:ss"). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|MatchStats")
	static FString FormatMatchTime(int32 TimeSeconds);

	/** HOLMatchStats::RecordUnitDeath (the slots grow as needed). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|MatchStats")
	static void RecordUnitDeath(UPARAM(ref) FHOLMatchStatsState& Stats, int32 VictimPlayer, bool bWorker, int32 KillerPlayer);

	/** HOLMatchStats::RecordUnitTrained. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|MatchStats")
	static void RecordUnitTrained(UPARAM(ref) FHOLMatchStatsState& Stats, int32 PlayerIndex);

	/** HOLMatchStats::RecordBuildingLost. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|MatchStats")
	static void RecordBuildingLost(UPARAM(ref) FHOLMatchStatsState& Stats, int32 PlayerIndex);

	/** HOLMatchStats::RecordEvent (Kind: the order of EHOLMatchStatsEvent; out of range: nothing recorded). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|MatchStats")
	static void RecordEvent(UPARAM(ref) FHOLMatchStatsState& Stats, int32 PlayerIndex, int32 Kind, int32 TimeSeconds);

	/** HOLMatchStats::AddSample. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|MatchStats")
	static void AddSample(UPARAM(ref) FHOLMatchStatsState& Stats, int32 PlayerIndex, const FHOLMatchStatsSampleState& Sample);

	/** HOLMatchStats::MakeChunk. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|MatchStats")
	static FHOLMatchStatsChunkState MakeChunk(const FHOLMatchStatsState& Stats, int32 FromSample, int32 FromEvent, int32 MaxSamples);

	/** HOLMatchStats::MergeChunk. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|MatchStats")
	static void MergeChunk(UPARAM(ref) FHOLMatchStatsState& Copy, const FHOLMatchStatsChunkState& Chunk);

	/** HOLMatchStats::GetNextFromSample. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|MatchStats")
	static int32 GetNextFromSample(const FHOLMatchStatsState& Copy);

	/** HOLMatchStats::MakeCurve (SeriesIndex as GetSeriesValue): one point a sample, X the match time, Y the value; out of range: Y 0. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|MatchStats")
	static TArray<FVector2D> MakeCurve(const FHOLPlayerMatchStatsState& Player, int32 SeriesIndex);

	/** What UHOLMatchStatsSubsystem::GetStats holds now (solo: the recording, client: the copy received), as the script's state; empty without a subsystem. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|MatchStats", meta = (WorldContext = "WorldContextObject"))
	static FHOLMatchStatsState GetWorldStats(const UObject* WorldContextObject);

	/** True when the subsystem holds at least one player table (the screen shows "waiting for the server" otherwise). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|MatchStats", meta = (WorldContext = "WorldContextObject"))
	static bool HasWorldStats(const UObject* WorldContextObject);

	/** The rows of the tables from the game state's launch configuration and results (name, colour, team, result), in the slots' order; empty without a game state. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|MatchStats", meta = (WorldContext = "WorldContextObject"))
	static TArray<FHOLStatsRowState> GetWorldStatsRows(const UObject* WorldContextObject);
};