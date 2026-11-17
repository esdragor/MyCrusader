// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLScriptRpcPlayerActor.h"

#include "Commands/HOLCommandSubsystem.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLPlayerState.h"
#include "UObject/UObjectIterator.h"

AActor* UHOLRpcBridgeLibrary::FindPlayerActor(const APlayerController* Controller)
{
	UWorld* World = Controller ? Controller->GetWorld() : nullptr;
	if (!World)
	{
		return nullptr;
	}
	for (TActorIterator<AHOLScriptRpcPlayerActor> It(World); It; ++It)
	{
		if (It->GetOwner() == Controller && !It->GetClass()->HasAnyClassFlags(CLASS_NewerVersionExists))
		{
			return *It;
		}
	}
	return nullptr;
}

int32 UHOLRpcBridgeLibrary::GetSenderSlot(const AActor* RpcActor)
{
	const APlayerController* Controller = RpcActor ? Cast<APlayerController>(RpcActor->GetOwner()) : nullptr;
	const AHOLPlayerState* State = Controller ? Controller->GetPlayerState<AHOLPlayerState>() : nullptr;
	return State ? State->GetPlayerIndex() : INDEX_NONE;
}

bool UHOLRpcBridgeLibrary::QueueDiplomacyOrder(AActor* RpcActor, const int32 Kind, const int32 OtherSlot, const int32 Value)
{
	const int32 Slot = GetSenderSlot(RpcActor);
	UWorld* World = RpcActor ? RpcActor->GetWorld() : nullptr;
	UHOLCommandSubsystem* Commands = World ? World->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
	if (Slot < 0 || !Commands || (Kind != 0 && Kind != 1))
	{
		return false;
	}
	Commands->IssueDiplomacyOrder(Slot, Kind == 0 ? EHOLDiplomacyOrderKind::Stance : EHOLDiplomacyOrderKind::Vision, OtherSlot, Value);
	return true;
}

int32 HOLScriptRpcPlayerBridge::SpawnAllFor(APlayerController* Controller)
{
	UWorld* World = Controller ? Controller->GetWorld() : nullptr;
	if (!World || World->GetNetMode() == NM_Client)
	{
		return 0;
	}
	TArray<UClass*> ScriptClasses;
	for (TObjectIterator<UClass> It; It; ++It)
	{
		// Angelscript hot reload keeps the older classes, flagged CLASS_NewerVersionExists.
		if (It->IsChildOf(AHOLScriptRpcPlayerActor::StaticClass())
			&& !It->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists | CLASS_Deprecated))
		{
			ScriptClasses.Add(*It);
		}
	}
	int32 Spawned = 0;
	for (UClass* ScriptClass : ScriptClasses)
	{
		FActorSpawnParameters Params;
		Params.Owner = Controller;
		AActor* Actor = World->SpawnActor<AActor>(ScriptClass, FTransform::Identity, Params);
		UE_LOG(LogHOLMatch, Log, TEXT("RpcPlayer: the C++ bridge spawned %s (%s) for %s"), Actor ? *Actor->GetName() : TEXT("nothing"), *ScriptClass->GetName(), *Controller->GetName());
		Spawned += Actor ? 1 : 0;
	}
	if (ScriptClasses.IsEmpty())
	{
		UE_LOG(LogHOLMatch, Log, TEXT("RpcPlayer: no script class derived from AHOLScriptRpcPlayerActor on this server"));
	}
	return Spawned;
}
