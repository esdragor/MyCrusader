// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineBaseTypes.h"

class UWorld;

/**
 * Who may cheat (R-Q7): the solo game keeps its cheats; a server, dedicated or listen, refuses every cheat a client asks
 * (hol.Cheat.*, hol.Victory.Defeat, the engine's ServerExec) unless launched with -HOLCheats (the test scripts).
 */
struct HEROESOFLIBERTY_API FHOLCheatRules
{
	/** The pure rule; bRefusedForTest (hol.Cheat.Refuse) refuses even with -HOLCheats. */
	static bool AreCheatsAllowed(ENetMode NetMode, bool bHOLCheatsOnCommandLine, bool bRefusedForTest);

	/** The rule for this world, this process's command line and hol.Cheat.Refuse. */
	static bool AreCheatsAllowed(const UWorld& World);

	/**
	 * Who may pause (D361: a networked match never pauses): the solo game alone, as the engine does; a server, dedicated
	 * or listen, refuses every pause, a client's console "pause" included (-HOLCheats or not).
	 */
	static bool IsPauseAllowed(ENetMode NetMode);
};
