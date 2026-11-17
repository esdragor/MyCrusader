// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLScriptRpcPlayerActor.h"

#include "Commands/HOLCommandSubsystem.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLGameState.h"
#include "Match/HOLPlayerState.h"
#include "Player/HOLPlayerController.h"
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

bool UHOLRpcBridgeLibrary::QueueDuelOrder(AActor* RpcActor, const int32 Kind, const int32 OtherSlot)
{
	const int32 Slot = GetSenderSlot(RpcActor);
	UWorld* World = RpcActor ? RpcActor->GetWorld() : nullptr;
	UHOLCommandSubsystem* Commands = World ? World->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
	if (Slot < 0 || !Commands || Kind < 0 || Kind > 2)
	{
		return false;
	}
	// Same lines as AHOLPlayerController::ServerDuelChallenge_Implementation / ServerDuelAnswer_Implementation.
	if (Kind == 0)
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Duel order: slot %d challenge at slot %d queued"), Slot, OtherSlot);
		Commands->IssueDuelOrder(Slot, EHOLDuelOrderKind::Challenge, OtherSlot);
	}
	else
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Duel order: slot %d %s queued"), Slot, Kind == 1 ? TEXT("accept") : TEXT("refuse"));
		Commands->IssueDuelOrder(Slot, Kind == 1 ? EHOLDuelOrderKind::Accept : EHOLDuelOrderKind::Refuse, INDEX_NONE);
	}
	return true;
}

bool UHOLRpcBridgeLibrary::RequestPause(AActor* RpcActor)
{
	const APlayerController* Controller = RpcActor ? Cast<APlayerController>(RpcActor->GetOwner()) : nullptr;
	const AHOLPlayerState* State = Controller ? Controller->GetPlayerState<AHOLPlayerState>() : nullptr;
	AHOLGameState* GameState = RpcActor && RpcActor->GetWorld() ? RpcActor->GetWorld()->GetGameState<AHOLGameState>() : nullptr;
	return State && GameState && GameState->RequestPause(*State, GameState->GetServerWorldTimeSeconds());
}

bool UHOLRpcBridgeLibrary::RequestResume(AActor* RpcActor)
{
	const APlayerController* Controller = RpcActor ? Cast<APlayerController>(RpcActor->GetOwner()) : nullptr;
	const AHOLPlayerState* State = Controller ? Controller->GetPlayerState<AHOLPlayerState>() : nullptr;
	AHOLGameState* GameState = RpcActor && RpcActor->GetWorld() ? RpcActor->GetWorld()->GetGameState<AHOLGameState>() : nullptr;
	return State && GameState && GameState->RequestResume(*State, GameState->GetServerWorldTimeSeconds());
}

bool UHOLRpcBridgeLibrary::Surrender(AActor* RpcActor)
{
	const int32 Slot = GetSenderSlot(RpcActor);
	AHOLGameState* GameState = RpcActor && RpcActor->GetWorld() ? RpcActor->GetWorld()->GetGameState<AHOLGameState>() : nullptr;
	if (!GameState || Slot == INDEX_NONE || !GameState->DefeatSlot(Slot, EHOLDefeatReason::Surrender))
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Victory: surrender of slot %d refused"), Slot);
		return false;
	}
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

// ---- HALDIR-1117o (NOT COMPILED), MIG2-S3 T4 lot L1: Specialization and Hero. The owner controller's _Implementation does the work (friend class): same body, same journal.
namespace HOLRpcBridgeL1_Haldir1117o
{
	AHOLPlayerController* OwnerController(const AActor* RpcActor)
	{
		return RpcActor ? Cast<AHOLPlayerController>(RpcActor->GetOwner()) : nullptr;
	}
}

bool UHOLRpcBridgeLibrary::StartSpecialization(AActor* RpcActor, const int32 Tier, const int32 Option)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerStartSpecialization_Implementation(Tier, Option); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CancelSpecialization(AActor* RpcActor)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerCancelSpecialization_Implementation(); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::ReturnHero(AActor* RpcActor)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerReturnHero_Implementation(); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::HeroTalent(AActor* RpcActor, const int32 Family)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerHeroTalent_Implementation(Family); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::HeroAbility(AActor* RpcActor, const EHOLHeroAbility Ability, const double X, const double Y, const double Z)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerHeroAbility_Implementation(Ability, FVector_NetQuantize(X, Y, Z)); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CastRage(AActor* RpcActor, const TArray<int32>& UnitIds)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller)
	{
		TArray<uint32> Ids;
		Ids.Reserve(UnitIds.Num());
		for (const int32 Id : UnitIds) { Ids.Add(static_cast<uint32>(Id)); }
		Controller->ServerCastRage_Implementation(Ids);
	}
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::WatchUnit(AActor* RpcActor, const int32 UnitId)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerWatchUnit_Implementation(static_cast<uint32>(UnitId)); }
	return Controller != nullptr;
}

// ---- CHEATS A (ELROND-1117q, NOT COMPILED), MIG2-S3 L4 group A (friend class: the _Implementation of the owner controller, same body, same journal, same cheat gate).
bool UHOLRpcBridgeLibrary::CheatRefused(AActor* RpcActor, const FString& What)
{
	const AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	return !Controller || Controller->IsCheatRefused(*What);
}

bool UHOLRpcBridgeLibrary::CheatResources(AActor* RpcActor, const int32 Food, const int32 Wood, const int32 Stone, const int32 Gold)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerCheatResources_Implementation(FHOLResources(Food, Wood, Stone, Gold)); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatFastBuild(AActor* RpcActor, const bool bOn)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerCheatFastBuild_Implementation(bOn); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatPopulation(AActor* RpcActor, const int32 Places)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerCheatPopulation_Implementation(Places); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatTechnology(AActor* RpcActor, const FName Technology)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerCheatTechnology_Implementation(Technology); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatTier(AActor* RpcActor, const int32 Tier, const int32 Slot)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerCheatTier_Implementation(Tier, Slot); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatUnitSpeed(AActor* RpcActor, const double Scale)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerCheatUnitSpeed_Implementation(static_cast<float>(Scale)); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatGatherSpeed(AActor* RpcActor, const double Scale)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerCheatGatherSpeed_Implementation(static_cast<float>(Scale)); }
	return Controller != nullptr;
}

// ---- CHEATS B (CELEBORN-1117q, NOT COMPILED), MIG2-S3 T9 lot L4: the cheat RPCs (friend class: the _Implementation of the owner controller, same body, same journal).
bool UHOLRpcBridgeLibrary::CheatsAllowed(AActor* RpcActor, const FString& What)
{
	const AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	return Controller && !Controller->IsCheatRefused(*What);
}

bool UHOLRpcBridgeLibrary::CheatWound(AActor* RpcActor, const int32 Percent)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerCheatWound_Implementation(Percent); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatDamageBuilding(AActor* RpcActor, const double X, const double Y, const double Z, const int32 Amount, const int32 AttackType)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerCheatDamageBuilding_Implementation(FVector_NetQuantize(FVector(X, Y, Z)), Amount, static_cast<uint8>(FMath::Clamp(AttackType, 0, 255))); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatSpawnBuilding(AActor* RpcActor, const FString& TypeName, const int32 Slot, const double X, const double Y, const double Z)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerCheatSpawnBuilding_Implementation(FName(*TypeName), Slot, FVector_NetQuantize(FVector(X, Y, Z))); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatDefeat(AActor* RpcActor, const int32 Slot)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerCheatDefeat_Implementation(Slot); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatFallForum(AActor* RpcActor, const int32 Slot)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerCheatFallForum_Implementation(Slot); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatSkipForum(AActor* RpcActor, const int32 Slot)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerCheatSkipForum_Implementation(Slot); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatHoldPresence(AActor* RpcActor, const int32 Slot, const bool bOff, const bool bAttackers, const bool bDefenders, const int32 AttackerSlot)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerCheatHoldPresence_Implementation(Slot, bOff, bAttackers, bDefenders, AttackerSlot); }
	return Controller != nullptr;
}

// ---- HALDIR-1117p (NOT COMPILED), MIG2-S3 T5 lot L1: Signals and chat (friend class: the _Implementation of the owner controller, same body, same journal).
bool UHOLRpcBridgeLibrary::PostSignal(AActor* RpcActor, const int32 Kind, const double X, const double Y, const double Z)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerPostSignal_Implementation(Kind, FVector(X, Y, Z)); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::ReceiveSignal(AActor* RpcActor, const int32 Sender, const int32 Kind, const double X, const double Y, const double Z)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ClientReceiveSignal_Implementation(Sender, Kind, FVector(X, Y, Z)); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::PostChat(AActor* RpcActor, const int32 Channel, const FString& Text, const int32 TargetSlot)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerPostChat_Implementation(Channel, Text, TargetSlot); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::ReceiveChat(AActor* RpcActor, const int32 SenderSlot, const int32 Channel, const FString& Text)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ClientReceiveChat_Implementation(SenderSlot, Channel, Text); }
	return Controller != nullptr;
}

// ---- HALDIR-1117q (NOT COMPILED), MIG2-S3 T8 lot L3: Economy (friend class: the _Implementation of the owner controller, same body).
bool UHOLRpcBridgeLibrary::QueueProduction(AActor* RpcActor, AHOLBuilding* Building, const FName TypeName)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerQueueProduction_Implementation(Building, TypeName); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CancelProduction(AActor* RpcActor, AHOLBuilding* Building, const int32 Index)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerCancelProduction_Implementation(Building, Index); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::Tribute(AActor* RpcActor, AHOLBuilding* Building, const int32 Receiver, const int32 Resource, const int32 Amount)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerTribute_Implementation(Building, Receiver, Resource, Amount); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::MarketTrade(AActor* RpcActor, AHOLBuilding* Building, const int32 Resource, const int32 Lots, const bool bBuy)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ServerMarketTrade_Implementation(Building, Resource, Lots, bBuy); }
	return Controller != nullptr;
}