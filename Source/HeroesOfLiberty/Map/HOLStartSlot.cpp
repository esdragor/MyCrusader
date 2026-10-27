// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map/HOLStartSlot.h"

#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionRules.h"
#include "Components/CapsuleComponent.h"
#include "EngineUtils.h"
#include "Match/HOLGameState.h"

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

FVector AHOLStartSlot::GetMainCentreLocation() const
{
	const FVector Ground = GetGroundLocation();
	const FHOLBuildingTypeRow* Type = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::MainCentre);
	const FVector2D Footprint = Type ? Type->Footprint : FVector2D::ZeroVector;
	const FVector2D Middle = HOLConstruction::SnapToGrid(FVector2D(Ground), Footprint, UHOLBuildingSettings::GetTileSize()).GetCenter();
	return FVector(Middle, Ground.Z);
}

float AHOLStartSlot::GetMainCentreYaw() const
{
	return HOLConstruction::SnapYaw(GetActorRotation().Yaw);
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

AHOLStartSlot* AHOLStartSlot::FindForSlot(const UWorld* World, const int32 Slot)
{
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	const int32 Place = GameState ? GameState->GetStartPlace(Slot) : Slot;
	return Place != INDEX_NONE ? Find(World, Place) : nullptr;
}

bool AHOLStartSlot::WorldHasSlots(const UWorld* World)
{
	return World && TActorIterator<AHOLStartSlot>(const_cast<UWorld*>(World));
}
