// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Simulation/HOLHeroAbilityRules.h" // EHOLHeroAbility (UENUM) for HeroAbility

#include "HOLScriptRpcPlayerActor.generated.h"

class APlayerController;

/**
 * MIG2-S3 pilot (D842 Q1 = A, ELROND-1117o): C++ base of the per-player RPC actor written in script (Script/Net/HOLRpcPlayer.as). Same model as the proven
 * probe (AHOLScriptRpcProbeActor): the script subclass declares the UFUNCTION(Server) / UFUNCTION(Client) RPCs; the server spawns one per controller, owned
 * by it, in PostLogin (HOLScriptRpcPlayerBridge::SpawnAllFor), replicated and always relevant. Never put the script in Script/Dev, Developers, Examples or Editor.
 */
UCLASS(Abstract, Blueprintable)
class HEROESOFLIBERTY_API AHOLScriptRpcPlayerActor : public AActor
{
	GENERATED_BODY()

public:
	AHOLScriptRpcPlayerActor()
	{
		bReplicates = true;
		bAlwaysRelevant = true;
	}
};

/**
 * What the script's RPC bodies use and cannot reach by themselves (script namespace HOLRpcBridge::): the actor of a controller, the sender's slot read from the
 * owner (never from the client: D288), and the order queue. Envelopes only; the rules stay where they are.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLRpcBridgeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** The player's RPC actor (an AHOLScriptRpcPlayerActor owned by Controller); null: none (client before replication, no script). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Rpc")
	static AActor* FindPlayerActor(const APlayerController* Controller);

	/** The match slot of the player who owns RpcActor (AHOLPlayerState::GetPlayerIndex); -1: no owner, no state, or a spectator. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Rpc")
	static int32 GetSenderSlot(const AActor* RpcActor);

	/** Server: queues a diplomacy order (UHOLCommandSubsystem::IssueDiplomacyOrder) for the owner's own slot. Kind 0 stance, 1 vision. False: nothing queued. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Rpc")
	static bool QueueDiplomacyOrder(AActor* RpcActor, int32 Kind, int32 OtherSlot, int32 Value);

	/**
	 * Server: queues a duel order (UHOLCommandSubsystem::IssueDuelOrder) for the owner's own slot (D288). Kind is an EHOLDuelOrderKind as an int (enum class without UENUM):
	 * 0 Challenge (OtherSlot = the challenged slot), 1 Accept, 2 Refuse (OtherSlot ignored). Logs the same "Duel order: ... queued" line as the C++ RPC. False: nothing queued.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Rpc")
	static bool QueueDuelOrder(AActor* RpcActor, int32 Kind, int32 OtherSlot);

	/** Server: AHOLGameState::RequestPause for the owner's player state, at the server time (body of ServerRequestPause_Implementation). False: refused (the game state logs why) or no state. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Rpc")
	static bool RequestPause(AActor* RpcActor);

	/** Server: AHOLGameState::RequestResume, same model as RequestPause. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Rpc")
	static bool RequestResume(AActor* RpcActor);

	/** Server: AHOLGameState::DefeatSlot(owner's slot, EHOLDefeatReason::Surrender) (body of ServerSurrender_Implementation); logs "Victory: surrender of slot N refused" when refused. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Rpc")
	static bool Surrender(AActor* RpcActor);

	// ---- HALDIR-1117o (NOT COMPILED), MIG2-S3 T4 lot L1: Specialization and Hero. Each calls the owner controller's _Implementation (same body, same journal line as the C++ RPC). False: no owner controller.
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Rpc")
	static bool StartSpecialization(AActor* RpcActor, int32 Tier, int32 Option);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Rpc")
	static bool CancelSpecialization(AActor* RpcActor);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Rpc")
	static bool ReturnHero(AActor* RpcActor);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Rpc")
	static bool HeroTalent(AActor* RpcActor, int32 Family);

	/** Ability is an EHOLHeroAbility (UENUM); the _Implementation keeps only CloseRanks and Banner. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Rpc")
	static bool HeroAbility(AActor* RpcActor, EHOLHeroAbility Ability, double X, double Y, double Z);

	/** UnitIds as int32 (uint32 is not BlueprintCallable): cast back to uint32 for the _Implementation. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Rpc")
	static bool CastRage(AActor* RpcActor, const TArray<int32>& UnitIds);

	/** UnitId as int32 (cast back to uint32); 0 stops watching. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Rpc")
	static bool WatchUnit(AActor* RpcActor, int32 UnitId);
};

/** Server entry point the script cannot have by itself (generalises HOLScriptRpcProbeBridge): one actor per script class derived from the base. */
namespace HOLScriptRpcPlayerBridge
{
	/** Server only: spawns, owned by Controller, one instance of each live script class derived from AHOLScriptRpcPlayerActor; returns how many. */
	HEROESOFLIBERTY_API int32 SpawnAllFor(APlayerController* Controller);
}
