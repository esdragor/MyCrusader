// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLRelayLibrary.h"

#include "Simulation/HOLRelayRules.h"
#include "Simulation/HOLRelaySubsystem.h"

#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"

namespace
{
	int32 AIRelayMode = 1;
	FAutoConsoleVariableRef CVarAIRelayMode(TEXT("hol.AI.RelayMode"), AIRelayMode,
		TEXT("What the AI does with the relays (RQ3 = B, D589): 0 ignores them (A), 1 a detachment by level (B, default), 2 the whole army (C)"));
}

int32 UHOLRelayRulesLibrary::GetHoldSeconds()
{
	return FMath::RoundToInt(HOLRelay::DefaultHoldSeconds);
}

int32 UHOLRelayRulesLibrary::GetDecaySteps()
{
	return HOLRelay::DefaultDecaySteps;
}

void UHOLRelayRulesLibrary::ReadAIRelays(const UObject* WorldContextObject, TArray<FHOLAIRelay>& OutRelays)
{
	OutRelays.Reset();
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const UHOLRelaySubsystem* Relays = World ? World->GetSubsystem<UHOLRelaySubsystem>() : nullptr;
	if (!Relays)
	{
		return;
	}
	for (const UHOLRelaySubsystem::FRelaySite& Site : Relays->GetRelays())
	{
		FHOLAIRelay Relay;
		Relay.Location = FVector(Site.Location.X, Site.Location.Y, 0.0);
		Relay.Radius = Site.Radius;
		Relay.Owner = Site.State.Owner;
		Relay.Taker = Site.State.Taker;
		OutRelays.Add(Relay);
	}
}

int32 UHOLRelayRulesLibrary::GetAIRelayMode()
{
	return FMath::Clamp(AIRelayMode, 0, 2);
}
