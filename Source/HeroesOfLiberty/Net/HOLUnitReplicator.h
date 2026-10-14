// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Iris/ReplicationState/PropertyReplicationState.h"
#include "Net/HOLUnitReplicationRules.h"
#include "HOLUnitReplicator.generated.h"

/** Which unit a slot holds (D270): sent when a unit takes or leaves the slot, or changes owner. UnitId 0: free slot. */
USTRUCT()
struct FHOLNetUnitIdentity
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

/**
 * A chunk of unit slots (D270): one replicated actor, always relevant. Iris tracks the elements of an array with one
 * change bit per 63 elements (bit = index modulo 63): in a single array of 2 000 units, one changed unit would resend
 * the ~32 units sharing its bit; a chunk of exactly 63 slots gives each unit its own bit, so only the changed units are
 * sent. Plain arrays, not fast arrays: Iris writes each fast array item with its ReplicationID (10 bits per state, D300);
 * it finds the changed elements of a plain array itself, by comparing them at each update.
 * Chunks are actors, not subobjects of the shared actor: Iris writes an actor and its subobjects as one batch, which at
 * ~300 states per update outgrew a packet and went through its huge object path every update (split, reliable, written
 * twice, the next update held until all of it was acknowledged); a chunk's batch fits in a packet. (S5: putting 4 runs
 * of 63 slots in one actor saved nothing and rolled back more batches at the ends of packets.) Slots are fixed: a
 * leaving unit frees its slot, nothing is removed from the arrays, the indices never move.
 */
UCLASS(NotPlaceable, Transient)
class HEROESOFLIBERTY_API AHOLUnitChunk : public AActor
{
	GENERATED_BODY()

public:
	static constexpr int32 NumSlots = UE::Net::FPropertyReplicationState::TArrayElementChangeMaskBits;

	AHOLUnitChunk();

	/** Server: fills the chunk with free slots. */
	void InitSlots(int32 InChunkIndex);
	void SetIdentity(int32 Slot, uint32 UnitId, uint8 TypeIndex, uint8 OwnerPlayer);
	void SetState(int32 Slot, const FHOLNetUnitState& State);

	int32 GetChunkIndex() const { return ChunkIndex; }
	/** False until the client has received every slot of the chunk. */
	bool HasAllSlots() const { return Identities.Num() == NumSlots && States.Num() == NumSlots; }
	const FHOLNetUnitIdentity& GetIdentity(int32 Slot) const { return Identities[Slot]; }
	const FHOLNetUnitState& GetState(int32 Slot) const { return States[Slot]; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/** Client: the chunk joins (leaves) the reception's list. */
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

private:
	/** The chunk's first slot is ChunkIndex * NumSlots. */
	UPROPERTY(Replicated)
	int32 ChunkIndex = INDEX_NONE;

	UPROPERTY(Replicated)
	TArray<FHOLNetUnitIdentity> Identities;

	UPROPERTY(Replicated)
	TArray<FHOLNetUnitState> States;
};

/**
 * The shared actor of the unit replication (D270): always relevant, it carries the server's step, from which the
 * clients estimate the server clock (D271), and owns the chunk actors on the server. The server spawns it in a
 * networked match only (D272: the solo game does not replicate).
 */
UCLASS(NotPlaceable, Transient)
class HEROESOFLIBERTY_API AHOLUnitReplicator : public AActor
{
	GENERATED_BODY()

public:
	AHOLUnitReplicator();

	/** Server: the chunk of that index, spawned with its predecessors when missing. */
	AHOLUnitChunk* GetOrAddChunk(int32 ChunkIndex);
	void SetServerStep(int64 Step);
	/** Server: updates per second of this actor and of its chunks. */
	void SetUpdateFrequency(float UpdateFrequency);

	int32 GetServerStep() const { return ServerStep; }
	/** Server only: the chunks it spawned (a client lists the ones it received in UHOLUnitReceptionSubsystem). */
	const TArray<TObjectPtr<AHOLUnitChunk>>& GetChunks() const { return Chunks; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	/** Step the unit states were last taken at (the end of the server's last fixed step). */
	UPROPERTY(Replicated)
	int32 ServerStep = 0;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AHOLUnitChunk>> Chunks;
};
