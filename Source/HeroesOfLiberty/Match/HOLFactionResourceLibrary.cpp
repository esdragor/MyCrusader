// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLFactionResourceLibrary.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLGameState.h"

namespace
{
	AHOLGameState* GetFactionGameStateOf(const UObject* WorldContextObject)
	{
		const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
		return World ? World->GetGameState<AHOLGameState>() : nullptr;
	}
}

int32 UHOLFactionResourceLibrary::Get(const UObject* WorldContextObject, const int32 Slot)
{
	const AHOLGameState* GameState = GetFactionGameStateOf(WorldContextObject);
	return GameState ? GameState->GetFactionResource(Slot) : 0;
}

void UHOLFactionResourceLibrary::Set(const UObject* WorldContextObject, const int32 Slot, const int32 Value)
{
	if (AHOLGameState* GameState = GetFactionGameStateOf(WorldContextObject))
	{
		GameState->SetFactionResource(Slot, Value);
	}
}