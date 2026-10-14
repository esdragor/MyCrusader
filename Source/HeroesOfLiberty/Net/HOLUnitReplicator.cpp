// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLUnitReplicator.h"

#include "Engine/World.h"
#include "Net/HOLUnitReceptionSubsystem.h"
#include "Net/HOLUnitReplicationSettings.h"
#include "Net/UnrealNetwork.h"

AHOLUnitChunk::AHOLUnitChunk()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetNetUpdateFrequency(GetDefault<UHOLUnitReplicationSettings>()->NetUpdateFrequency);
	NetPriority = 3.f;
}

void AHOLUnitChunk::InitSlots(const int32 InChunkIndex)
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

void AHOLUnitChunk::SetIdentity(const int32 Slot, const uint32 UnitId, const uint8 TypeIndex, const uint8 OwnerPlayer)
{
	FHOLNetUnitIdentityItem& Item = Identities.Items[Slot];
	Item.UnitId = UnitId;
	Item.TypeIndex = TypeIndex;
	Item.OwnerPlayer = OwnerPlayer;
	Identities.MarkItemDirty(Item);
}

void AHOLUnitChunk::SetState(const int32 Slot, const FHOLNetUnitState& State)
{
	FHOLNetUnitStateItem& Item = States.Items[Slot];
	Item.State = State;
	States.MarkItemDirty(Item);
}

void AHOLUnitChunk::BeginPlay()
{
	Super::BeginPlay();
	if (GetNetMode() == NM_Client)
	{
		if (UHOLUnitReceptionSubsystem* Reception = GetWorld()->GetSubsystem<UHOLUnitReceptionSubsystem>())
		{
			Reception->AddChunk(this);
		}
	}
}

void AHOLUnitChunk::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetNetMode() == NM_Client)
	{
		if (UHOLUnitReceptionSubsystem* Reception = GetWorld()->GetSubsystem<UHOLUnitReceptionSubsystem>())
		{
			Reception->RemoveChunk(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}

void AHOLUnitChunk::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AHOLUnitChunk, ChunkIndex, COND_InitialOnly);
	DOREPLIFETIME(AHOLUnitChunk, Identities);
	DOREPLIFETIME(AHOLUnitChunk, States);
}

AHOLUnitReplicator::AHOLUnitReplicator()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetNetUpdateFrequency(GetDefault<UHOLUnitReplicationSettings>()->NetUpdateFrequency);
	NetPriority = 3.f;
}

AHOLUnitChunk* AHOLUnitReplicator::GetOrAddChunk(const int32 ChunkIndex)
{
	while (Chunks.Num() <= ChunkIndex)
	{
		FActorSpawnParameters Parameters;
		Parameters.Owner = this;
		Parameters.Name = *FString::Printf(TEXT("HOLUnitChunk_%d"), Chunks.Num());
		AHOLUnitChunk* Chunk = GetWorld()->SpawnActor<AHOLUnitChunk>(Parameters);
		Chunk->SetNetUpdateFrequency(GetNetUpdateFrequency());
		Chunk->InitSlots(Chunks.Num());
		Chunks.Add(Chunk);
	}
	return Chunks[ChunkIndex];
}

void AHOLUnitReplicator::SetServerStep(const int64 Step)
{
	ServerStep = static_cast<int32>(Step);
}

void AHOLUnitReplicator::SetUpdateFrequency(const float UpdateFrequency)
{
	SetNetUpdateFrequency(UpdateFrequency);
	for (AHOLUnitChunk* Chunk : Chunks)
	{
		Chunk->SetNetUpdateFrequency(UpdateFrequency);
	}
}

void AHOLUnitReplicator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHOLUnitReplicator, ServerStep);
}
