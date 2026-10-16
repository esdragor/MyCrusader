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
	/** What it gathers, while Activity is Gathering (J5c.1: on a client, received with the activity). */
	EHOLResource GatherResource = EHOLResource::Food;
	bool bCarrying = false;
	EHOLResource LoadResource = EHOLResource::Food;
	/**
	 * How much it carries: always known in solo and on the server; on a client, only for the player's own unit selected
	 * alone (D303, AHOLPlayerController's watched unit), once the server has answered.
	 */
	int32 LoadAmount = 0;
	bool bLoadAmountKnown = false;
	/** What its type carries at most (0: it does not gather). */
	int32 CarryCapacity = 0;
};

namespace HOLUnitView
{
	/** The units of a game world: simulated (solo, server) or displayed (client). */
	HEROESOFLIBERTY_API void Gather(const UWorld& World, TArray<FHOLUnitView>& OutUnits);
	/** One unit of a game world, without gathering them all (the selection block, every frame); false when it is gone. */
	HEROESOFLIBERTY_API bool Find(const UWorld& World, uint32 UnitId, FHOLUnitView& OutUnit);
}
