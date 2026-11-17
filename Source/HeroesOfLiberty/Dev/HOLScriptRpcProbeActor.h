// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "HOLScriptRpcProbeActor.generated.h"

/**
 * MIG-P1d-C++ (D830/D835): C++ base of the script RPC probe (Script/Net/HOLScriptRpcProbe.as, PROVEN 2026-11-17 by rpc_probe_run.ps1). The bridge finds the
 * script class as every other script screen is found: by IsChildOf this base. The first passes found nothing because the script sat in
 * Script/Dev, a folder the Angelscript plugin skips outside the interactive editor (AngelscriptManager.cpp:825-835); never put
 * server or game script in Script/Dev, Developers, Examples or Editor. The RPCs themselves (ServerPing, ClientPong) are written in the script subclass: that is what the probe proves.
 */
UCLASS(Abstract, Blueprintable)
class HEROESOFLIBERTY_API AHOLScriptRpcProbeActor : public AActor
{
	GENERATED_BODY()
};
