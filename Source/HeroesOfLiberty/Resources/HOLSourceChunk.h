// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Iris/ReplicationState/PropertyReplicationState.h"
#include "HOLSourceChunk.generated.h"

/** What never changes of a source (D297): sent once, when the client joins. TypeIndex Empty: a slot with no source. */
USTRUCT()
struct FHOLNetSource
{
	GENERATED_BODY()

	static constexpr uint8 Empty = 0xFF;

	/** Index of the source's type in UHOLSourceSubsystem::GetTypeNames, the same on the server and the clients. */
	UPROPERTY()
	uint8 TypeIndex = Empty;

	/** Ground point (cm). */
	UPROPERTY()
	FVector3f Location = FVector3f::ZeroVector;
};

/**
 * A chunk of 63 resource sources (D297), the recipe of the unit chunks (D270, D300): an actor of fixed slots, plain
 * arrays (one change bit per element of a 63-element array, no identifier per element). The fixed part is sent once;
 * what remains (16 bits) is sent when it changes, at a low rate: a client needs it for the selection block (J5b), and a
 * used-up source pushes an update at once (the tree leaves the map on every screen). Spawned by the server of a
 * networked match (UHOLSourceSubsystem); a client hands it to its registry.
 */
UCLASS(NotPlaceable, Transient)
class HEROESOFLIBERTY_API AHOLSourceChunk : public AActor
{
	GENERATED_BODY()

public:
	static constexpr int32 NumSlots = UE::Net::FPropertyReplicationState::TArrayElementChangeMaskBits;

	AHOLSourceChunk();

	/** Server: fills the chunk with empty slots. */
	void InitSlots(int32 InChunkIndex);
	void SetSource(int32 Slot, uint8 TypeIndex, const FVector& Location, int32 Remaining);
	/** Server: a used-up source (Remaining 0) is sent at the next update. */
	void SetRemaining(int32 Slot, int32 Remaining);

	int32 GetChunkIndex() const { return ChunkIndex; }
	/** False until the client has received every slot of the chunk. */
	bool HasAllSlots() const { return Sources.Num() == NumSlots && Remaining.Num() == NumSlots; }
	const FHOLNetSource& GetSource(int32 Slot) const { return Sources[Slot]; }
	int32 GetRemaining(int32 Slot) const { return Remaining[Slot]; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/** Client: the chunk joins (leaves) the registry. */
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void OnRep_Sources();

	/** The chunk's first source is ChunkIndex * NumSlots. */
	UPROPERTY(Replicated)
	int32 ChunkIndex = INDEX_NONE;

	UPROPERTY(ReplicatedUsing = OnRep_Sources)
	TArray<FHOLNetSource> Sources;

	UPROPERTY(ReplicatedUsing = OnRep_Sources)
	TArray<uint16> Remaining;
};
