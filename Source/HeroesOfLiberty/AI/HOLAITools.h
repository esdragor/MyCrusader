// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLAITools.generated.h"

/**
 * The trial tools of the AI (EV-Q1 = B, D620): what lets a trial move the Evolving AI from one stage to the next without speeding the game
 * up, and see its level on the screen. The rules stay in script (Script/AI/HOLAILives.as, namespace HOLAILives); this holds the setting
 * the console command hol.AI.SetEvolveTimes writes and the slot's level the HUD reads. Nothing here changes a match played as usual:
 * without the command the times of the script (FHOLAILivesData) stand.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLAIToolsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** hol.AI.SetEvolveTimes <normal> <hard>: the game times (s) at which the Evolving AI plays Normal, then Hard (Hard is kept after Normal). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI")
	static void SetEvolveTimes(float NormalAt, float HardAt);

	/** The times of the script stand again (hol.AI.SetEvolveTimes with no value, or reset). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI")
	static void ClearEvolveTimes();

	/** Whether SetEvolveTimes was called (and not cleared). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|AI")
	static bool HasEvolveTimes();

	/** The set times (s); only meaningful when HasEvolveTimes. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|AI")
	static float GetEvolveNormalAt();

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|AI")
	static float GetEvolveHardAt();

	/** The configured level (EHOLAIDifficulty as an integer) of an AI slot of the match, -1 for a slot that is not an AI (or no game state). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static int32 GetSlotLevel(const UObject* WorldContextObject, int32 Slot);
};
