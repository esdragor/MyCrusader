// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLMatchStatsSubsystem.h"

#include "Buildings/HOLBuilding.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLPlayerState.h"
#include "Player/HOLPlayerController.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

namespace UE::HeroesOfLiberty::MatchStats
{
	int32 SampleIntervalSeconds = 10;
	FAutoConsoleVariableRef CVarSampleIntervalSeconds(TEXT("hol.Stats.SampleSeconds"), SampleIntervalSeconds,
		TEXT("Seconds of simulation between two samples of the match statistics' curves (C2), read at the start of the match"));

	FAutoConsoleCommandWithWorld CommandDump(TEXT("hol.Stats.Dump"),
		TEXT("Match statistics (C2): a fresh sample, then each player's table and last sample in the log"),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			UHOLMatchStatsSubsystem* MatchStats = World ? World->GetSubsystem<UHOLMatchStatsSubsystem>() : nullptr;
			if (!MatchStats)
			{
				UE_LOG(LogHOLMatch, Display, TEXT("Stats: no match statistics in this world (a client receives them, C3)"));
				return;
			}
			if (World->GetNetMode() == NM_Client)
			{
				// C3: the copy from the server, dumped once complete (or the refusal, in the log).
				MatchStats->RequestDump();
				return;
			}
			MatchStats->TakeSample();
			MatchStats->Dump();
		}));
}

bool UHOLMatchStatsSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHOLMatchStatsSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// Where the simulation runs only (solo, server): a client gets the statistics from the server (C3).
	UHOLSimulationSubsystem* Simulation = InWorld.GetNetMode() != NM_Client ? InWorld.GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
	if (!Simulation)
	{
		return;
	}
	// The steps between two samples at the first step: the simulation may learn its step length after this begin play.
	Stats.SampleIntervalSeconds = FMath::Max(UE::HeroesOfLiberty::MatchStats::SampleIntervalSeconds, 1);
	SampleSteps = 0;
	StepEndedHandle = Simulation->GetOnStepEnded().AddUObject(this, &UHOLMatchStatsSubsystem::OnStepEnded);
}

void UHOLMatchStatsSubsystem::Deinitialize()
{
	if (UHOLSimulationSubsystem* Simulation = GetWorld() ? GetWorld()->GetSubsystem<UHOLSimulationSubsystem>() : nullptr)
	{
		Simulation->GetOnStepEnded().Remove(StepEndedHandle);
	}
	Super::Deinitialize();
}

void UHOLMatchStatsSubsystem::OnStepEnded(const int64 StepIndex)
{
	LastStepIndex = StepIndex;
	if (SampleSteps <= 0)
	{
		const UHOLSimulationSubsystem* Simulation = GetWorld()->GetSubsystem<UHOLSimulationSubsystem>();
		SampleSteps = HOLMatchStats::GetSampleSteps(Simulation ? Simulation->GetStepSeconds() : 0.f, Stats.SampleIntervalSeconds);
	}
	// StepIndex counts the steps done: the step that just ended is StepIndex - 1, step 0 the first one (the curves start at t = 0).
	if (HOLMatchStats::IsSampleStep(StepIndex - 1, SampleSteps))
	{
		TakeSample();
	}
}

int32 UHOLMatchStatsSubsystem::GetTimeSeconds() const
{
	const UHOLSimulationSubsystem* Simulation = GetWorld() ? GetWorld()->GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
	return Simulation ? FMath::RoundToInt(static_cast<double>(LastStepIndex) * Simulation->GetStepSeconds()) : 0;
}

void UHOLMatchStatsSubsystem::RecordEvent(const int32 PlayerIndex, const EHOLMatchStatsEvent Kind)
{
	HOLMatchStats::RecordEvent(Stats, PlayerIndex, Kind, GetTimeSeconds());
}

void UHOLMatchStatsSubsystem::TakeSample()
{
	UWorld* World = GetWorld();
	const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	if (!GameState)
	{
		return;
	}
	// One walk of the unit table and of the buildings every SampleIntervalSeconds, not every step.
	TArray<int32> Workers;
	TArray<int32> Military;
	if (const UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>())
	{
		Units->GetLivingCountsByOwner(Workers, Military);
	}
	TMap<int32, int32> Buildings;
	for (TActorIterator<AHOLBuilding> It(World); It; ++It)
	{
		++Buildings.FindOrAdd(It->GetOwnerPlayer());
	}
	const UHOLSourceSubsystem* Sources = World->GetSubsystem<UHOLSourceSubsystem>();
	const int32 TimeSeconds = GetTimeSeconds();
	for (APlayerState* Each : GameState->PlayerArray)
	{
		const AHOLPlayerState* PlayerState = Cast<AHOLPlayerState>(Each);
		const int32 PlayerIndex = PlayerState ? PlayerState->GetPlayerIndex() : INDEX_NONE;
		if (PlayerIndex == INDEX_NONE)
		{
			continue;
		}
		FHOLMatchStatsSample Sample;
		Sample.TimeSeconds = TimeSeconds;
		Sample.Workers = Workers.IsValidIndex(PlayerIndex) ? Workers[PlayerIndex] : 0;
		Sample.Military = Military.IsValidIndex(PlayerIndex) ? Military[PlayerIndex] : 0;
		Sample.Population = PlayerState->GetPopulationUsed();
		Sample.Buildings = Buildings.FindRef(PlayerIndex);
		Sample.Gathered = Sources ? Sources->GetDeposited(static_cast<uint8>(PlayerIndex)) : FHOLResources();
		Sample.Stock = PlayerState->GetResources();
		// A dump at the very time of the last sample replaces it rather than doubling the point.
		const TArray<FHOLMatchStatsSample>* Samples = Stats.Players.IsValidIndex(PlayerIndex) ? &Stats.Players[PlayerIndex].Samples : nullptr;
		if (Samples && Samples->Num() > 0 && Samples->Last().TimeSeconds == TimeSeconds)
		{
			Stats.Players[PlayerIndex].Samples.Pop(EAllowShrinking::No);
		}
		HOLMatchStats::AddSample(Stats, PlayerIndex, Sample);
	}
}

void UHOLMatchStatsSubsystem::Dump() const
{
	if (GetWorld() && GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Stats: client copy received from the server (C3)"));
	}
	UE_LOG(LogHOLMatch, Display, TEXT("Stats: match time %d s, a sample every %d s (%d steps), %d events"), GetTimeSeconds(), Stats.SampleIntervalSeconds,
		SampleSteps, Stats.Events.Num());
	for (const FHOLPlayerMatchStats& Player : Stats.Players)
	{
		if (Player.PlayerIndex == INDEX_NONE)
		{
			continue;
		}
		UE_LOG(LogHOLMatch, Display, TEXT("Stats: %s"), *HOLMatchStats::DescribePlayer(Player));
		// The last three points of its curves.
		for (int32 Index = FMath::Max(Player.Samples.Num() - 3, 0); Index < Player.Samples.Num(); ++Index)
		{
			UE_LOG(LogHOLMatch, Display, TEXT("Stats:   slot %d %s"), Player.PlayerIndex, *HOLMatchStats::DescribeSample(Player.Samples[Index]));
		}
	}
	for (const FHOLMatchStatsEventEntry& Event : Stats.Events)
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Stats: event at %d s: slot %d %s"), Event.TimeSeconds, Event.PlayerIndex,
			*StaticEnum<EHOLMatchStatsEvent>()->GetNameStringByValue(static_cast<int64>(Event.Kind)));
	}
}

void UHOLMatchStatsSubsystem::RequestStats()
{
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() != NM_Client)
	{
		OnStatsUpdated.Broadcast();
		return;
	}
	AHOLPlayerController* PlayerController = World->GetFirstPlayerController<AHOLPlayerController>();
	if (!PlayerController)
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Stats: no player controller to ask the server with"));
		return;
	}
	PlayerController->ServerRequestMatchStats(HOLMatchStats::GetNextFromSample(Stats), Stats.Events.Num());
}

void UHOLMatchStatsSubsystem::RequestDump()
{
	bDumpWhenComplete = true;
	RequestStats();
}

void UHOLMatchStatsSubsystem::ReceiveChunk(const FHOLMatchStatsChunk& Chunk)
{
	HOLMatchStats::MergeChunk(Stats, Chunk);
	if (!Chunk.bComplete)
	{
		RequestStats();
		return;
	}
	OnStatsUpdated.Broadcast();
	if (bDumpWhenComplete)
	{
		bDumpWhenComplete = false;
		Dump();
	}
}

void UHOLMatchStatsSubsystem::ReceiveRefusal()
{
	bDumpWhenComplete = false;
	OnStatsRefused.Broadcast();
}
