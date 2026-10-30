// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/NetObjectGroupHandle.h"
#endif
#include "HOLFogNetSubsystem.generated.h"

class AHOLSourceChunk;
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

	/**
	 * F6f, the leak test: the sources sent to Camp (OutSent), and among them the ones on a cell the camp never explored
	 * (OutUnexplored, always 0 if the memory is what it saw). False for a camp with nothing sent yet.
	 */
	bool CountSentSources(int32 Camp, int32& OutSent, int32& OutUnexplored) const;

private:
	void OnStepEnded(int64 StepIndex);
	/** F6c: each building in the group of the camps that see it (its owner's always), the ones gone forgotten. */
	void PlaceBuildings();
	/**
	 * F6d (D524): each camp's source chunks, a family of its own (a slot stays empty until the camp knows the source): the
	 * sources its memory holds as last seen (UHOLVisionSubsystem::GetCampGhosts) and its own farms' fields. Without fog the
	 * source registry sends its chunks to everyone.
	 */
	void PlaceSources();
	AHOLSourceChunk* GetCampChunk(int32 Camp, int32 ChunkIndex);
	UReplicationSystem* GetReplicationSystem() const;

#if UE_WITH_IRIS
	UE::Net::FNetObjectGroupHandle GetGroup(uint32 Camps);
	TMap<uint32, UE::Net::FNetObjectGroupHandle> Groups;
#endif
	/** Connection identifier -> the camps it receives. */
	TMap<uint32, uint32> Connections;
	/** The objects placed, and the camps of their group. */
	TMap<TWeakObjectPtr<AActor>, uint32> Objects;

	/** F6d: what the server sent of the sources to one camp. */
	struct FCampSources
	{
		/** Source index -> the remaining amount last written in its slot. */
		TMap<int32, int32> Sent;
		TArray<TWeakObjectPtr<AHOLSourceChunk>> Chunks;
		uint32 GhostVersion = MAX_uint32;
		int32 NumStanding = INDEX_NONE;
	};
	TArray<FCampSources> SourceCamps;
	/** The source registry's chunks are decided (everyone's, without fog; the camps', with). */
	bool bSourcesDecided = false;
	FDelegateHandle StepHandle;
};
