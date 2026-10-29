// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLFogNetSubsystem.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Engine/NetConnection.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "HeroesOfLibertyLog.h"
#include "Map/HOLVisionRules.h"
#include "Map/HOLVisionSubsystem.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerState.h"
#include "Simulation/HOLSimulationSubsystem.h"
#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/Filtering/NetObjectFilter.h"
#include "Iris/ReplicationSystem/ReplicationSystem.h"
#include "Net/Iris/ReplicationSystem/EngineReplicationBridge.h"
#include "Net/Iris/ReplicationSystem/ReplicationSystemUtil.h"
#endif

bool UHOLFogNetSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHOLFogNetSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	const ENetMode NetMode = InWorld.GetNetMode();
	UHOLSimulationSubsystem* Simulation = NetMode == NM_DedicatedServer || NetMode == NM_ListenServer ? InWorld.GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
	if (Simulation)
	{
		StepHandle = Simulation->GetOnStepEnded().AddUObject(this, &UHOLFogNetSubsystem::OnStepEnded);
	}
}

void UHOLFogNetSubsystem::Deinitialize()
{
	if (UHOLSimulationSubsystem* Simulation = GetWorld() ? GetWorld()->GetSubsystem<UHOLSimulationSubsystem>() : nullptr)
	{
		Simulation->GetOnStepEnded().Remove(StepHandle);
	}
	Super::Deinitialize();
}

bool UHOLFogNetSubsystem::IsActive() const
{
	const UWorld* World = GetWorld();
	const UHOLVisionSubsystem* Vision = World ? World->GetSubsystem<UHOLVisionSubsystem>() : nullptr;
	const ENetMode NetMode = World ? World->GetNetMode() : NM_Standalone;
	return (NetMode == NM_DedicatedServer || NetMode == NM_ListenServer) && Vision && Vision->IsStarted()
		&& Vision->GetMode() != EHOLFogMode::Disabled && GetReplicationSystem() != nullptr;
}

UReplicationSystem* UHOLFogNetSubsystem::GetReplicationSystem() const
{
#if UE_WITH_IRIS
	const UWorld* World = GetWorld();
	return World ? UE::Net::FReplicationSystemUtil::GetReplicationSystem(World->GetNetDriver()) : nullptr;
#else
	return nullptr;
#endif
}

void UHOLFogNetSubsystem::OnStepEnded(const int64 StepIndex)
{
	if (IsActive())
	{
		RefreshConnections();
		PlaceBuildings();
	}
}

void UHOLFogNetSubsystem::PlaceBuildings()
{
	const UWorld* World = GetWorld();
	const UHOLVisionSubsystem* Vision = World ? World->GetSubsystem<UHOLVisionSubsystem>() : nullptr;
	if (!Vision)
	{
		return;
	}
	for (TActorIterator<AHOLBuilding> It(World); It; ++It)
	{
		AHOLBuilding* Building = *It;
		const FHOLBuildingTypeRow* Type = IsValid(Building) ? Building->GetType() : nullptr;
		if (!Type)
		{
			continue;
		}
		// Its owner's camp always receives it (a ruin sees nothing, its owner keeps the ghost of it, F6e).
		const int32 OwnerCamp = Vision->GetCampIndex(Building->GetOwnerPlayer());
		const uint32 Camps = Vision->GetCampsSeeing(Building->GetActorLocation(), 0.5 * Type->Footprint) | (OwnerCamp == INDEX_NONE ? 0u : 1u << OwnerCamp);
		SetObjectCamps(Building, Camps);
	}
	for (auto It = Objects.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid())
		{
			It.RemoveCurrent();
		}
	}
}

#if UE_WITH_IRIS
UE::Net::FNetObjectGroupHandle UHOLFogNetSubsystem::GetGroup(const uint32 Camps)
{
	if (const UE::Net::FNetObjectGroupHandle* Group = Groups.Find(Camps))
	{
		return *Group;
	}
	UReplicationSystem* ReplicationSystem = GetReplicationSystem();
	if (!ReplicationSystem)
	{
		return UE::Net::FNetObjectGroupHandle();
	}
	const UE::Net::FNetObjectGroupHandle Group = ReplicationSystem->CreateGroup(*FString::Printf(TEXT("HOLFog_%08x"), Camps));
	ReplicationSystem->AddExclusionFilterGroup(Group);
	for (const TPair<uint32, uint32>& Connection : Connections)
	{
		ReplicationSystem->SetGroupFilterStatus(Group, Connection.Key,
			HOLVisionRules::Receives(Connection.Value, Camps) ? UE::Net::ENetFilterStatus::Allow : UE::Net::ENetFilterStatus::Disallow);
	}
	return Groups.Add(Camps, Group);
}
#endif

void UHOLFogNetSubsystem::SetObjectCamps(AActor* Actor, const uint32 Camps)
{
#if UE_WITH_IRIS
	UReplicationSystem* ReplicationSystem = GetReplicationSystem();
	const UEngineReplicationBridge* Bridge = Actor ? UE::Net::FReplicationSystemUtil::GetActorReplicationBridge(Actor) : nullptr;
	const UE::Net::FNetRefHandle Handle = Bridge ? Bridge->GetReplicatedRefHandle(Actor) : UE::Net::FNetRefHandle::GetInvalid();
	if (!IsActive() || !ReplicationSystem || !Handle.IsValid())
	{
		return;
	}
	const uint32* Former = Objects.Find(Actor);
	if (Former && *Former == Camps)
	{
		return;
	}
	if (Former)
	{
		ReplicationSystem->RemoveFromGroup(GetGroup(*Former), Handle);
	}
	ReplicationSystem->AddToGroup(GetGroup(Camps), Handle);
	Objects.Add(Actor, Camps);
#endif
}

void UHOLFogNetSubsystem::ClearObject(AActor* Actor)
{
#if UE_WITH_IRIS
	const uint32* Former = Objects.Find(Actor);
	UReplicationSystem* ReplicationSystem = GetReplicationSystem();
	const UEngineReplicationBridge* Bridge = Actor ? UE::Net::FReplicationSystemUtil::GetActorReplicationBridge(Actor) : nullptr;
	const UE::Net::FNetRefHandle Handle = Bridge ? Bridge->GetReplicatedRefHandle(Actor) : UE::Net::FNetRefHandle::GetInvalid();
	if (Former && ReplicationSystem && Handle.IsValid())
	{
		ReplicationSystem->RemoveFromGroup(GetGroup(*Former), Handle);
	}
#endif
	Objects.Remove(Actor);
}

void UHOLFogNetSubsystem::RefreshConnections()
{
#if UE_WITH_IRIS
	UWorld* World = GetWorld();
	UReplicationSystem* ReplicationSystem = GetReplicationSystem();
	const UHOLVisionSubsystem* Vision = World ? World->GetSubsystem<UHOLVisionSubsystem>() : nullptr;
	const AHOLGameState* State = World ? World->GetGameState<AHOLGameState>() : nullptr;
	if (!ReplicationSystem || !Vision || !State)
	{
		return;
	}
	const FHOLMatchConfig& Config = State->GetLaunchConfig();
	TMap<uint32, uint32> Now;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* Controller = It->Get();
		const UNetConnection* Connection = Controller ? Controller->GetNetConnection() : nullptr;
		if (!Connection || !Connection->GetConnectionHandle().IsParentConnection())
		{
			continue;
		}
		const AHOLPlayerState* Player = Controller->GetPlayerState<AHOLPlayerState>();
		const int32 Slot = Player ? Player->GetPlayerIndex() : INDEX_NONE;
		const bool bTeam = Config.Slots.IsValidIndex(Slot) && Config.Slots[Slot].Team != 0;
		Now.Add(Connection->GetConnectionHandle().GetParentConnectionId(),
			HOLVisionRules::GetReceivedCamps(Vision->GetCampIndex(Slot), State->IsSlotDefeated(Slot), bTeam));
	}
	for (const TPair<uint32, uint32>& Connection : Now)
	{
		const uint32* Former = Connections.Find(Connection.Key);
		if (Former && *Former == Connection.Value)
		{
			continue;
		}
		for (const TPair<uint32, UE::Net::FNetObjectGroupHandle>& Group : Groups)
		{
			ReplicationSystem->SetGroupFilterStatus(Group.Value, Connection.Key,
				HOLVisionRules::Receives(Connection.Value, Group.Key) ? UE::Net::ENetFilterStatus::Allow : UE::Net::ENetFilterStatus::Disallow);
		}
		UE_LOG(LogHOLNet, Log, TEXT("Fog net: connection %u receives camps 0x%x"), Connection.Key, Connection.Value);
	}
	Connections = MoveTemp(Now);
#endif
}

FString UHOLFogNetSubsystem::MakeReport() const
{
	TArray<FString> Lines;
	for (const TPair<uint32, uint32>& Connection : Connections)
	{
		Lines.Add(FString::Printf(TEXT("%u: camps 0x%x"), Connection.Key, Connection.Value));
	}
	int32 NumGroups = 0;
#if UE_WITH_IRIS
	NumGroups = Groups.Num();
#endif
	return FString::Printf(TEXT("Fog net: %s, groups %d, connections %d (%s), objects %d"), IsActive() ? TEXT("active") : TEXT("off"), NumGroups,
		Connections.Num(), *FString::Join(Lines, TEXT(", ")), Objects.Num());
}
