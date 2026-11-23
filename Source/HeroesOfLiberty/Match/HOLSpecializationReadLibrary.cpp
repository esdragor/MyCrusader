// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLSpecializationReadLibrary.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLSpecializationSubsystem.h"

bool UHOLSpecializationReadLibrary::IsForumCompleteForSlot(const UObject* WorldContextObject, const int32 Slot)
{
	// One source: the calculation the server refuses an unlock with (NoForumComplete); nothing recopied here.
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	return UHOLSpecializationSubsystem::IsForumComplete(World, Slot);
}