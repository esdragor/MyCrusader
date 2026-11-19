// Copyright Epic Games, Inc. All Rights Reserved.

#include "Tests/HOLGameStateTestLibrary.h"

#include "Engine/World.h"
#include "Match/HOLDisconnectRules.h"
#include "Match/HOLForumMessages.h"
#include "Match/HOLForumRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLMatchStats.h"
#include "Match/HOLMatchStatsSubsystem.h"
#include "Match/HOLPlayerState.h"

// Every body below exists only where WITH_DEV_AUTOMATION_TESTS (the motif of HOLTestWorldLibrary.cpp): elsewhere 0 / false / empty.

bool UHOLGameStateTestLibrary::InitVictoryFromJson(AHOLGameState* State, const FString& Json)
{
#if WITH_DEV_AUTOMATION_TESTS
	FHOLMatchConfig Config;
	if (State && FHOLMatchConfig::FromJson(Json, Config))
	{
		State->InitVictory(Config);
		return true;
	}
#endif
	return false;
}

bool UHOLGameStateTestLibrary::DefeatSlot(AHOLGameState* State, const int32 Slot, const int32 Reason)
{
#if WITH_DEV_AUTOMATION_TESTS
	return State && State->DefeatSlot(Slot, static_cast<EHOLDefeatReason>(Reason));
#else
	return false;
#endif
}

bool UHOLGameStateTestLibrary::IsSlotDefeated(const AHOLGameState* State, const int32 Slot)
{
#if WITH_DEV_AUTOMATION_TESTS
	return State && State->IsSlotDefeated(Slot);
#else
	return false;
#endif
}

int32 UHOLGameStateTestLibrary::GetDefeatReason(const AHOLGameState* State, const int32 Slot)
{
#if WITH_DEV_AUTOMATION_TESTS
	return State ? static_cast<int32>(State->GetDefeatReason(Slot)) : 0;
#else
	return 0;
#endif
}

bool UHOLGameStateTestLibrary::IsMatchOver(const AHOLGameState* State)
{
#if WITH_DEV_AUTOMATION_TESTS
	return State && State->IsMatchOver();
#else
	return false;
#endif
}

int32 UHOLGameStateTestLibrary::GetLives(const AHOLGameState* State, const int32 Slot)
{
#if WITH_DEV_AUTOMATION_TESTS
	return State ? State->GetLives(Slot) : 0;
#else
	return 0;
#endif
}

bool UHOLGameStateTestLibrary::IsForumDown(const AHOLGameState* State, const int32 Slot)
{
#if WITH_DEV_AUTOMATION_TESTS
	return State && State->IsForumDown(Slot);
#else
	return false;
#endif
}

void UHOLGameStateTestLibrary::AddPlayerState(AHOLGameState* State, AHOLPlayerState* Player)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (State && Player)
	{
		State->AddPlayerState(Player);
	}
#endif
}

bool UHOLGameStateTestLibrary::GiveSlotStock(AHOLGameState* State, AHOLPlayerState* Player, const FHOLResources& Starting, const double Now)
{
#if WITH_DEV_AUTOMATION_TESTS
	return State && Player && State->GiveSlotStock(*Player, Starting, Now);
#else
	return false;
#endif
}

bool UHOLGameStateTestLibrary::KeepAbsentSlot(AHOLGameState* State, AHOLPlayerState* Player, const double Now, const double WindowSeconds)
{
#if WITH_DEV_AUTOMATION_TESTS
	return State && Player && State->KeepAbsentSlot(*Player, Now, WindowSeconds > 0. ? WindowSeconds : FHOLDisconnectRules::DefaultWindowSeconds);
#else
	return false;
#endif
}

bool UHOLGameStateTestLibrary::IsSlotAbsent(const AHOLGameState* State, const int32 Slot)
{
#if WITH_DEV_AUTOMATION_TESTS
	return State && State->IsSlotAbsent(Slot);
#else
	return false;
#endif
}

UHOLForumEventRecorder* UHOLGameStateTestLibrary::RecordForumEvents(AHOLGameState* State)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (!State)
	{
		return nullptr;
	}
	UHOLForumEventRecorder* Recorder = NewObject<UHOLForumEventRecorder>(GetTransientPackage());
	State->OnForumEvent.AddLambda([Weak = TWeakObjectPtr<UHOLForumEventRecorder>(Recorder)](const FHOLForumEvent& Event)
	{
		if (UHOLForumEventRecorder* Held = Weak.Get())
		{
			Held->Kinds.Add(static_cast<int32>(Event.Kind));
			Held->Slots.Add(Event.Slot);
			Held->OtherSlots.Add(Event.OtherSlot);
			Held->Lives.Add(Event.Lives);
			Held->Reasons.Add(static_cast<int32>(Event.Reason));
		}
	});
	return Recorder;
#else
	return nullptr;
#endif
}

int32 UHOLGameStateTestLibrary::CountStatsDefeats(const AHOLGameState* State, const int32 PlayerIndex)
{
	int32 Defeats = 0;
#if WITH_DEV_AUTOMATION_TESTS
	const UWorld* World = State ? State->GetWorld() : nullptr;
	if (const UHOLMatchStatsSubsystem* Stats = World ? World->GetSubsystem<UHOLMatchStatsSubsystem>() : nullptr)
	{
		for (const FHOLMatchStatsEventEntry& Entry : Stats->GetStats().Events)
		{
			Defeats += (Entry.Kind == EHOLMatchStatsEvent::Defeated && Entry.PlayerIndex == PlayerIndex) ? 1 : 0;
		}
	}
#endif
	return Defeats;
}

TArray<FString> UHOLGameStateTestLibrary::GetPlayersTableRows(const FString& Json, const int32 DefeatedMask, const int32 LocalSlot, const int32 AbsentMask, const int32 AIHeldMask)
{
	TArray<FString> Lines;
#if WITH_DEV_AUTOMATION_TESTS
	FHOLMatchConfig Config;
	if (FHOLMatchConfig::FromJson(Json, Config))
	{
		const TArray<FHOLForumState> Forums;
		const TArray<FHOLPlayersTableRow> Rows = HOLForumRules::GetPlayersTable(Config, Forums, static_cast<uint8>(DefeatedMask), LocalSlot,
			FHOLForumTuning(), {}, nullptr, static_cast<uint8>(AbsentMask), static_cast<uint8>(AIHeldMask));
		for (const FHOLPlayersTableRow& Row : Rows)
		{
			Lines.Add(FString::Printf(TEXT("%s\t%s\t%d\t%d\t%s"), *Row.Name.ToString(), *Row.State.ToString(), Row.bAway ? 1 : 0, Row.bAIHeld ? 1 : 0, *Row.Shown));
		}
	}
#endif
	return Lines;
}

FString UHOLGameStateTestLibrary::FormatSeatReport(const int32 Slot, const FString& Name, const int32 SeatState, const double SecondsLeft, const int32 AIMode)
{
#if WITH_DEV_AUTOMATION_TESTS
	return FHOLDisconnectRules::FormatSeatReport(Slot, Name, static_cast<FHOLDisconnectRules::ESeatState>(SeatState), SecondsLeft,
		static_cast<FHOLDisconnectRules::EAIMode>(AIMode));
#else
	return FString();
#endif
}
