// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class APlayerController;

/**
 * MIG-P1d-C++ (D830/D835): the server entry point the script cannot have by itself. A pure script world subsystem was not created
 * (or never ticked) on the dedicated server (probe of 2026-11-16, 2f2c732), so the game mode's PostLogin calls this bridge: it looks
 * for the script class AHOLScriptRpcProbe (Script/Dev/HOLScriptRpcProbe.as, found as UHOLScriptScreen::FindScriptClass finds its own,
 * by the class and not CLASS_NewerVersionExists) and spawns one instance per controller, owned by it, so the client may call its
 * Server RPC. Absent class (no script): does nothing. Server only.
 */
namespace HOLScriptRpcProbeBridge
{
	/** The newest script class named HOLScriptRpcProbe (an AActor); null: none. */
	HEROESOFLIBERTY_API UClass* FindScriptClass();

	/** Server only: spawns the script probe owned by Controller; returns it, or null (no class, no world, not the server). */
	HEROESOFLIBERTY_API AActor* SpawnFor(APlayerController* Controller);
}
