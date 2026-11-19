// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLCheatSettingsLibrary.generated.h"

/**
 * H-11a (the development cheats panel, Script/UI/HOLCheatPanelScript.as; HOLCheatSettings:: in script): the few things the panel reads or keeps
 * that script cannot reach: the kept open/folded choice (GGameUserSettingsIni, section HOL.CheatPanel), the window height, the server's own
 * settings by their console variables. Wrappers only: no rule here (the rules are in script and in HOLHUDLayoutRules.h).
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLCheatSettingsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** The kept choice (HOL.CheatPanel / bOpen of GameUserSettings.ini); false when absent. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HUD")
	static bool GetSavedOpen();

	/** Keeps the choice (written and flushed at once). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|HUD")
	static void SetSavedOpen(bool bOpen);

	/** The game viewport's height in pixels; 0 without a viewport. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HUD")
	static float GetWindowHeight();

	/** True when this machine holds the server's own settings (solo or listen server): the world's net mode is not a client. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HUD", meta = (WorldContext = "WorldContextObject"))
	static bool IsServerLocal(const UObject* WorldContextObject);

	/** hol.Prod.BuildTime (above 0: fast build is on); 0 when the variable is absent. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HUD")
	static float GetServerBuildTime();

	/** 0: hol.Units.SpeedScale, 1: hol.Gather.SpeedScale; 1.0 when absent or the index is not 0 or 1. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HUD")
	static float GetServerSpeed(int32 Index);
};
