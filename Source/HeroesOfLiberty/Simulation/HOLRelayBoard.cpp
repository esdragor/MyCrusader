// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLRelayBoard.h"

#include "Engine/World.h"

namespace
{
	FHOLRelayState ToState(const FHOLRelayBoardState& Board)
	{
		FHOLRelayState State;
		State.Owner = Board.Owner;
		State.Taker = Board.Taker;
		State.Held = Board.Held;
		return State;
	}

	FHOLRelayBoardState ToBoard(const FHOLRelayState& State)
	{
		FHOLRelayBoardState Board;
		Board.Owner = State.Owner;
		Board.Taker = State.Taker;
		Board.Held = State.Held;
		return Board;
	}

	// The C++ rule is gone (the script is the only source): without the board the answer is inert; said once, not at every step.
	void LogNoBoard(const TCHAR* What)
	{
		static bool bLogged = false;
		if (!bLogged)
		{
			bLogged = true;
			UE_LOG(LogTemp, Error, TEXT("HOLRelayBoard: %s asked without the script board (Script/Relays/HOLRelayBoardScript.as not compiled): the relay rules are script-only, the answer is inert"), What);
		}
	}
}

UHOLRelayBoard* UHOLRelayBoard::Find(const UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	const TArray<UHOLRelayBoard*> Boards = World->GetSubsystemArrayCopy<UHOLRelayBoard>();
	return Boards.Num() > 0 ? Boards[0] : nullptr;
}

bool UHOLRelayBoard::ShouldCreateSubsystem(UObject* Outer) const
{
	// Never the abstract C++ class, nor a script class replaced by a reload (as UHOLGarrisonBoard).
	if (IsUnreachable() || GetClass()->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists))
	{
		return false;
	}
	return Super::ShouldCreateSubsystem(Outer);
}

bool UHOLRelayBoard::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

FHOLRelayState UHOLRelayBoard::StepForRelay(const UWorld* World, const FHOLRelayState& State, const TArray<int32>& UnitsBySlot,
	const TArray<int32>& TeamBySlot, const int32 HoldSteps, const int32 DecaySteps)
{
	UHOLRelayBoard* Board = Find(World);
	if (!Board)
	{
		LogNoBoard(TEXT("StepForRelay"));
		return State;
	}
	return ToState(Board->StepNow(ToBoard(State), UnitsBySlot, TeamBySlot, HoldSteps, DecaySteps));
}

int32 UHOLRelayBoard::TerritoryXPFor(const UWorld* World, int32& Carry, const int32 Relays, const int32 Centres, const int32 StepsPerSecond)
{
	UHOLRelayBoard* Board = Find(World);
	if (!Board)
	{
		LogNoBoard(TEXT("TerritoryXPFor"));
		return 0;
	}
	const FHOLRelayXPResult Result = Board->TerritoryXPNow(Carry, Relays, Centres, StepsPerSecond);
	Carry = Result.Carry;
	return Result.Award;
}

FHOLRelayBoardState UHOLRelayBoard::StepNow(const FHOLRelayBoardState& State, const TArray<int32>& UnitsBySlot, const TArray<int32>& TeamBySlot,
	const int32 HoldSteps, const int32 DecaySteps)
{
	return RelayStep(State, UnitsBySlot, TeamBySlot, HoldSteps, DecaySteps);
}

FHOLRelayXPResult UHOLRelayBoard::TerritoryXPNow(const int32 Carry, const int32 Relays, const int32 Centres, const int32 StepsPerSecond)
{
	return RelayTerritoryXP(Carry, Relays, Centres, StepsPerSecond);
}

FHOLRelayBoardState UHOLRelayBoard::RelayStep_Implementation(const FHOLRelayBoardState& State, const TArray<int32>& UnitsBySlot,
	const TArray<int32>& TeamBySlot, const int32 HoldSteps, const int32 DecaySteps)
{
	LogNoBoard(TEXT("RelayStep"));
	return State;
}

FHOLRelayXPResult UHOLRelayBoard::RelayTerritoryXP_Implementation(int32 Carry, const int32 Relays, const int32 Centres, const int32 StepsPerSecond)
{
	LogNoBoard(TEXT("RelayTerritoryXP"));
	FHOLRelayXPResult Result;
	Result.Award = 0;
	Result.Carry = Carry;
	return Result;
}
