// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLSiteRulesLibrary.h"

#include "Simulation/HOLSiteRules.h"
#include "Engine/World.h"
#include "Match/HOLGameState.h"

int32 UHOLSiteRulesLibrary::GetHoldSeconds()
{
	return HOLSite::DefaultHoldSeconds;
}

void UHOLSiteRulesLibrary::GetVictoryCount(const UObject* WorldContextObject, int32& OutLeader, int32& OutSecondsLeft, bool& bOutFrozen, int32& OutWinner)
{
	FHOLSiteCountNet Count;
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr)
	{
		Count = GameState->GetSiteCount();
	}
	OutLeader = Count.Leader;
	OutSecondsLeft = Count.SecondsLeft;
	bOutFrozen = Count.bFrozen;
	OutWinner = Count.Winner;
}

int32 UHOLSiteRulesLibrary::GetDecaySteps()
{
	return HOLSite::DefaultDecaySteps;
}
