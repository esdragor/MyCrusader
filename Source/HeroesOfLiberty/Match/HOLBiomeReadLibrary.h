// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLBiomeReadLibrary.generated.h"

/**
 * B2 (D651-D656, BiomeInProgress.md): what the script reads of the match's biome (HOLBiomeRead:: in script). The biome is the lobby's
 * choice, written in the match configuration (FHOLMatchConfig::Biome) and replicated to every client with AHOLGameState::LaunchConfig, so
 * the server and each client read the same key. The table of the five biomes and the rules that turn a key into a look are in
 * Script/Biomes/HOLBiomeRules.as (HOLBiome::IndexOfKey, Resolve); this library carries only the key.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLBiomeReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * The biome key of this machine's match: "default" (the map's own biome) or one of the table's keys. "default" too when no match is
	 * running or the configuration has not reached this client yet.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Biome", meta = (WorldContext = "WorldContextObject"))
	static FString GetMatchBiomeKey(const UObject* WorldContextObject);
};
