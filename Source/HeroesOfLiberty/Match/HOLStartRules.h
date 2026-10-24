// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * R3.9: when a match launched by the lobby manager (a configuration with names) starts: once every named player is
 * here, or at the end of the wait limit, whoever is missing. Pure rules, for the tests.
 */
struct HEROESOFLIBERTY_API FHOLStartRules
{
	/** The expected names missing from the present ones, in the expected order (trimmed, case ignored). */
	static TArray<FString> GetMissing(TConstArrayView<FString> Expected, TConstArrayView<FString> Present);

	/** Whether the match starts now: nobody missing, or the wait limit reached. */
	static bool ShouldStart(int32 NumMissing, double ElapsedSeconds, double MaxWaitSeconds);
};
