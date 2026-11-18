// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLMatchStatsReadLibrary.h"

#include "Match/HOLMatchStats.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLForumMessages.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchStatsSubsystem.h"
#include "Match/HOLPlayerColors.h"

namespace
{
	FHOLMatchStatsSample ToSample(const FHOLMatchStatsSampleState& State)
	{
		FHOLMatchStatsSample Sample;
		Sample.TimeSeconds = State.TimeSeconds;
		Sample.Workers = State.Workers;
		Sample.Military = State.Military;
		Sample.Population = State.Population;
		Sample.Buildings = State.Buildings;
		Sample.Gathered = State.Gathered;
		Sample.Stock = State.Stock;
		return Sample;
	}

	FHOLMatchStatsSampleState FromSample(const FHOLMatchStatsSample& Sample)
	{
		FHOLMatchStatsSampleState State;
		State.TimeSeconds = Sample.TimeSeconds;
		State.Workers = Sample.Workers;
		State.Military = Sample.Military;
		State.Population = Sample.Population;
		State.Buildings = Sample.Buildings;
		State.Gathered = Sample.Gathered;
		State.Stock = Sample.Stock;
		return State;
	}

	FHOLPlayerMatchStats ToPlayer(const FHOLPlayerMatchStatsState& State)
	{
		FHOLPlayerMatchStats Player;
		Player.PlayerIndex = State.PlayerIndex;
		Player.UnitsKilled = State.UnitsKilled;
		Player.UnitsLost = State.UnitsLost;
		Player.WorkersLost = State.WorkersLost;
		Player.BuildingsLost = State.BuildingsLost;
		Player.UnitsTrained = State.UnitsTrained;
		Player.Gathered = State.Gathered;
		Player.MaxWorkers = State.MaxWorkers;
		Player.MaxMilitary = State.MaxMilitary;
		Player.MaxPopulation = State.MaxPopulation;
		for (const FHOLMatchStatsSampleState& Sample : State.Samples)
		{
			Player.Samples.Add(ToSample(Sample));
		}
		return Player;
	}

	FHOLPlayerMatchStatsState FromPlayer(const FHOLPlayerMatchStats& Player)
	{
		FHOLPlayerMatchStatsState State;
		State.PlayerIndex = Player.PlayerIndex;
		State.UnitsKilled = Player.UnitsKilled;
		State.UnitsLost = Player.UnitsLost;
		State.WorkersLost = Player.WorkersLost;
		State.BuildingsLost = Player.BuildingsLost;
		State.UnitsTrained = Player.UnitsTrained;
		State.Gathered = Player.Gathered;
		State.MaxWorkers = Player.MaxWorkers;
		State.MaxMilitary = Player.MaxMilitary;
		State.MaxPopulation = Player.MaxPopulation;
		for (const FHOLMatchStatsSample& Sample : Player.Samples)
		{
			State.Samples.Add(FromSample(Sample));
		}
		return State;
	}

	FHOLMatchStats ToStats(const FHOLMatchStatsState& State)
	{
		FHOLMatchStats Stats;
		Stats.SampleIntervalSeconds = State.SampleIntervalSeconds;
		for (const FHOLPlayerMatchStatsState& Player : State.Players)
		{
			Stats.Players.Add(ToPlayer(Player));
		}
		for (const FHOLMatchStatsEventState& Event : State.Events)
		{
			FHOLMatchStatsEventEntry& Entry = Stats.Events.AddDefaulted_GetRef();
			Entry.TimeSeconds = Event.TimeSeconds;
			Entry.PlayerIndex = Event.PlayerIndex;
			Entry.Kind = static_cast<EHOLMatchStatsEvent>(Event.Kind);
		}
		return Stats;
	}

	FHOLMatchStatsState FromStats(const FHOLMatchStats& Stats)
	{
		FHOLMatchStatsState State;
		State.SampleIntervalSeconds = Stats.SampleIntervalSeconds;
		for (const FHOLPlayerMatchStats& Player : Stats.Players)
		{
			State.Players.Add(FromPlayer(Player));
		}
		for (const FHOLMatchStatsEventEntry& Entry : Stats.Events)
		{
			FHOLMatchStatsEventState& Event = State.Events.AddDefaulted_GetRef();
			Event.TimeSeconds = Entry.TimeSeconds;
			Event.PlayerIndex = Entry.PlayerIndex;
			Event.Kind = static_cast<int32>(Entry.Kind);
		}
		return State;
	}
}

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

void UHOLMatchStatsReadLibrary::RecordUnitDeath(FHOLMatchStatsState& Stats, const int32 VictimPlayer, const bool bWorker, const int32 KillerPlayer)
{
	FHOLMatchStats Rules = ToStats(Stats);
	HOLMatchStats::RecordUnitDeath(Rules, VictimPlayer, bWorker, KillerPlayer);
	Stats = FromStats(Rules);
}

void UHOLMatchStatsReadLibrary::RecordUnitTrained(FHOLMatchStatsState& Stats, const int32 PlayerIndex)
{
	FHOLMatchStats Rules = ToStats(Stats);
	HOLMatchStats::RecordUnitTrained(Rules, PlayerIndex);
	Stats = FromStats(Rules);
}

void UHOLMatchStatsReadLibrary::RecordBuildingLost(FHOLMatchStatsState& Stats, const int32 PlayerIndex)
{
	FHOLMatchStats Rules = ToStats(Stats);
	HOLMatchStats::RecordBuildingLost(Rules, PlayerIndex);
	Stats = FromStats(Rules);
}

void UHOLMatchStatsReadLibrary::RecordEvent(FHOLMatchStatsState& Stats, const int32 PlayerIndex, const int32 Kind, const int32 TimeSeconds)
{
	if (Kind < 0 || Kind > static_cast<int32>(EHOLMatchStatsEvent::ForumFell))
	{
		return;
	}
	FHOLMatchStats Rules = ToStats(Stats);
	HOLMatchStats::RecordEvent(Rules, PlayerIndex, static_cast<EHOLMatchStatsEvent>(Kind), TimeSeconds);
	Stats = FromStats(Rules);
}

void UHOLMatchStatsReadLibrary::AddSample(FHOLMatchStatsState& Stats, const int32 PlayerIndex, const FHOLMatchStatsSampleState& Sample)
{
	FHOLMatchStats Rules = ToStats(Stats);
	HOLMatchStats::AddSample(Rules, PlayerIndex, ToSample(Sample));
	Stats = FromStats(Rules);
}

FHOLMatchStatsChunkState UHOLMatchStatsReadLibrary::MakeChunk(const FHOLMatchStatsState& Stats, const int32 FromSample, const int32 FromEvent, const int32 MaxSamples)
{
	const FHOLMatchStatsChunk Chunk = HOLMatchStats::MakeChunk(ToStats(Stats), FromSample, FromEvent, MaxSamples);
	FHOLMatchStatsChunkState State;
	State.Stats = FromStats(Chunk.Stats);
	State.FromSample = Chunk.FromSample;
	State.FromEvent = Chunk.FromEvent;
	State.bComplete = Chunk.bComplete;
	return State;
}

void UHOLMatchStatsReadLibrary::MergeChunk(FHOLMatchStatsState& Copy, const FHOLMatchStatsChunkState& Chunk)
{
	FHOLMatchStats Rules = ToStats(Copy);
	FHOLMatchStatsChunk RulesChunk;
	RulesChunk.Stats = ToStats(Chunk.Stats);
	RulesChunk.FromSample = Chunk.FromSample;
	RulesChunk.FromEvent = Chunk.FromEvent;
	RulesChunk.bComplete = Chunk.bComplete;
	HOLMatchStats::MergeChunk(Rules, RulesChunk);
	Copy = FromStats(Rules);
}

int32 UHOLMatchStatsReadLibrary::GetNextFromSample(const FHOLMatchStatsState& Copy)
{
	return HOLMatchStats::GetNextFromSample(ToStats(Copy));
}

TArray<FVector2D> UHOLMatchStatsReadLibrary::MakeCurve(const FHOLPlayerMatchStatsState& Player, const int32 SeriesIndex)
{
	if (SeriesIndex < 0 || SeriesIndex >= static_cast<int32>(EHOLMatchStatsSeries::Num))
	{
		TArray<FVector2D> Flat;
		for (const FHOLMatchStatsSampleState& Sample : Player.Samples)
		{
			Flat.Emplace(Sample.TimeSeconds, 0.0);
		}
		return Flat;
	}
	return HOLMatchStats::MakeCurve(ToPlayer(Player), static_cast<EHOLMatchStatsSeries>(SeriesIndex));
}

FHOLMatchStatsState UHOLMatchStatsReadLibrary::GetWorldStats(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const UHOLMatchStatsSubsystem* Subsystem = World ? World->GetSubsystem<UHOLMatchStatsSubsystem>() : nullptr;
	return Subsystem ? FromStats(Subsystem->GetStats()) : FHOLMatchStatsState();
}

bool UHOLMatchStatsReadLibrary::HasWorldStats(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const UHOLMatchStatsSubsystem* Subsystem = World ? World->GetSubsystem<UHOLMatchStatsSubsystem>() : nullptr;
	return Subsystem && Subsystem->GetStats().Players.ContainsByPredicate([](const FHOLPlayerMatchStats& Player) { return Player.PlayerIndex != INDEX_NONE; });
}

TArray<FHOLStatsRowState> UHOLMatchStatsReadLibrary::GetWorldStatsRows(const UObject* WorldContextObject)
{
	TArray<FHOLStatsRowState> Rows;
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	if (!GameState)
	{
		return Rows;
	}
	const TArray<FHOLMatchSlot>& Slots = GameState->GetLaunchConfig().Slots;
	for (int32 SlotIndex = 0; SlotIndex < Slots.Num(); ++SlotIndex)
	{
		if (Slots[SlotIndex].State == EHOLSlotState::Closed)
		{
			continue;
		}
		FHOLStatsRowState& Row = Rows.AddDefaulted_GetRef();
		Row.SlotIndex = SlotIndex;
		Row.Name = HOLForumMessages::GetPlayerName(GameState->GetLaunchConfig(), SlotIndex, INDEX_NONE);
		Row.Color = FLinearColor(HOLPlayerColors::GetColor(Slots[SlotIndex].ColorIndex));
		Row.Team = Slots[SlotIndex].Team;
		Row.Result = GameState->IsWinner(SlotIndex) ? 1 : GameState->IsSlotDefeated(SlotIndex) ? 2 : 0;
	}
	return Rows;
}

FText UHOLMatchStatsReadLibrary::GetWorldSlotName(const UObject* WorldContextObject, const int32 SlotIndex)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	return GameState ? HOLForumMessages::GetPlayerName(GameState->GetLaunchConfig(), SlotIndex, INDEX_NONE) : FText::GetEmpty();
}

FLinearColor UHOLMatchStatsReadLibrary::GetWorldSlotColor(const UObject* WorldContextObject, const int32 SlotIndex)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	if (!GameState)
	{
		return FLinearColor::White;
	}
	const TArray<FHOLMatchSlot>& Slots = GameState->GetLaunchConfig().Slots;
	return Slots.IsValidIndex(SlotIndex) ? FLinearColor(HOLPlayerColors::GetColor(Slots[SlotIndex].ColorIndex)) : FLinearColor::White;
}

FString UHOLMatchStatsReadLibrary::DescribePlayer(const FHOLPlayerMatchStatsState& Player)
{
	return HOLMatchStats::DescribePlayer(ToPlayer(Player));
}
