// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLUnitScriptLibrary.h"

// STUB (red first, L15-5): compiles, answers "alive at the origin" for any id. The real read follows in the next commit.
bool UHOLUnitScriptLibrary::GetUnitLocation(const UObject* WorldContextObject, const int32 UnitId, FVector& OutLocation)
{
	OutLocation = FVector::ZeroVector;
	return true;
}

bool UHOLUnitScriptLibrary::IsUnitAlive(const UObject* WorldContextObject, const int32 UnitId)
{
	return true;
}
