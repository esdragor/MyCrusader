// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Match/HOLMatchConfig.h"
#include "HeroesOfLibertyGameModeBase.generated.h"

/**
 * Game mode of a match: reads the match configuration when the map starts (D243) and gives each joining human
 * the next free human slot (a slot frees itself when its player state leaves PlayerArray). Without a configuration
 * it runs with the default one (one human in slot 0), so the stress test and BattleMap opened directly keep working.
 * On a match map (one with start slots, D256) the players get the match controller and its RTS camera, and no pawn
 * (D255); a map without start slots (StressTest, BattleMap) keeps the engine's controller and default pawn.
 */
UCLASS()
class HEROESOFLIBERTY_API AHeroesOfLibertyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHeroesOfLibertyGameModeBase();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	const FHOLMatchConfig& GetMatchConfig() const { return MatchConfig; }

private:
	FHOLMatchConfig MatchConfig;
};
