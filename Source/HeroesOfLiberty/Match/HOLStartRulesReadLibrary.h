// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLStartRulesReadLibrary.generated.h"

/**
 * The start rule of a match launched by the lobby manager (FHOLStartRules, Match/HOLStartRules.h, C++, applied by the server) for
 * Angelscript (namespace HOLStartRulesRead there): D836 keeps the rule in C++ and the script reads it here, so that the test of the
 * rule (HOLStartRulesTests.cpp) can be ported to script (socle step 1 of the migration, D830, D839). Wrappers only: no rule here.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLStartRulesReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** FHOLStartRules::GetMissing: the expected names missing from the present ones, in the expected order (trimmed, case ignored). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|StartRules")
	static TArray<FString> GetMissing(const TArray<FString>& Expected, const TArray<FString>& Present);

	/** FHOLStartRules::ShouldStart: whether the match starts now: nobody missing, or the wait limit reached. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|StartRules")
	static bool ShouldStart(int32 NumMissing, double ElapsedSeconds, double MaxWaitSeconds);
};
