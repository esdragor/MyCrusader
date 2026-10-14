// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UWorld;

/** A unit as this machine knows it (J4): what the player can see, select and order. */
struct FHOLUnitView
{
	uint32 UnitId = 0;
	uint8 OwnerPlayer = 0;
	FName TypeName;
	/** Where it is: the simulation in solo, the display entity on a client (D271, D272). */
	FVector Location = FVector::ZeroVector;
	/** Body radius (cm), from the type table. */
	float Radius = 60.f;
	/** Drawing scale of its mesh, from the type table: the selection aims at the body as drawn (D288). */
	float MeshScale = 1.f;
};

namespace HOLUnitView
{
	/** The units of a game world: simulated (solo, server) or displayed (client). */
	HEROESOFLIBERTY_API void Gather(const UWorld& World, TArray<FHOLUnitView>& OutUnits);
}
