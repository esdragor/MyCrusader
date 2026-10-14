// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLUnitReplicator.h"

#include "Net/HOLUnitReplicationSettings.h"
#include "Net/UnrealNetwork.h"

void UHOLUnitChunk::InitSlots(const int32 InChunkIndex)
{
	ChunkIndex = InChunkIndex;
	Identities.Items.SetNum(NumSlots);
	States.Items.SetNum(NumSlots);
	for (int32 Slot = 0; Slot < NumSlots; ++Slot)
	{
		Identities.MarkItemDirty(Identities.Items[Slot]);
		States.MarkItemDirty(States.Items[Slot]);
	}
	Identities.MarkArrayDirty();
	States.MarkArrayDirty();
}

void UHOLUnitChunk::SetIdentity(const int32 Slot, const uint32 UnitId, const uint8 TypeIndex, const uint8 OwnerPlayer)
{
	FHOLNetUnitIdentityItem& Item = Identities.Items[Slot];
	Item.UnitId = UnitId;
	Item.TypeIndex = TypeIndex;
	Item.OwnerPlayer = OwnerPlayer;
	Identities.MarkItemDirty(Item);
}

void UHOLUnitChunk::SetState(const int32 Slot, const FHOLNetUnitState& State)
{
	FHOLNetUnitStateItem& Item = States.Items[Slot];
	Item.State = State;
	States.MarkItemDirty(Item);
}

void UHOLUnitChunk::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UHOLUnitChunk, ChunkIndex, COND_InitialOnly);
	DOREPLIFETIME(UHOLUnitChunk, Identities);
	DOREPLIFETIME(UHOLUnitChunk, States);
}

AHOLUnitReplicator::AHOLUnitReplicator()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	bReplicateUsingRegisteredSubObjectList = true;
	SetNetUpdateFrequency(GetDefault<UHOLUnitReplicationSettings>()->NetUpdateFrequency);
	NetPriority = 3.f;
}

UHOLUnitChunk* AHOLUnitReplicator::GetOrAddChunk(const int32 ChunkIndex)
{
	while (Chunks.Num() <= ChunkIndex)
	{
		UHOLUnitChunk* Chunk = NewObject<UHOLUnitChunk>(this);
		Chunk->InitSlots(Chunks.Num());
		Chunks.Add(Chunk);
		AddReplicatedSubObject(Chunk);
	}
	return Chunks[ChunkIndex];
}

void AHOLUnitReplicator::SetServerStep(const int64 Step)
{
	ServerStep = static_cast<int32>(Step);
}

void AHOLUnitReplicator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHOLUnitReplicator, ServerStep);
	DOREPLIFETIME(AHOLUnitReplicator, Chunks);
}
