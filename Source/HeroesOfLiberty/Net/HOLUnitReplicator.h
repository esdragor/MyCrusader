// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Iris/ReplicationState/IrisFastArraySerializer.h"
#include "Net/HOLUnitReplicationRules.h"
#include "HOLUnitReplicator.generated.h"

/** Which unit a slot holds (D270): sent when a unit takes or leaves the slot, or changes owner. UnitId 0: free slot. */
USTRUCT()
struct FHOLNetUnitIdentityItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	uint32 UnitId = 0;

	/** Index of the unit's type in UHOLUnitSubsystem::GetTypeNames, the same on the server and the clients. */
	UPROPERTY()
	uint8 TypeIndex = 0;

	/** D233. */
	UPROPERTY()
	uint8 OwnerPlayer = 0;
};

USTRUCT()
struct FHOLNetUnitIdentityArray : public FIrisFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FHOLNetUnitIdentityItem> Items;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FHOLNetUnitIdentityItem, FHOLNetUnitIdentityArray>(Items, DeltaParams, *this);
	}
};

template<>
struct TStructOpsTypeTraits<FHOLNetUnitIdentityArray> : public TStructOpsTypeTraitsBase2<FHOLNetUnitIdentityArray>
{
	enum { WithNetDeltaSerializer = true };
};

/** Where the unit of a slot is and what it does (D269), rewritten only when the send rule says so. */
USTRUCT()
struct FHOLNetUnitStateItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	FHOLNetUnitState State;
};

USTRUCT()
struct FHOLNetUnitStateArray : public FIrisFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FHOLNetUnitStateItem> Items;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FHOLNetUnitStateItem, FHOLNetUnitStateArray>(Items, DeltaParams, *this);
	}
};

template<>
struct TStructOpsTypeTraits<FHOLNetUnitStateArray> : public TStructOpsTypeTraitsBase2<FHOLNetUnitStateArray>
{
	enum { WithNetDeltaSerializer = true };
};

/**
 * A run of unit slots (D270), replicated subobject of AHOLUnitReplicator. Iris tracks the items of a fast array with
 * one change bit per 63 items (bit = index modulo 63): in a single array of 2 000 units, one marked unit would resend
 * the ~32 units sharing its bit. A chunk of exactly 63 slots gives each unit its own bit, so only the marked units are
 * sent. Slots are fixed: a leaving unit frees its slot, nothing is removed from the arrays, the indices never move.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLUnitChunk : public UObject
{
	GENERATED_BODY()

public:
	static constexpr int32 NumSlots = FIrisFastArraySerializer::IrisFastArrayChangeMaskBits;

	/** Server: fills the chunk with free slots. */
	void InitSlots(int32 InChunkIndex);
	void SetIdentity(int32 Slot, uint32 UnitId, uint8 TypeIndex, uint8 OwnerPlayer);
	void SetState(int32 Slot, const FHOLNetUnitState& State);

	int32 GetChunkIndex() const { return ChunkIndex; }
	const TArray<FHOLNetUnitIdentityItem>& GetIdentities() const { return Identities.Items; }
	const TArray<FHOLNetUnitStateItem>& GetStates() const { return States.Items; }

	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	/** The chunk's first slot is ChunkIndex * NumSlots. */
	UPROPERTY(Replicated)
	int32 ChunkIndex = INDEX_NONE;

	UPROPERTY(Replicated)
	FHOLNetUnitIdentityArray Identities;

	UPROPERTY(Replicated)
	FHOLNetUnitStateArray States;
};

/**
 * The one shared actor of the unit replication (D270): always relevant, it carries the chunks of unit slots and the
 * server's step, from which the clients estimate the server clock (D271). The server spawns it in a networked match
 * only (D272: the solo game does not replicate).
 */
UCLASS(NotPlaceable, Transient)
class HEROESOFLIBERTY_API AHOLUnitReplicator : public AActor
{
	GENERATED_BODY()

public:
	AHOLUnitReplicator();

	/** Server: the chunk of that index, created with its predecessors when missing. */
	UHOLUnitChunk* GetOrAddChunk(int32 ChunkIndex);
	void SetServerStep(int64 Step);

	int32 GetServerStep() const { return ServerStep; }
	const TArray<TObjectPtr<UHOLUnitChunk>>& GetChunks() const { return Chunks; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	/** Step the unit states were last taken at (the end of the server's last fixed step). */
	UPROPERTY(Replicated)
	int32 ServerStep = 0;

	UPROPERTY(Replicated)
	TArray<TObjectPtr<UHOLUnitChunk>> Chunks;
};
