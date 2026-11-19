// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLMatchFlowSubsystem.h"

#include "Engine/World.h"

UHOLMatchFlowSubsystem* UHOLMatchFlowSubsystem::Find(const UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	const TArray<UHOLMatchFlowSubsystem*> Boards = World->GetSubsystemArrayCopy<UHOLMatchFlowSubsystem>();
	return Boards.Num() > 0 ? Boards[0] : nullptr;
}

int32 UHOLMatchFlowSubsystem::AskCanPause(const UWorld* World, const bool bSolo, const int32 PausedBySlot, const int32 PausesLeft, const bool bRequesterInMatch)
{
	UHOLMatchFlowSubsystem* Board = Find(World);
	return Board ? Board->PauseAnswer(bSolo, PausedBySlot, PausesLeft, bRequesterInMatch) : -1;
}

int32 UHOLMatchFlowSubsystem::AskCanResume(const UWorld* World, const bool bSolo, const int32 PausedBySlot, const double PausedAtSeconds, const int32 RequesterSlot, const bool bRequesterInMatch, const double Now)
{
	UHOLMatchFlowSubsystem* Board = Find(World);
	return Board ? Board->ResumeAnswer(bSolo, PausedBySlot, PausedAtSeconds, RequesterSlot, bRequesterInMatch, Now) : -1;
}

int32 UHOLMatchFlowSubsystem::AskPausesLeftAfterPause(const UWorld* World, const bool bSolo, const int32 PausesLeft)
{
	UHOLMatchFlowSubsystem* Board = Find(World);
	return Board ? Board->PausesAfterPause(bSolo, PausesLeft) : -1;
}

int32 UHOLMatchFlowSubsystem::AskStanceGate(const UWorld* World, const bool bAuthority, const bool bDiplomacyMode, const bool bFromOccupied, const bool bToOccupied)
{
	UHOLMatchFlowSubsystem* Board = Find(World);
	return Board ? Board->StanceGateAnswer(bAuthority, bDiplomacyMode, bFromOccupied, bToOccupied) : -1;
}

int32 UHOLMatchFlowSubsystem::AskVisionGate(const UWorld* World, const bool bAuthority, const bool bDiplomacyMode)
{
	UHOLMatchFlowSubsystem* Board = Find(World);
	return Board ? Board->VisionGateAnswer(bAuthority, bDiplomacyMode) : -1;
}

int32 UHOLMatchFlowSubsystem::AskEvaluateMatchEnd(const UWorld* World, const bool bAnnounce, const bool bFormed)
{
	UHOLMatchFlowSubsystem* Board = Find(World);
	return Board ? Board->EvaluateEndAnswer(bAnnounce, bFormed) : -1;
}

int32 UHOLMatchFlowSubsystem::AskDefeatGate(const UWorld* World, const bool bAuthority, const bool bMatchOver, const bool bReasonNone, const bool bSlotValid, const bool bDefeated, const bool bCampOccupied)
{
	UHOLMatchFlowSubsystem* Board = Find(World);
	return Board ? Board->DefeatGateAnswer(bAuthority, bMatchOver, bReasonNone, bSlotValid, bDefeated, bCampOccupied) : -1;
}

int32 UHOLMatchFlowSubsystem::AskFallGate(const UWorld* World, const bool bAuthority, const bool bPrimaryForum, const bool bSlotValid, const bool bDefeated, const bool bPendingForum, const bool bCampOccupied)
{
	UHOLMatchFlowSubsystem* Board = Find(World);
	return Board ? Board->FallGateAnswer(bAuthority, bPrimaryForum, bSlotValid, bDefeated, bPendingForum, bCampOccupied) : -1;
}

int32 UHOLMatchFlowSubsystem::AskKingGate(const UWorld* World, const bool bAuthority, const bool bMatchOver, const bool bSlotValid, const bool bDefeated, const bool bPendingKing, const bool bCampOccupied)
{
	UHOLMatchFlowSubsystem* Board = Find(World);
	return Board ? Board->KingGateAnswer(bAuthority, bMatchOver, bSlotValid, bDefeated, bPendingKing, bCampOccupied) : -1;
}

int32 UHOLMatchFlowSubsystem::AskMatchEnd(const UWorld* World, const TArray<int32>& States, const TArray<int32>& Teams, const int32 DefeatedMask, const bool bDiplomacyMode, const int32 AlliedWinnerMask)
{
	UHOLMatchFlowSubsystem* Board = Find(World);
	return Board ? Board->MatchEndAnswer(States, Teams, DefeatedMask, bDiplomacyMode, AlliedWinnerMask) : -1;
}

bool UHOLMatchFlowSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Never the abstract C++ class, nor a script class replaced by a reload (as UHOLUnitTierSubsystem).
	if (IsUnreachable() || GetClass()->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists))
	{
		return false;
	}
	return Super::ShouldCreateSubsystem(Outer);
}

bool UHOLMatchFlowSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

int32 UHOLMatchFlowSubsystem::PauseAnswer_Implementation(const bool, const int32, const int32, const bool) { return -1; }
int32 UHOLMatchFlowSubsystem::ResumeAnswer_Implementation(const bool, const int32, const double, const int32, const bool, const double) { return -1; }
int32 UHOLMatchFlowSubsystem::PausesAfterPause_Implementation(const bool, const int32) { return -1; }
int32 UHOLMatchFlowSubsystem::StanceGateAnswer_Implementation(const bool, const bool, const bool, const bool) { return -1; }
int32 UHOLMatchFlowSubsystem::VisionGateAnswer_Implementation(const bool, const bool) { return -1; }
int32 UHOLMatchFlowSubsystem::EvaluateEndAnswer_Implementation(const bool, const bool) { return -1; }
int32 UHOLMatchFlowSubsystem::DefeatGateAnswer_Implementation(const bool, const bool, const bool, const bool, const bool, const bool) { return -1; }
int32 UHOLMatchFlowSubsystem::FallGateAnswer_Implementation(const bool, const bool, const bool, const bool, const bool, const bool) { return -1; }
int32 UHOLMatchFlowSubsystem::KingGateAnswer_Implementation(const bool, const bool, const bool, const bool, const bool, const bool) { return -1; }
int32 UHOLMatchFlowSubsystem::MatchEndAnswer_Implementation(const TArray<int32>&, const TArray<int32>&, const int32, const bool, const int32) { return -1; }
