// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLScriptRpcPlayerActor.h"

#include "Commands/HOLCommandSubsystem.h"
#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionRules.h"
#include "Buildings/HOLConstructionSubsystem.h"
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
	// D845: the one body of the duel challenge / answer RPCs (the controller's C++ twins are gone).
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

// ---- HALDIR-1117o (NOT COMPILED), MIG2-S3 T4 lot L1: Specialization and Hero. The owner controller's Do<X> does the work (friend class): same body, same journal (DoHeroTalent, DoHeroAbility, DoCastRage, DoWatchUnit called by the bridge, D845, lot E).
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
	if (Controller) { Controller->DoStartSpecialization(Tier, Option); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CancelSpecialization(AActor* RpcActor)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoCancelSpecialization(); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::ReturnHero(AActor* RpcActor)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoReturnHero(); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::HeroTalent(AActor* RpcActor, const int32 Family)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoHeroTalent(Family); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::HeroAbility(AActor* RpcActor, const EHOLHeroAbility Ability, const double X, const double Y, const double Z)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoHeroAbility(Ability, FVector_NetQuantize(X, Y, Z)); }
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
		Controller->DoCastRage(Ids);
	}
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::WatchUnit(AActor* RpcActor, const int32 UnitId)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoWatchUnit(static_cast<uint32>(UnitId)); }
	return Controller != nullptr;
}

// ---- CHEATS A (ELROND-1117q, NOT COMPILED), MIG2-S3 L4 group A (friend class: DoCheat<X> of the owner controller called by the bridge, D845, same body, same journal, same cheat gate).
bool UHOLRpcBridgeLibrary::CheatRefused(AActor* RpcActor, const FString& What)
{
	const AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	return !Controller || Controller->IsCheatRefused(*What);
}

bool UHOLRpcBridgeLibrary::CheatResources(AActor* RpcActor, const int32 Food, const int32 Wood, const int32 Stone, const int32 Gold)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoCheatResources(FHOLResources(Food, Wood, Stone, Gold)); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatFastBuild(AActor* RpcActor, const bool bOn)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoCheatFastBuild(bOn); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatPopulation(AActor* RpcActor, const int32 Places)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoCheatPopulation(Places); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatTechnology(AActor* RpcActor, const FName Technology)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoCheatTechnology(Technology); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatTier(AActor* RpcActor, const int32 Tier, const int32 Slot)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoCheatTier(Tier, Slot); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatSpecialization(AActor* RpcActor, const int32 Tier, const int32 Option)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoCheatSpecialization(Tier, Option); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatUnitSpeed(AActor* RpcActor, const double Scale)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoCheatUnitSpeed(static_cast<float>(Scale)); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatGatherSpeed(AActor* RpcActor, const double Scale)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoCheatGatherSpeed(static_cast<float>(Scale)); }
	return Controller != nullptr;
}

// ---- CHEATS B (CELEBORN-1117q, NOT COMPILED), MIG2-S3 T9 lot L4: the cheat RPCs (friend class: DoCheat<X> of the owner controller called by the bridge, D845, same body, same journal).
bool UHOLRpcBridgeLibrary::CheatsAllowed(AActor* RpcActor, const FString& What)
{
	const AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	return Controller && !Controller->IsCheatRefused(*What);
}

bool UHOLRpcBridgeLibrary::CheatWound(AActor* RpcActor, const int32 Percent)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoCheatWound(Percent); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatDamageBuilding(AActor* RpcActor, const double X, const double Y, const double Z, const int32 Amount, const int32 AttackType)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoCheatDamageBuilding(FVector_NetQuantize(FVector(X, Y, Z)), Amount, static_cast<uint8>(FMath::Clamp(AttackType, 0, 255))); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatSpawnBuilding(AActor* RpcActor, const FString& TypeName, const int32 Slot, const double X, const double Y, const double Z)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoCheatSpawnBuilding(FName(*TypeName), Slot, FVector_NetQuantize(FVector(X, Y, Z))); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatDefeat(AActor* RpcActor, const int32 Slot)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoCheatDefeat(Slot); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatFallForum(AActor* RpcActor, const int32 Slot)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoCheatFallForum(Slot); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatSkipForum(AActor* RpcActor, const int32 Slot)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoCheatSkipForum(Slot); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CheatHoldPresence(AActor* RpcActor, const int32 Slot, const bool bOff, const bool bAttackers, const bool bDefenders, const int32 AttackerSlot)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoCheatHoldPresence(Slot, bOff, bAttackers, bDefenders, AttackerSlot); }
	return Controller != nullptr;
}

// ---- HALDIR-1117p (NOT COMPILED), MIG2-S3 T5 lot L1: Signals and chat (friend class: the owner controller's PostSignal / PostChat / ReceiveSignal / ReceiveChat, same body, same journal).
bool UHOLRpcBridgeLibrary::PostSignal(AActor* RpcActor, const int32 Kind, const double X, const double Y, const double Z)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoPostSignal(Kind, FVector(X, Y, Z)); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::ReceiveSignal(AActor* RpcActor, const int32 Sender, const int32 Kind, const double X, const double Y, const double Z)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ReceiveSignal(Sender, Kind, FVector(X, Y, Z)); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::PostChat(AActor* RpcActor, const int32 Channel, const FString& Text, const int32 TargetSlot)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoPostChat(Channel, Text, TargetSlot); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::ReceiveChat(AActor* RpcActor, const int32 SenderSlot, const int32 Channel, const FString& Text)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->ReceiveChat(SenderSlot, Channel, Text); }
	return Controller != nullptr;
}

// ---- HALDIR-1117q (NOT COMPILED), MIG2-S3 T8 lot L3: Economy (friend class: the _Implementation of the owner controller, same body).
bool UHOLRpcBridgeLibrary::QueueProduction(AActor* RpcActor, AHOLBuilding* Building, const FName TypeName)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoQueueProduction(Building, TypeName); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::CancelProduction(AActor* RpcActor, AHOLBuilding* Building, const int32 Index)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoCancelProduction(Building, Index); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::Tribute(AActor* RpcActor, AHOLBuilding* Building, const int32 Receiver, const int32 Resource, const int32 Amount)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoTribute(Building, Receiver, Resource, Amount); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::MarketTrade(AActor* RpcActor, AHOLBuilding* Building, const int32 Resource, const int32 Lots, const bool bBuy)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoMarketTrade(Building, Resource, Lots, bBuy); }
	return Controller != nullptr;
}

// ---- L2 ORDERS (ELROND-1118f C2, NOT COMPILED), D845/D846: the 16 player orders. The owner controller's Do<X> (friend class) does the work: same body, same journal. ids int32 -> uint32, points X, Y, Z -> FVector_NetQuantize.
namespace HOLRpcBridgeL2Orders_Elrond1118f
{
	TArray<uint32> ToIds(const TArray<int32>& UnitIds)
	{
		TArray<uint32> Ids;
		Ids.Reserve(UnitIds.Num());
		for (const int32 Id : UnitIds) { Ids.Add(static_cast<uint32>(Id)); }
		return Ids;
	}
}

bool UHOLRpcBridgeLibrary::OrderMove(AActor* RpcActor, const TArray<int32>& UnitIds, const double X, const double Y, const double Z, const bool bQueue)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoMoveUnits(HOLRpcBridgeL2Orders_Elrond1118f::ToIds(UnitIds), FVector_NetQuantize(FVector(X, Y, Z)), bQueue); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::OrderAttackUnit(AActor* RpcActor, const TArray<int32>& UnitIds, const int32 TargetUnitId, const bool bQueue)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoAttackUnit(HOLRpcBridgeL2Orders_Elrond1118f::ToIds(UnitIds), static_cast<uint32>(TargetUnitId), bQueue); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::OrderHealUnit(AActor* RpcActor, const TArray<int32>& UnitIds, const int32 TargetUnitId, const bool bQueue)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoHealUnit(HOLRpcBridgeL2Orders_Elrond1118f::ToIds(UnitIds), static_cast<uint32>(TargetUnitId), bQueue); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::OrderAttackBuilding(AActor* RpcActor, const TArray<int32>& UnitIds, AHOLBuilding* Building, const bool bQueue)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoAttackBuilding(HOLRpcBridgeL2Orders_Elrond1118f::ToIds(UnitIds), Building, bQueue); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::OrderAttackMove(AActor* RpcActor, const TArray<int32>& UnitIds, const double X, const double Y, const double Z, const bool bQueue)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoAttackMove(HOLRpcBridgeL2Orders_Elrond1118f::ToIds(UnitIds), FVector_NetQuantize(FVector(X, Y, Z)), bQueue); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::OrderStop(AActor* RpcActor, const TArray<int32>& UnitIds, const bool bHold)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoStopUnits(HOLRpcBridgeL2Orders_Elrond1118f::ToIds(UnitIds), bHold); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::OrderToggleDeploy(AActor* RpcActor, const TArray<int32>& UnitIds)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoToggleDeploy(HOLRpcBridgeL2Orders_Elrond1118f::ToIds(UnitIds)); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::OrderMoveInLine(AActor* RpcActor, const TArray<int32>& UnitIds, const double StartX, const double StartY, const double StartZ, const double EndX, const double EndY, const double EndZ, const bool bWidth, const bool bQueue)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoMoveUnitsInLine(HOLRpcBridgeL2Orders_Elrond1118f::ToIds(UnitIds), FVector_NetQuantize(FVector(StartX, StartY, StartZ)), FVector_NetQuantize(FVector(EndX, EndY, EndZ)), bWidth, bQueue); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::OrderGather(AActor* RpcActor, const TArray<int32>& UnitIds, const int32 SourceIndex, const bool bQueue)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoGatherUnits(HOLRpcBridgeL2Orders_Elrond1118f::ToIds(UnitIds), SourceIndex, bQueue); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::OrderPlaceBuilding(AActor* RpcActor, const FName TypeName, const double X, const double Y, const double Z, const TArray<int32>& UnitIds, const bool bQueue, const int32 CostPercent, const int32 CostCellIndex)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoPlaceBuilding(TypeName, FVector_NetQuantize(FVector(X, Y, Z)), HOLRpcBridgeL2Orders_Elrond1118f::ToIds(UnitIds), bQueue, CostPercent, CostCellIndex); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::OrderBuildUnits(AActor* RpcActor, const TArray<int32>& UnitIds, AHOLBuilding* Building, const bool bQueue)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoBuildUnits(HOLRpcBridgeL2Orders_Elrond1118f::ToIds(UnitIds), Building, bQueue); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::OrderDropOff(AActor* RpcActor, const TArray<int32>& UnitIds, AHOLBuilding* Building, const double X, const double Y, const double Z, const bool bQueue)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoDropOffUnits(HOLRpcBridgeL2Orders_Elrond1118f::ToIds(UnitIds), Building, FVector_NetQuantize(FVector(X, Y, Z)), bQueue); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::OrderGarrison(AActor* RpcActor, const TArray<int32>& UnitIds, AHOLBuilding* Building)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoGarrisonUnits(HOLRpcBridgeL2Orders_Elrond1118f::ToIds(UnitIds), Building); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::OrderUngarrisonAll(AActor* RpcActor, AHOLBuilding* Building)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoUngarrisonAll(Building); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::OrderUngarrisonUnit(AActor* RpcActor, const int32 UnitId)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoUngarrisonUnit(static_cast<uint32>(UnitId)); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::OrderRemoveOwnBuilding(AActor* RpcActor, AHOLBuilding* Building)
{
	const int32 Slot = GetSenderSlot(RpcActor);
	UWorld* World = RpcActor ? RpcActor->GetWorld() : nullptr;
	UHOLConstructionSubsystem* Construction = World ? World->GetSubsystem<UHOLConstructionSubsystem>() : nullptr;
	if (!Construction || !Building || Slot == INDEX_NONE || static_cast<int32>(Building->GetOwnerPlayer()) != Slot || !Building->IsFinished())
	{
		return false;
	}
	UHOLConstructionSubsystem::DestroyBuilding(*Building);
	return true;
}

bool UHOLRpcBridgeLibrary::OrderRemoveOwnBuildingRefund(AActor* RpcActor, AHOLBuilding* Building, const int32 RefundPercent)
{
	const int32 Slot = GetSenderSlot(RpcActor);
	UWorld* World = RpcActor ? RpcActor->GetWorld() : nullptr;
	UHOLConstructionSubsystem* Construction = World ? World->GetSubsystem<UHOLConstructionSubsystem>() : nullptr;
	if (!Construction || !Building || Slot == INDEX_NONE || static_cast<int32>(Building->GetOwnerPlayer()) != Slot || !Building->IsFinished())
	{
		return false;
	}
	// The cost is read before the building is gone.
	const FHOLResources Cost = UHOLBuildingSettings::GetBuildCost(World, Building->GetTypeName(), Building->GetOwnerPlayer());
	const FHOLResources Refund = HOLConstruction::GetPercentRefund(Cost, RefundPercent);
	UHOLConstructionSubsystem::DestroyBuilding(*Building);
	FHOLResources Stock;
	const bool bCredited = AHOLGameState::AddSlotStock(*World, Slot, Refund, Stock);
	UE_LOG(LogHOLCommands, Display, TEXT("Building removed with refund: slot %d, %d %% of %s = %s (%s)"), Slot, RefundPercent, *Cost.ToString(), *Refund.ToString(), bCredited ? *Stock.ToString() : TEXT("no stock"));
	return true;
}

bool UHOLRpcBridgeLibrary::OrderCancelBuilding(AActor* RpcActor, AHOLBuilding* Building)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoCancelBuilding(Building); }
	return Controller != nullptr;
}

bool UHOLRpcBridgeLibrary::OrderSetRallyPoint(AActor* RpcActor, AHOLBuilding* Building, const double X, const double Y, const double Z, const int32 SourceIndex, AHOLBuilding* Site, const bool bQueue)
{
	AHOLPlayerController* Controller = HOLRpcBridgeL1_Haldir1117o::OwnerController(RpcActor);
	if (Controller) { Controller->DoSetRallyPoint(Building, FVector_NetQuantize(FVector(X, Y, Z)), SourceIndex, Site, bQueue); }
	return Controller != nullptr;
}
