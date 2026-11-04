// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLBiomeReadLibrary.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"

namespace
{
	/** The key of the biome of Game's match: its configuration's, "default" when it has none (an empty key from a corrupt file included). */
	FString GetBiomeKeyOf(const AHOLGameState* GameState)
	{
		const FString Key = GameState ? GameState->GetLaunchConfig().Biome.TrimStartAndEnd() : FString();
		return Key.IsEmpty() ? FString(TEXT("default")) : Key;
	}

	/**
	 * hol.Biome.Report: "Biome: snow (server)" for each game world of this process (an editor's server and a client may share one),
	 * what the net tests read in the server's and in each client's log to see that everyone holds the lobby's biome.
	 */
	FAutoConsoleCommand CommandBiomeReport(TEXT("hol.Biome.Report"),
		TEXT("The biome key of the match this process plays, one line per game world: 'Biome: snow (client)'"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				const UWorld* World = Context.World();
				if (!World || !World->IsGameWorld())
				{
					continue;
				}
				const TCHAR* Role = World->GetNetMode() == NM_Client ? TEXT("client") : (World->GetNetMode() == NM_Standalone ? TEXT("standalone") : TEXT("server"));
				UE_LOG(LogHOLMatch, Display, TEXT("Biome: %s (%s)"), *GetBiomeKeyOf(World->GetGameState<AHOLGameState>()), Role);
			}
		}));
}

FString UHOLBiomeReadLibrary::GetMatchBiomeKey(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	return GetBiomeKeyOf(World ? World->GetGameState<AHOLGameState>() : nullptr);
}
