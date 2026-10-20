// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * The runner of hol.Test.Run (HOLTestSequence.cpp), started from code: the scripted checks run a file of steps and quit
 * on a step that never comes; a test scenario (hol.Scenario, HOLScenario.cpp) runs its steps the same way but leaves
 * the game open for the user to play on.
 */
namespace HOLTestSequence
{
	struct FRunOptions
	{
		/** True (hol.Test.Run): a step's timeout quits the game. False (scenarios): an error on the screen, the run stops. */
		bool bQuitOnTimeout = true;
		/** Called once the last step is done (not on a timeout). */
		TFunction<void()> OnDone;
	};

	/** Reads a file of steps, one per line, empty lines and lines starting with # skipped; false if it cannot be read. */
	bool LoadSteps(const FString& File, TArray<FString>& OutSteps);

	/**
	 * Runs steps one after the other (the verbs of hol.Test.Run). Each Owner has its own run, so that a scenario and a
	 * check can go on together; a run of the same Owner still going is stopped first.
	 */
	void Run(FName Owner, TArray<FString>&& Steps, FRunOptions&& Options);
}
