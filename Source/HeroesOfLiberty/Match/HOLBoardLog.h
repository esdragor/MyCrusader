// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UWorld;

/**
 * One journal line per script board (bridge) and per game world, written once when the world starts the match (called from
 * UHOLSimulationSubsystem::OnWorldBeginPlay, which is born on the dedicated server, on a client and in solo; D839: a bridge tool).
 * Every bridge is an Abstract UWorldSubsystem that only its script subclass can be born from: when the script class is not born
 * (cooked server, skipped script folder, load error) the bridge answers "no answer" and the game falls back on the C++ rule in
 * silence. These lines make that visible, and the net proofs of Tools/Tests/ grep them:
 *   LogHOLMatch: Board: <Bridge> script <ScriptClass>            (Display, the script board is there)
 *   LogHOLMatch: Board: <Bridge> ABSENT (C++ fallback)           (Warning, it is missing)
 * Nothing is written in a world without a game instance (the hand-made C++ test worlds), and a world is reported once only.
 */
namespace HOLBoardLog
{
	/** Writes the lines of the world (once per world); a no-op in a world that is not a game world with a game instance. */
	HEROESOFLIBERTY_API void LogWorldBoards(UWorld& World);
}
