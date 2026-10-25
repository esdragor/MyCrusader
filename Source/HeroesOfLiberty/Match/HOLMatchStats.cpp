// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLMatchStats.h"

int32 HOLMatchStats::GetSampleSteps(const float StepSeconds, const int32 IntervalSeconds)
{
	if (StepSeconds <= 0.f)
	{
		return 1;
	}
	return FMath::Max(FMath::RoundToInt(static_cast<float>(FMath::Max(IntervalSeconds, 1)) / StepSeconds), 1);
}

bool HOLMatchStats::IsSampleStep(const int64 StepIndex, const int32 SampleSteps)
{
	return StepIndex >= 0 && StepIndex % FMath::Max(SampleSteps, 1) == 0;
}

FHOLPlayerMatchStats& HOLMatchStats::FindOrAddPlayer(FHOLMatchStats& Stats, const int32 PlayerIndex)
{
	check(PlayerIndex >= 0);
	if (Stats.Players.Num() <= PlayerIndex)
	{
		Stats.Players.SetNum(PlayerIndex + 1);
	}
	FHOLPlayerMatchStats& Player = Stats.Players[PlayerIndex];
	Player.PlayerIndex = PlayerIndex;
	return Player;
}

void HOLMatchStats::RecordUnitDeath(FHOLMatchStats& Stats, const int32 VictimPlayer, const bool bWorker, const int32 KillerPlayer)
{
	if (VictimPlayer >= 0)
	{
		FHOLPlayerMatchStats& Victim = FindOrAddPlayer(Stats, VictimPlayer);
		++Victim.UnitsLost;
		Victim.WorkersLost += bWorker ? 1 : 0;
	}
	if (KillerPlayer >= 0 && KillerPlayer != VictimPlayer)
	{
		++FindOrAddPlayer(Stats, KillerPlayer).UnitsKilled;
	}
}

void HOLMatchStats::RecordUnitTrained(FHOLMatchStats& Stats, const int32 PlayerIndex)
{
	if (PlayerIndex >= 0)
	{
		++FindOrAddPlayer(Stats, PlayerIndex).UnitsTrained;
	}
}

void HOLMatchStats::RecordBuildingLost(FHOLMatchStats& Stats, const int32 PlayerIndex)
{
	if (PlayerIndex >= 0)
	{
		++FindOrAddPlayer(Stats, PlayerIndex).BuildingsLost;
	}
}

void HOLMatchStats::RecordEvent(FHOLMatchStats& Stats, const int32 PlayerIndex, const EHOLMatchStatsEvent Kind, const int32 TimeSeconds)
{
	FHOLMatchStatsEventEntry& Entry = Stats.Events.AddDefaulted_GetRef();
	Entry.TimeSeconds = TimeSeconds;
	Entry.PlayerIndex = PlayerIndex;
	Entry.Kind = Kind;
}

void HOLMatchStats::AddSample(FHOLMatchStats& Stats, const int32 PlayerIndex, const FHOLMatchStatsSample& Sample)
{
	if (PlayerIndex < 0)
	{
		return;
	}
	FHOLPlayerMatchStats& Player = FindOrAddPlayer(Stats, PlayerIndex);
	Player.Samples.Add(Sample);
	Player.Gathered = Sample.Gathered;
	Player.MaxWorkers = FMath::Max(Player.MaxWorkers, Sample.Workers);
	Player.MaxMilitary = FMath::Max(Player.MaxMilitary, Sample.Military);
	Player.MaxPopulation = FMath::Max(Player.MaxPopulation, Sample.Population);
}

int32 HOLMatchStats::GetMilitaryScore(const FHOLPlayerMatchStats& Player)
{
	return 10 * Player.UnitsKilled;
}

int32 HOLMatchStats::GetEconomyScore(const FHOLPlayerMatchStats& Player)
{
	const FHOLResources& Gathered = Player.Gathered;
	return (Gathered.Food + Gathered.Wood + Gathered.Stone + Gathered.Gold) / 10 + 5 * Player.UnitsTrained;
}

int32 HOLMatchStats::GetSeriesValue(const FHOLMatchStatsSample& Sample, const EHOLMatchStatsSeries Series)
{
	const auto Total = [](const FHOLResources& Resources) { return Resources.Food + Resources.Wood + Resources.Stone + Resources.Gold; };
	switch (Series)
	{
	case EHOLMatchStatsSeries::Workers: return Sample.Workers;
	case EHOLMatchStatsSeries::Military: return Sample.Military;
	case EHOLMatchStatsSeries::Population: return Sample.Population;
	case EHOLMatchStatsSeries::Buildings: return Sample.Buildings;
	case EHOLMatchStatsSeries::Gathered: return Total(Sample.Gathered);
	case EHOLMatchStatsSeries::Stock: return Total(Sample.Stock);
	default: return 0;
	}
}

TArray<FVector2D> HOLMatchStats::MakeCurve(const FHOLPlayerMatchStats& Player, const EHOLMatchStatsSeries Series)
{
	TArray<FVector2D> Curve;
	Curve.Reserve(Player.Samples.Num());
	for (const FHOLMatchStatsSample& Sample : Player.Samples)
	{
		Curve.Emplace(Sample.TimeSeconds, GetSeriesValue(Sample, Series));
	}
	return Curve;
}

int32 HOLMatchStats::GetNiceStep(const int32 MaxValue, const int32 MaxTicks, const bool bTimeAxis)
{
	const double Raw = static_cast<double>(FMath::Max(MaxValue, 0)) / FMath::Max(MaxTicks, 1);
	// 1, 2 or 5 times a power of ten, the smallest that covers Raw.
	const auto Decimal = [](const double Value)
	{
		for (int64 Power = 1; Power < MAX_int32 / 10; Power *= 10)
		{
			for (const int64 Factor : { 1, 2, 5 })
			{
				if (Factor * Power >= Value)
				{
					return static_cast<int32>(Factor * Power);
				}
			}
		}
		return MAX_int32 / 10;
	};
	if (!bTimeAxis)
	{
		return FMath::Max(Decimal(Raw), 1);
	}
	// What a clock reads: seconds up to a minute, minutes up to an hour, then hours as numbers.
	for (const int32 Step : { 1, 2, 5, 10, 15, 30, 60, 120, 300, 600, 900, 1800, 3600 })
	{
		if (Step >= Raw)
		{
			return Step;
		}
	}
	return 3600 * Decimal(Raw / 3600.0);
}

int32 HOLMatchStats::GetAxisEnd(const int32 MaxValue, const int32 Step, const bool bHeadroom)
{
	const int32 SafeStep = FMath::Max(Step, 1);
	const int32 SafeMax = FMath::Max(MaxValue, 0);
	// Headroom: a tenth of the highest value (1 at least) above it, so that the end is never the highest value itself.
	const int32 Top = bHeadroom && SafeMax > 0 ? SafeMax + FMath::DivideAndRoundUp(SafeMax, 10) : SafeMax;
	return FMath::Max(FMath::DivideAndRoundUp(Top, SafeStep), 1) * SafeStep;
}

FString HOLMatchStats::FormatMatchTime(const int32 TimeSeconds)
{
	const int32 Seconds = FMath::Max(TimeSeconds, 0);
	return FString::Printf(TEXT("%d:%02d"), Seconds / 60, Seconds % 60);
}

FString HOLMatchStats::DescribePlayer(const FHOLPlayerMatchStats& Player)
{
	return FString::Printf(TEXT("slot %d: killed %d, lost %d (%d workers), buildings lost %d, trained %d, gathered %s, max workers %d, max military %d, max population %d, %d samples"),
		Player.PlayerIndex, Player.UnitsKilled, Player.UnitsLost, Player.WorkersLost, Player.BuildingsLost, Player.UnitsTrained, *Player.Gathered.ToString(),
		Player.MaxWorkers, Player.MaxMilitary, Player.MaxPopulation, Player.Samples.Num());
}

FString HOLMatchStats::DescribeSample(const FHOLMatchStatsSample& Sample)
{
	return FString::Printf(TEXT("t=%d s workers %d military %d population %d buildings %d gathered %s stock %s"), Sample.TimeSeconds, Sample.Workers,
		Sample.Military, Sample.Population, Sample.Buildings, *Sample.Gathered.ToString(), *Sample.Stock.ToString());
}

bool HOLMatchStats::CanReceiveStats(const bool bOnlyASpectator, const bool bMatchOver)
{
	return bMatchOver || bOnlyASpectator;
}

FHOLMatchStatsChunk HOLMatchStats::MakeChunk(const FHOLMatchStats& Stats, const int32 FromSample, const int32 FromEvent, const int32 MaxSamples)
{
	FHOLMatchStatsChunk Chunk;
	Chunk.FromSample = FMath::Max(FromSample, 0);
	Chunk.FromEvent = FMath::Clamp(FromEvent, 0, Stats.Events.Num());
	Chunk.bComplete = true;
	Chunk.Stats.SampleIntervalSeconds = Stats.SampleIntervalSeconds;
	Chunk.Stats.Players.Reserve(Stats.Players.Num());
	for (const FHOLPlayerMatchStats& Player : Stats.Players)
	{
		// The table whole, the curves from FromSample on (MaxSamples at most).
		FHOLPlayerMatchStats& Copy = Chunk.Stats.Players.Add_GetRef(Player);
		Copy.Samples.Reset();
		const int32 Begin = FMath::Min(Chunk.FromSample, Player.Samples.Num());
		const int32 End = FMath::Min(Begin + FMath::Max(MaxSamples, 1), Player.Samples.Num());
		Copy.Samples.Append(Player.Samples.GetData() + Begin, End - Begin);
		Chunk.bComplete &= End == Player.Samples.Num();
	}
	Chunk.Stats.Events.Append(Stats.Events.GetData() + Chunk.FromEvent, Stats.Events.Num() - Chunk.FromEvent);
	return Chunk;
}

void HOLMatchStats::MergeChunk(FHOLMatchStats& Copy, const FHOLMatchStatsChunk& Chunk)
{
	Copy.SampleIntervalSeconds = Chunk.Stats.SampleIntervalSeconds;
	Copy.Players.SetNum(Chunk.Stats.Players.Num());
	for (int32 Index = 0; Index < Chunk.Stats.Players.Num(); ++Index)
	{
		FHOLPlayerMatchStats& Player = Copy.Players[Index];
		TArray<FHOLMatchStatsSample> Samples = MoveTemp(Player.Samples);
		Player = Chunk.Stats.Players[Index];
		Samples.SetNum(FMath::Min(Chunk.FromSample, Samples.Num()));
		Samples.Append(Chunk.Stats.Players[Index].Samples);
		Player.Samples = MoveTemp(Samples);
	}
	Copy.Events.SetNum(FMath::Min(Chunk.FromEvent, Copy.Events.Num()));
	Copy.Events.Append(Chunk.Stats.Events);
}

int32 HOLMatchStats::GetNextFromSample(const FHOLMatchStats& Copy)
{
	// The shortest curve's last sample: every player's curve is asked again from there (the server may have replaced it).
	int32 Next = TNumericLimits<int32>::Max();
	for (const FHOLPlayerMatchStats& Player : Copy.Players)
	{
		if (Player.PlayerIndex != INDEX_NONE)
		{
			Next = FMath::Min(Next, Player.Samples.Num() - 1);
		}
	}
	return Next == TNumericLimits<int32>::Max() ? 0 : FMath::Max(Next, 0);
}
