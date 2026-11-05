// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAITools.h"

#include "AI/HOLAIBrain.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"

namespace UE::HeroesOfLiberty::AITools
{
	bool bEvolveTimes = false;
	float EvolveNormalAt = 0.f;
	float EvolveHardAt = 0.f;

	void RunSetEvolveTimes(const TArray<FString>& Args)
	{
		if (Args.Num() == 0 || Args[0].Equals(TEXT("reset"), ESearchCase::IgnoreCase))
		{
			UHOLAIToolsLibrary::ClearEvolveTimes();
			return;
		}
		if (Args.Num() < 2 || !Args[0].IsNumeric() || !Args[1].IsNumeric())
		{
			UE_LOG(LogHOLMatch, Warning, TEXT("hol.AI.SetEvolveTimes <normal> <hard> (game seconds), or reset"));
			return;
		}
		UHOLAIToolsLibrary::SetEvolveTimes(FCString::Atof(*Args[0]), FCString::Atof(*Args[1]));
	}

	FAutoConsoleCommand CmdSetEvolveTimes(TEXT("hol.AI.SetEvolveTimes"),
		TEXT("hol.AI.SetEvolveTimes <normal> <hard>: the game times (s) at which the Evolving AI plays Normal then Hard (trial tool, D620); no value or reset: the usual times"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&RunSetEvolveTimes));
}

void UHOLAIToolsLibrary::SetEvolveTimes(const float NormalAt, const float HardAt)
{
	using namespace UE::HeroesOfLiberty::AITools;
	bEvolveTimes = true;
	EvolveNormalAt = FMath::Max(0.f, NormalAt);
	EvolveHardAt = FMath::Max(EvolveNormalAt, HardAt);
	UE_LOG(LogHOLMatch, Log, TEXT("AI trial tool: the Evolving AI plays Normal from %.1f s and Hard from %.1f s"), EvolveNormalAt, EvolveHardAt);
}

void UHOLAIToolsLibrary::ClearEvolveTimes()
{
	UE::HeroesOfLiberty::AITools::bEvolveTimes = false;
	UE_LOG(LogHOLMatch, Log, TEXT("AI trial tool: the usual times of the Evolving AI stand"));
}

bool UHOLAIToolsLibrary::HasEvolveTimes()
{
	return UE::HeroesOfLiberty::AITools::bEvolveTimes;
}

float UHOLAIToolsLibrary::GetEvolveNormalAt()
{
	return UE::HeroesOfLiberty::AITools::EvolveNormalAt;
}

float UHOLAIToolsLibrary::GetEvolveHardAt()
{
	return UE::HeroesOfLiberty::AITools::EvolveHardAt;
}

int32 UHOLAIToolsLibrary::GetSlotLevel(const UObject* WorldContextObject, const int32 Slot)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	if (!GameState || !GameState->GetLaunchConfig().Slots.IsValidIndex(Slot))
	{
		return -1;
	}
	// D717 (T2): the seat of a player who left, held by the AI, plays at the Normal level.
	const UHOLAIBrainSubsystem* Brain = UHOLAIBrainSubsystem::Find(World);
	if (Brain && (Brain->IsHeld(Slot) || Brain->IsFullBrain(Slot)))
	{
		return static_cast<int32>(EHOLAIDifficulty::Normal);
	}
	const FHOLMatchSlot& MatchSlot = GameState->GetLaunchConfig().Slots[Slot];
	return MatchSlot.State == EHOLSlotState::AI ? static_cast<int32>(MatchSlot.Difficulty) : -1;
}
