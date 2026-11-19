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
