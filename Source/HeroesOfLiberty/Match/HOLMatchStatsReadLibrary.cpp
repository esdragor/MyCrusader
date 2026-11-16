// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLMatchStatsReadLibrary.h"

#include "Match/HOLMatchStats.h"

int32 UHOLMatchStatsReadLibrary::GetSampleSteps(const float StepSeconds, const int32 IntervalSeconds)
{
	return HOLMatchStats::GetSampleSteps(StepSeconds, IntervalSeconds);
}

bool UHOLMatchStatsReadLibrary::IsSampleStep(const int64 StepIndex, const int32 SampleSteps)
{
	return HOLMatchStats::IsSampleStep(StepIndex, SampleSteps);
}

bool UHOLMatchStatsReadLibrary::CanReceiveStats(const bool bOnlyASpectator, const bool bMatchOver)
{
	return HOLMatchStats::CanReceiveStats(bOnlyASpectator, bMatchOver);
}

int32 UHOLMatchStatsReadLibrary::GetMilitaryScore(const int32 UnitsKilled)
{
	FHOLPlayerMatchStats Player;
	Player.UnitsKilled = UnitsKilled;
	return HOLMatchStats::GetMilitaryScore(Player);
}

int32 UHOLMatchStatsReadLibrary::GetEconomyScore(const FHOLResources& Gathered, const int32 UnitsTrained)
{
	FHOLPlayerMatchStats Player;
	Player.Gathered = Gathered;
	Player.UnitsTrained = UnitsTrained;
	return HOLMatchStats::GetEconomyScore(Player);
}

int32 UHOLMatchStatsReadLibrary::GetSeriesValue(const int32 Workers, const int32 Military, const int32 Population, const int32 Buildings,
	const FHOLResources& Gathered, const FHOLResources& Stock, const int32 SeriesIndex)
{
	if (SeriesIndex < 0 || SeriesIndex >= static_cast<int32>(EHOLMatchStatsSeries::Num))
	{
		return 0;
	}
	FHOLMatchStatsSample Sample;
	Sample.Workers = Workers;
	Sample.Military = Military;
	Sample.Population = Population;
	Sample.Buildings = Buildings;
	Sample.Gathered = Gathered;
	Sample.Stock = Stock;
	return HOLMatchStats::GetSeriesValue(Sample, static_cast<EHOLMatchStatsSeries>(SeriesIndex));
}

int32 UHOLMatchStatsReadLibrary::GetNiceStep(const int32 MaxValue, const int32 MaxTicks, const bool bTimeAxis)
{
	return HOLMatchStats::GetNiceStep(MaxValue, MaxTicks, bTimeAxis);
}

int32 UHOLMatchStatsReadLibrary::GetAxisEnd(const int32 MaxValue, const int32 Step, const bool bHeadroom)
{
	return HOLMatchStats::GetAxisEnd(MaxValue, Step, bHeadroom);
}

FString UHOLMatchStatsReadLibrary::FormatMatchTime(const int32 TimeSeconds)
{
	return HOLMatchStats::FormatMatchTime(TimeSeconds);
}