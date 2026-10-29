// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLPauseLibrary.generated.h"

/**
 * The limited pause (P5, D513) for Blueprint and Angelscript (HOLPause:: in script): what every machine knows of it from
 * the game state's replicated copy (who paused, the timer, the pauses left), and whether a slot may pause or resume now
 * (FHOLPauseRules, as the server will decide). The requests go through AHOLPlayerController::RequestPause / RequestResume.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLPauseLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Pause", meta = (WorldContext = "WorldContextObject"))
	static bool IsMatchPaused(const UObject* WorldContextObject);

	/** The name of the player who paused, "" while the match runs. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Pause", meta = (WorldContext = "WorldContextObject"))
	static FString GetPausedByName(const UObject* WorldContextObject);

	/** The slot who paused, -1 while the match runs. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Pause", meta = (WorldContext = "WorldContextObject"))
	static int32 GetPausedBySlot(const UObject* WorldContextObject);

	/** The pauses left to the match, shared by all its players (-1 without a game state). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Pause", meta = (WorldContext = "WorldContextObject"))
	static int32 GetPausesLeft(const UObject* WorldContextObject);

	/** Whole seconds left on the pause's timer (rounded up), 0 once over, while running or in solo. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Pause", meta = (WorldContext = "WorldContextObject"))
	static int32 GetTimerSecondsLeft(const UObject* WorldContextObject);

	/** Solo: the pause is free (no limit, no timer). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Pause", meta = (WorldContext = "WorldContextObject"))
	static bool IsPauseFree(const UObject* WorldContextObject);

	/** Slot may pause now (a player still in the match, a pause left, the match running); -1 (a spectator) never. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Pause", meta = (WorldContext = "WorldContextObject"))
	static bool CanPauseNow(const UObject* WorldContextObject, int32 Slot);

	/** Slot may resume now (a player still in the match; its own pause, or the timer over). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Pause", meta = (WorldContext = "WorldContextObject"))
	static bool CanResumeNow(const UObject* WorldContextObject, int32 Slot);
};
