// Copyright Epic Games, Inc. All Rights Reserved.

#include "Presentation/HOLStealthOwnUnits.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Map/HOLVisionSubsystem.h"
#include "Match/HOLPlayerState.h"
#include "Net/HOLUnitReceptionSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitView.h"

void UHOLStealthOwnUnitsLibrary::GetPlayerUnits(const UObject* WorldObject, const int32 Slot, TArray<int32>& OutUnitIds, TArray<FVector2D>& OutPositions)
{
	OutUnitIds.Reset();
	OutPositions.Reset();
	const UWorld* World = WorldObject ? WorldObject->GetWorld() : nullptr;
	if (!World || Slot < 0 || Slot > MAX_uint8)
	{
		return;
	}
	const uint8 Owner = static_cast<uint8>(Slot);

	// A client holds the units it received (the fog reads them the same way, UHOLVisionSubsystem::UpdateClient); the server and solo hold the simulation's.
	const UHOLVisionSubsystem* Vision = World->GetSubsystem<UHOLVisionSubsystem>();
	if (Vision && Vision->IsClient())
	{
		const UHOLUnitReceptionSubsystem* Reception = World->GetSubsystem<UHOLUnitReceptionSubsystem>();
		if (!Reception)
		{
			return;
		}
		TArray<FHOLUnitView> Views;
		Reception->GetDisplayedUnits(Views);
		for (const FHOLUnitView& View : Views)
		{
			if (View.OwnerPlayer == Owner)
			{
				OutUnitIds.Add(static_cast<int32>(View.UnitId));
				OutPositions.Add(FVector2D(View.Location));
			}
		}
		return;
	}

	const UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	if (!Units)
	{
		return;
	}
	Units->ForEachLivingUnitWithType([Owner, &OutUnitIds, &OutPositions](const uint32 UnitId, const FName, const uint8 UnitOwner, const FVector& Location)
	{
		if (UnitOwner == Owner)
		{
			OutUnitIds.Add(static_cast<int32>(UnitId));
			OutPositions.Add(FVector2D(Location));
		}
	});
}

int32 UHOLStealthOwnUnitsLibrary::GetLocalPlayerSlot(const UObject* WorldObject)
{
	const UWorld* World = WorldObject ? WorldObject->GetWorld() : nullptr;
	const APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
	const AHOLPlayerState* PlayerState = Controller ? Controller->GetPlayerState<AHOLPlayerState>() : nullptr;
	return PlayerState ? PlayerState->GetPlayerIndex() : INDEX_NONE;
}
