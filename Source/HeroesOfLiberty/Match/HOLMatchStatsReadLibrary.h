// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Match/HOLResources.h"
#include "HOLMatchStatsReadLibrary.generated.h"

/**
 * The pure rules of the match statistics (HOLMatchStats, Match/HOLMatchStats.h, C++, recorded by the server) for Angelscript (namespace
 * HOLMatchStatsRead there): D836 keeps the rules in C++ and the script reads them here, so that their test (HOLMatchStatsTests.cpp) can be
 * ported to script (socle step 1, S1g, D830, D839). Wrappers only: no rule here. Only the functions with flat arguments are here: the
 * statistics structs (FHOLMatchStats, FHOLPlayerMatchStats, FHOLMatchStatsSample, FHOLMatchStatsChunk) are plain USTRUCT()s, not exposed.
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
};