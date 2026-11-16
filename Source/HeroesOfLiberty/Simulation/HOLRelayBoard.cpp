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
		return HOLRelay::Step(State, UnitsBySlot, TeamBySlot, HoldSteps, DecaySteps);
	}
	return ToState(Board->StepNow(ToBoard(State), UnitsBySlot, TeamBySlot, HoldSteps, DecaySteps));
}

int32 UHOLRelayBoard::TerritoryXPFor(const UWorld* World, int32& Carry, const int32 Relays, const int32 Centres, const int32 StepsPerSecond)
{
	UHOLRelayBoard* Board = Find(World);
	if (!Board)
	{
		return HOLRelay::TerritoryXP(Carry, Relays, Centres, StepsPerSecond);
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
	// The C++ rule, the same answer as the script's.
	return ToBoard(HOLRelay::Step(ToState(State), UnitsBySlot, TeamBySlot, HoldSteps, DecaySteps));
}

FHOLRelayXPResult UHOLRelayBoard::RelayTerritoryXP_Implementation(int32 Carry, const int32 Relays, const int32 Centres, const int32 StepsPerSecond)
{
	FHOLRelayXPResult Result;
	Result.Award = HOLRelay::TerritoryXP(Carry, Relays, Centres, StepsPerSecond);
	Result.Carry = Carry;
	return Result;
}
