// Copyright Epic Games, Inc. All Rights Reserved.

#include "Dev/HOLScriptRpcProbeBridge.h"

#include "Dev/HOLScriptRpcProbeActor.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "HeroesOfLibertyLog.h"
#include "UObject/UObjectIterator.h"

UClass* HOLScriptRpcProbeBridge::FindScriptClass()
{
	// Angelscript drops the A prefix of the class name (AHOLScriptRpcProbe -> HOLScriptRpcProbe); its hot reload keeps the older
	// classes, flagged CLASS_NewerVersionExists.
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->IsChildOf(AHOLScriptRpcProbeActor::StaticClass())
			&& !It->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists | CLASS_Deprecated))
		{
			return *It;
		}
	}
	return nullptr;
}

AActor* HOLScriptRpcProbeBridge::SpawnFor(APlayerController* Controller)
{
	UWorld* World = Controller ? Controller->GetWorld() : nullptr;
	if (!World || World->GetNetMode() == NM_Client)
	{
		return nullptr;
	}
	UClass* ScriptClass = FindScriptClass();
	if (!ScriptClass)
	{
		UE_LOG(LogHOLMatch, Log, TEXT("RpcProbe: no script class derived from AHOLScriptRpcProbeActor on this server"));
		return nullptr;
	}
	FActorSpawnParameters Params;
	Params.Owner = Controller;
	AActor* Probe = World->SpawnActor<AActor>(ScriptClass, FTransform::Identity, Params);
	UE_LOG(LogHOLMatch, Log, TEXT("RpcProbe: the C++ bridge spawned %s for %s"), Probe ? *Probe->GetName() : TEXT("nothing"), *Controller->GetName());
	return Probe;
}
