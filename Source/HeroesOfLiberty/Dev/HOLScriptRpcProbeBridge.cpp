// Copyright Epic Games, Inc. All Rights Reserved.

#include "Dev/HOLScriptRpcProbeBridge.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "HeroesOfLibertyLog.h"
#include "UObject/UObjectIterator.h"

UClass* HOLScriptRpcProbeBridge::FindScriptClass()
{
	// Angelscript drops the A prefix of the class name (AHOLScriptRpcProbe -> HOLScriptRpcProbe); its hot reload keeps the older
	// classes, flagged CLASS_NewerVersionExists.
	static const FName ClassName(TEXT("HOLScriptRpcProbe"));
	static const FName ClassNameWithPrefix(TEXT("AHOLScriptRpcProbe"));
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if ((It->GetFName() == ClassName || It->GetFName() == ClassNameWithPrefix) && It->IsChildOf(AActor::StaticClass())
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
		return nullptr;
	}
	FActorSpawnParameters Params;
	Params.Owner = Controller;
	AActor* Probe = World->SpawnActor<AActor>(ScriptClass, FTransform::Identity, Params);
	UE_LOG(LogHOLMatch, Log, TEXT("RpcProbe: the C++ bridge spawned %s for %s"), Probe ? *Probe->GetName() : TEXT("nothing"), *Controller->GetName());
	return Probe;
}
