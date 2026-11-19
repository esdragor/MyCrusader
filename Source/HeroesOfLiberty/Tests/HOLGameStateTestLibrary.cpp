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

// ---- Lot 2, rest (ELROND-1119u) ----

FHOLTestForumState UHOLGameStateTestLibrary::GetForumState(const AHOLGameState* State, const int32 Slot)
{
	FHOLTestForumState Out;
#if WITH_DEV_AUTOMATION_TESTS
	if (State)
	{
		const FHOLForumState& Forum = State->GetForumState(Slot);
		Out.Falls = Forum.Falls;
		Out.Phase = static_cast<int32>(Forum.Phase);
		Out.RuinStepsLeft = Forum.RuinStepsLeft;
		Out.HoldSteps = Forum.HoldSteps;
		Out.Hold = static_cast<int32>(Forum.Hold);
		Out.HolderSlot = Forum.HolderSlot;
		Out.bDown = Forum.IsDown();
	}
#endif
	return Out;
}

AHOLPlayerState* UHOLGameStateTestLibrary::GetPlayerStateOfSlot(const AHOLGameState* State, const int32 Slot)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (State)
	{
		for (APlayerState* Each : State->PlayerArray)
		{
			AHOLPlayerState* HOLEach = Cast<AHOLPlayerState>(Each);
			if (HOLEach && HOLEach->GetPlayerIndex() == Slot)
			{
				return HOLEach;
			}
		}
	}
#endif
	return nullptr;
}

TArray<FString> UHOLGameStateTestLibrary::GetAbsentNotices(const AHOLGameState* State)
{
	TArray<FString> Lines;
#if WITH_DEV_AUTOMATION_TESTS
	if (State)
	{
		for (const FHOLAbsentNotice& Notice : State->GetAbsentNotices())
		{
			Lines.Add(FString::Printf(TEXT("%d\t%s\t%.3f\t%.3f\t%.3f"), Notice.Slot, *Notice.Name, Notice.EndsAt, Notice.BackAt, Notice.WindowSeconds));
		}
	}
#endif
	return Lines;
}

int32 UHOLGameStateTestLibrary::CountAbsentNotices(const AHOLGameState* State)
{
#if WITH_DEV_AUTOMATION_TESTS
	return State ? State->GetAbsentNotices().Num() : 0;
#else
	return 0;
#endif
}

bool UHOLGameStateTestLibrary::HasAbsentNotice(const AHOLGameState* State, const int32 Slot, const bool bBack)
{
#if WITH_DEV_AUTOMATION_TESTS
	return State && State->GetAbsentNotices().ContainsByPredicate([Slot, bBack](const FHOLAbsentNotice& Notice)
	{
		return Notice.Slot == Slot && (bBack ? Notice.BackAt > 0. : Notice.BackAt <= 0.);
	});
#else
	return false;
#endif
}

TArray<FString> UHOLGameStateTestLibrary::GetAbsenceBannerLines(const AHOLGameState* State, const double Now)
{
	TArray<FString> Lines;
#if WITH_DEV_AUTOMATION_TESTS
	if (State)
	{
		for (const FText& Line : State->GetAbsenceBannerLines(Now))
		{
			Lines.Add(Line.ToString());
		}
	}
#endif
	return Lines;
}

double UHOLGameStateTestLibrary::GetWindowClock(const AHOLGameState* State, const double Now)
{
#if WITH_DEV_AUTOMATION_TESTS
	return State ? State->GetWindowClock(Now) : 0.;
#else
	return 0.;
#endif
}

TArray<int32> UHOLGameStateTestLibrary::GetSlotsInUse(const AHOLGameState* State, const AHOLPlayerState* Except)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (State)
	{
		return State->GetSlotsInUse(Except);
	}
#endif
	return TArray<int32>();
}

TArray<int32> UHOLGameStateTestLibrary::DefeatExpiredAbsences(AHOLGameState* State, const double Now, const double WindowSeconds)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (State)
	{
		return State->DefeatExpiredAbsences(Now, WindowSeconds > 0. ? WindowSeconds : FHOLDisconnectRules::DefaultWindowSeconds);
	}
#endif
	return TArray<int32>();
}

TArray<FString> UHOLGameStateTestLibrary::DescribeDisconnections(const AHOLGameState* State, const double Now, const double WindowSeconds)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (State)
	{
		return State->DescribeDisconnections(Now, WindowSeconds > 0. ? WindowSeconds : FHOLDisconnectRules::DefaultWindowSeconds);
	}
#endif
	return TArray<FString>();
}

bool UHOLGameStateTestLibrary::SimulateReturn(AHOLGameState* State, const int32 Slot, const FHOLResources& Starting, const double Now)
{
#if WITH_DEV_AUTOMATION_TESTS
	return State && State->SimulateReturn(Slot, Starting, Now);
#else
	return false;
#endif
}

int32 UHOLGameStateTestLibrary::FindSlotOnLogin(const AHOLGameState* State, const FString& Json, const FString& Name, const TArray<int32>& TakenSlots)
{
#if WITH_DEV_AUTOMATION_TESTS
	FHOLMatchConfig Config;
	if (State && FHOLMatchConfig::FromJson(Json, Config))
	{
		return State->FindSlotOnLogin(Config, Name, TakenSlots);
	}
#endif
	return INDEX_NONE;
}

int32 UHOLGameStateTestLibrary::FindSlotForLogin(const AHOLGameState* State, const FString& Json, const FString& Name, const AHOLPlayerState* Logging)
{
#if WITH_DEV_AUTOMATION_TESTS
	FHOLMatchConfig Config;
	if (State && Logging && FHOLMatchConfig::FromJson(Json, Config))
	{
		AHOLPlayerState* Replaced = nullptr;
		return State->FindSlotForLogin(Config, Name, *Logging, Replaced);
	}
#endif
	return INDEX_NONE;
}

AHOLPlayerState* UHOLGameStateTestLibrary::FindReplacedForLogin(const AHOLGameState* State, const FString& Json, const FString& Name, const AHOLPlayerState* Logging)
{
#if WITH_DEV_AUTOMATION_TESTS
	FHOLMatchConfig Config;
	if (State && Logging && FHOLMatchConfig::FromJson(Json, Config))
	{
		AHOLPlayerState* Replaced = nullptr;
		State->FindSlotForLogin(Config, Name, *Logging, Replaced);
		return Replaced;
	}
#endif
	return nullptr;
}

void UHOLGameStateTestLibrary::RetireHolder(AHOLGameState* State, AHOLPlayerState* Old, const double Now, const double WindowSeconds)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (State && Old)
	{
		State->RetireHolder(*Old, Now, WindowSeconds > 0. ? WindowSeconds : FHOLDisconnectRules::DefaultWindowSeconds);
	}
#endif
}

bool UHOLGameStateTestLibrary::RequestPause(AHOLGameState* State, AHOLPlayerState* Requester, const double Now)
{
#if WITH_DEV_AUTOMATION_TESTS
	return State && Requester && State->RequestPause(*Requester, Now);
#else
	return false;
#endif
}

bool UHOLGameStateTestLibrary::RequestResume(AHOLGameState* State, AHOLPlayerState* Requester, const double Now)
{
#if WITH_DEV_AUTOMATION_TESTS
	return State && Requester && State->RequestResume(*Requester, Now);
#else
	return false;
#endif
}

bool UHOLGameStateTestLibrary::LiftPauseOf(AHOLGameState* State, const int32 Slot, const double Now)
{
#if WITH_DEV_AUTOMATION_TESTS
	return State && State->LiftPauseOf(Slot, Now);
#else
	return false;
#endif
}

bool UHOLGameStateTestLibrary::IsMatchPaused(const AHOLGameState* State)
{
#if WITH_DEV_AUTOMATION_TESTS
	return State && State->IsMatchPaused();
#else
	return false;
#endif
}

int32 UHOLGameStateTestLibrary::GetPausesLeft(const AHOLGameState* State)
{
#if WITH_DEV_AUTOMATION_TESTS
	return State ? State->GetPausesLeft() : 0;
#else
	return 0;
#endif
}

FHOLResources UHOLGameStateTestLibrary::GetResources(const AHOLPlayerState* Player)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (Player)
	{
		return Player->GetResources();
	}
#endif
	return FHOLResources();
}

void UHOLGameStateTestLibrary::SetResources(AHOLPlayerState* Player, const FHOLResources& Resources)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (Player)
	{
		Player->SetResources(Resources);
	}
#endif
}

bool UHOLGameStateTestLibrary::AddTechnology(AHOLPlayerState* Player, const FName Technology)
{
#if WITH_DEV_AUTOMATION_TESTS
	return Player && Player->AddTechnology(Technology);
#else
	return false;
#endif
}

int32 UHOLGameStateTestLibrary::GetPlayerHeroXP(const AHOLPlayerState* Player)
{
#if WITH_DEV_AUTOMATION_TESTS
	return Player ? Player->GetHeroPrivate().XP : 0;
#else
	return 0;
#endif
}
