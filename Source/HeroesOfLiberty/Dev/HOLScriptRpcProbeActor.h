// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "HOLScriptRpcProbeActor.generated.h"

/**
 * MIG-P1d-C++ (D830/D835): C++ base of the script RPC probe (Script/Dev/HOLScriptRpcProbe.as). The bridge finds the script class as
 * every other script screen is found: by IsChildOf this base (looking up a script class by name found nothing on the dedicated server,
 * 2026-11-17 09:40). The RPCs themselves (ServerPing, ClientPong) are written in the script subclass: that is what the probe proves.
 */
UCLASS(Abstract, Blueprintable)
class HEROESOFLIBERTY_API AHOLScriptRpcProbeActor : public AActor
{
	GENERATED_BODY()
};
