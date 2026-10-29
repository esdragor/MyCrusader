// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/NetObjectGroupHandle.h"
#endif
#include "HOLFogNetSubsystem.generated.h"

class UReplicationSystem;

/**
 * F6a (FogInProgress.md, D518): the server sends a client only what its camp sees. An Iris exclusion group per set of
 * camps (a bit per camp of the fog, HOLVisionRules): an object in the group of the camps that see it reaches only the
 * connections receiving one of them (HOLVisionRules::GetReceivedCamps: a player its camp, a spectator every camp, a
 * defeated slot its former team's or every camp, D376). The connections are read again each step (a player joins, a slot
 * falls). Nothing without fog, in solo, nor on a client: the objects stay as they were (always relevant).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLFogNetSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	/** A networked server whose match has a fog (UHOLVisionSubsystem started, not Disabled), Iris replicating. */
	bool IsActive() const;

	/** The object reaches only the connections receiving one of Camps (0: no connection at all). */
	void SetObjectCamps(AActor* Actor, uint32 Camps);

	/** The object leaves its group (destroyed, or sent to everyone again). */
	void ClearObject(AActor* Actor);

	/** Reads the connections again (each step): their camps, and every group's status for the ones that changed. */
	void RefreshConnections();

	/** "Fog net: active, groups 3, connections 2 (1: camps 0x1, 2: camps 0x2), objects 12". */
	FString MakeReport() const;

private:
	void OnStepEnded(int64 StepIndex);
	UReplicationSystem* GetReplicationSystem() const;

#if UE_WITH_IRIS
	UE::Net::FNetObjectGroupHandle GetGroup(uint32 Camps);
	TMap<uint32, UE::Net::FNetObjectGroupHandle> Groups;
#endif
	/** Connection identifier -> the camps it receives. */
	TMap<uint32, uint32> Connections;
	/** The objects placed, and the camps of their group. */
	TMap<TWeakObjectPtr<AActor>, uint32> Objects;
	FDelegateHandle StepHandle;
};
