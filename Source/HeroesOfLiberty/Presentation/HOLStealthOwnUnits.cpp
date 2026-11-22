// Copyright Epic Games, Inc. All Rights Reserved.

#include "Presentation/HOLStealthOwnUnits.h"

void UHOLStealthOwnUnitsLibrary::GetPlayerUnits(const UObject* WorldObject, const int32 Slot, TArray<int32>& OutUnitIds, TArray<FVector2D>& OutPositions)
{
	// STUB (red commit): lists nothing; the real walk comes in the next commit.
	OutUnitIds.Reset();
	OutPositions.Reset();
}

int32 UHOLStealthOwnUnitsLibrary::GetLocalPlayerSlot(const UObject* WorldObject)
{
	return INDEX_NONE;
}
