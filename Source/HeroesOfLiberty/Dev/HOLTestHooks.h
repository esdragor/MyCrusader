// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct FHOLMatchConfig;

/**
 * What the scripted checks (hol.Test.Run) ask of the interface, which lives in a module this one does not depend on
 * (D247): the interface module fills these in when its root layout is built. Unset on a dedicated server.
 */
namespace HOLTestHooks
{
	/** True while a screen of the interface is still fading in or out. */
	HEROESOFLIBERTY_API TFunction<bool()>& IsInterfaceBusy();

	/** The screens on top of the interface layers, for the log of a capture ("menu=X modal=Y"). */
	HEROESOFLIBERTY_API TFunction<FString()>& DescribeInterface();

	/** What the scenario's panel shows (hol.Scenario, HOLScenario.cpp): kept here, for a panel made after a map opened. */
	struct FScenarioDisplay
	{
		enum class EState : uint8
		{
			/** Its steps are running. */
			Preparing,
			Ready,
			/** A step never came: Error says which. */
			Failed,
		};

		FString Name;
		EState State = EState::Preparing;
		FString Error;
		/** The notes of its steps (note <Text>), in their order. */
		TArray<FString> Notes;
		/** Not empty while a step waits for the continue key (waitkey): what the panel says, in its own colour. */
		FString WaitingForKey;
		/** The player closed the panel: hidden until the next scenario. */
		bool bClosed = false;
	};

	/** The scenario started last; an empty Name before any. */
	HEROESOFLIBERTY_API FScenarioDisplay& GetScenarioDisplay();

	/** Called when the scenario's display changed: the interface shows it (its game HUD's scenario panel). */
	HEROESOFLIBERTY_API TFunction<void()>& OnScenarioChanged();

	/** Calls OnScenarioChanged; when nobody set it (dedicated server), puts Fallback on the screen as a debug message. */
	HEROESOFLIBERTY_API void NotifyScenarioChanged(const FString& Fallback, const FColor& FallbackColor);

	/**
	 * The continue key was pressed in game (Enter, the player controller; not while the console or a menu has it): a
	 * run waiting on a waitkey step goes on. False when none was waiting (the key did nothing).
	 */
	HEROESOFLIBERTY_API bool PressContinueKey();

	/**
	 * The configuration a test scenario starts with (hol.Scenario, HOLScenario.cpp), from the text of its .json (Json null:
	 * it has none, the default configuration). Plain when it names no map; IA1.1: no starting Peasants (StartingPeasants 0)
	 * unless the .json gives the field ("startingPeasants": 5), the scenarios making their own; J8a.2: no hero
	 * (StartingHero 0) unless it asks ("startingHero": true). False on a malformed .json.
	 * Development builds only.
	 */
	HEROESOFLIBERTY_API bool MakeScenarioConfig(const FString* Json, FHOLMatchConfig& OutConfig, FString* OutError = nullptr);
}
