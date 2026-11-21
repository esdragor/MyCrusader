// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLUnitScriptLibrary.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Simulation/HOLUnitView.h"

bool UHOLUnitScriptLibrary::GetUnitLocation(const UObject* WorldContextObject, const int32 UnitId, FVector& OutLocation)
{
	OutLocation = FVector::ZeroVector;
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	FHOLUnitView View;
	// HOLUnitView::Find: the simulation in solo and on the server, the displayed units on a client; a Dying unit is left out (bIncludeDying false).
	if (!World || UnitId <= 0 || !HOLUnitView::Find(*World, static_cast<uint32>(UnitId), View, false))
	{
		return false;
	}
	OutLocation = View.Location;
	return true;
}

bool UHOLUnitScriptLibrary::IsUnitAlive(const UObject* WorldContextObject, const int32 UnitId)
{
	FVector Unused;
	return GetUnitLocation(WorldContextObject, UnitId, Unused);
}
