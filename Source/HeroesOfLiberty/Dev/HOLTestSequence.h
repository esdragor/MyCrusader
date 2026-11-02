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
	 * Writes out the steps that stand for several (the capture sequence of a moving thing):
	 *   captureseq <Name> <Count> <StepSeconds> [Command]
	 * becomes a clock start (seqclock), then for each i in 0..Count-1 a wait for the game's clock to reach i * StepSeconds (waitclock,
	 * the game's time, not the wall clock: the same on a fixed time step), Command if given (a report of what is shown), and
	 * capture <Name>_<i>. A line with bad arguments becomes a mark that says so, so that the run does not go on in silence.
	 */
	void ExpandSequences(TArray<FString>& Steps);

	/**
	 * Runs steps one after the other (the verbs of hol.Test.Run). Each Owner has its own run, so that a scenario and a
	 * check can go on together; a run of the same Owner still going is stopped first.
	 */
	void Run(FName Owner, TArray<FString>&& Steps, FRunOptions&& Options);
}
