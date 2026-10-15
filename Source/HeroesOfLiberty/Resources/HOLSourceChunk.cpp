// Copyright Epic Games, Inc. All Rights Reserved.

#include "Resources/HOLSourceChunk.h"

#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Resources/HOLSourceTypes.h"

AHOLSourceChunk::AHOLSourceChunk()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;	// until the fog of war (M7)
	SetReplicatingMovement(false);
	SetNetUpdateFrequency(GetDefault<UHOLSourceSettings>()->NetUpdateFrequency);
	NetPriority = 2.f;
}

void AHOLSourceChunk::InitSlots(const int32 InChunkIndex)
{
	ChunkIndex = InChunkIndex;
	Sources.SetNum(NumSlots);
	Remaining.SetNumZeroed(NumSlots);
}

void AHOLSourceChunk::SetSource(const int32 Slot, const uint8 TypeIndex, const FVector& Location, const int32 InRemaining)
{
	Sources[Slot].TypeIndex = TypeIndex;
	Sources[Slot].Location = FVector3f(Location);
	Remaining[Slot] = static_cast<uint16>(FMath::Clamp(InRemaining, 0, int32(MAX_uint16)));
	ForceNetUpdate();
}

void AHOLSourceChunk::SetRemaining(const int32 Slot, const int32 InRemaining)
{
	const uint16 Value = static_cast<uint16>(FMath::Clamp(InRemaining, 0, int32(MAX_uint16)));
	if (Remaining[Slot] == Value)
	{
		return;
	}
	Remaining[Slot] = Value;
	if (Value == 0)
	{
		ForceNetUpdate();
	}
}

void AHOLSourceChunk::BeginPlay()
{
	Super::BeginPlay();
	if (GetNetMode() == NM_Client)
	{
		if (UHOLSourceSubsystem* Registry = GetWorld()->GetSubsystem<UHOLSourceSubsystem>())
		{
			Registry->AddChunk(this);
		}
	}
}

void AHOLSourceChunk::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetNetMode() == NM_Client)
	{
		if (UHOLSourceSubsystem* Registry = GetWorld()->GetSubsystem<UHOLSourceSubsystem>())
		{
			Registry->RemoveChunk(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}

void AHOLSourceChunk::OnRep_Sources()
{
	// The arrays may arrive before BeginPlay: the registry reads the chunk again when it joins.
	if (HasActorBegunPlay())
	{
		if (UHOLSourceSubsystem* Registry = GetWorld()->GetSubsystem<UHOLSourceSubsystem>())
		{
			Registry->ReadChunk(*this);
		}
	}
}

void AHOLSourceChunk::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AHOLSourceChunk, ChunkIndex, COND_InitialOnly);
	DOREPLIFETIME(AHOLSourceChunk, Sources);
	DOREPLIFETIME(AHOLSourceChunk, Remaining);
}
