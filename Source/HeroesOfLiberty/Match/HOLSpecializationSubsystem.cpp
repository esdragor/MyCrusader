// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLSpecializationSubsystem.h"

#include "Engine/World.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLGameState.h"
#include "Match/HOLPlayerState.h"

UHOLSpecializationSubsystem* UHOLSpecializationSubsystem::Find(const UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	const TArray<UHOLSpecializationSubsystem*> Boards = World->GetSubsystemArrayCopy<UHOLSpecializationSubsystem>();
	return Boards.Num() > 0 ? Boards[0] : nullptr;
}

bool UHOLSpecializationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Never the abstract C++ class, nor a script class replaced by a reload (as UHOLSignalSubsystem).
	if (IsUnreachable() || GetClass()->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists))
	{
		return false;
	}
	return Super::ShouldCreateSubsystem(Outer);
}

bool UHOLSpecializationSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId UHOLSpecializationSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UHOLSpecializationSubsystem, STATGROUP_Tickables);
}

bool UHOLSpecializationSubsystem::IsForumComplete(const UWorld* World, const int32 Slot)
{
	// Answer B of Q-nomad-forum Q2: HOLNomad::HasCompleteForum of the seat (the script board's state); a standard seat is always true.
	if (!World || Slot < 0)
	{
		return false;
	}
	UHOLSpecializationSubsystem* Board = Find(World);
	return !Board || Board->HasCompleteForumNow(Slot);
}

void UHOLSpecializationSubsystem::SetNomadNow(const int32 Slot, const bool bNomad)
{
	SetSeatNomad(Slot, bNomad);
}

void UHOLSpecializationSubsystem::NoteForumCompleteNow(const int32 Slot)
{
	NoteSeatForumComplete(Slot);
}

bool UHOLSpecializationSubsystem::HasCompleteForumNow(const int32 Slot)
{
	return SeatHasCompleteForum(Slot);
}

bool UHOLSpecializationSubsystem::IsForumStanding(const UWorld* World, const int32 Slot)
{
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	return !GameState || !GameState->IsForumDown(Slot);
}

int32 UHOLSpecializationSubsystem::StartNow(const int32 Slot, const int32 Tier, const int32 Option, const int32 PlayerTier, const bool bForumComplete)
{
	return StartSpecialization(Slot, Tier, Option, PlayerTier, bForumComplete);
}

int32 UHOLSpecializationSubsystem::CancelNow(const int32 Slot)
{
	return CancelSpecialization(Slot);
}

bool UHOLSpecializationSubsystem::IsOpenNow(const int32 Slot, const int32 Tier, const int32 PlayerTier, const bool bForumComplete)
{
	return IsSpecializationOpen(Slot, Tier, PlayerTier, bForumComplete);
}

int32 UHOLSpecializationSubsystem::AdvanceNow(const int32 Slot, const int32 ElapsedMs, const bool bForumStanding)
{
	const int32 Applied = AdvanceSpecialization(Slot, ElapsedMs, bForumStanding);
	if (Applied > 0)
	{
		const int32 Option = ViewNow(Slot).GetChosen(Applied);
		UE_LOG(LogHOLMatch, Display, TEXT("Specialization: slot %d tier %d option %d applied"), Slot, Applied, Option);
		OnApplied.Broadcast(Slot, Applied, Option);
	}
	return Applied;
}

FHOLSpecializationView UHOLSpecializationSubsystem::ViewNow(const int32 Slot)
{
	return ViewSpecialization(Slot);
}

void UHOLSpecializationSubsystem::RefreshView(AHOLPlayerState& PlayerState)
{
	const int32 Slot = PlayerState.GetPlayerIndex();
	if (Slot >= 0)
	{
		PlayerState.SetSpecialization(ViewNow(Slot));
	}
}

void UHOLSpecializationSubsystem::AdvanceAllNow(const int32 ElapsedMs)
{
	UWorld* World = GetWorld();
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	if (!GameState || ElapsedMs <= 0)
	{
		return;
	}
	for (APlayerState* Each : GameState->PlayerArray)
	{
		AHOLPlayerState* PlayerState = Cast<AHOLPlayerState>(Each);
		const int32 Slot = PlayerState ? PlayerState->GetPlayerIndex() : INDEX_NONE;
		if (Slot < 0)
		{
			continue;
		}
		AdvanceNow(Slot, ElapsedMs, IsForumStanding(World, Slot));
		RefreshView(*PlayerState);
	}
}

void UHOLSpecializationSubsystem::Tick(const float DeltaTime)
{
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		return;
	}
	// The world's time (stopped while the game is paused), in whole milliseconds so no fraction is lost from frame to frame.
	const int64 NowMs = static_cast<int64>(World->GetTimeSeconds() * 1000.0);
	if (LastTickMs >= 0 && NowMs > LastTickMs)
	{
		AdvanceAllNow(static_cast<int32>(FMath::Min<int64>(NowMs - LastTickMs, MAX_int32)));
	}
	LastTickMs = NowMs;
}
