// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map/HOLStartSlot.h"

#include "Components/CapsuleComponent.h"
#include "EngineUtils.h"

AHOLStartSlot::AHOLStartSlot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FVector AHOLStartSlot::GetGroundLocation() const
{
	FVector Ground = GetActorLocation();
	Ground.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	return Ground;
}

AHOLStartSlot* AHOLStartSlot::Find(const UWorld* World, const int32 Index)
{
	if (World)
	{
		for (TActorIterator<AHOLStartSlot> It(const_cast<UWorld*>(World)); It; ++It)
		{
			if (It->SlotIndex == Index)
			{
				return *It;
			}
		}
	}
	return nullptr;
}

bool AHOLStartSlot::WorldHasSlots(const UWorld* World)
{
	return World && TActorIterator<AHOLStartSlot>(const_cast<UWorld*>(World));
}
