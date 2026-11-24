// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLSpecializationReadLibrary.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLPlayerState.h"
#include "Match/HOLSpecializationSubsystem.h"
#include "Simulation/HOLHeroSubsystem.h"

bool UHOLSpecializationReadLibrary::IsForumCompleteForSlot(const UObject* WorldContextObject, const int32 Slot)
{
	// One source: the calculation the server refuses an unlock with (NoForumComplete); nothing recopied here.
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	return UHOLSpecializationSubsystem::IsForumComplete(World, Slot);
}

int32 UHOLSpecializationReadLibrary::GetChosenOptionForSlot(const UObject* WorldContextObject, const int32 Slot, const int32 Tier)
{
	// The seat's player state through the same finder the hero's gold uses; its view's own reader bounds the tier (-1 out of range).
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const AHOLPlayerState* Seat = World ? UHOLHeroSubsystem::FindPlayerState(*World, Slot) : nullptr;
	return Seat ? Seat->GetSpecialization().GetChosen(Tier) : -1;
}