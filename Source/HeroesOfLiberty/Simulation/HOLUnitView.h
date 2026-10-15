// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLResources.h"
#include "Simulation/HOLSimulationFragments.h"

class UWorld;

/** A unit as this machine knows it (J4): what the player can see, select and order. */
struct FHOLUnitView
{
	uint32 UnitId = 0;
	uint8 OwnerPlayer = 0;
	FName TypeName;
	/**
	 * Where it is drawn this frame: the simulation in solo, between its last two fixed steps (D219), the display entity
	 * on a client (D271, D272).
	 */
	FVector Location = FVector::ZeroVector;
	/** Body radius (cm), from the type table. */
	float Radius = 60.f;
	/** Drawing scale of its mesh, from the type table: the selection aims at the body as drawn (D288). */
	float MeshScale = 1.f;
	/** What it is doing and carrying (J5, D298): the selection block shows it. A client has them from the last state received. */
	EHOLUnitActivity Activity = EHOLUnitActivity::Idle;
	EHOLResource LoadResource = EHOLResource::Food;
	int32 LoadAmount = 0;
	/** What its type carries at most (0: it does not gather). */
	int32 CarryCapacity = 0;
};

namespace HOLUnitView
{
	/** The units of a game world: simulated (solo, server) or displayed (client). */
	HEROESOFLIBERTY_API void Gather(const UWorld& World, TArray<FHOLUnitView>& OutUnits);
}
