// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLScriptLogLibrary.generated.h"

/**
 * A line in one of the game's log categories at a given level, for the code written in script (C2 step 5, SAURON-1119u, D839 "liaison a exposer").
 * The script's own Log / Warning have no category and no Error: a script that replaces a C++ UE_LOG(LogHOLMatch, Warning / Error, ...) calls these to
 * keep the SAME category and the SAME level (what the scenarios and the log readers grep). Categories known (the ones of HeroesOfLibertyLog.h):
 * LogHOLSimulation, LogHOLPresentation, LogHOLCommands, LogHOLStressTest, LogHOLMatch, LogHOLTest, LogHOLNet. LogHOLUI lives in the UI module
 * and is NOT reachable from here. No Fatal level: a script must not abort the process. Script namespace: HOLScriptLog.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLScriptLogLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Whether Category is one of the categories listed above (exact spelling, e.g. "LogHOLMatch"). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Log")
	static bool IsKnownCategory(FName Category);

	/** UE_LOG(Category, Display, "%s", Line). False (and one Warning line on LogHOLMatch) when the category is unknown: nothing else is written. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Log")
	static bool LogDisplay(FName Category, const FString& Line);

	/** UE_LOG(Category, Warning, "%s", Line). False when the category is unknown. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Log")
	static bool LogWarning(FName Category, const FString& Line);

	/** UE_LOG(Category, Error, "%s", Line). False when the category is unknown. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Log")
	static bool LogError(FName Category, const FString& Line);

private:
	static bool Emit(FName Category, ELogVerbosity::Type Verbosity, const FString& Line);
};
